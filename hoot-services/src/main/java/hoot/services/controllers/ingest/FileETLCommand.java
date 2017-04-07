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
package hoot.services.controllers.ingest;

import static hoot.services.HootProperties.HOOTAPI_DB_URL;

import java.io.File;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.stream.Collectors;

import hoot.services.command.ExternalCommand;

/*
    #
    #  ETL Make file
    #

    HOOT_OPTS+= -D osm2ogr.ops=hoot::DecomposeBuildingRelationsVisitor
    HOOT_OPTS+= -D hootapi.db.writer.overwrite.map=true -D hootapi.db.writer.create.user=true
    HOOT_OPTS+= -D api.db.email=test@test.com

    OP_INPUT=$(INPUT)
    OP_TRANSLATION=$(HOOT_HOME)/$(TRANSLATION)
    OP_INPUT_PATH=$(INPUT_PATH)

    # This replaces semicolon with vsizip and path
    ifeq "$(INPUT_TYPE)" "ZIP"
        #OP_INPUT="/vsizip/$(OP_INPUT_PATH)/$(subst ;,/" "/vsizip/$(OP_INPUT_PATH)/,$(INPUT))/"
        OP_INPUT="/vsizip/$(OP_INPUT_PATH)/$(subst ;," "/vsizip/$(OP_INPUT_PATH),$(INPUT))"
    endif

    ifeq "$(INPUT_TYPE)" "GEONAMES"
        HOOT_OPTS+= -D convert.ops=hoot::TranslationOp
        HOOT_OPTS+= -D translation.script="$(OP_TRANSLATION)"
    endif

    ifeq "$(INPUT_TYPE)" "OSM"
        ifneq "$(NONE_TRANSLATION)" "true"
            HOOT_OPTS+= -D convert.ops=hoot::TranslationOp
            HOOT_OPTS+= -D translation.script="$(OP_TRANSLATION)"
        endif
    endif

    ###
    # Transform and load data
    ###
    step1:

    # Unzip when semicolon separated lists are provided
    ifneq ($(strip $(UNZIP_LIST)), )
        bash $(HOOT_HOME)/scripts/util/unzipfiles.sh "$(UNZIP_LIST)" "$(OP_INPUT_PATH)"
    endif

    ifeq "$(INPUT_TYPE)" "OGR"
        cd "$(OP_INPUT_PATH)" && hoot ogr2osm $(HOOT_OPTS) "$(OP_TRANSLATION)" "$(DB_URL)/$(INPUT_NAME)" $(OP_INPUT)
    endif

    ifeq "$(INPUT_TYPE)" "OSM"
        cd "$(OP_INPUT_PATH)" && hoot convert $(HOOT_OPTS) $(OP_INPUT) "$(DB_URL)/$(INPUT_NAME)"
    endif

    ifeq "$(INPUT_TYPE)" "ZIP"
        cd "$(OP_INPUT_PATH)" && hoot ogr2osm $(HOOT_OPTS) "$(OP_TRANSLATION)" "$(DB_URL)/$(INPUT_NAME)" $(OP_INPUT)
    endif

    ifeq "$(INPUT_TYPE)" "FGDB"
        cd "$(OP_INPUT_PATH)" && hoot ogr2osm $(HOOT_OPTS) "$(OP_TRANSLATION)" "$(DB_URL)/$(INPUT_NAME)" $(OP_INPUT)
    endif

    ifeq "$(INPUT_TYPE)" "GEONAMES"
        cd "$(OP_INPUT_PATH)" && hoot convert $(HOOT_OPTS) $(OP_INPUT) "$(DB_URL)/$(INPUT_NAME)"
    endif

    rm -rf "$(OP_INPUT_PATH)"
*/

class FileETLCommand extends ExternalCommand {

    FileETLCommand(List<Map<String, String>> requests, List<File> zips, String translationPath, String etlName,
                   Boolean isNoneTranslation, String debugLevel, String inputType, Class<?> caller) {

        StringBuilder stringBuilder = new StringBuilder();
        for (Map<String,String> request : requests) {
            stringBuilder.append(quote(request.get("name"))).append(" ");
        }
        String inputs = stringBuilder.toString().trim();

        //HOOT_OPTS+= -D osm2ogr.ops=hoot::DecomposeBuildingRelationsVisitor
        //HOOT_OPTS+= -D hootapi.db.writer.overwrite.map=true -D hootapi.db.writer.create.user=true
        //HOOT_OPTS+= -D api.db.email=test@test.com

        List<String> options = new LinkedList<>();
        options.add("osm2ogr.ops=hoot::DecomposeBuildingRelationsVisitor");
        options.add("hootapi.db.writer.overwrite.map=true");
        options.add("hootapi.db.writer.create.user=true");
        options.add("api.db.email=test@test.com");

        //ifeq "$(INPUT_TYPE)" "GEONAMES"
        //    HOOT_OPTS+= -D convert.ops=hoot::TranslationOp
        //    HOOT_OPTS+= -D translation.script="$(OP_TRANSLATION)"
        //endif

        // OP_TRANSLATION=$(HOOT_HOME)/$(TRANSLATION)

        if (inputType.equalsIgnoreCase("GEONAMES")) {
            options.add("convert.ops=hoot::TranslationOp");
            options.add(quote("translation.script=" + translationPath));
        }

        //ifeq "$(INPUT_TYPE)" "OSM"
        //    ifneq "$(NONE_TRANSLATION)" "true"
        //        HOOT_OPTS+= -D convert.ops=hoot::TranslationOp
        //       HOOT_OPTS+= -D translation.script="$(OP_TRANSLATION)"
        //    endif
        //endif

        if (inputType.equalsIgnoreCase("OSM") && !isNoneTranslation) {
            options.add("convert.ops=hoot::TranslationOp");
            options.add(quote("translation.script=" + translationPath));
        }

        //# This replaces semicolon with vsizip and path
        //ifeq "$(INPUT_TYPE)" "ZIP"
        //   #OP_INPUT="/vsizip/$(OP_INPUT_PATH)/$(subst ;,/" "/vsizip/$(OP_INPUT_PATH)/,$(INPUT))/"
        //   OP_INPUT="/vsizip/$(OP_INPUT_PATH)/$(subst ;," "/vsizip/$(OP_INPUT_PATH),$(INPUT))"
        //endif

        if (!zips.isEmpty()) {
            //Reading a GDAL dataset in a .gz file or a .zip archive
            // OP_INPUT_PATH (INPUT_PATH)
            inputs = zips.stream()
                    .map(zip -> "/vsizip/" + zip.getAbsolutePath())
                    .collect(Collectors.joining(" "));
        }

        //ifeq "$(INPUT_TYPE)" "OGR"
        //    cd "$(OP_INPUT_PATH)" && hoot ogr2osm $(HOOT_OPTS) "$(OP_TRANSLATION)" "$(DB_URL)/$(INPUT_NAME)" $(OP_INPUT)
        //endif

        //ifeq "$(INPUT_TYPE)" "FGDB"
        //    cd "$(OP_INPUT_PATH)" && hoot ogr2osm $(HOOT_OPTS) "$(OP_TRANSLATION)" "$(DB_URL)/$(INPUT_NAME)" $(OP_INPUT)
        //endif

        //ifeq "$(INPUT_TYPE)" "ZIP"
        //    cd "$(OP_INPUT_PATH)" && hoot ogr2osm $(HOOT_OPTS) "$(OP_TRANSLATION)" "$(DB_URL)/$(INPUT_NAME)" $(OP_INPUT)
        //endif

        //ifeq "$(INPUT_TYPE)" "OSM"
        //    cd "$(OP_INPUT_PATH)" && hoot convert $(HOOT_OPTS) $(OP_INPUT) "$(DB_URL)/$(INPUT_NAME)"
        //endif

        //ifeq "$(INPUT_TYPE)" "GEONAMES"
        //    cd "$(OP_INPUT_PATH)" && hoot convert $(HOOT_OPTS) $(OP_INPUT) "$(DB_URL)/$(INPUT_NAME)"
        //endif

        String hootOptions = hootOptionsToString(options);
        String inputName = HOOTAPI_DB_URL + "/" + etlName;
        String command = null;

        Map<String, String> substitutionMap = new HashMap<>();
        substitutionMap.put("DEBUG_LEVEL", debugLevel);
        substitutionMap.put("HOOT_OPTIONS", hootOptions);
        substitutionMap.put("INPUT_NAME", inputName);
        substitutionMap.put("INPUTS", inputs);

        if ("OGR".equalsIgnoreCase(inputType) || "FGDB".equalsIgnoreCase(inputType) || "ZIP".equalsIgnoreCase(inputType)) {
            if (!isNoneTranslation) {
                substitutionMap.put("TRANSLATION_PATH", translationPath);

                // '' around ${} signifies that quoting is needed
                command = "hoot ogr2osm --${DEBUG_LEVEL} ${HOOT_OPTIONS} '${TRANSLATION_PATH}' '${INPUT_NAME}' ${INPUTS}";
            }
            else {
                // '' around ${} signifies that quoting is needed
                command = "hoot convert --${DEBUG_LEVEL} ${HOOT_OPTIONS} ${INPUTS} '${INPUT_NAME}'";
            }
        }
        else if ("OSM".equals(inputType) || "GEONAMES".equals(inputType)) {
            // '' around ${} signifies that quoting is needed
            command = "hoot convert --${DEBUG_LEVEL} ${HOOT_OPTIONS} ${INPUTS} '${INPUT_NAME}'";
        }

        super.configureCommand(command, substitutionMap, caller);
    }
}
