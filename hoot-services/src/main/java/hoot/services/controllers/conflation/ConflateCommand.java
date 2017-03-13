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
package hoot.services.controllers.conflation;

import static hoot.services.HootProperties.*;

import java.io.File;
import java.io.IOException;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;

import org.apache.commons.io.FileUtils;
import org.json.simple.JSONArray;
import org.json.simple.JSONObject;
import org.json.simple.parser.ParseException;

import hoot.services.command.ExternalCommand;
import hoot.services.geo.BoundingBox;
import hoot.services.utils.JsonUtils;

/*
#
#  Conflate Make file
#
-include $(HOOT_HOME)/HOOT_VERSION_FILE
HOOT_OPTS+= -D osm2ogr.ops=hoot::DecomposeBuildingRelationsVisitor -D conflate.add.score.tags=yes
HOOT_OPTS+= -D hootapi.db.writer.overwrite.map=true -D hootapi.db.writer.create.user=true
HOOT_OPTS+= -D api.db.email=test@test.com

#DB_URL=hootapidb://hoot:hoottest@localhost:5432/hoot
#OSM_API_DB_URL="osmapidb://hoot:hoottest@localhost:5432/osmapi_test"

OUTPUT_DIR=$(HOOT_HOME)/test-out/$(jobid)
OP_INPUT1=$(HOOT_HOME)/$(INPUT1)
OP_INPUT2=$(HOOT_HOME)/$(INPUT2)
OP_OUTPUT=$(OUTPUT_DIR)/$(jobid).osm
OP_OUTPUT_RENUM=$(OUTPUT_DIR)/$(jobid)_renum.osm
DB_OUTPUT=$(DB_URL)/$(OUTPUT_NAME)
OP_REPORT_CONF_TYPE=advancedConflation
OP_STAT=
OP_CMD=

OP_CONFLATE_TYPE=--conflate
HOOT_OPTS+= $(ADV_OPTIONS)

ifeq "$(REFERENCE_LAYER)" "2"
	HOOT_OPTS+= -D tag.merger.default=hoot::OverwriteTag1Merger
endif

ifeq "$(CONFLATION_TYPE)" "Average"
	OP_REPORT_CONF_TYPE=average
endif

ifeq "$(CONFLATION_TYPE)" "Reference"
	OP_REPORT_CONF_TYPE=reference
endif

ifeq "$(INPUT1_TYPE)" "DB"
	OP_INPUT1=$(DB_URL)/$(INPUT1)
endif

ifeq "$(INPUT2_TYPE)" "DB"
	OP_INPUT2=$(DB_URL)/$(INPUT2)
endif

# This is also depending on some extra input validation present in ConflationResource.
ifeq "$(REFERENCE_LAYER)" "1"
	ifeq "$(INPUT1_TYPE)" "OSM_API_DB"
    OP_INPUT1=$(OSM_API_DB_URL)
		HOOT_OPTS+= -D convert.bounding.box=$(conflateaoi) -D conflate.use.data.source.ids=true -D osm.map.reader.factory.reader=hoot::OsmApiDbAwareHootApiDbReader -D osm.map.writer.factory.writer=hoot::OsmApiDbAwareHootApiDbWriter -D osmapidb.id.aware.url="$(OSM_API_DB_URL)"
	endif
endif
ifeq "$(REFERENCE_LAYER)" "2"
	ifeq "$(INPUT2_TYPE)" "OSM_API_DB"
    OP_INPUT2=$(OSM_API_DB_URL)
		HOOT_OPTS+= -D convert.bounding.box=$(conflateaoi) -D conflate.use.data.source.ids=true -D osm.map.reader.factory.reader=hoot::OsmApiDbAwareHootApiDbReader -D osm.map.writer.factory.writer=hoot::OsmApiDbAwareHootApiDbWriter -D osmapidb.id.aware.url="$(OSM_API_DB_URL)"
	endif
endif

ifeq "$(GENERATE_REPORT)" "true"
	HOOT_OPTS+= -D stats.format=asciidoc -D stats.output=$(HOOT_HOME)/userfiles/reports/$(jobid)/reportBody -D conflate.stats.types=$(OP_REPORT_CONF_TYPE)
	OP_STAT= --stats
	OP_CMD=$(subst ;,!semi!,$(HOOT_OPTS))
endif

ifeq "$(COLLECT_STATS)" "true"
	OP_STAT= --stats > $(HOOT_HOME)/userfiles/reports/$(OUTPUT_NAME)-stats.csv
	# don't include non-error log messages in stdout because we are redirecting to file
	HOOT_OPTS+= --error
endif

###
# Conflate
###
step1:
ifeq "$(COLLECT_STATS)" "true"
	mkdir -p $(HOOT_HOME)/userfiles/reports
endif
ifeq "$(GENERATE_REPORT)" "true"
	mkdir -p $(HOOT_HOME)/userfiles/reports/$(jobid)
	cp -a $(HOOT_HOME)/report/. $(HOOT_HOME)/reports/$(jobid)
endif
	hoot $(OP_CONFLATE_TYPE) -C RemoveReview2Pre.conf $(HOOT_OPTS) "$(OP_INPUT1)" "$(OP_INPUT2)" "$(DB_OUTPUT)" $(OP_STAT)
ifeq "$(GENERATE_REPORT)" "true"

	cd $(HOOT_HOME)/userfiles/reports/$(jobid) && a2x -a docinfo --dblatex-opts "-P latex.output.revhistory=0 -P latex.unicode.use=1 -s reportStyle.sty --param doc.publisher.show=0" -a HasLatexMath -a 'revdate=v`$HOOT_HOME/bin/hoot version --error | sed "s/Hootenanny \([^ ]* \) Built.* /\\1/g"`, `date "+%B %d, %Y"`' -a "input1=$(OP_INPUT1)" -a "input2=$(OP_INPUT2)" -a "output=$(DB_OUTPUT)" -a "args=" -a "cmd1=hoot $(OP_CONFLATE_TYPE) $(OP_CMD) $(OP_INPUT2)  $(OP_STAT)" -v -f pdf report.asciidoc

        echo '{"name":"$(OUTPUT_NAME)","description":"$(OUTPUT_NAME)","created":"$(TIME_STAMP)","reportpath":"$(HOOT_HOME)/userfiles/reports/$(jobid)/report.pdf"}' > $(HOOT_HOME)/userfiles/reports/$(jobid)/meta.data
        endif
        */

class ConflateCommand extends ExternalCommand {

    ConflateCommand(String jobId, String params, BoundingBox bounds, Class<?> caller) {
        JSONArray commandArgs = new JSONArray();

        Map<String, String> paramMap;
        try {
            paramMap = JsonUtils.paramsToMap(params);
        }
        catch (ParseException pe) {
            throw new RuntimeException("Error parsing: " + params, pe);
        }

        //HOOT_OPTS+= -D osm2ogr.ops=hoot::DecomposeBuildingRelationsVisitor -D conflate.add.score.tags=yes
        //HOOT_OPTS+= -D hootapi.db.writer.overwrite.map=true -D hootapi.db.writer.create.user=true
        //HOOT_OPTS+= -D api.db.email=test@test.com

        String conflateaoi = null;
        if (bounds != null) {
            conflateaoi = bounds.getMinLon() + "," + bounds.getMinLat() + "," + bounds.getMaxLon() + "," + bounds.getMaxLat();
        }

        List<String> hootOptions = new LinkedList<>();
        hootOptions.add("-D osm2ogr.ops=hoot::DecomposeBuildingRelationsVisitor");
        hootOptions.add("-D conflate.add.score.tags=yes");
        hootOptions.add("-D hootapi.db.writer.overwrite.map=true");
        hootOptions.add("-D hootapi.db.writer.create.user=true");
        hootOptions.add("-D api.db.email=test@test.com");

        //HOOT_OPTS+= $(ADV_OPTIONS)
        if (paramMap.containsKey("ADV_OPTIONS")) {
            hootOptions.add(paramMap.get("ADV_OPTIONS"));
        }

        /*
            ifeq "$(INPUT1_TYPE)" "DB"
                OP_INPUT1=$(DB_URL)/$(INPUT1)
            endif

            ifeq "$(INPUT2_TYPE)" "DB"
                OP_INPUT2=$(DB_URL)/$(INPUT2)
            endif
         */

        String input1Type = paramMap.get("INPUT1_TYPE");
        String input1 = paramMap.get("INPUT1");
        if (input1Type.equals("DB")) {
            input1 = HOOTAPI_DB_URL + "/" + input1;
        }

        String input2Type = paramMap.get("INPUT2_TYPE");
        String input2 = paramMap.get("INPUT2");
        if (input2Type.equals("DB")) {
            input2 = HOOTAPI_DB_URL + "/" + input2;
        }

        /*
          ifeq "$(REFERENCE_LAYER)" "2"
              HOOT_OPTS+= -D tag.merger.default=hoot::OverwriteTag1Merger
          endif

          # This is also depending on some extra input validation present in ConflationResource.
          ifeq "$(REFERENCE_LAYER)" "1"
              ifeq "$(INPUT1_TYPE)" "OSM_API_DB"
                  OP_INPUT1=$(OSM_API_DB_URL)
                  HOOT_OPTS+= -D convert.bounding.box=$(conflateaoi) -D conflate.use.data.source.ids=true -D osm.map.reader.factory.reader=hoot::OsmApiDbAwareHootApiDbReader -D osm.map.writer.factory.writer=hoot::OsmApiDbAwareHootApiDbWriter -D osmapidb.id.aware.url="$(OSM_API_DB_URL)"
              endif
          endif

          ifeq "$(REFERENCE_LAYER)" "2"
              ifeq "$(INPUT2_TYPE)" "OSM_API_DB"
                  OP_INPUT2=$(OSM_API_DB_URL)
                  HOOT_OPTS+= -D convert.bounding.box=$(conflateaoi) -D conflate.use.data.source.ids=true -D osm.map.reader.factory.reader=hoot::OsmApiDbAwareHootApiDbReader -D osm.map.writer.factory.writer=hoot::OsmApiDbAwareHootApiDbWriter -D osmapidb.id.aware.url="$(OSM_API_DB_URL)"
              endif
          endif
         */
        String referenceLayer = paramMap.get("REFERENCE_LAYER");
        if (referenceLayer.equals("1")) {
            if (input1Type.equals("OSM_API_DB")) {
                input1 = OSMAPI_DB_URL;
                hootOptions.add("-D convert.bounding.box=" + conflateaoi);
                hootOptions.add("-D conflate.use.data.source.ids=true");
                hootOptions.add("-D osm.map.reader.factory.reader=hoot::OsmApiDbAwareHootApiDbReader");
                hootOptions.add("-D osm.map.writer.factory.writer=hoot::OsmApiDbAwareHootApiDbWriter");
                hootOptions.add("-D osmapidb.id.aware.url=\"" + OSMAPI_DB_URL + "\"");
            }
        }
        else if (referenceLayer.equals("2")) {
            hootOptions.add("-D tag.merger.default=hoot::OverwriteTag1Merger");
            if (input2Type.equals("OSM_API_DB")) {
                input2 = OSMAPI_DB_URL;
                hootOptions.add("-D convert.bounding.box=" + conflateaoi);
                hootOptions.add("-D conflate.use.data.source.ids=true");
                hootOptions.add("-D osm.map.reader.factory.reader=hoot::OsmApiDbAwareHootApiDbReader");
                hootOptions.add("-D osm.map.writer.factory.writer=hoot::OsmApiDbAwareHootApiDbWriter");
                hootOptions.add("-D osmapidb.id.aware.url=\"" + OSMAPI_DB_URL + "\"");
            }
        }


        /*
          ifeq "$(CONFLATION_TYPE)" "Average"
              OP_REPORT_CONF_TYPE=average
          endif

          ifeq "$(CONFLATION_TYPE)" "Reference"
              OP_REPORT_CONF_TYPE=reference
          endif
         */
        String conflationType = paramMap.get("CONFLATION_TYPE");
        String reportConfType = null;
        if (conflationType != null) {
            if (conflationType.equals("Average")) {
                reportConfType = "average";
            }
            else if (conflationType.equals("Reference")) {
                reportConfType = "reference";
            }
        }

        /*
          ifeq "$(GENERATE_REPORT)" "true"
              HOOT_OPTS+= -D stats.format=asciidoc -D stats.output=$(HOOT_HOME)/userfiles/reports/$(jobid)/reportBody -D conflate.stats.types=$(OP_REPORT_CONF_TYPE)
              OP_STAT= --stats
              OP_CMD=$(subst ;,!semi!,$(HOOT_OPTS))
          endif
        */
        Boolean generateReport = Boolean.valueOf(paramMap.get("GENERATE_REPORT"));
        String opStat = null;
        if (generateReport) {
            opStat = "--stats";
            hootOptions.add("-D stats.format=asciidoc");
            hootOptions.add("-D stats.output=" + RPT_STORE_PATH + "/" + jobId + "/reportBody");
            hootOptions.add("-D conflate.stats.types=" + reportConfType);

            // TODO: Port the line below
            //OP_CMD=$(subst ;,!semi!,$(HOOT_OPTS))

            /*
              ifeq "$(GENERATE_REPORT)" "true"
                mkdir -p $(HOOT_HOME)/userfiles/reports/$(jobid)
                cp -a $(HOOT_HOME)/report/. $(HOOT_HOME)/reports/$(jobid)
              endif
            */

            File reportsFolder = new File(RPT_STORE_PATH, jobId);
            try {
                FileUtils.forceMkdir(reportsFolder);
            }
            catch (IOException ioe) {
                throw new RuntimeException("Error creating " + reportsFolder, ioe);
            }

            File srcDir = new File(HOME_FOLDER, "report");
            try {
                FileUtils.copyDirectory(srcDir, reportsFolder, true);
            }
            catch (IOException ioe) {
                throw new RuntimeException("Error copying " + srcDir + " to " + reportsFolder, ioe);
            }
        }

        /*
          ifeq "$(COLLECT_STATS)" "true"
              OP_STAT= --stats > $(HOOT_HOME)/userfiles/reports/$(OUTPUT_NAME)-stats.csv
              # don't include non-error log messages in stdout because we are redirecting to file
              HOOT_OPTS+= --error
          endif
         */
        Boolean collectStats = Boolean.valueOf(paramMap.get("COLLECT_STATS"));
        String outputName = paramMap.get("OUTPUT_NAME");

        if (collectStats) {
            opStat = "--stats > " + RPT_STORE_PATH + File.separator + outputName + "-stats.csv";
            hootOptions.add("--error");
            /*
              ifeq "$(COLLECT_STATS)" "true"
                  mkdir -p $(HOOT_HOME)/userfiles/reports
              endif
             */
        }

        String dbOutput = HOOTAPI_DB_URL + "/" + outputName;

        /*
            hoot $(OP_CONFLATE_TYPE) -C RemoveReview2Pre.conf $(HOOT_OPTS) "$(OP_INPUT1)" "$(OP_INPUT2)" "$(DB_OUTPUT)" $(OP_STAT)
        */
        JSONObject arg = new JSONObject();
        arg.put("CONFLATE_TYPE", conflationType);
        commandArgs.add(arg);

        arg = new JSONObject();
        arg.put("REMOVE_REVIEW", "-C RemoveReview2Pre.conf");
        commandArgs.add(arg);

        arg = new JSONObject();
        arg.put("HOOT_OPTIONS", hootOptions);
        commandArgs.add(arg);

        arg = new JSONObject();
        arg.put("INPUT1", input1);
        commandArgs.add(arg);

        arg = new JSONObject();
        arg.put("INPUT2", input2);
        commandArgs.add(arg);

        arg = new JSONObject();
        arg.put("DB_OUTPUT", dbOutput);
        commandArgs.add(arg);

        arg = new JSONObject();
        arg.put("STAT", opStat);
        commandArgs.add(arg);

        super.configureAsHootCommand("--conflate", caller, commandArgs);
    }
}
