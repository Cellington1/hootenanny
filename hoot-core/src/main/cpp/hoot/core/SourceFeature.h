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
 * @copyright Copyright (C) 2012, 2013 DigitalGlobe (http://www.digitalglobe.com/)
 */
#ifndef SOURCEFEATURE_H
#define SOURCEFEATURE_H

// Boost Includes
#include <boost/shared_ptr.hpp>

// OGR Includes
class OGRFeature;

/**
 * This class represents a single source feature used for conflation. If this SourceFeature needs
 * to manipulate the geometry it should be first copied. Similar to the concepts in the RoadMatcher
 * documentation the individual features may be in one of several source states.
 */
class SourceFeature
{
public:
    enum SourceState
    {
        Unknown,
        Matched,
        Standalone,
        Retired
    };

    const boost::shared_ptr<OGRFeature> getFeature() { return _feature; }

    SourceState getSourceState() { return _state; }

    void setSourceState(SourceState s) { _state = s; }

private:

    SourceState _state;
    boost::shared_ptr<OGRFeature> _feature;
};

#endif // SOURCEFEATURE_H
