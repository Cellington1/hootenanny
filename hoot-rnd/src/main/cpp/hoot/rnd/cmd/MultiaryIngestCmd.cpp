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
 * @copyright Copyright (C) 2017 DigitalGlobe (http://www.digitalglobe.com/)
 */

// Hoot
#include <hoot/core/util/Factory.h>
#include <hoot/core/cmd/BaseCommand.h>
#include <hoot/core/io/ChangesetDeriver.h>
#include <hoot/core/io/ElementSorter.h>
#include <hoot/core/io/OsmMapReaderFactory.h>
#include <hoot/core/io/OsmMapWriterFactory.h>
#include <hoot/core/io/HootApiDbWriter.h>
#include <hoot/rnd/io/SparkChangesetWriter.h>
#include <hoot/core/io/HootApiDbReader.h>
#include <hoot/core/io/OsmChangeWriterFactory.h>
#include <hoot/rnd/io/ElementCriterionVisitorInputStream.h>
#include <hoot/core/filters/PoiCriterion.h>
#include <hoot/core/visitors/TranslationVisitor.h>
#include <hoot/core/io/ElementOutputStream.h>
#include <hoot/core/io/GeoNamesReader.h>
//#include <hoot/core/io/OsmPbfReader.h>

// Qt
#include <QUuid>

using namespace std;

namespace hoot
{

/**
 * This command takes an input along with database and changeset outputs.  The input is filtered
 * down to POIs only and translated to OSM, then sorted by element ID if necessary, and then
 * compared to the database output layer in order to derive a changeset.  The changeset changes
 * are written both to the database output layer as features and the changeset output as change
 * statements.
 */
class MultiaryIngestCmd : public BaseCommand
{
public:

  static string className() { return "hoot::MultiaryIngestCmd"; }

  MultiaryIngestCmd() :
  _sortInput(false)
  {
  }

  virtual ~MultiaryIngestCmd()
  {
    //delete the temporary db layer used for sorting
    if (_sortInput)
    {
      LOG_DEBUG("Deleting temporary map: " << _sortedNewDataInput << "...");
      HootApiDbWriter().deleteMap(_sortedNewDataInput);
    }
  }

  virtual QString getName() const { return "multiary-ingest"; }

  virtual int runSimple(QStringList args)
  {
    if (args.size() != 4)
    {
      cout << getHelp() << endl << endl;
      throw HootException(QString("%1 takes four parameters.").arg(getName()));
    }

    const QString newDataInput = args[0];   //this must be streamable
    const QString dbLayerOutput = args[1];  //this must be hootapidb://
    const QString changesetOutput = args[2];    //this must be .spark.x
    _sortInput = false;
    if (args[3].toLower() == "true")
    {
      _sortInput = true;
    }

    LOG_VARD(newDataInput);
    LOG_VARD(dbLayerOutput);
    LOG_VARD(changesetOutput);
    LOG_VARD(_sortInput);

    if (!OsmMapReaderFactory::getInstance().hasElementInputStream(newDataInput))
    {
      throw IllegalArgumentException(
        "This command does not support non-streamable inputs: " + newDataInput);
    }

    if (!HootApiDbReader().isSupported(dbLayerOutput))
    {
      throw HootException(
        getName() + " only supports a hootapidb:// data source as the target changeset layer.  " +
        "Specified target: " + dbLayerOutput);
    }

    if (!SparkChangesetWriter().isSupported(changesetOutput))
    {
      throw HootException(
        getName() + " only supports a .spark.x file for changeset output.  Specified output: " +
        changesetOutput);
    }

    LOG_INFO(
      "Streaming multiary data ingest from input: " << newDataInput <<
      " to output database layer: " << dbLayerOutput << " and output changeset: " <<
      changesetOutput << "...");

    //inputs must be sorted by id for changeset derivation to work
    conf().set(ConfigOptions::getApiDbReaderSortByIdKey(), true);
    //translating inputs to OSM
    conf().set(ConfigOptions::getTranslationScriptKey(), "translations/OSM_Ingest.js");

    //sort incoming data by ID, if necessary, for changeset derivation (only passing nodes
    //through, so don't need to also sort by element type) -
    _sortedNewDataInput = _getSortedInput(newDataInput);

    //create changeset changes and write them to the existing db layer and a changeset file for
    //external use in spark
    _writeChangesetData(
      //as the incoming data is read, filter it down to POIs only and translate each element
      _getNewDataInputStream(_sortedNewDataInput),
      dbLayerOutput,
      changesetOutput);

    LOG_INFO(
      "Multiary data ingest complete for input: " << newDataInput <<
      " to output database layer: " << dbLayerOutput << " and output changeset: " <<
      changesetOutput << "...");

    return 0;
  }

private:

  bool _sortInput;
  QString _sortedNewDataInput;

  QString _getSortedInput(const QString newDataInput)
  {
    //OsmPbfReader tmpPbfReader; //getSortedTypeThenId
    if (!_sortInput) //TODO: if pbf, check pbf format flag
    {
      return newDataInput;
    }

    //write the unsorted input to temp db layer; later it will be queried back out sorted by id
    //TODO: assuming performance here is a bottleneck, will later implement something that
    //performs faster - either a file based merge sort, or using the db but with sql bulk copies

    boost::shared_ptr<PartialOsmMapReader> unsortedNewInputReader =
      boost::dynamic_pointer_cast<PartialOsmMapReader>(
        OsmMapReaderFactory::getInstance().createReader(newDataInput));
    boost::shared_ptr<ElementInputStream> unsortedNewInputStream =
      boost::dynamic_pointer_cast<ElementInputStream>(unsortedNewInputReader);
    boost::shared_ptr<PoiCriterion> elementCriterion;
    //all geonames data are pois by definition, so skip the element criterion filtering expense
    //if (!GeoNamesReader().isSupported(newDataInput)) //TODO: re-enable this
    //{
      elementCriterion.reset(new PoiCriterion());
    //}
    unsortedNewInputStream.reset(
      new ElementCriterionVisitorInputStream(
        unsortedNewInputStream,
        elementCriterion,
        boost::shared_ptr<TranslationVisitor>(new TranslationVisitor())));

    //TODO: add guid back in
    const QString tempNewInputLayer =
      HootApiDb::getBaseUrl().toString() + "/MultiaryIngest-temp-";// + QUuid::createUuid().toString();
    boost::shared_ptr<HootApiDbWriter> unsortedNewInputLayerWriter(new HootApiDbWriter());
    unsortedNewInputLayerWriter->setCreateUser(true);
    unsortedNewInputLayerWriter->setOverwriteMap(true);
    unsortedNewInputLayerWriter->setRemap(false);
    unsortedNewInputLayerWriter->open(tempNewInputLayer);
    boost::shared_ptr<ElementOutputStream> unsortedNewOutputStream =
      boost::dynamic_pointer_cast<ElementOutputStream>(unsortedNewInputLayerWriter);

    LOG_DEBUG("Writing multiary input to temp location: " << tempNewInputLayer << "...");
    ElementOutputStream::writeAllElements(*unsortedNewInputStream, *unsortedNewOutputStream);
    LOG_DEBUG("Multiary input written to temp location: " << tempNewInputLayer);

    return tempNewInputLayer;
  }

  boost::shared_ptr<ElementInputStream> _getNewDataInputStream(const QString sortedNewDataInput)
  {
    boost::shared_ptr<PartialOsmMapReader> newDataInputReader =
      boost::dynamic_pointer_cast<PartialOsmMapReader>(
        OsmMapReaderFactory::getInstance().createReader(sortedNewDataInput));
    newDataInputReader->open(sortedNewDataInput);
    return boost::dynamic_pointer_cast<ElementInputStream>(newDataInputReader);
  }

  void _writeChangesetData(boost::shared_ptr<ElementInputStream> newDataInputStream,
                           const QString dbLayerOutput, const QString changesetOutput)
  {
    LOG_DEBUG(
      "Writing multiary changeset data to existing layer: " << dbLayerOutput <<
      " and changeset: " << changesetOutput << "...")

    boost::shared_ptr<HootApiDbReader> existingDbLayerReader(new HootApiDbReader());
    existingDbLayerReader->open(dbLayerOutput);

    ChangesetDeriverPtr changesetDeriver(
      new ChangesetDeriver(
        boost::dynamic_pointer_cast<ElementInputStream>(existingDbLayerReader),
        newDataInputStream));

    HootApiDbWriter existingDbLayerChangeWriter;
    existingDbLayerChangeWriter.setCreateUser(false);
    existingDbLayerChangeWriter.setOverwriteMap(false);
    existingDbLayerChangeWriter.open(dbLayerOutput);

    SparkChangesetWriter changesetFileWriter;
    changesetFileWriter.open(changesetOutput);

    while (changesetDeriver->hasMoreChanges())
    {
      const Change change = changesetDeriver->readNextChange();
      if (change.type != Change::Unknown)
      {
        changesetFileWriter.writeChange(change);
        existingDbLayerChangeWriter.writeChange(change);
      }
    }

    //all readers/writers involved are cleaning up by themselves in their destructors

    LOG_DEBUG("Multiary change writing complete.");
  }
};

HOOT_FACTORY_REGISTER(Command, MultiaryIngestCmd)

}
