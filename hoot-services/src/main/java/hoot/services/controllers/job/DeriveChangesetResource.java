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
package hoot.services.controllers.job;

import static hoot.services.HootProperties.*;

import java.io.File;
import java.io.IOException;
import java.util.UUID;

import javax.ws.rs.GET;
import javax.ws.rs.POST;
import javax.ws.rs.Path;
import javax.ws.rs.PathParam;
import javax.ws.rs.Produces;
import javax.ws.rs.QueryParam;
import javax.ws.rs.WebApplicationException;
import javax.ws.rs.core.MediaType;
import javax.ws.rs.core.Response;
import javax.xml.transform.dom.DOMSource;

import org.apache.commons.io.FileUtils;
import org.json.simple.JSONArray;
import org.json.simple.JSONObject;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.stereotype.Controller;
import org.w3c.dom.Document;

import hoot.services.models.db.JobStatus;
import hoot.services.utils.XmlDocumentBuilder;


/**
 * Derives an OSM XML changeset (.osc)
 * 
 * Keeping this logic separate from that of ChangesetResource since it is not part of the official 
 * OSM API and ChangesetResource currently contains only OSM API functions.
 */
@Controller
@Path("/changeset")
public class DeriveChangesetResource extends JobControllerBase {
    @SuppressWarnings("unused")
    private static final Logger logger = LoggerFactory.getLogger(DeriveChangesetResource.class);

    public DeriveChangesetResource() {
        super(DERIVE_CHANGESET_SCRIPT);
    }

    /**
     * Derives a diff between two OSM datasets, where one dataset is the source and the other a 
     * target, in the form of an OSM XML changeset.
     * 
     * @param input1 path to the first input dataset
     * @param input2 path to the second input dataset
     * @param aoi bounding box to which the changeset derivation should be applied
     * @return job ID of the changeset derivation job
     */
    @POST
    @Path("/derive")
    @Produces(MediaType.APPLICATION_JSON)
    public JobId derive(@QueryParam("input1") String input1, @QueryParam("input2") String input2, @QueryParam("aoi") String aoi) {
        
        String jobId = UUID.randomUUID().toString();
        jobId = "ex_" + jobId.replace("-", "");

        try {
            JSONArray commandArgs = new JSONArray();

            // ignoring outputname, since we're only going to have a single mapedit
            // connection configured in the core for now configured in the core for now
            JSONObject arg = new JSONObject();
            arg.put("input1", input1);
            arg.put("input2", input2);
            arg.put("aoi", aoi);
            File tempOutputDir = new File(TEMP_OUTPUT_PATH);
            if (!tempOutputDir.exists())
            {
              tempOutputDir.mkdir();   
            }
            //services currently always write changeset with xml
            arg.put("output", File.createTempFile("changeset", ".osc", tempOutputDir).getAbsolutePath());
            arg.put("jobId", jobId);
            
            commandArgs.add(arg);
        }
        catch (Exception e) {
            throw new WebApplicationException(e, Response.serverError().entity("Error deriving changeset for inputs: " + input1 + ", " + input2).build());
        }

        return new JobId(jobId);
    }

    /**
     * Returns the contents of an XML changeset file 
     * 
     * @param jobId job ID the changeset file is associated with
     * @return changeset XML contents
     * @throws WebApplicationException if the job with ID = jobID does not exist; or the referenced
     * temp changeset file no longer exists
     */
    @GET
    @Path("/getderived")
    @Produces(MediaType.TEXT_XML)
    public Response get(@PathParam("jobId") String jobId) {
        
        Document responseDoc = null;
        String changesetFileName = null;
        try {
            JobStatusManager jobStatusManager = new JobStatusManager();
            JobStatus jobStatus = jobStatusManager.getJobStatusObj(jobId);
            if (jobStatus == null)
            {
                throw new Exception("Job with ID: " + jobId + " does not exist.");
            }
            changesetFileName = jobStatus.getStatusDetail();
            responseDoc = XmlDocumentBuilder.parse(FileUtils.readFileToString(new File(changesetFileName), "UTF-8"));
        }
        catch (IOException e) {
            throw new WebApplicationException(e, Response.serverError().entity("Error fetching changeset contents for job ID=" + jobId + ".  Unable to read changeset temp file at " + changesetFileName + ".").build());
        }
        catch (Exception e) {
            throw new WebApplicationException(e, Response.serverError().entity("Error fetching changeset contents for job ID=" + jobId).build());
        }
        return Response.ok(new DOMSource(responseDoc)).build();
    }
}
