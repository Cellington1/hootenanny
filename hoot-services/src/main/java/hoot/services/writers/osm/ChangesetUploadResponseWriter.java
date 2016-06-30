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

import java.util.List;

import javax.ws.rs.WebApplicationException;
import javax.ws.rs.core.Response;
import javax.ws.rs.core.Response.Status;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.w3c.dom.Document;
import org.w3c.dom.Element;

import hoot.services.HootProperties;
import hoot.services.models.osm.XmlSerializable;
import hoot.services.utils.XmlDocumentBuilder;


/**
 * Writes an HTTP response for a changeset upload request
 */
public class ChangesetUploadResponseWriter {
    private static final Logger logger = LoggerFactory.getLogger(ChangesetUploadResponseWriter.class);

    /**
     * Writes a changeset upload response to an XML document
     *
     * @param changesetId
     *            ID of the uploaded changeset
     * @param changesetDiffElements
     *            Elements that have been modified in the corresponding
     *            changeset request
     * @return a changeset upload response XML document
     */
    public Document writeResponse(long changesetId, List<XmlSerializable> changesetDiffElements) {
        Document responseDoc = null;
        try {
            logger.debug("Building response...");

            responseDoc = XmlDocumentBuilder.create();

            Element osmElement = OsmResponseHeaderGenerator.getOsmDataHeader(responseDoc);

            Element diffResultXmlElement = responseDoc.createElement("diffResult");

            diffResultXmlElement.setAttribute("generator", HootProperties.getPropertyOrDefault("generator"));
            diffResultXmlElement.setAttribute("version", HootProperties.getPropertyOrDefault("osmVersion"));

            for (XmlSerializable element : changesetDiffElements) {
                diffResultXmlElement.appendChild(element.toChangesetResponseXml(diffResultXmlElement));
            }

            osmElement.appendChild(diffResultXmlElement);
            responseDoc.appendChild(osmElement);
        }
        catch (Exception ex) {
            String msg = "Error creating response for changeset with ID: " + changesetId
                    + " - data: (" + ex.getMessage() + ") ";
            logger.error(msg, ex);
            throw new WebApplicationException(ex, Response.status(Status.BAD_REQUEST).entity(msg).build());
        }

        return responseDoc;
    }
}
