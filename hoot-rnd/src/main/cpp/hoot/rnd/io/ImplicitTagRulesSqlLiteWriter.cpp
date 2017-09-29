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
#include "ImplicitTagRulesSqlLiteWriter.h"

// hoot
#include <hoot/core/util/HootException.h>
#include <hoot/core/util/DbUtils.h>

// Qt
#include <QSqlError>
#include <QSqlQuery>

namespace hoot
{

ImplicitTagRulesSqlLiteWriter::ImplicitTagRulesSqlLiteWriter()
{
}

ImplicitTagRulesSqlLiteWriter::~ImplicitTagRulesSqlLiteWriter()
{
  close();
}

void ImplicitTagRulesSqlLiteWriter::open(const QString output)
{
  close();

  _db = QSqlDatabase::addDatabase("QSQLITE");
  _db.setDatabaseName(output);
  if (!_db.open())
  {
    throw HootException("Error opening database: " + output);
  }
  LOG_DEBUG("Opened: " << output << ".");
}

void ImplicitTagRulesSqlLiteWriter::write(const QMap<QString, QMap<QString, long> >& tagRules)
{
  //DbUtils::execNoPrepare(_db, "CREATE TABLE rules (word TEXT PRIMARY KEY, kvp TEXT, count INT)");
  DbUtils::execNoPrepare(_db, "CREATE TABLE rules (word TEXT PRIMARY KEY, kvp TEXT)");
  DbUtils::execNoPrepare(_db, "BEGIN");

  QSqlQuery query(_db);
  //if (!query.prepare("INSERT INTO rules (word, kvp, count) VALUES(:word, :kvp, :count)"))
  if (!query.prepare("INSERT INTO rules (word, kvp, count) VALUES(:word, :kvp)"))
  {
    throw HootException(QString("Error preparing query: %1").arg(query.lastError().text()));
  }

  for (QMap<QString, QMap<QString, long> >::const_iterator tokenItr = tagRules.begin();
       tokenItr != tagRules.end(); ++tokenItr)
  {
    const QString word = tokenItr.key();
    const QMap<QString, long> kvps = tokenItr.value();
    for (QMap<QString, long>::const_iterator kvpItr = kvps.begin(); kvpItr != kvps.end(); ++kvpItr)
    {
      query.bindValue(":word", word);
      query.bindValue(":kvp", kvpItr.key());
      //query.bindValue(":count", kvpItr.value());
    }
  }

  if (!query.exec())
  {
    QString err =
      QString("Error executing query: %1 (%2)")
        .arg(query.executedQuery()).arg(query.lastError().text());
    throw HootException(err);
  }

  DbUtils::execNoPrepare(_db, "COMMIT");
}

void ImplicitTagRulesSqlLiteWriter::close()
{
  if (!_db.open())
  {
    _db.close();
  }
}

}
