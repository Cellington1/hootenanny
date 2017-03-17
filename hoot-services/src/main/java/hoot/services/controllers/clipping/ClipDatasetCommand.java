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
 * @copyright Copyright (C) 2016, 2017 DigitalGlobe (http://www.digitalglobe.com/)
 */
package hoot.services.controllers.clipping;

import static hoot.services.HootProperties.HOOTAPI_DB_URL;

import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.stream.Collectors;

import hoot.services.command.ExternalCommand;
import hoot.services.utils.JsonUtils;

/*
#
#  Clip Dataset Make file
#
-include $(HOOT_HOME)/HOOT_VERSION_FILE

#HOOT_OPTS+= -D osm2ogr.ops=hoot::DecomposeBuildingRelationsVisitor -D add.review.tags=yes
HOOT_OPTS+= -D hootapi.db.writer.overwrite.map=true -D hootapi.db.writer.create.user=true
HOOT_OPTS+= -D api.db.email=test@test.com

#DB_URL=hootapidb://hoot:hoottest@localhost:5432/hoot
#OUTPUT_DIR=$(HOOT_HOME)/test-out/$(jobid)

# Clip
# crop-map (input) (output) (bounds)
OP_INPUT=$(DB_URL)/$(INPUT_NAME)
OP_OUTPUT=$(DB_URL)/$(OUTPUT_NAME)

step1:
    hoot crop-map $(HOOT_OPTS) "$(OP_INPUT)" "$(OP_OUTPUT)" "$(BBOX)"
 */

class ClipDatasetCommand extends ExternalCommand {

    ClipDatasetCommand(String params, String debugLevel, Class<?> caller) {
        Map<String, String> paramsMap = JsonUtils.paramsToMap(params);

        List<String> options = new LinkedList<>();
        options.add("-D hootapi.db.writer.overwrite.map=true");
        options.add("-D hootapi.db.writer.create.user=true");
        options.add("-D api.db.email=test@test.com");

        String input = HOOTAPI_DB_URL + "/" + paramsMap.get("INPUT_NAME");
        String output = HOOTAPI_DB_URL + "/" + paramsMap.get("OUTPUT_NAME");
        String bounds = paramsMap.get("BBOX");
        String hootOptions = options.stream().collect(Collectors.joining(" "));

        /*
            "crop-map" - Crops the input map to the given bounds. Individual features on the border are modified to make
                     sure nothing is outside the given bounds.
            • input - The input OSM data path.
            • output - The output OSM data path.
            • bounds - Comma delimited bounds. minx,miny,maxx,maxy e.g.38,-105,39,-104

            hoot crop-map $(HOOT_OPTS) "$(OP_INPUT)" "$(OP_OUTPUT)" "$(BBOX)"
        */
        String command = "hoot crop-map --" + debugLevel + " " + hootOptions + " " + input + " " + output + " " + bounds;

        super.configureAsHootCommand(command, caller);
    }
}
