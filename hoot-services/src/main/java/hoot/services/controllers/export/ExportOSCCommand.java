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
 * @copyright Copyright (C) 2015, 2016, 2017 DigitalGlobe (http://www.digitalglobe.com/)
 */
package hoot.services.controllers.export;

import static hoot.services.HootProperties.HOOTAPI_DB_URL;
import static hoot.services.HootProperties.OSMAPI_DB_URL;

import java.util.List;
import java.util.stream.Collectors;

import hoot.services.geo.BoundingBox;


class ExportOSCCommand extends ExportCommand {

    ExportOSCCommand(String jobId, ExportParams params, String debugLevel, Class<?> caller) {
        super(jobId, params, debugLevel, caller);

        String aoi = getBoundingBox(params);
        String outputPath = super.getOutputPath();

        List<String> options = super.getCommonExportHootOptions();
        options.add("-D convert.bounding.box=" + aoi);
        options.add("-D osm.changeset.sql.file.writer.generate.new.ids=false");
        String hootOptions = options.stream().collect(Collectors.joining(" "));

        String input1 = OSMAPI_DB_URL;
        String input2 = HOOTAPI_DB_URL + "/" + params.getInput();

        //hoot derive-changeset $(HOOT_OPTS) -D convert.bounding.box=$(aoi) -D osm.changeset.sql.file.writer.generate.new.ids=false $(input1) $(input2) "$(OP_OUTPUT)"
        String command = "hoot derive-changeset --" + debugLevel + " " + hootOptions + " " + quote(input1) + " " + quote(input2) + " " + quote(outputPath);

        super.configureCommand(command, caller);
    }

    private static String getBoundingBox(ExportParams params) {
        if (params.getBounds() != null) {
            throw new IllegalArgumentException("When exporting to a changeset, bounds must be specified.");
        }

        BoundingBox bounds = new BoundingBox(params.getBounds());
        return bounds.getMinLon() + "," + bounds.getMinLat() + "," + bounds.getMaxLon() + "," + bounds.getMaxLat();
    }
}
