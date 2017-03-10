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

#include "OsmApiDbCsvTableFileSetWriter.h"

#include <QDateTime>
#include <QFileInfo>
#include <QStringBuilder>

#include <hoot/core/util/HootException.h>
#include <hoot/core/util/Factory.h>
#include <hoot/core/util/Settings.h>
#include <hoot/core/util/DbUtils.h>
#include <hoot/core/io/OsmApiDb.h>
#include <hoot/core/elements/Tags.h>

#include <tgs/System/SystemInfo.h>

namespace hoot
{

using namespace Tgs;

unsigned int OsmApiDbCsvTableFileSetWriter::logWarnCount = 0;

HOOT_FACTORY_REGISTER(OsmMapWriter, OsmApiDbCsvTableFileSetWriter)

OsmApiDbCsvTableFileSetWriter::OsmApiDbCsvTableFileSetWriter() :
_unflushedRecords(0)
{
  setConfiguration(conf());
}

OsmApiDbCsvTableFileSetWriter::~OsmApiDbCsvTableFileSetWriter()
{
  close();
}

bool OsmApiDbCsvTableFileSetWriter::isSupported(QString urlStr)
{
  return urlStr.toLower().endsWith(".csv");
}

void OsmApiDbCsvTableFileSetWriter::open(QString url)
{
  _init(url);
}

void OsmApiDbCsvTableFileSetWriter::_init(const QString outputUrl)
{
  QStringList outputFileTypes;
  //TODO: deal with this
  //outputFileTypes.push_back(ApiDb::getChangesetsTableName());
  outputFileTypes.push_back(ApiDb::getCurrentNodesTableName());
  outputFileTypes.push_back(ApiDb::getCurrentNodeTagsTableName());
  outputFileTypes.push_back(ApiDb::getNodesTableName());
  outputFileTypes.push_back(ApiDb::getNodeTagsTableName());
  outputFileTypes.push_back(ApiDb::getCurrentWaysTableName());
  outputFileTypes.push_back(ApiDb::getCurrentWayNodesTableName());
  outputFileTypes.push_back(ApiDb::getCurrentWayTagsTableName());
  outputFileTypes.push_back(ApiDb::getWaysTableName());
  outputFileTypes.push_back(ApiDb::getWayNodesTableName());
  outputFileTypes.push_back(ApiDb::getWayTagsTableName());
  outputFileTypes.push_back(ApiDb::getCurrentRelationsTableName());
  outputFileTypes.push_back(ApiDb::getCurrentRelationMembersTableName());
  outputFileTypes.push_back(ApiDb::getCurrentRelationTagsTableName());
  outputFileTypes.push_back(ApiDb::getRelationsTableName());
  outputFileTypes.push_back(ApiDb::getRelationMembersTableName());
  outputFileTypes.push_back(ApiDb::getRelationTagsTableName());

  for (int i = 0 ; i < outputFileTypes.size(); i++)
  {
    QFileInfo baseFileInfo(outputUrl);
    shared_ptr<QFile> outputFile(
      new QFile(baseFileInfo.path() + "/" + baseFileInfo.baseName() + "-" +
                outputFileTypes.at(i) + "." + baseFileInfo.completeSuffix()));
    LOG_VARD(outputFile->fileName());
    if (outputFile->exists())
    {
      outputFile->remove();
    }
    outputFile->open(QIODevice::Append);
    _outputFiles.insert(outputFileTypes.at(i), outputFile);

    shared_ptr<QTextStream> outStream(new QTextStream(outputFile.get()));
    _outputStreams.insert(outputFileTypes.at(i), outStream);

    _numRecordsWritten[outputFileTypes.at(i)] = 0;
  }
}

void OsmApiDbCsvTableFileSetWriter::close()
{
  for (QMap<QString, shared_ptr<QFile> >::iterator outputFileItr = _outputFiles.begin();
       outputFileItr != _outputFiles.end(); outputFileItr++)
  {
    bool closed = false;
    if (outputFileItr.value().get())
    {
      outputFileItr.value()->close();
      closed = true;
      QFileInfo info(*_outputFiles[outputFileItr.key()]);
      LOG_VART(SystemInfo::humanReadable(info.size()));
    }
    if (!closed)
    {
      LOG_WARN("Unable to close file for table: " << outputFileItr.key());
    }
  }
}

void OsmApiDbCsvTableFileSetWriter::finalizePartial()
{
  for (QMap<QString, shared_ptr<QTextStream> >::iterator streamItr = _outputStreams.begin();
       streamItr != _outputStreams.end(); streamItr++)
  {
    if (streamItr.value().get())
    {
      streamItr.value()->flush();
    }
    else
    {
      throw HootException("Unable to flush output stream for table: " + streamItr.key());
    }
  }

  for (QMap<QString, long>::const_iterator countItr = _numRecordsWritten.begin();
       countItr != _numRecordsWritten.end(); countItr++)
  {
    LOG_INFO(*countItr << " total records written for " << countItr.key());
  }
}

//TODO: add extra unresolved refs logic that OsmApiDbBulkWriter has

void OsmApiDbCsvTableFileSetWriter::writePartial(const ConstNodePtr& node)
{
  shared_ptr<QTextStream> currentNodesOutStream =
    _outputStreams[ApiDb::getCurrentNodesTableName()];
  *currentNodesOutStream << QString::number(node->getId()) << ",";
  *currentNodesOutStream << QString::number(OsmApiDb::toOsmApiDbCoord(node->getY())) << ",";
  *currentNodesOutStream << QString::number(OsmApiDb::toOsmApiDbCoord(node->getX())) << ",";
  //TODO: fix
  *currentNodesOutStream << QString::number(1) << ",";
  *currentNodesOutStream << "t" << ",";
  *currentNodesOutStream <<
    QDateTime::currentDateTime().toUTC().toString("yyyy-MM-dd hh:mm:ss.zzz") << ",";
  *currentNodesOutStream << QString::number(ApiDb::tileForPoint(node->getY(), node->getX())) <<
    ",";
  *currentNodesOutStream << QString::number(1) << "\n";
  _unflushedRecords++;
  _numRecordsWritten[ApiDb::getCurrentNodesTableName()]++;

  shared_ptr<QTextStream> historicalNodesOutStream = _outputStreams[ApiDb::getNodesTableName()];
  *historicalNodesOutStream << QString::number(node->getId()) << ",";
  *historicalNodesOutStream << QString::number(OsmApiDb::toOsmApiDbCoord(node->getY())) << ",";
  *historicalNodesOutStream << QString::number(OsmApiDb::toOsmApiDbCoord(node->getX())) << ",";
  //TODO: fix
  *historicalNodesOutStream << QString::number(1) << ",";
  *historicalNodesOutStream << "t" << ",";
  *historicalNodesOutStream <<
    QDateTime::currentDateTime().toUTC().toString("yyyy-MM-dd hh:mm:ss.zzz") << ",";
  *historicalNodesOutStream << QString::number(ApiDb::tileForPoint(node->getY(), node->getX())) <<
    ",";
  *historicalNodesOutStream << QString::number(1) << ",";
  *historicalNodesOutStream << "\\N\n";
  _unflushedRecords++;
  _numRecordsWritten[ApiDb::getNodesTableName()]++;

  _writeTags(node);

  if (_unflushedRecords % _fileOutputLineBufferSize == 0)
  {
    currentNodesOutStream->flush();
    historicalNodesOutStream->flush();
    _outputStreams[
      OsmApiDb::elementTypeToElementTableName(ElementType::Node, false, true)]->flush();
    _outputStreams[
      OsmApiDb::elementTypeToElementTableName(ElementType::Node, true, true)]->flush();
    _unflushedRecords = 0;
  }

  if (_numRecordsWritten[ApiDb::getCurrentNodesTableName()] % _statusUpdateInterval == 0)
  {
    PROGRESS_INFO(_numRecordsWritten[ApiDb::getCurrentNodesTableName()] << " nodes written.");
  }
}

void OsmApiDbCsvTableFileSetWriter::writePartial(const ConstWayPtr& way)
{
  shared_ptr<QTextStream> currentWaysOutStream =
    _outputStreams[ApiDb::getCurrentWaysTableName()];
  *currentWaysOutStream << QString::number(way->getId()) << ",";
  //TODO: fix
  *currentWaysOutStream << QString::number(1) << ",";
  *currentWaysOutStream <<
    QDateTime::currentDateTime().toUTC().toString("yyyy-MM-dd hh:mm:ss.zzz") << ",";
  *currentWaysOutStream << "t" << ",";
  *currentWaysOutStream << QString::number(1) << "\n";
  _unflushedRecords++;
  _numRecordsWritten[ApiDb::getCurrentWaysTableName()]++;

  shared_ptr<QTextStream> historicalWaysOutStream =
    _outputStreams[ApiDb::getWaysTableName()];
  *historicalWaysOutStream << QString::number(way->getId()) << ",";
  //TODO: fix
  *historicalWaysOutStream << QString::number(1) << ",";
  *historicalWaysOutStream <<
    QDateTime::currentDateTime().toUTC().toString("yyyy-MM-dd hh:mm:ss.zzz") << ",";
  *historicalWaysOutStream << QString::number(1) << "\n";
  *historicalWaysOutStream << "t" << ",";
  *historicalWaysOutStream << "\\N\n";
  _unflushedRecords++;
  _numRecordsWritten[ApiDb::getWaysTableName()]++;

  shared_ptr<QTextStream> currentWayNodesOutStream =
    _outputStreams[ApiDb::getCurrentWayNodesTableName()];
  shared_ptr<QTextStream> historicalWayNodesOutStream =
    _outputStreams[ApiDb::getWayNodesTableName()];
  long nodeCtr = 1;
  for (vector<long>::const_iterator wayNodeItr = way->getNodeIds().begin();
       wayNodeItr != way->getNodeIds().end(); wayNodeItr++)
  {
    *currentWayNodesOutStream << QString::number(way->getId()) << ",";
    *currentWayNodesOutStream << QString::number(*wayNodeItr) << ",";
    *currentWayNodesOutStream << QString::number(nodeCtr) << "\n";

    *historicalWayNodesOutStream << QString::number(way->getId()) << ",";
    *historicalWayNodesOutStream << QString::number(way->getVersion()) << ",";
    *historicalWayNodesOutStream << QString::number(*wayNodeItr) << ",";
    *historicalWayNodesOutStream << QString::number(nodeCtr) << "\n";

    nodeCtr++;
  }

  _writeTags(way);

  if (_unflushedRecords % _fileOutputLineBufferSize == 0)
  {
    currentWaysOutStream->flush();
    currentWayNodesOutStream->flush();
    historicalWaysOutStream->flush();
    historicalWayNodesOutStream->flush();
    _outputStreams[
      OsmApiDb::elementTypeToElementTableName(ElementType::Way, false, true)]->flush();
    _outputStreams[
      OsmApiDb::elementTypeToElementTableName(ElementType::Way, true, true)]->flush();
    _unflushedRecords = 0;
  }

  if (_numRecordsWritten[ApiDb::getCurrentWaysTableName()] % _statusUpdateInterval == 0)
  {
    PROGRESS_INFO(_numRecordsWritten[ApiDb::getCurrentWaysTableName()] << " ways written.");
  }
}

void OsmApiDbCsvTableFileSetWriter::writePartial(const ConstRelationPtr& relation)
{
  shared_ptr<QTextStream> currentRelationsOutStream =
  _outputStreams[ApiDb::getCurrentRelationsTableName()];
  *currentRelationsOutStream << QString::number(relation->getId()) << ",";
  //TODO: fix
  *currentRelationsOutStream << QString::number(1) << ",";
  *currentRelationsOutStream <<
    QDateTime::currentDateTime().toUTC().toString("yyyy-MM-dd hh:mm:ss.zzz") << ",";
  *currentRelationsOutStream << "t" << ",";
  *currentRelationsOutStream << QString::number(1) << "\n";
  _unflushedRecords++;
  _numRecordsWritten[ApiDb::getCurrentRelationsTableName()]++;

  shared_ptr<QTextStream> historicalRelationsOutStream =
    _outputStreams[ApiDb::getRelationsTableName()];
  *historicalRelationsOutStream << QString::number(relation->getId()) << ",";
  //TODO: fix
  *historicalRelationsOutStream << QString::number(1) << ",";
  *historicalRelationsOutStream <<
    QDateTime::currentDateTime().toUTC().toString("yyyy-MM-dd hh:mm:ss.zzz") << ",";
  *historicalRelationsOutStream << QString::number(1) << "\n";
  *historicalRelationsOutStream << "t" << ",";
  *historicalRelationsOutStream << "\\N\n";
  _unflushedRecords++;
  _numRecordsWritten[ApiDb::getRelationsTableName()]++;

  shared_ptr<QTextStream> currentRelationMembersOutStream =
    _outputStreams[ApiDb::getCurrentRelationMembersTableName()];
  shared_ptr<QTextStream> historicalRelationMembersOutStream =
    _outputStreams[ApiDb::getRelationMembersTableName()];
  const vector<RelationData::Entry> relationMembers = relation->getMembers();
  for (size_t i = 0; i < relationMembers.size(); i++)
  {
    const RelationData::Entry member = relationMembers[i];

    *currentRelationMembersOutStream << QString::number(relation->getId()) << ",";
    *currentRelationMembersOutStream << member.getElementId().getType().toString() << ",";
    *currentRelationMembersOutStream << QString::number(member.getElementId().getId()) << ",";
    *currentRelationMembersOutStream << member.getRole() << ",";
    *currentRelationMembersOutStream << QString::number(i + 1) << "\n";

    *currentRelationMembersOutStream << QString::number(relation->getId()) << ",";
    *currentRelationMembersOutStream << member.getElementId().getType().toString() << ",";
    *currentRelationMembersOutStream << QString::number(member.getElementId().getId()) << ",";
    *currentRelationMembersOutStream << member.getRole() << ",";
    *currentRelationMembersOutStream << QString::number(relation->getVersion()) << ",";
    *currentRelationMembersOutStream << QString::number(i + 1) << "\n";
  }

  _writeTags(relation);

  if (_unflushedRecords % _fileOutputLineBufferSize == 0)
  {
    currentRelationsOutStream->flush();
    currentRelationMembersOutStream->flush();
    historicalRelationsOutStream->flush();
    historicalRelationMembersOutStream->flush();
    _outputStreams[
      OsmApiDb::elementTypeToElementTableName(ElementType::Relation, false, true)]->flush();
    _outputStreams[
      OsmApiDb::elementTypeToElementTableName(ElementType::Relation, true, true)]->flush();
    _unflushedRecords = 0;
  }

  if (_numRecordsWritten[ApiDb::getCurrentRelationsTableName()] % _statusUpdateInterval == 0)
  {
    PROGRESS_INFO(
      _numRecordsWritten[ApiDb::getCurrentRelationsTableName()] << " relations written.");
  }
}

void OsmApiDbCsvTableFileSetWriter::_writeTags(ConstElementPtr element)
{
  const QString currentTagsTableName =
    OsmApiDb::elementTypeToElementTableName(element->getElementType(), false, true);
  const QString historicalTagsTableName =
    OsmApiDb::elementTypeToElementTableName(element->getElementType(), true, true);
  shared_ptr<QTextStream> currentTagsOutStream = _outputStreams[currentTagsTableName];
  shared_ptr<QTextStream> historicalTagsOutStream = _outputStreams[historicalTagsTableName];
  for (Tags::const_iterator tagItr = element->getTags().begin(); tagItr != element->getTags().end();
       tagItr++)
  {
    *currentTagsOutStream << QString::number(element->getId()) << ",";
    *currentTagsOutStream << /*_escapeCopyToData(*/tagItr.key()/*)*/ << ",";
    *currentTagsOutStream << /*_escapeCopyToData(*/tagItr.value()/*)*/ << "\n";
    _unflushedRecords++;
    _numRecordsWritten[currentTagsTableName]++;

    *historicalTagsOutStream << QString::number(element->getId()) << ",";
    *historicalTagsOutStream << QString::number(element->getVersion()) << ",";
    *historicalTagsOutStream << /*_escapeCopyToData(*/tagItr.key()/*)*/ << ",";
    *historicalTagsOutStream << /*_escapeCopyToData(*/tagItr.value()/*)*/ << "\n";
    _unflushedRecords++;
    _numRecordsWritten[historicalTagsTableName]++;
  }
}

void OsmApiDbCsvTableFileSetWriter::setConfiguration(const hoot::Settings& conf)
{
  const ConfigOptions confOptions(conf);
  setFileOutputLineBufferSize(confOptions.getOsmapidbBulkWriterFileOutputBufferMaxLineSize());
  setStatusUpdateInterval(confOptions.getOsmapidbBulkWriterFileOutputStatusUpdateInterval());
}

}
