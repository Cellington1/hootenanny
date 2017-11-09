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
#ifndef POIIMPLICITTAGRAWRULESGENERATOR_H
#define POIIMPLICITTAGRAWRULESGENERATOR_H

// Hoot
#include <hoot/rnd/io/ImplicitTagRuleWordPartWriterFactory.h>
#include <hoot/core/util/Configurable.h>

// Qt
#include <QString>
#include <QMap>
#include <QHash>
#include <QTemporaryFile>

namespace hoot
{

class Tags;

/**
 * Derives implicit tag rules for POIs and writes the rules to various output formats
 */
class PoiImplicitTagRawRulesGenerator : public Configurable
{

public:

  PoiImplicitTagRawRulesGenerator();

  /**
   * Derives implicit tag rules for POIs given input data and writes the rules to output
   *
   * @param inputs a list of hoot supported feature input formats to derive rules from
   * @param translationScripts list of OSM translation scripts corresponding to the datasets
   * specified by the inputs parameter
   * @param outputs a list of hoot supported implicit tag rule output formats
   */
  void generateRules(const QStringList inputs, const QStringList translationScripts,
                     const QString output);

  virtual void setConfiguration(const Settings& conf);

  void setTokenizeNames(bool tokenizeNames) { _tokenizeNames = tokenizeNames; }

private:

  //for testing
  friend class PoiImplicitTagRawRulesGeneratorTest;

  long _statusUpdateInterval;
  bool _tokenizeNames;
  boost::shared_ptr<QFile> _output;

  boost::shared_ptr<QTemporaryFile> _countFile;
  boost::shared_ptr<QTemporaryFile> _sortedCountFile;
  //boost::shared_ptr<QTemporaryFile> _sortedDedupedCountFile;

  QHash<QString, QString> _wordCaseMappings;
  QHash<QString, long> _wordKeysToCounts;

  void _updateForNewWord(QString word, const QString kvp);
  QStringList _getPoiKvps(const Tags& tags) const;

  void _sortByTagOccurrence();
  void _removeDuplicatedKeyTypes();
};

}

#endif // POIIMPLICITTAGRAWRULESGENERATOR_H
