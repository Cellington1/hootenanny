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
 * @copyright Copyright (C) 2015, 2016 DigitalGlobe (http://www.digitalglobe.com/)
 */
package hoot.services.writers.osm;

import java.sql.Connection;
import java.util.Map;

import javax.ws.rs.WebApplicationException;
import javax.ws.rs.core.Response;
import javax.ws.rs.core.Response.Status;
import javax.xml.parsers.ParserConfigurationException;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.w3c.dom.Document;

import com.mysema.query.Tuple;

import hoot.services.db2.QUsers;
import hoot.services.db2.Users;
import hoot.services.geo.BoundingBox;
import hoot.services.models.osm.Element;
import hoot.services.models.osm.Element.ElementType;
import hoot.services.models.osm.ElementFactory;
import hoot.services.utils.XmlDocumentBuilder;


/**
 * Writes the response for a map query
 */
public class MapQueryResponseWriter {
    private static final Logger logger = LoggerFactory.getLogger(MapQueryResponseWriter.class);

    private final long mapId;
    private final Connection conn;

    public MapQueryResponseWriter(long mapId, Connection conn) {
        this.mapId = mapId;
        this.conn = conn;
    }

    /**
     * Writes a map query response with no element data
     * 
     * @return a XML document response
     * @throws ParserConfigurationException
     */
    public static Document writeEmptyResponse() throws ParserConfigurationException {
        Document responseDoc = XmlDocumentBuilder.create();
        org.w3c.dom.Element elementRootXml = OsmResponseHeaderGenerator.getOsmDataHeader(responseDoc);
        responseDoc.appendChild(elementRootXml);
        return responseDoc;
    }

    /**
     * Writes the query response to an XML document
     * 
     * @param results
     *            query results; a mapping of element IDs to records, grouped by
     *            element type
     * @param queryBounds
     *            bounds of the query
     * @param multiLayerUniqueElementIds
     *            if true, IDs are prepended with <map id>_<first letter of the
     *            element type>_; this setting activated is not compatible with
     *            standard OSM clients (specific to Hootenanny iD)
     * @return an XML document
     */
    public Document writeResponse(Map<ElementType, Map<Long, Tuple>> results, BoundingBox queryBounds,
            boolean multiLayerUniqueElementIds) {
        Document responseDoc = null;
        try {
            logger.debug("Building response...");

            responseDoc = XmlDocumentBuilder.create();
            org.w3c.dom.Element elementRootXml = OsmResponseHeaderGenerator.getOsmDataHeader(responseDoc);
            responseDoc.appendChild(elementRootXml);

            if (!results.isEmpty()) {
                elementRootXml.appendChild(queryBounds.toXml(elementRootXml));

                for (ElementType elementType : ElementType.values()) {
                    if (elementType != ElementType.Changeset) {
                        Map<Long, Tuple> resultsForType = results.get(elementType);
                        if (resultsForType != null) {
                            for (Map.Entry<Long, Tuple> entry : resultsForType.entrySet()) {
                                Tuple record = entry.getValue();

                                Element element = ElementFactory.create(elementType, record, conn, mapId);

                                // the query that sent this in should have
                                // already handled filtering out invisible elements

                                Users usersTable = record.get(QUsers.users);
                                assert (element.getVisible());
                                org.w3c.dom.Element elementXml = element.toXml(elementRootXml, usersTable.getId(),
                                        usersTable.getDisplayName(), multiLayerUniqueElementIds, true);
                                elementRootXml.appendChild(elementXml);
                            }
                        }
                    }
                }
            }
        }
        catch (Exception ex) {
            String msg = "Error creating response for map query for map with ID: " + mapId + ". ("
                    + ex.getMessage() + ") ";
            throw new WebApplicationException(ex, Response.status(Status.INTERNAL_SERVER_ERROR).entity(msg).build());
        }

        return responseDoc;
    }
}
