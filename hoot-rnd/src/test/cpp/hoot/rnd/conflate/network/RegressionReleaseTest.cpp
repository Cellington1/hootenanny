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
 * @copyright Copyright (C) 2015 DigitalGlobe (http://www.digitalglobe.com/)
 */
#include "RegressionReleaseTest.h"

// hoot
#include <hoot/core/util/ConfigOptions.h>
#include <hoot/core/util/Log.h>
#include <hoot/core/TestUtils.h>
#include <hoot/core/test/TestSetup.h>

// Qt
#include <QTextStream>

namespace hoot
{

RegressionReleaseTest::RegressionReleaseTest(QDir d, QStringList confs) :
AbstractTest(d, confs),
_minPassingScore(-1.0)
{
}

void RegressionReleaseTest::runTest()
{
  //TestUtils::resetEnvironment();
  LOG_DEBUG("Running regression release test...");

  // configures and cleans up the conf() environment
  //LOG_VART(_confs);
  //TestSetup st(_confs);

  QFileInfo makeFile(_d, "Makefile");
  if (!makeFile.exists())
  {
    throw IllegalArgumentException(
      "Unable to find Makefile for regression release test: " + _d.absolutePath());
  }

  const QString startingDir = QDir::currentPath();
  if (!QDir::setCurrent(_d.absolutePath()))
  {
    throw IllegalArgumentException("Unable to change to test directory: " + _d.absolutePath());
  }

  const QString cmd = "make test";
  const int retval = system(cmd.toStdString().c_str());
  if (retval != 0)
  {
    CPPUNIT_ASSERT_MESSAGE(
      QString("Failed executing regression release test.  Status: " +
      QString::number(retval)).toStdString(),
      false);
  }

  //check test score and pass if >= _minPassingScore; fail otherwise
  QDir scoresDir("scores");
  QStringList nameFilters;
  nameFilters.append("*scores.txt");
  const QStringList scoresDirContents = scoresDir.entryList(nameFilters, QDir::Files);
  if (scoresDirContents.size() != 1)
  {
    throw HootException(
      "Found " + QString::number(scoresDirContents.size()) + " score files and expected to " +
      "find one scores file.");
  }
  LOG_VARD(scoresDirContents[0]);
  QFile scoresFile(scoresDirContents[0]);
  if (!scoresFile.open(QIODevice::ReadOnly))
  {
    throw HootException("Unable to open scores file: " + scoresDirContents[0]);
  }
  QTextStream inStream(&scoresFile);
  QString line;
  bool foundConflatedScoreLine = false;
  unsigned int overallScore = -1;
  do
  {
    line = inStream.readLine();
    LOG_VART(line);
    if (line.toLower().contains("conflated"))
    {
      foundConflatedScoreLine = true;
    }
    else if (foundConflatedScoreLine && line.toLower().startsWith("overall"))
    {
      overallScore = line.split("\s")[1].toInt();
      LOG_VARD(overallScore);
    }
  }
  while (!line.isNull() && overallScore == -1);

  LOG_VARD(_minPassingScore);
  if (overallScore >= _minPassingScore)
  {
    _minPassingScore = overallScore;
  }
  else
  {
    CPPUNIT_ASSERT_MESSAGE(
      QString("Failed executing regression release test: " +
        QString::fromStdString(getName())).toStdString(),
      false);
  }
  LOG_VARD(_minPassingScore);

  if (!QDir::setCurrent(startingDir))
  {
    throw HootException("Unable to change back to hoot tests directory: " + startingDir);
  }
}

}
