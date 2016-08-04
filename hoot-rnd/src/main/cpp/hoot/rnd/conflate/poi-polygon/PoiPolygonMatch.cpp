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
#include "PoiPolygonMatch.h"

// geos
#include <geos/geom/Geometry.h>

// hoot
#include <hoot/core/algorithms/LevenshteinDistance.h>
#include <hoot/core/algorithms/MeanWordSetDistance.h>
#include <hoot/core/algorithms/string/MinSumWordSetDistance.h>
//#include <hoot/core/algorithms/string/WeightedWordDistance.h>
//#include <hoot/core/conflate/polygon/extractors/NameExtractor.h>
#include <hoot/rnd/conflate/poi-polygon/extractors/PoiPolygonNameExtractor.h>
//#include <hoot/core/algorithms/string/SqliteWordWeightDictionary.h>
#include <hoot/core/schema/OsmSchema.h>
#include <hoot/core/schema/TranslateStringDistance.h>
#include <hoot/core/util/ConfigOptions.h>
#include <hoot/core/util/ElementConverter.h>
#include <hoot/core/conflate/MatchThreshold.h>
//#include <hoot/core/util/ConfPath.h>

namespace hoot
{

QString PoiPolygonMatch::_matchName = "POI to Polygon";

PoiPolygonMatch::PoiPolygonMatch(const ConstOsmMapPtr& map, const ElementId& eid1,
                                 const ElementId& eid2, ConstMatchThresholdPtr threshold) :
Match(threshold),
_ancestorTypeMatch(false)
{
  ConstElementPtr e1 = map->getElement(eid1);
  ConstElementPtr e2 = map->getElement(eid2);
  ConstElementPtr poi, poly;
  if (isPoiIsh(e1) && isBuildingIsh(e2))
  {
    _poiEid = eid1;
    _polyEid = eid2;
    poi = e1;
    poly = e2;
  }
  else if (isPoiIsh(e2) && isBuildingIsh(e1))
  {
    _poiEid = eid2;
    _polyEid = eid1;
    poi = e2;
    poly = e1;
  }
  else
  {
    LOG_WARN(e1->toString());
    LOG_WARN(e2->toString());
    throw IllegalArgumentException("Expected a POI & polygon, got: " + eid1.toString() + " " +
                                   eid2.toString());
  }

  shared_ptr<Geometry> gpoly = ElementConverter(map).convertToGeometry(poly);
  shared_ptr<Geometry> gpoi = ElementConverter(map).convertToGeometry(poi);

  bool typeMatch = _calculateTypeMatch(poi, poly);
  bool ancestorTypeMatch = false;
  if (ConfigOptions().getPoiPolygonUseTagAncestorTypeMatching())
  {
    ancestorTypeMatch = _calculateAncestorTypeMatch(map, poi, poly);
    if (ancestorTypeMatch)
    {
      typeMatch = true;
    }
  }
  LOG_VARD(ancestorTypeMatch);
  //bool buildingCategoryTypeMatch = _getTagDistance("category", "building", map, e1, e2) == 0;
  //LOG_VARD(buildingCategoryTypeMatch);
  //bool poiCategoryTypeMatch = _getTagDistance("category", "poi", map, e1, e2) == 0;
  //LOG_VARD(poiCategoryTypeMatch);
  //TagCategoryDifferencer differ(OsmSchemaCategory::fromString("building"));
  //bool buildingCategoryTypeMatch = differ.diff(map, e1, e2) == 0;
  double nameScore = _calculateNameScore(poi, poly);
  bool nameMatch = nameScore >= ConfigOptions().getPoiPolygonMatchNameThreshold();
  bool exactNameMatch = nameMatch == 1.0;

  double distance = gpoly->distance(gpoi.get());

  // calculate the 2 sigma for the distance between the two objects
  double sigma1 = e1->getCircularError() / 2.0;
  double sigma2 = e1->getCircularError() / 2.0;
  double ce = sqrt(sigma1 * sigma1 + sigma2 * sigma2) * 2;

  //double reviewDistance = ConfigOptions().getPoiPolygonMatchReviewDistance() + ce;
  double reviewDistance = ConfigOptions().getPoiPolygonMatchReviewDistance();
  /*double reviewDistance;
  if (_oneGeneric(e1, e2))
  {
    reviewDistance = max(_getReviewDistance(e1), _getReviewDistance(e2));
  }
  else
  {
    reviewDistance = min(_getReviewDistance(e1), _getReviewDistance(e2));
  }*/
  if (ConfigOptions().getPoiPolygonAddCircularErrorToReviewDistance())
  {
    reviewDistance += ce;
  }
  bool closeMatch = distance <= reviewDistance;

  int evidence = 0;
  evidence += typeMatch ? 1 : 0;
  //evidence += ancestorTypeMatch ? 1 : 0;
  //evidence += ancestorTypeMatch || typeMatch ? 1 : 0;
  //evidence += buildingCategoryTypeMatch /*|| poiCategoryTypeMatch*/ ? 1 : 0;
  evidence += nameMatch ? 1 : 0;
  evidence += exactNameMatch ? 1 : 0;
  evidence += distance <= ConfigOptions().getPoiPolygonMatchDistance() ? 2 : 0;

  if (!closeMatch)
  {
    _c.setMiss();
    //LOG_DEBUG("poipoly miss");
  }
  else if (evidence >= 3)
  {
    _c.setMatch();
    //LOG_DEBUG("poipoly match");
  }
  else if (evidence >= 1)
  {
    _c.setReview();
    //LOG_DEBUG("poipoly review");
  }
  /*else if (!e1->getTags().contains("name") || !e2->getTags().contains("name"))
  {
    _c.setReview();
  }*/
  else
  {
    _c.setMiss();
    //LOG_DEBUG("poipoly miss");
  }

  _uuid1 = e1->getTags().get("uuid");
  _uuid2 = e2->getTags().get("uuid");
  _typeMatch = typeMatch;
  _nameMatch = nameMatch;
  _nameScore = nameScore;
  QStringList names1 = e1->getTags().getNames();
  names1.append(e1->getTags().getPseudoNames());
  _names1 = names1.join(",");
  QStringList names2 = e2->getTags().getNames();
  names2.append(e2->getTags().getPseudoNames());
  _names2 = names2.join(",");
  //_addressMatch = addressMatch;
  //_addrTag1 = e1AddrTag;
  //_combAddr1 = e1AddrComb;
  //_addrTag2 = e2AddrTag;
  //_combAddr2 = e2AddrComb;
  _closeMatch = closeMatch;
  _distance = distance;
  _reviewDistance = reviewDistance;
  _ce = ce;
  _circularError1 = e1->getCircularError();
  _circularError2 = e2->getCircularError();
  _evidence = evidence;

  if (e1->getTags().get("uuid") == "{aeb5cc44-25f2-51df-9ff0-89def62fb613}" ||
      e2->getTags().get("uuid") == "{aeb5cc44-25f2-51df-9ff0-89def62fb613}")
  {
    LOG_VARD(eid1);\
    LOG_VARD(e1->getTags().get("uuid"));
    LOG_VARD(e1->getTags());
    LOG_VARD(eid2);
    LOG_VARD(e2->getTags().get("uuid"));
    LOG_VARD(e2->getTags());
    LOG_VARD(typeMatch);
    LOG_VARD(_typeMatchAttributeKey);
    LOG_VARD(_typeMatchAttributeValue);
    LOG_VARD(nameMatch);
    LOG_VARD(nameScore);
    LOG_VARD(names1);
    LOG_VARD(names2);
    LOG_VARD(closeMatch);
    LOG_VARD(distance);
    LOG_VARD(reviewDistance);
    LOG_VARD(ce);
    LOG_VARD(e1->getCircularError());
    LOG_VARD(e2->getCircularError());
    LOG_VARD(evidence);
  }
}

double PoiPolygonMatch::_calculateNameScore(ConstElementPtr e1, ConstElementPtr e2) const
{
  if (ConfigOptions().getPoiPolygonUseMeanWordDistanceNameComparison())
  {
    return
      /*NameExtractor*/PoiPolygonNameExtractor(
        new TranslateStringDistance(
          new MeanWordSetDistance(
            new LevenshteinDistance(ConfigOptions().getLevenshteinDistanceAlpha()))))
        .extract(e1, e2);
  }
  else
  {
    /*SqliteWordWeightDictionary* dict =
      new SqliteWordWeightDictionary(
        ConfPath::search(ConfigOptions().getWeightedWordDistanceDictionary()));*/
    return
      /*NameExtractor*/PoiPolygonNameExtractor(
        new TranslateStringDistance(
          new /*MeanWordSetDistance*/MinSumWordSetDistance/*WeightedWordDistance*/(
            new LevenshteinDistance(ConfigOptions().getLevenshteinDistanceAlpha())/*, dict*/)))
        .extract(e1, e2);
  }
}

bool PoiPolygonMatch::_calculateTypeMatch(ConstElementPtr e1, ConstElementPtr e2) //const
{
  const Tags& t1 = e1->getTags();
  const Tags& t2 = e2->getTags();
  for (Tags::const_iterator it = t1.begin(); it != t1.end(); it++)
  {
    // if it is a use or POI category
    if ((OsmSchema::getInstance().getCategories(it.key(), it.value()) &
         (OsmSchemaCategory::building() | OsmSchemaCategory::use() | OsmSchemaCategory::poi()))
          != OsmSchemaCategory::Empty)
    {
      bool result = t2.get(it.key()) == it.value();
      if (result &&
          ((it.key().toLower() != "building" && it.value().toLower() != "yes") ||
            ConfigOptions().getPoiPolygonAllowGenericBuildingMatches()))
      {
        _typeMatchAttributeKey = it.key();
        _typeMatchAttributeValue = it.value();

        return true;
      }
    }
  }

  //TODO: hack - having trouble figuring out how to do this the right way...will fix
  if ((e1->getTags().get("amenity").toLower() == "hospital" &&
       e2->getTags().get("use").toLower() == "healthcare") ||
      (e2->getTags().get("amenity").toLower() == "hospital" &&
       e1->getTags().get("use").toLower() == "healthcare"))
  {
    return true;
  }

  return false;
}

bool PoiPolygonMatch::_calculateAncestorTypeMatch(const ConstOsmMapPtr& map, ConstElementPtr e1,
                                                  ConstElementPtr e2) //const
{
  QStringList types;

  types.append("tourism");
  types.append("amenity");

  types.append("leisure");
  types.append("historic");
  types.append("landuse");
  types.append("man_made");
  types.append("natural");
  types.append("place");
  types.append("power");
  types.append("railway");
  types.append("shop");
  types.append("sport");
  types.append("station");
  types.append("transport");
  types.append("barrier");
  types.append("use");
  types.append("industrial");

  //??
  types.append("building");

  for (int i = 0; i < types.length(); i++)
  {
    const QString type = types.at(i);
    LOG_VARD(type);
    bool eitherHaveOnlyBuildingGenericTag = false;
    //bool bothHaveOnlyBuildingGenericTag = false;
    if (type == "building")
    {
      const QStringList buildingTags1 = e1->getTags().getList("building");
      const QStringList buildingTags2 = e2->getTags().getList("building");
      if ((buildingTags1.length() == 1 && buildingTags1.at(0) == "yes") ||
          (buildingTags2.length() == 1 && buildingTags2.at(0) == "yes"))
      {
        eitherHaveOnlyBuildingGenericTag = true;
      }
      /*if ((buildingTags1.length() == 1 && buildingTags1.at(0) == "yes") &&
          (buildingTags2.length() == 1 && buildingTags2.at(0) == "yes"))
      {
        bothHaveOnlyBuildingGenericTag = true;
      }*/
    }
    LOG_VARD(eitherHaveOnlyBuildingGenericTag);

    if (!ConfigOptions().getPoiPolygonAllowGenericBuildingMatches() && type == "building" &&
        eitherHaveOnlyBuildingGenericTag/*bothHaveOnlyBuildingGenericTag*/)
    {
      LOG_DEBUG("generic building only");
    }
    else if (e1->getTags().contains(type) && e2->getTags().contains(type))
    {
      const double ancestorDistance = _getTagDistance("ancestor", type, map, e1, e2);
      LOG_VARD(ancestorDistance);
      if (ancestorDistance == 0.0)
      {
        _ancestorTypeMatch = true;
        LOG_VARD(_ancestorTypeMatch);
        return true;
      }
    }
  }

  return false;
}

set< pair<ElementId, ElementId> > PoiPolygonMatch::getMatchPairs() const
{
  set< pair<ElementId, ElementId> > result;
  result.insert(pair<ElementId, ElementId>(_poiEid, _polyEid));
  return result;
}

bool PoiPolygonMatch::isBuildingIsh(ConstElementPtr e)
{
  return OsmSchema::getInstance().isArea(e->getTags(), e->getElementType()) &&
    OsmSchema::getInstance().getCategories(e->getTags()).intersects(
        OsmSchemaCategory::building() | OsmSchemaCategory::poi());
}

bool PoiPolygonMatch::isPoiIsh(ConstElementPtr e)
{
  return e->getElementType() == ElementType::Node &&
    (OsmSchema::getInstance().getCategories(e->getTags()).intersects(
        OsmSchemaCategory::building() | OsmSchemaCategory::poi()) ||
     e->getTags().getNames().size() > 0);
}

double PoiPolygonMatch::_getTagDistance(const QString type, const QString kvp, ConstOsmMapPtr map,
                                       ConstElementPtr e1, ConstElementPtr e2)
{
  shared_ptr<TagFilteredDifferencer> differencer;
  if (type == "ancestor")
  {
    if (!_tagAncestorDifferencers.contains(kvp))
    {
      differencer.reset(new TagAncestorDifferencer(kvp));
      _tagAncestorDifferencers[kvp] = dynamic_pointer_cast<TagAncestorDifferencer>(differencer);
    }
    else
    {
      differencer = _tagAncestorDifferencers[kvp];
    }
  }
  else if (type == "category")
  {
    if (!_tagCategoryDifferencers.contains(kvp))
    {
      differencer.reset(new TagCategoryDifferencer(OsmSchemaCategory::fromString(kvp)));
      _tagCategoryDifferencers[kvp] = dynamic_pointer_cast<TagCategoryDifferencer>(differencer);
    }
    else
    {
      differencer = _tagCategoryDifferencers[kvp];
    }
  }
  else
  {
    throw HootException();
  }
  return differencer->diff(map, e1, e2);
}

double PoiPolygonMatch::_getTagDistance(const QString kvp, ConstElementPtr e1,
                                        ConstElementPtr e2) const
{
  double result = 1.0;

  const QStringList relatedTags1 = _getRelatedTags(kvp, e1->getTags());
  const QStringList relatedTags2 = _getRelatedTags(kvp, e2->getTags());
  if (relatedTags1.size() == 0 || relatedTags2.size() == 0)
  {
    return FeatureExtractor::nullValue();
  }

  for (int i = 0; i < relatedTags1.length(); i++)
  {
    for (int j = 0; j < relatedTags2.length(); j++)
    {
      result =
        min(1.0 - OsmSchema::getInstance().score(relatedTags1.at(i), relatedTags2.at(j)), result);
    }
  }

  return result;
}

QStringList PoiPolygonMatch::_getRelatedTags(const QString relateToKvp, const Tags& tags) const
{
  QStringList result;
  for (Tags::const_iterator it = tags.constBegin(); it != tags.constEnd(); it++)
  {
    QString kvp = it.key() + "=" + it.value();
    if (kvp != "poi=yes" && kvp != "place=locality" && kvp != "building=yes")
    {
      if (OsmSchema::getInstance().score(relateToKvp, kvp) > 0)
      {
        result.append(kvp);
      }
    }
  }
  return result;
}

QStringList PoiPolygonMatch::_getTagsByCategory(const QString category, const Tags& tags) const
{
  QStringList result;
  for (Tags::const_iterator it = tags.constBegin(); it != tags.constEnd(); it++)
  {
    QString kvp = it.key() + "=" + it.value();
    if (kvp != "poi=yes" && kvp != "place=locality" && kvp != "building=yes")
    {
      if (OsmSchema::getInstance().getCategories(kvp).toStringList().indexOf(category) >= 0)
      {
        result.append(kvp);
      }
    }
  }
  return result;
}

bool PoiPolygonMatch::_oneGeneric(ConstElementPtr e1, ConstElementPtr e2) const
{
  if (isPoiIsh(e1) && isBuildingIsh(e2))
  {
    return _getTagsByCategory("poi", e1->getTags()).length() == 0 ||
           _getTagsByCategory("building", e2->getTags()).length() == 0;
  }
  else if (isPoiIsh(e2) && isBuildingIsh(e1))
  {
    return _getTagsByCategory("building", e1->getTags()).length() == 0 ||
           _getTagsByCategory("poi", e2->getTags()).length() == 0;
  }
  else
  {
    throw HootException();
  }
}

//not currently using this...accounted for, hardcoded, in PoiPolygonNameExtractor for now
bool PoiPolygonMatch::_getAddressMatch(ConstElementPtr e1, ConstElementPtr e2)
{
  Tags e1Tags = e1->getTags();
  Tags e2Tags = e2->getTags();
  QString e1HouseNum = e1Tags.get("addr:housenumber").trimmed();
  QString e1Street = e1Tags.get("addr:street").trimmed().toLower();
  QString e1AddrComb = e1HouseNum + " " + e1Street;
  QString e1AddrTag = e1Tags.get("address").trimmed().toLower();
  QString e2HouseNum = e2Tags.get("addr:housenumber").trimmed();
  QString e2Street = e2Tags.get("addr:street").trimmed().toLower();
  QString e2AddrComb = e2HouseNum + " " + e2Street;
  QString e2AddrTag = e2Tags.get("address").trimmed().toLower();
  bool addressMatch = false;
  StringDistancePtr addrComp =
    StringDistancePtr(
      new TranslateStringDistance(
        new MeanWordSetDistance(
          new LevenshteinDistance(ConfigOptions().getLevenshteinDistanceAlpha()))));
  const double addrMatchThresh = 0.8;
  if (addrComp->compare(e1AddrComb, e2AddrComb) >= addrMatchThresh ||
      addrComp->compare(e1AddrComb, e2AddrTag) >= addrMatchThresh ||
      addrComp->compare(e2AddrComb, e1AddrTag) >= addrMatchThresh)
  {
    addressMatch = true;
  }

  return addressMatch;
}

//attempt to borrow some logic from poi to poi...not using this right now
double PoiPolygonMatch::_getReviewDistance(ConstElementPtr element)
{
  Tags tags = element->getTags();
  if (tags.get("amenity") == "grave_yard")
  {
    return max(1000.0, element->getCircularError());
  }
  else if (tags.get("amenity") == "hospital")
  {
    return max(100.0, element->getCircularError());
  }
  else if (tags.get("building") == "hospital")
  {
    return max(500.0, element->getCircularError());
  }
  else if (tags.get("building") == "train_station")
  {
    return max(500.0, element->getCircularError());
  }
  else if (tags.get("barrier") == "toll_booth")
  {
    return max(50.0, element->getCircularError());
  }
  else if (tags.get("barrier") == "border_control")
  {
    return max(100.0, element->getCircularError());
  }
  else if (tags.get("landuse") == "built_up_area")
  {
    return max(3000.0, element->getCircularError());
  }
  else if (tags.get("place") == "built_up_area")
  {
    return max(3000.0, element->getCircularError());
  }
  else if (tags.get("place") == "locality")
  {
    return max(3000.0, element->getCircularError());
  }
  else if (tags.get("place") == "populated")
  {
    return max(3000.0, element->getCircularError());
  }
  else if (tags.get("place") == "region")
  {
    return max(2000.0, element->getCircularError());
  }
  else if (tags.get("place") == "village")
  {
    return max(3000.0, element->getCircularError());
  }
  else if (tags.get("railway") == "station")
  {
    return max(1000.0, element->getCircularError());
  }
  else if (tags.get("station") == "light_rail")
  {
    return max(1000.0, element->getCircularError());
  }
  else if (tags.get("tourism") == "hotel")
  {
    return max(400.0, element->getCircularError());
  }
  else if (tags.get("transport") == "station")
  {
    return max(1000.0, element->getCircularError());
  }
  else if (tags.get("use") == "healthcare")
  {
    return max(100.0, element->getCircularError());
  }
  else if (tags.contains("amenity"))
  {
    return max(200.0, element->getCircularError());
  }
  else if (tags.contains("building"))
  {
    return max(200.0, element->getCircularError());
  }
  else if (tags.contains("historic"))
  {
    return max(200.0, element->getCircularError());
  }
  else if (tags.contains("landuse"))
  {
    return max(1000.0, element->getCircularError());
  }
  else if (tags.contains("leisure"))
  {
    return max(500.0, element->getCircularError());
  }
  else if (tags.contains("man_made"))
  {
    return max(200.0, element->getCircularError());
  }
  else if (tags.contains("natural"))
  {
    return max(1000.0, element->getCircularError());
  }
  else if (tags.contains("place"))
  {
    return max(1000.0, element->getCircularError());
  }
  else if (tags.contains("power"))
  {
    return max(50.0, element->getCircularError());
  }
  else if (tags.contains("railway"))
  {
    return max(500.0, element->getCircularError());
  }
  else if (tags.contains("shop"))
  {
    return max(200.0, element->getCircularError());
  }
  else if (tags.contains("sport"))
  {
    return max(100.0, element->getCircularError());
  }
  else if (tags.contains("station"))
  {
    return max(200.0, element->getCircularError());
  }
  else if (tags.contains("tourism"))
  {
    return max(200.0, element->getCircularError());
  }
  //industrial -->
  else
  {
    return ConfigOptions().getPoiPolygonMatchReviewDistance();
  }

  return -1.0;
}

QString PoiPolygonMatch::toString() const
{
  return QString("PoiPolygonMatch %1 %2 P: %3").arg(_poiEid.toString()).
      arg(_polyEid.toString()).arg(_c.toString());

  /*QString str =
    QString("PoiPolygonMatch %1 %2 P: %3").arg(_poiEid.toString()).
      arg(_polyEid.toString()).arg(_c.toString());
  str += " UUID1: " + _uuid1 + "\n";
  str += "UUID2: " + _uuid2 + "\n";
  str += "type match: " + QString::number(_typeMatch) + "\n";
  str += "type match attribute key: " + _typeMatchAttributeKey + "\n";
  str += "type match attribute value: " + _typeMatchAttributeValue + "\n";
  str += "ancestor type match: " + QString::number(_ancestorTypeMatch) + "\n";
  str += "name match: " + QString::number(_nameMatch) + "\n";
  str += "name score: " + QString::number(_nameScore) + "\n";
  str += "names 1: " + _names1 + "\n";
  str += "names 2: " + _names2 + "\n";
  //str += "address match: " + QString::number(_addressMatch) + "\n";
  //str += "address tag 1: " + _addrTag1 + "\n";
  //str += "combined address 1: " + _combAddr1 + "\n";
  //str += "address tag 2: " + _addrTag2 + "\n";
  //str += "combined address 2: " + _combAddr2 + "\n";
  str += "close match: " + QString::number(_closeMatch) + "\n";
  str += "distance: " + QString::number(_distance) + "\n";
  str += "review distance: " + QString::number(_reviewDistance) + "\n";
  str += "overall circular error: " + QString::number(_ce) + "\n";
  str += "circular error 1: " + QString::number(_circularError1) + "\n";
  str += "circular error 2: " + QString::number(_circularError2) + "\n";
  str += "evidence: " + QString::number(_evidence);
  return str;*/
}

}

