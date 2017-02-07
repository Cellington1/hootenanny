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
 * @copyright Copyright (C) 2016 DigitalGlobe (http://www.digitalglobe.com/)
 */
package hoot.services.controllers.nonblocking.ogr;

import static hoot.services.HootProperties.GET_OGR_ATTRIBUTE_SCRIPT;

import java.util.List;

import org.apache.commons.lang3.StringUtils;
import org.json.simple.JSONArray;
import org.json.simple.JSONObject;

import hoot.services.command.ExternalCommand;


public class GetAttributesCommand extends ExternalCommand {

    public GetAttributesCommand(String jobId, List<String> fileList, List<String> zipList, Class<?> callerClass) {
        JSONArray commandArgs = new JSONArray();

        JSONObject arg = new JSONObject();
        arg.put("INPUT_FILES", StringUtils.join(fileList.toArray(), ' '));
        commandArgs.add(arg);

        arg = new JSONObject();
        arg.put("INPUT_ZIPS", StringUtils.join(zipList.toArray(), ';'));
        commandArgs.add(arg);

        arg = new JSONObject();
        arg.put("jobid", jobId);
        commandArgs.add(arg);

        this.put("exectype", "make");
        this.put("exec", GET_OGR_ATTRIBUTE_SCRIPT);
        this.put("caller", callerClass.getName());
        this.put("params", commandArgs);
    }
 }
