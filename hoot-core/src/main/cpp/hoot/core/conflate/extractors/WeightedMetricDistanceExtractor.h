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
#ifndef WEIGHTEDMETRICDISTANCEEXTRACTOR_H
#define WEIGHTEDMETRICDISTANCEEXTRACTOR_H

#include "WayFeatureExtractor.h"

#include <hoot/core/util/Configurable.h>
#include <hoot/core/algorithms/ProbabilityOfMatch.h>

namespace hoot
{

/**
 * Similar to Metric Distance as described in [1].
 * 1. Savary & Zeitouni, 2005
 */
class WeightedMetricDistanceExtractor : public WayFeatureExtractor, public Configurable
{
public:
  static string className() { return "hoot::WeightedMetricDistanceExtractor"; }

  WeightedMetricDistanceExtractor(ValueAggregator* wayAgg = 0, ValueAggregator* pointAgg = 0,
                                  Meters searchRadius = -1);

  virtual string getClassName() const { return className(); }

  virtual string getName() const;

  virtual void setConfiguration(const Settings& conf);

  void setPointAggregator(const QString aggregator);

  void setSearchRadius(const double radius);

protected:
  double _extract(const OsmMap& /*map*/, const ConstWayPtr& w1, const ConstWayPtr& w2) const;

  auto_ptr<ValueAggregator> _pointAgg;
  Meters _searchRadius;
};

}

#endif // WEIGHTEDMETRICDISTANCEEXTRACTOR_H
