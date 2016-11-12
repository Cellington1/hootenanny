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
#include <hoot/core/util/HootException.h>

#include "Element.h"

#include "ElementListener.h"

namespace hoot {

Element::Element(Status s) : _status(s)
{
  _listener = 0;
}

QString Element::getStatusString() const
{
  return _status.toString().toLower();
}

// This returns "Input1" etc instead of "Unknown1"
QString Element::getTextStatus() const
{
  return _status.toTextStatus();
}

void Element::_postGeometryChange()
{
  if (_listener != 0)
  {
    _listener->postGeometryChange(this);
  }
}

void Element::_preGeometryChange()
{
  if (_listener != 0)
  {
    _listener->preGeometryChange(this);
  }
}


}
