package hoot.services.writers.review;

import hoot.services.HootProperties;
import hoot.services.db.DbUtils;
import hoot.services.db.DbUtils.RecordBatchType;
import hoot.services.db2.ElementIdMappings;
import hoot.services.db2.QElementIdMappings;
import hoot.services.db2.QReviewItems;
import hoot.services.db2.ReviewItems;
import hoot.services.models.osm.Element;
import hoot.services.models.osm.Element.ElementType;
import hoot.services.review.ReviewUtils;
import hoot.services.utils.XmlUtils;
import hoot.services.validators.review.ReviewMapValidator;

import java.sql.Connection;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import javax.xml.transform.TransformerException;

import org.apache.commons.lang3.StringUtils;
import org.apache.xpath.XPathAPI;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.w3c.dom.DOMException;
import org.w3c.dom.Document;
import org.w3c.dom.NodeList;

import com.mysema.query.sql.SQLQuery;
import com.mysema.query.sql.dml.SQLDeleteClause;
import com.mysema.query.types.expr.BooleanExpression;

/**
 * Responsible for maintaining review SQL data consistency based on saved OSM changesets
 */
public class ReviewItemsUpdater
{
  private static final Logger log = LoggerFactory.getLogger(ReviewItemsUpdater.class);
  
  private static final QReviewItems reviewItems = QReviewItems.reviewItems;
  private static final QElementIdMappings elementIdMappings = QElementIdMappings.elementIdMappings;
	
	private List<ElementIdMappings> elementIdMappingRecordsToInsertFromChangeset;
  private List<ReviewItems> reviewItemRecordsToInsertFromChangeset;
  private List<ReviewItems> reviewItemRecordsToUpdateFromChangeset;
  
  private long mapId;
  private Connection conn;
  protected int maxRecordBatchSize;
	
  public ReviewItemsUpdater(final Connection conn, final String mapId) throws Exception
  {
    this.conn = conn;
    //Check to see if the map exists in the maps table. (404); input mapId may be a map ID or a
    //map name; this will throw if it doesn't find the map
    log.debug("Checking maps table for map with ID: " + mapId + " ...");
    (new ReviewMapValidator(conn)).verifyMapPrepared(mapId);
    this.mapId = Long.parseLong(mapId);
    maxRecordBatchSize = 
  		Integer.parseInt(HootProperties.getInstance()
  		  .getProperty("maxRecordBatchSize", HootProperties.getDefault("maxRecordBatchSize")));
  }
  
  private String[] reviewAgainstUuidsFromChangesetElement(final org.w3c.dom.Node elementXml) 
  	throws DOMException, TransformerException
  {
  	String[] reviewAgainstUuids = null;
  	final String reviewAgainstUuidStr = 
    	XPathAPI.selectSingleNode(elementXml, "tag[@k = 'hoot:review:uuid']/@v").getNodeValue();
  	if (StringUtils.trimToNull(reviewAgainstUuidStr) != null)
  	{
  		if (reviewAgainstUuidStr.contains(";"))
  		{
  			reviewAgainstUuids = reviewAgainstUuidStr.split(";");
  		}
  		else
  		{
  			reviewAgainstUuids = new String[1];
  			reviewAgainstUuids[0] = reviewAgainstUuidStr;
  		}
  	}
  	return reviewAgainstUuids;
  }
  
  private void createReviewRecordsFromCreateChangeset(final Document changesetDoc,
    final Map<ElementType, HashMap<Long, Element>> parsedElementIdsToElementsByType) 
    throws Exception
  {
  	log.debug("updateCreatedReviewItems");
  	
    //Create element id mappings for every record, regardless if its involved in a review.  This
  	//will result in unused records, but makes the code simpler overall.  We're making a big 
  	//assumption here that any review against items already have an element id mapping record entry...
  	//which should be the case.
  	final NodeList createdElements = XPathAPI.selectNodeList(changesetDoc, "//osmChange/create/*");
  	log.debug(String.valueOf(createdElements.getLength()));
  	log.debug(XmlUtils.nodeListToString(createdElements));
  	for (int i = 0; i < createdElements.getLength(); i++)
  	{
  		final org.w3c.dom.Node elementXml = createdElements.item(i);
  		final String uuid = 
  			XPathAPI.selectSingleNode(elementXml, "tag[@k = 'uuid']/@v").getNodeValue();
  		final long changesetOsmElementId = 
  			Long.parseLong(elementXml.getAttributes().getNamedItem("id").getNodeValue());
  		final ElementType elementType = Element.elementTypeFromString(elementXml.getNodeName());
  		//final Element element = 
  		  //parsedElementIdsToElementsByType.get(elementType).get(changesetOsmElementId);
  		final long actualOsmElementId = 
  			parsedElementIdsToElementsByType.get(elementType).get(changesetOsmElementId).getId();
  		elementIdMappingRecordsToInsertFromChangeset.add(
  			ReviewUtils.createElementIdMappingRecord(
  				uuid, 
  				actualOsmElementId, 
  				elementType, 
  				mapId));
  	}
  	
  	//check create changeset for any newly created reviewable items
  	final NodeList createdReviewItems = 
  	  XPathAPI.selectNodeList(
  	    changesetDoc, 
        "//osmChange/create/*/tag[@k = 'hoot:review:needs' and @v = 'yes']/..");
  	log.debug(String.valueOf(createdReviewItems.getLength()));
  	log.debug(XmlUtils.nodeListToString(createdReviewItems));
  	for (int i = 0; i < createdReviewItems.getLength(); i++)
  	{
  		//add the associated review data; not checking to see if the element already exists in the
  		//review data, b/c it shouldn't
  		final org.w3c.dom.Node elementXml = createdReviewItems.item(i);
  		final String uuid = 
  			XPathAPI.selectSingleNode(elementXml, "tag[@k = 'uuid']/@v").getNodeValue();
  		final String[] reviewAgainstUuids = reviewAgainstUuidsFromChangesetElement(elementXml);
  		if (reviewAgainstUuids != null)
  		{
    		for (int j = 0; j < reviewAgainstUuids.length; j++)
    		{
    			final String reviewAgainstUuid = reviewAgainstUuids[j];
    			reviewItemRecordsToInsertFromChangeset.add(
    	  		ReviewUtils.createReviewItemRecord(
    	  			uuid, 
    	  		  //TODO: the way to retrieve the correct score would be to trace back the elements 
    	  			//that made up this (presumably) merged element...although you could argue that the 
    	  			//old score is obsolete after a merge; this is possibly complicated, so holding off on 
    	  			//doing fixing this for now
    	  			1.0,
    	  			reviewAgainstUuid, 
    	  			mapId));
    		}
  		}
  		else
  		{
  			//adding the item here as reviewed if it has no review against id's left for bookkeeping 
  			//purposes
  			ReviewItems reviewItemRecord = 
  				ReviewUtils.createReviewItemRecord(
	  			  uuid, 
	  			  1.0, //TODO: see explanation above
	  			  null, 
	  			  mapId);
  			reviewItemRecord.setReviewStatus(DbUtils.review_status_enum.reviewed);
  			reviewItemRecordsToInsertFromChangeset.add(reviewItemRecord);
  		}
  	}
  }
  
  private void createReviewRecordsFromModifyChangeset(final Document changesetDoc) throws Exception
  {
    log.debug("createReviewRecordsFromModifyChangeset");
    
    //We're making a big assumption here that any records being simply modified to be reviewable 
    //already have an element id mapping record entry.
    
  	//map modified elements by unique id
    final NodeList modifiedItems = 
  	  XPathAPI.selectNodeList(changesetDoc, "//osmChange/modify/*");
    Map<String, org.w3c.dom.Node> uuidsToXml = new HashMap<String, org.w3c.dom.Node>();
  	for (int i = 0; i < modifiedItems.getLength(); i++)
  	{
  		final org.w3c.dom.Node elementXml = modifiedItems.item(i);
  		final String uuid = 
    		XPathAPI.selectSingleNode(elementXml, "tag[@k = 'uuid']/@v").getNodeValue();
  		uuidsToXml.put(uuid, elementXml);
  	}
  	
  	//determine all modified items that actually have a review record
  	final List<String> modifiedUniqueIds =
      new SQLQuery(conn, DbUtils.getConfiguration(mapId))
    		.from(elementIdMappings)
        .where(
      	  elementIdMappings.elementId.in(uuidsToXml.keySet())
      		  .and(elementIdMappings.mapId.eq(mapId)))
      	.list(elementIdMappings.elementId);
  	
  	//for each id that already has a review record, create an update record with the updated 
  	//hoot:review:uuid tag contents; only the hoot:review:uuid tag should have changed client side, 
  	//so we only need to update that one
  	for (String uuid : modifiedUniqueIds)
  	{
  		final org.w3c.dom.Node elementXml = uuidsToXml.get(uuid);
  		assert(
  		  XPathAPI.selectSingleNode(
  		    elementXml, "tag[@k = 'hoot:review:needs' and @v = 'yes']")/*.getNodeValue()*/ != null);
    	final String[] reviewAgainstUuids = reviewAgainstUuidsFromChangesetElement(elementXml);
  		if (reviewAgainstUuids != null)
  		{
  			for (int j = 0; j < reviewAgainstUuids.length; j++)
  			{
  				reviewItemRecordsToUpdateFromChangeset.add(
  		  		ReviewUtils.createReviewItemRecord(
  		  			uuid, 
  		  			1.0, //TODO: see comment in updateCreatedReviewItems
  		  			reviewAgainstUuids[j], 
  		  			mapId));
  			}
  		}
  		else
  		{
  			//record has nothing left to review against it, so set it to reviewed; the client is
  			//expected to have dropped all the review tags from the feature
  			ReviewItems reviewItemRecord = 
  				ReviewUtils.createReviewItemRecord(
    			  uuid, 
    			  1.0, //TODO: see comment in updateCreatedReviewItems
    			  null, 
    			  mapId);
  			reviewItemRecord.setReviewStatus(DbUtils.review_status_enum.reviewed);
  			reviewItemRecordsToUpdateFromChangeset.add(reviewItemRecord);
  		}
  		uuidsToXml.remove(uuid);
  	}
    
    //anything left in uuidsToXml must be associated with a new review, so create the new 
    //associated review records
    for (Map.Entry<String, org.w3c.dom.Node> nodeEntry : uuidsToXml.entrySet())
    {
    	final org.w3c.dom.Node elementXml = nodeEntry.getValue();
  		assert(
  		  XPathAPI.selectSingleNode(
  		    elementXml, "tag[@k = 'hoot:review:needs' and @v = 'yes']")/*.getNodeValue()*/ != null);
  		final String uuid = nodeEntry.getKey();
    	final String[] reviewAgainstUuids = reviewAgainstUuidsFromChangesetElement(elementXml);
  		if (reviewAgainstUuids != null)
  		{
  			for (int j = 0; j < reviewAgainstUuids.length; j++)
  			{
  				reviewItemRecordsToInsertFromChangeset.add(
  		  		ReviewUtils.createReviewItemRecord(
  		  				uuid, 
  		  			1.0, //TODO: see comment in updateCreatedReviewItems
  		  			reviewAgainstUuids[j], 
  		  			mapId));
  			}
  		}
  		else
  		{
  			//record has nothing left to review against it, so set it to reviewed; the client is
  			//expected to have dropped all the review tags from the feature
  			ReviewItems reviewItemRecord = 
  				ReviewUtils.createReviewItemRecord(
    			  uuid, 
    			  1.0, //TODO: see comment in updateCreatedReviewItems
    			  null, 
    			  mapId);
  			reviewItemRecord.setReviewStatus(DbUtils.review_status_enum.reviewed);
  			reviewItemRecordsToInsertFromChangeset.add(reviewItemRecord);
  		}
    }
    
    //Technically, we also go through and clean element ID mappings records that are no longer
    //in use b/c they aren't involved in reviews, but that seems difficult and they aren't hurting
    //anything by being in the database and not being used...
  }
  
  private List<String> getDeleteUniqueIdsFromChangeset(final Document changesetDoc) 
  	throws TransformerException
  {
  	log.debug("getDeleteUniqueIdsFromChangeset");
  	
  	final NodeList deletedItems = 
  	  XPathAPI.selectNodeList(changesetDoc, "//osmChange/delete/*/tag[@k = 'uuid']/@v");
  	List<String> deletedItemUuids = new ArrayList<String>();
  	for (int i = 0; i < deletedItems.getLength(); i++)
  	{
  		deletedItemUuids.add(deletedItems.item(i).getNodeValue());  //TODO: is this right?
  	}
  	log.debug("deletedItemUuids: " + deletedItemUuids.toString());
  	return deletedItemUuids;
  }
   
  /**
   * Synchronizes review table data based on OSM changeset input
   * 
   * @param changesetDoc OSM changeset
   * @param parsedElementIdsToElementsByType mapping of element ID's passed in the changeset to
   * actual element ID's stored in the database
   * @return the number of review records updated
   * @throws Exception 
   */
  public void updateReviewItems(final Document changesetDoc, 
    final Map<ElementType, HashMap<Long, Element>> parsedElementIdsToElementsByType) 
    throws Exception
  {
  	int numElementIdsUpdated = 0;
  	int numReviewItemsUpdated = 0;
    log.debug("Updating review items for changeset...");
    
    elementIdMappingRecordsToInsertFromChangeset = new ArrayList<ElementIdMappings>();
  	reviewItemRecordsToInsertFromChangeset = new ArrayList<ReviewItems>();
  	reviewItemRecordsToUpdateFromChangeset = new ArrayList<ReviewItems>();
    
    //Items in the create changeset are passed in with temporary id's that the server replaces.  So,
    //we need to pass the id mapping data structure to this method so it has the correct ID's 
    //without having to do extra database queries for them.
  	createReviewRecordsFromCreateChangeset(changesetDoc, parsedElementIdsToElementsByType);
  	createReviewRecordsFromModifyChangeset(changesetDoc);
  	
  	DbUtils.batchRecords(
    	mapId, elementIdMappingRecordsToInsertFromChangeset, elementIdMappings, null, 
    	RecordBatchType.INSERT, conn, maxRecordBatchSize);
  	numElementIdsUpdated += elementIdMappingRecordsToInsertFromChangeset.size();
  	DbUtils.batchRecords(
    	mapId, reviewItemRecordsToInsertFromChangeset, reviewItems, null, RecordBatchType.INSERT, conn, 
    	maxRecordBatchSize);
  	numReviewItemsUpdated += reviewItemRecordsToInsertFromChangeset.size();
  	
  	List<List<BooleanExpression>> predicatelist = new ArrayList<List<BooleanExpression>>();
  	for (int i = 0; i < reviewItemRecordsToUpdateFromChangeset.size(); i++)
  	{
  		List<BooleanExpression> predicates = new ArrayList<BooleanExpression>();
  		predicates.add(reviewItems.mapId.eq(mapId));
  		predicates.add(
  			reviewItems.reviewableItemId.eq(
  				reviewItemRecordsToUpdateFromChangeset.get(i).getReviewableItemId()));
  		predicates.add(
    		reviewItems.reviewAgainstItemId.eq(
    			reviewItemRecordsToUpdateFromChangeset.get(i).getReviewAgainstItemId()));
  		predicatelist.add(predicates);
  	}
  	DbUtils.batchRecords(
    	mapId, reviewItemRecordsToUpdateFromChangeset, reviewItems, predicatelist, 
    	RecordBatchType.UPDATE, conn, maxRecordBatchSize);
    numReviewItemsUpdated += reviewItemRecordsToUpdateFromChangeset.size();
    
    List<String> deletedItemUuidsFromDeleteChangeset = getDeleteUniqueIdsFromChangeset(changesetDoc);
  	final String[] existingReviewItemUuids = 
  		new SQLQuery(conn, DbUtils.getConfiguration(mapId))
	      .from(reviewItems)
		    .where(reviewItems.reviewableItemId.in(
		    	deletedItemUuidsFromDeleteChangeset.toArray(new String[]{})))
		    .list(reviewItems.reviewableItemId)
		    .toArray(new String[]{});
  	numReviewItemsUpdated +=
  		new SQLDeleteClause(conn, DbUtils.getConfiguration(mapId), reviewItems)
  	    .where(
  	    	reviewItems.reviewableItemId.in(existingReviewItemUuids)
  	    	  .or(reviewItems.reviewAgainstItemId.in(existingReviewItemUuids)))
			  .execute();
  	numElementIdsUpdated +=
  		new SQLDeleteClause(
			  conn, DbUtils.getConfiguration(mapId), elementIdMappings)
        .where(elementIdMappings.elementId.in(existingReviewItemUuids))
	      .execute();
  	
  	log.debug(
      String.valueOf(numReviewItemsUpdated) + " review records and " + 
  	  String.valueOf(numElementIdsUpdated) + " element ID records were updated as a result of " +
      "the changeset save.");
  }
}
