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

// Hoot
#include <hoot/core/util/Factory.h>
#include <hoot/core/cmd/BaseCommand.h>
#include <hoot/core/io/OsmMapReader.h>
#include <hoot/core/io/OsmMapReaderFactory.h>
#include <hoot/core/util/GeometryUtils.h>
#include <hoot/core/util/OpenCv.h>
#include <hoot/core/conflate/TileBoundsCalculator.h>
#include <hoot/core/util/FileUtils.h>

namespace hoot
{

class CalculateTilesCmd : public BaseCommand
{

  public:

    static string className() { return "hoot::CalculateTilesCmd"; }

    CalculateTilesCmd() { }

    virtual QString getName() const { return "calculate-tiles"; }

    virtual int runSimple(QStringList args)
    {
      if (args.size() < 2 || args.size() > 4)
      {
        cout << getHelp() << endl << endl;
        throw HootException(QString("%1 takes two to four parameters.").arg(getName()));
      }

      QStringList inputs;
      const QString input = args[0];
      LOG_VARD(input);
      if (!input.contains(";"))
      {
        inputs.append(input);
      }
      else
      {
        //multiple inputs
        inputs = input.split(";");
      }
      LOG_VARD(inputs);

      const QString output = args[1];
      LOG_VARD(output);

      long maxNodesPerTile = 1000;
      if (args.size() > 2)
      {
        bool parseSuccess = false;
        maxNodesPerTile = args[2].toLong(&parseSuccess);
        if (!parseSuccess || maxNodesPerTile < 1)
        {
          throw HootException("Invalid maximum nodes per tile value: " + args[2]);
        }
      }
      LOG_VARD(maxNodesPerTile);

      double pixelSize = 0.001; //.1km?
      if (args.size() > 3)
      {
        bool parseSuccess = false;
        pixelSize = args[3].toDouble(&parseSuccess);
        if (!parseSuccess || pixelSize <= 0.0)
        {
          throw HootException("Invalid pixel size value: " + args[3]);
        }
      }
      LOG_VARD(pixelSize);

      OsmMapPtr map(new OsmMap());
      for (int i = 0; i < inputs.size(); i++)
      {
        boost::shared_ptr<OsmMapReader> reader =
          OsmMapReaderFactory::getInstance().createReader(inputs.at(i), true, Status::Unknown1);
        reader->open(inputs.at(i));
        reader->read(map);
      }
      LOG_VARD(map->getNodeCount());

      TileBoundsCalculator tileBoundsCalculator(pixelSize);
      tileBoundsCalculator.setMaxNodesPerBox(maxNodesPerTile);
      //tbc.setSlop(0.1);
      cv::Mat r1, r2;
      tileBoundsCalculator.renderImage(map, r1, r2);
      //we're calculating for unknown1 only, so fill the second matrix with all zeroes
      cv::Mat zeros = cv::Mat::zeros(r1.size(), r1.type());
      tileBoundsCalculator.setImages(r1, zeros);
      const std::vector< std::vector<geos::geom::Envelope> > tiles =
        tileBoundsCalculator.calculateTiles();

      //write a semi-colon delimited string of bounds obj's to output
      QString outputTilesStr;
      LOG_VARD(tiles.size());
      for (size_t tx = 0; tx < tiles.size(); tx++)
      {
        LOG_VART(tiles[tx].size());
        for (size_t ty = 0; ty < tiles[tx].size(); ty++)
        {
          outputTilesStr += GeometryUtils::envelopeToConfigString(tiles[tx][ty]) + ";";
        }
      }
      outputTilesStr.chop(1);
      LOG_VARD(outputTilesStr);
      FileUtils::writeFully(output, outputTilesStr);

      return 0;
    }
};

HOOT_FACTORY_REGISTER(Command, CalculateTilesCmd)

}
