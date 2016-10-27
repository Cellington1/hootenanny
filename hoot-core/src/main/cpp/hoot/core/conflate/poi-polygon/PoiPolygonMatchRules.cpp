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
#include "PoiPolygonMatchRules.h"

// geos
#include <geos/geom/LineString.h>
#include <geos/util/TopologyException.h>

// hoot
#include <hoot/core/conflate/MatchClassification.h>
#include <hoot/core/schema/OsmSchema.h>
#include <hoot/core/util/ElementConverter.h>

#include "PoiPolygonTypeMatch.h"
#include "PoiPolygonNameMatch.h"
#include "PoiPolygonAddressMatch.h"


namespace hoot
{

PoiPolygonMatchRules::PoiPolygonMatchRules(const ConstOsmMapPtr& map,
                                                     const set<ElementId>& polyNeighborIds,
                                                     const set<ElementId>& poiNeighborIds,
                                                     double distance,
                                                     shared_ptr<Geometry> polyGeom,
                                                     shared_ptr<Geometry> poiGeom,
                                                     const QString testUuid = "") :
_map(map),
_polyNeighborIds(polyNeighborIds),
_poiNeighborIds(poiNeighborIds),
_distance(distance),
_polyGeom(polyGeom),
_poiGeom(poiGeom),
_badGeomCount(0),
_isRecCenterMatch(false),
_poiNeighborWithAddressContainedInPoly(false),
_testUuid(testUuid)
{
}

void PoiPolygonMatchRules::collectInfo(ConstElementPtr poi, ConstElementPtr poly)
{
  if (!_poiGeom.get())
  {
    //TODO: change back to warn?
    /*LOG_WARN*/LOG_DEBUG("Invalid poi geometry.");
    return;
  }
  if (!_polyGeom.get())
  {
    //TODO: change back to warn?
    /*LOG_WARN*/LOG_DEBUG("Invalid poly geometry.");
    return;
  }
  const bool testFeatureFound =
    poly->getTags().get("uuid") == _testUuid || poi->getTags().get("uuid") == _testUuid;

  //The rules below are roughly grouped by processing expense (more granular ordering can still be
  //done to further reduce runtime), with the rules requiring the least expensive computations
  //occurring earlier.

  const bool poiHasType = PoiPolygonTypeMatch::hasType(poi);
  const bool poiIsRecCenter = PoiPolygonTypeMatch::isRecCenter(poi);

  const bool polyHasName = PoiPolygonNameMatch::elementHasName(poly);
  const bool polyIsPark = PoiPolygonTypeMatch::isPark(poly);
  const bool polyHasType = PoiPolygonTypeMatch::hasType(poly);
  const QString polyAddress =
    poly->getTags().get(PoiPolygonAddressMatch::FULL_ADDRESS_TAG_NAME).toLower().trimmed();
  const bool polyIsBuildingIsh = PoiPolygonTypeMatch::isBuildingIsh(poly);
  const bool polyHasMoreThanOneType = PoiPolygonTypeMatch::hasMoreThanOneType(poly);
  bool polyHasSpecificType = polyHasType;
  if ((poly->getTags().get("building") == "yes" || poly->getTags().get("poi") == "yes") &&
      !polyHasMoreThanOneType)
  {
    polyHasSpecificType = false;
  }

  if (testFeatureFound)
  {
    LOG_VARD(poiHasType);
    LOG_VARD(poiIsRecCenter);

    LOG_VARD(polyHasName);
    LOG_VARD(polyIsPark);
    LOG_VARD(polyHasType);
  }

  bool poiContainedInAnotherParkPoly = false;
  bool polyContainedInAnotherParkPoly = false;

  //PoiPolygonTypeMatch typeScorer(_typeScoreThreshold, _testUuid);
  //PoiPolygonNameMatch nameScorer(_nameScoreThreshold);

  if (testFeatureFound)
  {
    LOG_VARD(polyIsBuildingIsh);
    LOG_VARD(polyHasMoreThanOneType);
    LOG_VARD(polyHasSpecificType);
  }

  if (!poiHasType && poiIsRecCenter && polyIsBuildingIsh && (!polyHasSpecificType || !polyHasName))
  {
    set<ElementId>::const_iterator polyNeighborItr = _polyNeighborIds.begin();
    while (polyNeighborItr != _polyNeighborIds.end())
    {
      ConstElementPtr polyNeighbor = _map->getElement(*polyNeighborItr);
      if (polyNeighbor->getElementId() != poly->getElementId())
      {
        shared_ptr<Geometry> polyNeighborGeom;
        try
        {
          //TODO: temp suppress "unable to connect all ways..." message here?
          polyNeighborGeom = ElementConverter(_map).convertToGeometry(polyNeighbor);

          if (polyNeighborGeom.get() &&
              QString::fromStdString(polyNeighborGeom->toString()).toUpper().contains("EMPTY"))
          {
            if (_badGeomCount <= ConfigOptions().getOgrLogLimit())
            {
              //TODO: change back
              /*LOG_WARN*/LOG_DEBUG(
                "Invalid area neighbor polygon passed to PoiPolygonMatchCreator: " <<
                polyNeighborGeom->toString());
              _badGeomCount++;
            }
          }
          else if (polyNeighborGeom.get())
          {
            /*if (testFeatureFound)
            {
              LOG_VARD(area->getTags().get("uuid"))
              LOG_VARD(_isPlayground(area));
              LOG_VARD(_isPlayArea(area));
              LOG_VARD(_polyGeom->contains(areaGeom.get()));
              LOG_VARD(_isSport(area));
              LOG_VARD(poiIsSport);
              LOG_VARD(areaGeom->contains(_poiGeom.get()));
              LOG_VARD(typeScorer.isExactTypeMatch(poi, area));
              QString t1, t2;
              LOG_VARD(typeScorer.getTypeScore(poi, area, t1, t2));
              LOG_VARD(t1);
              LOG_VARD(t2);
            }*/

            if (PoiPolygonTypeMatch::isPark(polyNeighbor))
            {
              if (polyNeighborGeom->contains(_poiGeom.get()))
              {
                poiContainedInAnotherParkPoly = true;

                if (testFeatureFound)
                {
                  LOG_DEBUG(
                    "poi examined and found to be contained within a park poly outside of this match " <<
                    "comparison: " << poi->toString());
                  LOG_DEBUG("park poly it is very close to: " << polyNeighbor->toString());
                }
              }

              if (polyNeighborGeom->contains(_polyGeom.get()))
              {
                //TODO: probably need to be specific that the poi and the poly are in the same park...
                //TODO: log
                polyContainedInAnotherParkPoly = true;
              }
            }
          }
        }
        catch (const geos::util::TopologyException& e)
        {
          if (_badGeomCount <= ConfigOptions().getOgrLogLimit())
          {
            //TODO: change back
            /*LOG_WARN*/LOG_DEBUG(
              "Feature passed to PoiPolygonMatchCreator caused topology exception on conversion to a " <<
              "geometry: " << polyNeighbor->toString() << "\n" << e.what());
            _badGeomCount++;
          }
        }
      }
      polyNeighborItr++;
    }
  }

  const bool poiContainedInParkPoly =
    poiContainedInAnotherParkPoly || (polyIsPark && _distance == 0);

  if (testFeatureFound)
  {
    LOG_VARD(poiContainedInAnotherParkPoly);
    LOG_VARD(poiContainedInParkPoly);
  }

  //TODO:
  if (poiContainedInParkPoly && !poiHasType && poiIsRecCenter && polyIsBuildingIsh &&
      (!polyHasSpecificType || !polyHasName) && polyContainedInAnotherParkPoly)
  {
    if (testFeatureFound)
    {
      LOG_DEBUG("Returning review per rule #23a...");
    }
    _isRecCenterMatch = true;
  }

  if (polyAddress.isEmpty())
  {
    return;
  }

  set<ElementId>::const_iterator poiNeighborItr = _poiNeighborIds.begin();
  while (poiNeighborItr != _poiNeighborIds.end())
  {
    ConstElementPtr poiNeighbor = _map->getElement(*poiNeighborItr);
    if (poiNeighbor->getElementId() != poi->getElementId())
    {
      shared_ptr<Geometry> poiNeighborGeom ;
      try
      {
        poiNeighborGeom = ElementConverter(_map).convertToGeometry(poiNeighbor);

        /*if (Log::getInstance().getLevel() == Log::Debug &&
            (poi->getTags().get("uuid") == _testUuid ||
             poly->getTags().get("uuid") == _testUuid))
        {
          LOG_VARD(poiNeighbor->getTags().get("uuid"))
          LOG_VARD(_elementIsPark(poiNeighbor));
          LOG_VARD(_elementIsPlayground(poiNeighbor));
          LOG_VARD(_elementIsPlayArea(poiNeighbor))
          LOG_VARD(polyGeom->contains(poiNeighborGeom.get()));
        }*/

        //TODO: make work for all name tags
        const QString poiNeighborName = poiNeighbor->getTags().get("name").toLower().trimmed();
        if (!poiNeighborName.isEmpty() && poiNeighborName == polyAddress)
        {
          _poiNeighborWithAddressContainedInPoly = true;
        }
      }
      catch (const geos::util::TopologyException& e)
      {
        if (_badGeomCount <= ConfigOptions().getOgrLogLimit())
        {
          //TODO: change back
          /*LOG_WARN*/LOG_DEBUG(
            "Feature passed to PoiPolygonMatchCreator caused topology exception on conversion to a " <<
            "geometry: " << poiNeighbor->toString() << "\n" << e.what());
          _badGeomCount++;
        }
      }
    }
    poiNeighborItr++;
  }
}

}
