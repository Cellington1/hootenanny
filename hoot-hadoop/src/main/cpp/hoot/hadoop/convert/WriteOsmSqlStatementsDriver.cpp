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
#include <hoot/core/util/Settings.h>
#include <hoot/core/util/ConfPath.h>
#include <hoot/hadoop/pbf/PbfInputFormat.h>
#include <hoot/hadoop/pbf/PbfRecordReader.h>

// Pretty Pipes
#include <pp/mapreduce/Job.h>
#include <pp/Hdfs.h>
#include <pp/io/LineRecordWriter.h>

// Qt
#include <QDir>
#include <QFileInfo>

#include "WriteOsmSqlStatementsMapper.h"
#include "WriteOsmSqlStatementsReducer.h"
#include "WriteOsmSqlStatementsDriver.h"

namespace hoot
{

WriteOsmSqlStatementsDriver::WriteOsmSqlStatementsDriver()
{
}

void WriteOsmSqlStatementsDriver::write(const QString input, const QString output)
{
  pp::Job sqlStatementWriteJob;
  sqlStatementWriteJob.setVerbose(Log::getInstance().getLevel() <= Log::Debug);
  sqlStatementWriteJob.setName("WriteOsmSqlStatementsDriver");
  // be nice and don't start the reduce tasks until most of the map tasks are done.
  //job.getConfiguration().setDouble("mapred.reduce.slowstart.completed.maps", 0.98);

  pp::Hdfs fs;
  sqlStatementWriteJob.setInput(fs.getAbsolutePath(input.toStdString()));
  //we'll ignore the output file name for now and dump all the output in separate files in the
  //output dir
  QFileInfo outputInfo(output);
  QString hdfsOutput = QString::fromStdString(fs.getAbsolutePath(output.toStdString()));
  hdfsOutput.replace(outputInfo.fileName(), "");
  //LOG_VARD(hdfsOutput.toStdString());
  sqlStatementWriteJob.setOutput(hdfsOutput.toStdString());

  sqlStatementWriteJob.setMapperClass(WriteOsmSqlStatementsMapper::className());
  sqlStatementWriteJob.setReducerClass(WriteOsmSqlStatementsReducer::className());
  sqlStatementWriteJob.setInputFormatClass(PbfInputFormat::className());
  sqlStatementWriteJob.setRecordReaderClass(PbfRecordReader::className());
  sqlStatementWriteJob.setRecordWriterClass(pp::LineRecordWriter::className());

  // Adds all libraries in this directory to the job.
  sqlStatementWriteJob.addLibraryDirs(ConfigOptions().getHootHadoopLibpath());
  sqlStatementWriteJob.addFile(ConfPath::search("hoot.json").toStdString());
  // This library will be used to provide mapper/reducer classes and anything else referenced
  // by the factory.
  sqlStatementWriteJob.addPlugin(getenv("HOOT_HOME") + string("/lib/libHootHadoop.so.1"));
  _addDefaultJobSettings(sqlStatementWriteJob);
  // conflation runs can go for a _long_ time. Setting timeout to 6 hours.
  //job.getConfiguration().setInt("mapred.task.timeout", 6 * 3600 * 1000);

  // run the job.
  sqlStatementWriteJob.run();

  //TODO: update changeset ids

  //merge all the output files into one and copy back to the local file system
  const QString cmd = "hadoop fs -getmerge " + hdfsOutput + " " + output;
  LOG_VARD(cmd);
  if (system(cmd.toStdString().c_str()) != 0)
  {
    throw HootException("Failed merging SQL statement output into a single SQL file: " + output);
  }

  //TODO: append the sequence id update statements to the end of the merged sql file

}

}
