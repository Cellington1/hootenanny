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
#include <hoot/core/TestUtils.h>
#include <hoot/rnd/io/ImplicitTagRulesJsonWriter.h>

// Qt
#include <QDir>

namespace hoot
{

class ImplicitTagRulesJsonWriterTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(ImplicitTagRulesJsonWriterTest);
  CPPUNIT_TEST(runWriteTest);
  CPPUNIT_TEST_SUITE_END();

public:

  void runWriteTest()
  {
    const QString outputDir = "test-output/io/ImplicitTagRulesJsonWriterTest";
    const QString outputFile = outputDir + "/rules-out.json";
    QDir().mkpath(outputDir);

    QMap<QString, QMap<QString, long> > rules;
    QMap<QString, long> kvps;

    kvps["amenity=place_of_worship"] = 1246;
    kvps["amenity=school"] = 2;
    kvps["amenity=fountain"] = 1;
    kvps["amenity=hospital"] = 1;
    kvps["amenity=religious_activities"] = 6;
    rules["Mosque"] = kvps;

    kvps.clear();
    kvps["amenity=place_of_worship"] = 2672;
    kvps["amenity=religious_activities"] = 9;
    rules["Masjid"] = kvps;

    kvps.clear();
    kvps["amenity=grave_yard"] = 21;
    rules["Sidi Muhammad"] = kvps;

    kvps.clear();
    kvps["amenity=place_of_worship"] = 18;
    rules["Eid Prayer Ground"] = kvps;

    ImplicitTagRulesJsonWriter writer;
    writer.open(outputFile);
    writer.write(rules);
    writer.close();

    HOOT_FILE_EQUALS("test-files/io/ImplicitTagRulesJsonWriterTest/rules.json", outputFile);
  }
};

CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(ImplicitTagRulesJsonWriterTest, "quick");

}
