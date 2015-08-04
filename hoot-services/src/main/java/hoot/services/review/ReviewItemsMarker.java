/*
 * This file is part of Hootenanny.
 *
 * Hootenanny is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * --------------------------------------------------------------------
 *
 * The following copyright notices are generated automatically. If you
 * have a new notice to add, please use the format:
 * " * @copyright Copyright ..."
 * This will properly maintain the copyright information. DigitalGlobe
 * copyrights will be updated automatically.
 *
 * @copyright Copyright (C) 2015 DigitalGlobe (http://www.digitalglobe.com/)
 */
package hoot.services.review;

import java.sql.Connection;
import java.sql.Date;
import java.sql.Timestamp;
import java.util.List;

import hoot.services.HootProperties;
import hoot.services.db.DbUtils;
import hoot.services.db.DbUtils.review_status_enum;
import hoot.services.db2.QChangesets;
import hoot.services.db2.QMaps;
import hoot.services.db2.QReviewItems;
import hoot.services.db2.QUsers;
import hoot.services.models.osm.Changeset;
import hoot.services.models.review.MarkItemsReviewedRequest;
import hoot.services.models.review.MarkItemsReviewedResponse;
import hoot.services.utils.XmlDocumentBuilder;
import hoot.services.validators.osm.ChangesetUploadXmlValidator;
import hoot.services.validators.review.ReviewMapValidator;
import hoot.services.writers.osm.ChangesetDbWriter;
import hoot.services.writers.review.ReviewedItemsWriter;

import org.apache.commons.lang3.StringUtils;
import org.json.simple.JSONObject;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.w3c.dom.Document;

import com.mysema.query.Tuple;
import com.mysema.query.sql.SQLQuery;
import com.mysema.query.sql.SQLSubQuery;
import com.mysema.query.sql.dml.SQLUpdateClause;
import com.mysema.query.types.query.ListSubQuery;

/**
 * Marks reviewable items as reviewed;  This is a wrapper around the process of auto
 * creating/closing a changeset and marking items as reviewed.
 */
public class ReviewItemsMarker
{
  private static final Logger log = LoggerFactory.getLogger(ReviewItemsMarker.class);

  private long mapId;
  private long userId;
  private Connection conn;
  // 5 min
  public static long LOCK_TIME = 300000;

  public ReviewItemsMarker(final Connection conn, final String mapId) throws Exception
  {
    this.conn = conn;
    //Check to see if the map exists in the maps table. (404); input mapId may be a map ID or a
    //map name; this will throw if it doesn't find the map
    log.debug("Checking maps table for map with ID: " + mapId + " ...");
    this.mapId = _verifyMap(mapId);
    //now make sure the user associated with the map exists;
    try
    {
    	//reviewLockTime
    	LOCK_TIME = Long.parseLong(HootProperties.getProperty("reviewLockTime").toString());
      assert(this.mapId != -1);
      log.debug(
        "Retrieving user ID associated with map having ID: " + String.valueOf(this.mapId) + " ...");

      QMaps maps = QMaps.maps;
      //SQLQuery query = new SQLQuery(conn, DbUtils.getConfiguration());

      userId =
      		_getUserIdFromMapId();

      log.debug("Retrieved user ID: " + userId);
    }
    catch (Exception e)
    {
      throw new Exception("Error locating user associated with map with ID: " + this.mapId);
    }
  }
  
  
  protected final long _verifyMap(final String mapId) throws Exception
  {
  	return (new ReviewMapValidator(conn)).verifyMapPrepared(mapId);
  }
  
  protected final long _getUserIdFromMapId() throws Exception
  {
  	QMaps maps = QMaps.maps;
    //SQLQuery query = new SQLQuery(conn, DbUtils.getConfiguration());

    return
    		new SQLQuery(conn, DbUtils.getConfiguration(mapId)).from(maps)
    .where(maps.id.eq(this.mapId))
    .singleResult(maps.userId);
  }
  
  public ReviewItemsMarker()
  {
  	
  }
  
  public void setUserId(final long userid) throws Exception
  {
  	this.userId = userid;
  }
  
  
  public void setMapId(final String mapId) throws Exception
  {
  	this.mapId = _verifyMap(mapId);
  }
  
  public void setMapId(final long mapId) throws Exception
  {
  	this.mapId = mapId;
  }

  /**
   * Uploads an OSM changeset with items marked as reviewed to the services database and then parses
   * the contents of the changeset, marking items as reviewed based on custom hoot review tags it
   * contains
   *
   * @param markItemsReviewedRequest a request to mark items as reviewed which contains an
   * object describing the items to be reviewed, as well as an optional OSM xml changeset; the two
   * sets of data are not cross validated with each other in any way
   * @param markAll an option to mark all data for the map layer as reviewed; when true, the
   * reviewed items object is no needed and ignored if populated
   * @return an mark items as reviewed response
   * @throws Exception
   */
  public MarkItemsReviewedResponse markItemsReviewed(
    final MarkItemsReviewedRequest markItemsReviewedRequest, final boolean markAll)
    throws Exception
  {
    MarkItemsReviewedResponse markItemsReviewedResponse = new MarkItemsReviewedResponse();

    Document changesetUploadResponse = null;
    long changesetId = -1;
    if (StringUtils.trimToNull(markItemsReviewedRequest.getReviewedItemsChangeset()) != null)
    {
      boolean changesetHasElements = false;
      final Document changesetDiffDoc =
        XmlDocumentBuilder.parse(markItemsReviewedRequest.getReviewedItemsChangeset());
      changesetHasElements = ChangesetUploadXmlValidator.changesetHasElements(changesetDiffDoc);
      if (changesetHasElements)
      {
        changesetId =
          Changeset.createChangeset(
            Changeset.getChangesetCreateDoc("marking items reviewed for map ID: " + mapId),
            mapId,
            userId,
            conn);
        //TODO: There really needs to also be a check in here that makes sure every element tag
        //already has a changeset attribute (or iterate through the element tag DOM attributes.
        //For now, just assuming that iD has already added the changeset attributes.  Whether the
        //value is empty or not doesn't matter, since it will be overwritten here.
        markItemsReviewedRequest.setReviewedItemsChangeset(
          markItemsReviewedRequest.getReviewedItemsChangeset()
            .replaceAll("changeset=\"\"", "changeset=\"" + changesetId + "\"")
            .replaceAll("changeset=\"\\d+\"", "changeset=\"" + changesetId + "\""));
        //the changeset upload process will catch any elements in the changeset xml which are out
        //of sync with the element versions in the OSM element tables, by design
        changesetUploadResponse =
          (new ChangesetDbWriter(conn)).write(mapId,
            changesetId, markItemsReviewedRequest.getReviewedItemsChangeset());
        Changeset.closeChangeset(mapId, changesetId, conn);

        markItemsReviewedResponse.setChangesetUploadResponse(
          XmlDocumentBuilder.toString(changesetUploadResponse));
      }
    }

    //mark all items as reviewed in review_items; record the changeset ID and other review
    //details
    if (markAll)
    {
      markItemsReviewedRequest.setReviewedItems(
        ReviewUtils.getReviewedItemsCollectionForAllRecords(mapId, conn));
    }
    final int numItemsMarkedReviewed =
      (new ReviewedItemsWriter(conn, mapId, changesetId)).writeReviewedItems(
        markItemsReviewedRequest.getReviewedItems());

    //this will still be = -1 at this point if nothing was in the changeset input parameter or it
    //was invalid
    markItemsReviewedResponse.setChangesetId(changesetId);
    //this will null at this point if nothing was in the changeset input parameter or it was invalid
    markItemsReviewedResponse.setNumItemsMarkedReviewed(numItemsMarkedReviewed);
    return markItemsReviewedResponse;
  }
  
  protected final SQLUpdateClause _getLastAccessUpdateClause(final String reviewItemId, 
  		final Timestamp newLastAccessTime) throws Exception
  {
  	QReviewItems rm = QReviewItems.reviewItems;
  	return new SQLUpdateClause(conn, DbUtils.getConfiguration(), rm)
    .set(rm.lastAccessed, newLastAccessTime)
    .where(rm.mapId.eq(mapId).and(rm.reviewableItemId.eq(reviewItemId)));
  }
  // Update Review LastAccess column
  public void updateReviewLastAccessTime(final String reviewItemId, final Timestamp newLastAccessTime) throws Exception
  {
  	_getLastAccessUpdateClause(reviewItemId, newLastAccessTime)
    	.execute(); 
  }
  
 /* 
  
  public JSONObject getReviewAvailability(final String reviewItemId) throws Exception
  {
  	// select review_status, last_accesstime from
  	QReviewItems rm = QReviewItems.reviewItems;
  	List<Tuple> res = new SQLQuery(conn, DbUtils.getConfiguration())
  		.from(rm)
  		.where(rm.mapId.eq(mapId).and(rm.reviewableItemId.eq(reviewItemId)))
  		.limit(1)
  		.list(rm.reviewStatus,rm.lastAccessed);
  	
  	JSONObject ret = new JSONObject();
  	
  	if(res.size() > 0)
  	{
  		Tuple tup = res.get(0);
  		DbUtils.review_status_enum stat = (review_status_enum) tup.get(rm.reviewStatus);
  		Timestamp dt = (Timestamp)tup.get(rm.lastAccessed);
  		
  		if(dt != null) {
  			long lastAccess = dt.getTime();
  			java.util.Date d = new java.util.Date();
    		long now = d.getTime();
    		
    		boolean isLocked = ((now-lastAccess) < LOCK_TIME);
    		ret.put("islocked", "" + isLocked);
    		ret.put("evaltime", "" + now);
    		ret.put("lastaccesstime", "" + lastAccess);
  		}
  		else
  		{
  			ret.put("islocked", "" + false);
    		ret.put("evaltime", null);
    		ret.put("lastaccesstime", null);
  		}
  		
  		
  		
  		
  		
  		ret.put("status", "" + stat);
  		
  	}
  	return ret;
  }
 */
  protected final SQLQuery _getAvailableReviewQuery(final Timestamp compareTime) throws Exception
  {
  	QReviewItems rm = QReviewItems.reviewItems;
  	
  	return new SQLQuery(conn, DbUtils.getConfiguration())
  	.from(rm)
  	.where(rm.mapId.eq(mapId).and(rm.reviewStatus.eq(DbUtils.review_status_enum.unreviewed)
  			.and(rm.lastAccessed.lt(compareTime).or(rm.lastAccessed.isNull()))))
  	.orderBy(rm.reviewScore.desc(), rm.reviewId.asc());
  }
  
  // returns total available review count
  public long getAvailableReviewCnt() throws Exception
  {
  	long nRet = 0;
  	

  	QReviewItems rm = QReviewItems.reviewItems;
  	java.util.Date date= new java.util.Date();
  	
  	long waittime = date.getTime() - LOCK_TIME;
  	Timestamp compareTime = new Timestamp(waittime);
  	
  	SQLQuery q = _getAvailableReviewQuery(compareTime);
  	
  	nRet = q.count();
  	
  	return nRet;
  }
  
  protected final SQLQuery _getAvailableReviewWithOffsetQuery(final Timestamp compareTime
  		, final String offsetId) throws Exception
  {
  	QReviewItems rm = QReviewItems.reviewItems;
  	
  	return new SQLQuery(conn, DbUtils.getConfiguration())
  	.from(rm)
  	.where(rm.mapId.eq(mapId).and(rm.reviewStatus.eq(DbUtils.review_status_enum.unreviewed)
  			.and(rm.lastAccessed.lt(compareTime).or(rm.lastAccessed.isNull())).or(rm.reviewableItemId.eq(offsetId))))
  	.orderBy(rm.reviewScore.desc(), rm.reviewId.asc());
  }

  protected final SQLUpdateClause _updateLastAccessWithSubSelect(final Timestamp now, 
  		final Timestamp compareTime, final String avItem) throws Exception
  {
  	QReviewItems rm = QReviewItems.reviewItems;
  	return new SQLUpdateClause(conn, DbUtils.getConfiguration(), rm)
    .set(rm.lastAccessed, now)
    .where(rm.mapId.eq(mapId).and(rm.reviewableItemId.in(new SQLSubQuery().from(rm).
    		where(rm.mapId.eq(mapId).and(rm.reviewableItemId.eq(avItem).and(rm.reviewStatus.ne(DbUtils.review_status_enum.reviewed)
    				.and(rm.lastAccessed.lt(compareTime).or(rm.lastAccessed.isNull()))))).list(rm.reviewableItemId))));
  }
  public JSONObject getAvaiableReviewItem(int offset, final boolean isForward, final String offsetId) throws Exception
  {
  	JSONObject nextItem = new JSONObject();
  	QReviewItems rm = QReviewItems.reviewItems;
  	java.util.Date date= new java.util.Date();
  	Timestamp now = new Timestamp(date.getTime());
  	
  	long waittime = date.getTime() - LOCK_TIME;
  	Timestamp compareTime = new Timestamp(waittime);
  	
  	SQLQuery q = _getAvailableReviewQuery(compareTime).limit(offset+10);
  	
  	if(offsetId != null)
  	{
  		q = _getAvailableReviewWithOffsetQuery(compareTime, offsetId)
    	.limit(offset+10);
  	}
 
  	//investigate queryDSL for cursor fetching..
  	// But for now we use internal loop
  	List<String> avList = q.list(rm.reviewableItemId);
  	
		if(offset > -1)
		{

			for(int i=0; i<avList.size(); i++)
			{
				if(avList.get(i).toString().equals(offsetId))
				{
					offset = i;
					break;
				}
			}
		}
  	
  	
  	String avItem = null;
  	if(avList.size() > 0)
  	{
	  	if(isForward)
	  	{
	  		if(avList.size() > offset+1)
	  		{
	  			// next available item
	  			avItem = avList.get(offset+1);
	  		} 	
	  		else
	  		{
	  			// goto first
	  			avItem = avList.get(0);
	  		}

	  	}
	  	else // backward
	  	{
	  		if(avList.size() >= offset+1)
	  		{
	  			if(offset-1 < 0) {
	  				avItem = avList.get(avList.size()-1);
	  			} else {
	  			// get previous item
		  			avItem = avList.get(offset-1);
	  			}
	  			
	  		}
	  		
	  	}
  	}
  	
  	
  	nextItem.put("status", "failed");
  	if(avItem != null)
  	{  	
  		// lock the item if still available
  		long rowsEffected =  _updateLastAccessWithSubSelect(now, compareTime, avItem)
      .execute(); 
  		
  		if(rowsEffected > 0)
  		{
  			nextItem.put("nextitemid", avItem);
  			nextItem.put("status", "success");
  		}
  	}
  	else
  	{
  		nextItem.put("status", "noneavailable");
  	}
  	
  	return nextItem;
  	
  }
  

}
