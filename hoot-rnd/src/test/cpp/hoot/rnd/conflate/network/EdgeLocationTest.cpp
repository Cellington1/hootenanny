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

// Hoot
#include <hoot/core/elements/Node.h>
#include <hoot/rnd/conflate/network/EdgeLocation.h>
#include <hoot/rnd/conflate/network/NetworkVertex.h>
#include <hoot/rnd/conflate/network/NetworkEdge.h>
#include <hoot/core/TestUtils.h>

namespace hoot
{

class EdgeLocationTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(EdgeLocationTest);
  CPPUNIT_TEST(runBasicTest);
  //CPPUNIT_TEST(runOperatorTests);
  CPPUNIT_TEST_SUITE_END();

public:

  void runBasicTest()
  {
    shared_ptr<OsmMap> map(new OsmMap());
    NodePtr node1 = TestUtils::createNode(map, Status::Unknown1, 0, 0);
    NetworkVertexPtr vertex1(new NetworkVertex(node1));
    NodePtr node2 = TestUtils::createNode(map, Status::Unknown1, 100, 0);
    NetworkVertexPtr vertex2(new NetworkVertex(node2));
    NetworkEdgePtr edge(new NetworkEdge(vertex1, vertex2, true));

    EdgeLocation edgeLoc(edge, 0.5);
    ConstNetworkEdgePtr outputEdge = edgeLoc.getEdge();

    CPPUNIT_ASSERT(outputEdge->getFrom() == vertex1);
    CPPUNIT_ASSERT(outputEdge->getTo() == vertex2);
    CPPUNIT_ASSERT(outputEdge->isDirected());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.5, edgeLoc.getPortion(), 0.0);

    CPPUNIT_ASSERT(edgeLoc.isExtreme(0.9));
    CPPUNIT_ASSERT(!edgeLoc.isExtreme(0.1));
    CPPUNIT_ASSERT(edgeLoc.isFirst(0.9));
    CPPUNIT_ASSERT(!edgeLoc.isFirst(0.1));
    CPPUNIT_ASSERT(!edgeLoc.isLast(0.1));
    CPPUNIT_ASSERT(edgeLoc.isLast(0.9));
    CPPUNIT_ASSERT(edgeLoc.isValid());

    CPPUNIT_ASSERT(edgeLoc.getVertex(0.9) == vertex2);
    QString exceptionMsg("");
    try
    {
      edgeLoc.getVertex(0.1);
    }
    catch (HootException e)
    {
      exceptionMsg = e.what();
    }
    HOOT_STR_EQUALS(
      "Attempted to get a vertex on an edge location that isn't on a vertex.",
      exceptionMsg.toStdString());
  }

  void runOperatorTests()
  {

  }

};

CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(EdgeLocationTest, "quick");

}
