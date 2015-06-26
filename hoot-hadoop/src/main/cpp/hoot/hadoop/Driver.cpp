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
 */

#include "Driver.h"

// hoot
#include <hoot/core/schema/JsonSchemaLoader.h>
#include <hoot/core/schema/OsmSchema.h>
#include <hoot/core/util/ConfPath.h>
#include <hoot/core/util/HootException.h>

// pretty pipes
#include <pp/mapreduce/Job.h>

// Qt
#include <QFileInfo>

namespace hoot
{

Driver::Driver()
{
}

void Driver::_addDefaultJobSettings(pp::Job& job)
{
  OsmSchema dummySchema;
  JsonSchemaLoader jsl(dummySchema);
  jsl.load(ConfPath::search("schema.json"));
  set<QString> deps = jsl.getDependencies();

  QHash<QString,QString> fileList;

  for (set<QString>::iterator it = deps.begin(); it != deps.end(); ++it)
  {
    QString fileName(QFileInfo(*it).fileName());

    if (!fileList.contains(fileName))
    {\
      fileList.insert(fileName,*it);

      job.addFile(it->toStdString());
    }
    else
    {
      throw HootException("Duplicate config files: " + *it + " and " + fileList.value(fileName));
    }
  }

  if (!fileList.contains("dictionary.json"))
  {
    job.addFile(ConfPath::search("dictionary.json").toStdString());
  }
  else
  {
    throw HootException("Duplicate config files: " + ConfPath::search("dictionary.json") +
                        " and " + fileList.value("dictionary.json"));
  }
}

}
