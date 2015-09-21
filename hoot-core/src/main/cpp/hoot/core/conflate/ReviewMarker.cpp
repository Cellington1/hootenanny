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
#include "ReviewMarker.h"

#include <hoot/core/util/Log.h>

namespace hoot
{

QString ReviewMarker::_complexGeometryType = "Bad Geometry";
QString ReviewMarker::_revieweeKey = "reviewee";
QString ReviewMarker::_reviewUuidKey = "hoot:review:uuid";
QString ReviewMarker::_reviewScoreKey = "hoot:review:score";
QString ReviewMarker::_reviewNeedsKey = "hoot:review:needs";
QString ReviewMarker::_reviewNoteKey = "hoot:review:note";
QString ReviewMarker::_reviewTypeKey = "hoot:review:type";


ReviewMarker::ReviewMarker()
{
}

set<ElementId> ReviewMarker::_getReviewRelations(const ConstOsmMapPtr &map, ElementId eid) const
{
  set<ElementId> result = map->getParents(eid);

  for (set<ElementId>::iterator it = result.begin(); it != result.end();)
  {
    set<ElementId>::iterator current = it++;
    ElementId p = *current;
    if (p.getType() != ElementType::Relation ||
        map->getRelation(p.getId())->getType() != Relation::REVIEW)
    {
      result.erase(current);
    }
  }

  return result;
}

QString ReviewMarker::getReviewType(const ConstOsmMapPtr &map, ReviewUid uid)
{
  assert(isReviewUid(map, uid));

  ConstRelationPtr r = map->getRelation(uid.getId());

  return r->getTags()[_reviewNoteKey];
}

set<ReviewMarker::ReviewUid> ReviewMarker::getReviewUids(const ConstOsmMapPtr &map,
  ConstElementPtr e1) const
{
  return _getReviewRelations(map, e1->getElementId());
}

bool ReviewMarker::isNeedsReview(const ConstOsmMapPtr &map, ConstElementPtr e1)
{
  // get all the review relations for e1
  set<ElementId> review1 = _getReviewRelations(map, e1->getElementId());

  // if there are more than one relations in the intersection, return true.
  return review1.size() >= 1;
}

bool ReviewMarker::isNeedsReview(const ConstOsmMapPtr &map, ConstElementPtr e1, ConstElementPtr e2)
{
  // get all the review relations for e1
  set<ElementId> review1 = _getReviewRelations(map, e1->getElementId());
  // get all the review relations for e2
  set<ElementId> review2 = _getReviewRelations(map, e2->getElementId());

  // intersect the relations
  set<ElementId> intersection;
  set_intersection(review1.begin(), review1.end(), review2.begin(), review2.end(),
    std::inserter(intersection, intersection.begin()));

  // if there are more than one relations in the intersection, return true.
  return intersection.size() >= 1;
}

bool ReviewMarker::isReviewUid(const ConstOsmMapPtr &map, ReviewUid uid)
{
  bool result = false;

  if (uid.getType() == ElementType::Relation)
  {
    ConstRelationPtr r = map->getRelation(uid.getId());

    if (r->getTags().isTrue(_reviewNeedsKey))
    {
      result = true;
    }
  }

  return result;
}

void ReviewMarker::mark(const OsmMapPtr &map, ElementPtr& e1, ElementPtr& e2, const QString& note,
  const QString &reviewType, double score)
{
  RelationPtr r(new Relation(Status::Conflated, map->createNextRelationId(), 0, Relation::REVIEW));
  r->getTags().set(_reviewNeedsKey, true);
  if (note.isEmpty())
  {
    throw IllegalArgumentException("You must specify a review note.");
  }
  r->getTags().appendValueIfUnique(_reviewNoteKey, note);
  #warning uncomment me
  //r->getTags().appendValueIfUnique(_reviewTypeKey, reviewType);
  r->getTags().set(_reviewScoreKey, score);
  r->addElement(_revieweeKey, e1->getElementId());
  r->addElement(_revieweeKey, e2->getElementId());
  r->setCircularError(-1);
  map->addElement(r);
}

void ReviewMarker::mark(const OsmMapPtr& map, ElementPtr& e, const QString& note,
  const QString &reviewType, double score)
{
  RelationPtr r(new Relation(Status::Conflated, map->createNextRelationId(), 0, Relation::REVIEW));
  r->getTags().set(_reviewNeedsKey, true);
  if (note.isEmpty())
  {
    throw IllegalArgumentException("You must specify a review note.");
  }
  r->getTags().appendValueIfUnique(_reviewNoteKey, note);
  #warning uncomment me
  //r->getTags().appendValueIfUnique(_reviewTypeKey, reviewType);
  r->getTags().set(_reviewScoreKey, score);
  r->addElement(_revieweeKey, e->getElementId());
  r->setCircularError(-1);
  map->addElement(r);
}

}
