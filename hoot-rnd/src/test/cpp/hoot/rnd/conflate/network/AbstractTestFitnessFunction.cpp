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

#include "AbstractTestFitnessFunction.h"

// Hoot
#include <hoot/core/util/Settings.h>
#include <hoot/core/test/SimpleTestListener.h>
#include <hoot/core/test/TempFileName.h>

namespace hoot
{

AbstractTestFitnessFunction::AbstractTestFitnessFunction() :
_testCount(0),
_lowestNumFailingTestsPerRun(-1)
{
}

double AbstractTestFitnessFunction::f(const Tgs::ConstStatePtr& s)
{
  LOG_DEBUG("Running fitness function...");

  Settings settings;
  foreach (QString k, s->getAllValues().keys())
  {
    settings.set(k, s->get(k));
  }
  //if you need to add any other temporary custom settings for this test that wouldn't
  //normally be used with the network conflation case tests, add those here
  //settings.set("", "");
  LOG_VARD(settings);
  TempFileName temp;
  _settingsFileName = temp.getFileName();
  LOG_VARD(_settingsFileName);
  settings.storeJson(_settingsFileName);

  for (int i = 0; i < _testCount; ++i)
  {
    _test = dynamic_cast<AbstractTest*>(_testSuite->getChildTestAt(i));
    const QString testName = QString::fromStdString(_test->getName());
    initTest(_test);
    //add our custom sa test option values
    _test->addConfig(_settingsFileName);
    CppUnit::TestResult result;
    SimpleTestListener listener;
    result.addListener(&listener);
    LOG_DEBUG("Running " << testName << "...");
    _test->run(&result);
    afterTestRun(_test);

    if (listener.isFailure())
    {
      LOG_DEBUG("Failure: " << testName);
      _failedTests.append(testName);
    }
  }

  //TODO: if this doesn't end up being important to RegressionReleaseTestFitnessFunction, then
  //move it down to CaseTestFitnessFunction
  QString failedTestsStr;
  if (_failedTests.size() > 0)
  {
    failedTestsStr = _failedTestsToString(_failedTests);
  }
  if (_failedTests.size() < _lowestNumFailingTestsPerRun || _lowestNumFailingTestsPerRun == -1)
  {
    _lowestNumFailingTestsPerRun = _failedTests.size();
    _failingTestsForBestRuns.clear();
    if (!failedTestsStr.isEmpty())
    {
      _failingTestsForBestRuns.append(failedTestsStr);
    }
  }
  else if (_failedTests.size() == _lowestNumFailingTestsPerRun &&
           !failedTestsStr.isEmpty() && !_failingTestsForBestRuns.contains(failedTestsStr))
  {
    _failingTestsForBestRuns.append(failedTestsStr);
  }

  //we're letting the child classes actually determine the fitness value...the code in this method
  //is mainly intended to run the tests
  return -1.0;
}

QString AbstractTestFitnessFunction::_failedTestsToString(const QStringList failedTests) const
{
  QString concatTestNames;
  for (int i = 0; i < failedTests.size(); i++)
  {
    concatTestNames += failedTests.at(i) + ";";
  }
  concatTestNames.chop(1);
  return concatTestNames;
}

}
