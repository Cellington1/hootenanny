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
 * @copyright Copyright (C) 2014 DigitalGlobe (http://www.digitalglobe.com/)
 */
package hoot.services.controllers.wps;

import java.io.IOException;

import javax.ws.rs.core.Response.Status;
import javax.xml.xpath.XPath;
import javax.xml.xpath.XPathExpressionException;

import org.apache.commons.lang3.StringEscapeUtils;
import org.apache.xpath.XPathAPI;
import org.junit.Assert;
import org.junit.Test;
import org.junit.experimental.categories.Category;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.w3c.dom.Document;
import org.w3c.dom.NodeList;

import hoot.services.IntegrationTest;
import hoot.services.review.ReviewTestUtils;
import hoot.services.utils.JsonUtils;
import hoot.services.utils.XmlDocumentBuilder;
import hoot.services.wps.WpsTestAbstract;

public class MarkItemsReviewedWpsTest extends WpsTestAbstract
{
  @SuppressWarnings("unused")
  private static final Logger log = LoggerFactory.getLogger(MarkItemsReviewedWpsTest.class);
  
  public MarkItemsReviewedWpsTest() throws NumberFormatException, IOException
  {
    super("hoot.services.controllers.job");
    
    processId = "MarkItemsReviewed";
  }
  
  //WPS specifies that both GET and POST be supported, even when one or the other doesn't make
  //sense in context of the request.  It also supports both key/value pair and XML as inputs for
  //both request types.  These tests don't test every possible input parameter combination like 
  //the processlet level tests do.
  
  //TODO: One of out input params takes in XML, so for now, we're not supporting GET.  If we're
  //able to pass the xml input as a complext type, then GET could maybe be supported.
  
  @Override
  protected String getWpsRequestXml(final String mapId) throws Exception
  {
    return 
      "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>" +
      "<wps:Execute service=\"WPS\" version=\"1.0.0\" " +
        "xmlns:wps=\"http://www.opengis.net/wps/1.0.0\" " + 
        "xmlns:ows=\"http://www.opengis.net/ows/1.1\" " + 
        "xmlns:xlink=\"http://www.w3.org/1999/xlink\" " + 
        "xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" " + 
        "xsi:schemaLocation=\"http://www.opengis../wpsExecute_request.xsd\">" + 
        "<ows:Identifier>" + processId + "</ows:Identifier>" + 
        "<wps:DataInputs>" + 
          "<wps:Input>" + 
            "<ows:Identifier>mapId</ows:Identifier>" + 
            "<wps:Data>" + 
              "<wps:LiteralData>" + String.valueOf(mapId) + "</wps:LiteralData>" + 
            "</wps:Data>" + 
          "</wps:Input>" + 
          "<wps:Input>" + 
            "<ows:Identifier>markAll</ows:Identifier>" + 
            "<wps:Data>" + 
              "<wps:LiteralData>" + String.valueOf(false) + "</wps:LiteralData>" + 
            "</wps:Data>" + 
          "</wps:Input>" + 
          "<wps:Input>" + 
            "<ows:Identifier>reviewedItems</ows:Identifier>" + 
            "<wps:Data>" + 
              "<wps:LiteralData>" + 
                /*StringEscapeUtils.escapeXml(*/
                  JsonUtils.objectToJson(ReviewTestUtils.createReviewedItems())/*)*/ + 
              "</wps:LiteralData>" + 
            "</wps:Data>" + 
          "</wps:Input>" + 
          "<wps:Input>" + 
            "<ows:Identifier>reviewedItemsChangeset</ows:Identifier>" + 
            "<wps:Data>" + 
              "<wps:LiteralData>" + 
                StringEscapeUtils.escapeXml(
                  XmlDocumentBuilder.toString(
                    ReviewTestUtils.createReviewedItemsChangeset())) + 
              "</wps:LiteralData>" + 
            "</wps:Data>" + 
          "</wps:Input>" + 
        "</wps:DataInputs>" + 
        "<wps:ResponseForm>" + 
          "<wps:ResponseDocument storeExecuteResponse=\"false\" status=\"false\">" + 
            "<wps:Output>" + 
              "<ows:Identifier>changesetUploadResponse</ows:Identifier>" + 
            "</wps:Output>" + 
            "<wps:Output>" + 
              "<ows:Identifier>numItemsMarkedReviewed</ows:Identifier>" + 
            "</wps:Output>" + 
            "<wps:Output>" + 
              "<ows:Identifier>changesetId</ows:Identifier>" + 
            "</wps:Output>" + 
          "</wps:ResponseDocument>" + 
        "</wps:ResponseForm>" + 
      "</wps:Execute>";
  }
  
  protected String getWpsRequestString(final String mapId) throws Exception { return null; }
  
  @Override
  protected String verifyWpsResponse(final String responseStr) throws Exception
  {    
    final Document responseData = XmlDocumentBuilder.parse(responseStr, false);
    Assert.assertNotNull(responseData);
    System.out.println(XmlDocumentBuilder.toString(responseData));
    
    XPath xpath = XmlDocumentBuilder.createXPath(); 
    long changesetId = -1;
    try
    {
      Assert.assertEquals(
        processId, xpath.evaluate(".//Process/Identifier", responseData));
      NodeList returnedNodes = XPathAPI.selectNodeList(responseData, ".//ProcessOutputs/Output");
      Assert.assertEquals(3, returnedNodes.getLength());
      
      Assert.assertEquals(
        "changesetUploadResponse", 
        xpath.evaluate(".//ProcessOutputs/Output[1]/Identifier", responseData));
      Assert.assertEquals(
        "string", 
        xpath.evaluate(".//ProcessOutputs/Output[1]/Data/LiteralData/@dataType", responseData));
      final Document changesetResponse = 
        XmlDocumentBuilder.parse(
          StringEscapeUtils.unescapeXml(
            xpath.evaluate(".//ProcessOutputs/Output[1]/Data/LiteralData", responseData)));
      Assert.assertNotNull(changesetResponse);
      //Changeset uploading is heavily tested in the OSM controller tests, so not testing that here;
      
      Assert.assertEquals(
        "numItemsMarkedReviewed", 
        xpath.evaluate(".//ProcessOutputs/Output[2]/Identifier", responseData));
      Assert.assertEquals(
        "integer", 
        xpath.evaluate(".//ProcessOutputs/Output[2]/Data/LiteralData/@dataType", responseData));
      Assert.assertEquals(
        5,  
        Integer.parseInt(
          xpath.evaluate(".//ProcessOutputs/Output[2]/Data/LiteralData", responseData)));
      
      Assert.assertEquals(
        "changesetId", 
        xpath.evaluate(".//ProcessOutputs/Output[3]/Identifier", responseData));
      Assert.assertEquals(
        "integer", 
        xpath.evaluate(".//ProcessOutputs/Output[3]/Data/LiteralData/@dataType", responseData));
      changesetId = 
        Integer.parseInt(
          xpath.evaluate(".//ProcessOutputs/Output[3]/Data/LiteralData", responseData));
      Assert.assertTrue(changesetId > -1);
    }
    catch (XPathExpressionException e)
    {
      Assert.fail("Error parsing response document: " + e.getMessage());
    }
    
    //verify the changeset data was written
    ReviewTestUtils.changesetId = changesetId;
    ReviewTestUtils.verifyDataMarkedAsReviewed(true);
    
    return null;
  }
  
  //The OGCFrontController expects a get request to either be entirely made of XML or contain no
  //XML.  There doesn't seem to be any way to mix the two params, even if they are escaped.  
  //Therefore, not supporting a GET here.
  
  @Test
  @Category(IntegrationTest.class)
  public void testWpsPostXml() throws Exception
  {
    ReviewTestUtils.createPreparedData(resource());
    
    final String response = testWps(false, false, String.valueOf(mapId));
    verifyWpsResponse(response);
  }
  
  @Test
  @Category(IntegrationTest.class)
  public void testWpsError() throws Exception
  {
    ReviewTestUtils.createPreparedData(resource());
    
    final String response = testWps(false, false, String.valueOf(mapId + 2));
    verifyWpsErrorResponse(response, Status.NOT_FOUND, "No record exists");
  }
}