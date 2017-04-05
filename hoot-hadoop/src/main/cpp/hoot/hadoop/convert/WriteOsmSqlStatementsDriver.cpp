/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

// Hoot
#include <hoot/core/util/ConfPath.h>
#include <hoot/hadoop/pbf/PbfInputFormat.h>
#include <hoot/hadoop/pbf/PbfRecordReader.h>
#include <hoot/core/io/ApiDb.h>
#include <hoot/core/util/UuidHelper.h>
#include <hoot/core/io/OsmApiDbSqlStatementFormatter.h>

// Pretty Pipes
#include <pp/mapreduce/Job.h>
#include <pp/Hdfs.h>
#include <pp/HadoopPipesUtils.h>
using namespace pp;

// Qt
#include <QFileInfo>
#include <QTextStream>

#include <geos/geom/Envelope.h>
using namespace geos::geom;

#include "WriteOsmSqlStatementsMapper.h"
#include "WriteOsmSqlStatementsReducer.h"
#include "WriteOsmSqlStatementsDriver.h"
#include "SqlStatementLineRecordWriter.h"

namespace hoot
{

WriteOsmSqlStatementsDriver::WriteOsmSqlStatementsDriver() :
_outputFileCopyLocation(""),
_outputDelimiter("\t")
{
  _sqlFormatter.reset(new OsmApiDbSqlStatementFormatter(_outputDelimiter));
  setConfiguration(conf());
}

WriteOsmSqlStatementsDriver::~WriteOsmSqlStatementsDriver()
{

}

//void WriteOsmSqlStatementsDriver::_reset()
//{
//  _output = "";
//  _changesetUserId = -1;
//  _outputFileCopyLocation = "";
//}

bool WriteOsmSqlStatementsDriver::_destinationIsDatabase(const QString output) const
{
  return output.toLower().startsWith("osmapidb://");
}

bool WriteOsmSqlStatementsDriver::isSupported(QString urlStr)
{
  LOG_VARD(urlStr);
  QUrl url(urlStr);
  //if we ever want any other writers that the convert command invokes to output sql, then
  //this will have to be made more specific
  return urlStr.toLower().endsWith(".sql") || _database.isSupported(url);
}

void WriteOsmSqlStatementsDriver::open(QString url)
{
  _output = url;

  // Make sure we're not already open and the URL is valid
  if (!isSupported(url))
  {
    throw HootException(QString("Could not open URL ") + url);
  }
}

void WriteOsmSqlStatementsDriver::close()
{
  LOG_DEBUG("Closing writer...");

  if (_destinationIsDatabase(_output))
  {
    _database.close();
  }

  //_reset();
  //setConfiguration(conf());
}

void WriteOsmSqlStatementsDriver::write(const QString inputMapFile)
{
  if (_output.isEmpty())
  {
    throw HootException("Writer has not been opened.");
  }

  Hdfs fs;
  LOG_DEBUG("Creating work directory...");
  QString hdfsOutput =
    "tmp/" + UuidHelper::createUuid().toString().replace("{", "").replace("}", "") +
    "-WriteOsmSqlStatementsDriver/";
  fs.mkdirs(hdfsOutput.toStdString());
  const QString sqlFile = _getSqlFileOutputLocation(hdfsOutput, _output);

  _runElementSqlStatementsWriteJob(
    fs.getAbsolutePath(inputMapFile.toStdString()), hdfsOutput.toStdString());

  //merge all the output files into one and copy back to the local file system; how expensive
  //is this going to be for a ton of big files?
  LOG_INFO("Merging temporary output files into one SQL file...");
  HadoopPipesUtils::mergeFilesToLocalFileSystem(hdfsOutput.toStdString(), sqlFile.toStdString());

  //append the single changeset entry to the file
  _writeChangesetToSqlFile(sqlFile);

  if (_destinationIsDatabase(_output))
  {
    //TODO: add file name and record count to this message
    LOG_INFO("Executing element SQL...");
    OsmApiDb database;
    try
    {
      database.open(_output);
      //We have to turn off constraints before writing the sql file to the db, since the table
      //copy commands are out of dependency order and will violate ref integrity.
      database.disableConstraints();

      //write the sql file
      ApiDb::execSqlFile(_output, sqlFile);

      //now re-enable the constraints to make sure the db is valid before reading from it
      database.enableConstraints();
      database.close();
    }
    catch (const HootException& e)
    {
      database.enableConstraints();
      database.close();
      throw e;
    }
  }
}

void WriteOsmSqlStatementsDriver::_writeChangesetToSqlFile(const QString sqlFileLocation)
{
  LOG_DEBUG("Writing changeset to output SQL file...");
  const QString changesetHeader = _sqlFormatter->getChangesetSqlHeaderString();
  Envelope bounds;
  bounds.init();
  //TODO: fix num changes?...how?
  const QString changesetStr = _sqlFormatter->changesetToSqlString(1, _changesetUserId, 1, bounds);
  QFile outputSqlFile(sqlFileLocation);
  if (!outputSqlFile.open(QIODevice::Append))
  {
    throw HootException("Could not open SQL output file: " + sqlFileLocation);
  }
  QTextStream outStream(&outputSqlFile);
  outStream << changesetHeader;
  outStream << changesetStr;
  outStream.flush();
  outputSqlFile.close();
}

QString WriteOsmSqlStatementsDriver::_getSqlFileOutputLocation(const QString hdfsOutput,
                                                               const QString userSpecifiedOutput) const
{
  if (!_destinationIsDatabase(userSpecifiedOutput))
  {
    return userSpecifiedOutput;
  }
  else if (!_outputFileCopyLocation.isEmpty())
  {
    return _outputFileCopyLocation;
  }
  return hdfsOutput + "/output.sql";
}

void WriteOsmSqlStatementsDriver::_runElementSqlStatementsWriteJob(const string& input,
                                                                   const string& output)
{
  LOG_INFO("Running element SQL statements write job...");

  Job job;
  job.setVerbose(Log::getInstance().getLevel() <= Log::Debug);
  job.setName("WriteElementSqlStatements");

  job.setInput(input);
  job.setOutput(output);

  job.setMapperClass(WriteOsmSqlStatementsMapper::className());
  job.setReducerClass(WriteOsmSqlStatementsReducer::className());
  job.setInputFormatClass(PbfInputFormat::className());
  job.setRecordReaderClass(PbfRecordReader::className());
  job.setRecordWriterClass(SqlStatementLineRecordWriter::className());

  // Adds all libraries in this directory to the job.
  job.addLibraryDirs(ConfigOptions().getHootHadoopLibpath());
  job.addFile(ConfPath::search("hoot.json").toStdString());
  // This library will be used to provide mapper/reducer classes and anything else referenced
  // by the factory.
  job.addPlugin(getenv("HOOT_HOME") + string("/lib/libHootHadoop.so.1"));
  _addDefaultJobSettings(job);

  //job.getConfiguration().setInt("mapred.map.tasks", 8);

  job.getConfiguration().setLong("changesetUserId", _changesetUserId);
  job.getConfiguration().setLong("writeBufferSize", _fileOutputElementBufferSize);
  //job.getConfiguration().setInt("execSqlWithMapreduce", QString::number(_execSqlWithMapreduce));
  if (_numReduceTasks != -1)
  {
      job.getConfiguration().setInt("mapred.reduce.tasks", _numReduceTasks);
  }

  LOG_DEBUG(job.getJobTracker());
  job.run();
}

void WriteOsmSqlStatementsDriver::setConfiguration(const Settings& conf)
{
  const ConfigOptions confOptions(conf);

  setOutputFilesCopyLocation(confOptions.getOsmapidbBulkWriterOutputFilesCopyLocation().trimmed());
  setFileOutputElementBufferSize(confOptions.getOsmapidbBulkWriterFileOutputElementBufferSize());
  setChangesetUserId(confOptions.getChangesetUserId());
  //setExecSqlWithMapreduce(confOptions.getBigConvertExecSqlWithMapreduce());
  setNumReduceTasks(confOptions.getBigConvertNumReduceTasks());

  LOG_VART(_outputFileCopyLocation);
  LOG_VART(_fileOutputElementBufferSize);
  LOG_VART(_changesetUserId);
  //LOG_VART(_execSqlWithMapreduce);
  LOG_VART(_numReduceTasks);
}

}
