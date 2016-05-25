#include "OsmChangesetSqlFileWriter.h"

// hoot
#include <hoot/core/io/HootApiDb.h>
#include <hoot/core/util/ConfigOptions.h>
#include <hoot/core/util/Log.h>

// Qt
#include <QSqlError>
#include <QSqlQuery>

namespace hoot
{

OsmChangesetSqlFileWriter::OsmChangesetSqlFileWriter(QUrl url) :
_changesetId(0),
_writeChangesetBounds(true)
{
  _db.open(url);
}

void OsmChangesetSqlFileWriter::write(const QString path, ChangeSetProviderPtr changesetProvider)
{
  LOG_DEBUG("Writing changeset to " << path);

  _resetChangesetVars();

  _outputSql.setFileName(path);
  if (_outputSql.open(QIODevice::WriteOnly | QIODevice::Text) == false)
  {
    throw HootException(QObject::tr("Error opening %1 for writing").arg(path));
  }

  int changes = 0;
  _createChangeSet();

  while (changesetProvider->hasMoreChanges())
  {
    Change change = changesetProvider->readNextChange();
    switch (change.type)
    {
      case Change::Create:
        _createNewElement(change.e);
        break;
      case Change::Modify:
        _updateExistingElement(change.e);
        break;
      case Change::Delete:
        _deleteExistingElement(change.e);
        break;
      default:
        throw IllegalArgumentException("Unexpected change type.");
    }

    if (_writeChangesetBounds)
    {
      _collectChangesetBoundsInfo(change.e);
    }

    changes++;

    if (changes > ConfigOptions().getChangesetMaxSize())
    {
      if (_writeChangesetBounds)
      {
        _updateChangesetBounds();
      }
      _createChangeSet();
      changes = 0;
    }
  }

  if (_writeChangesetBounds)
  {
    _updateChangesetBounds();
  }

  _outputSql.close();
  _db.close();
}

void OsmChangesetSqlFileWriter::_resetChangesetVars()
{
  _changesetBounds.init();
  _nodeIdsUsedInChangesetBoundsCalculation.clear();
  _nodeIdsNotUsedInChangesetBoundsCalculation.clear();
  _wayIdsUsedInChangesetBoundsCalculation.clear();
  _wayIdsNotUsedInChangesetBoundsCalculation.clear();
}

/*
 * This is collecting the bounds info for every node involved in the changeset.  I believe it
 * handles all cases, but that may still need to be proven with more granular unit tests.  Since
 * the ChangesetDeriver directly feeds this logic, not every combination of possible changeset will
 * be encountered here (deleting an element after creating or modifying it in the same changeset...
 * ChangesetDeriver shouldn't do that...right?).
 */
void OsmChangesetSqlFileWriter::_collectChangesetBoundsInfo(ConstElementPtr element)
{
  if (element->getElementType().getEnum() == ElementType::Node)
  {
    ConstNodePtr node = dynamic_pointer_cast<const Node>(element);
    //We have the node coords directly here, so add them to the changeset bounds and record
    //the node as having been parsed
    _changesetBounds.expandToInclude(node->getX(), node->getY());
    _nodeIdsUsedInChangesetBoundsCalculation.insert(element->getId());
    //If we previously recorded this node as not being used in the changeset calculation when
    //parsing a way or relation that referenced it, that reference can now be removed.
    if (_nodeIdsNotUsedInChangesetBoundsCalculation.contains(element->getId()))
    {
      _nodeIdsNotUsedInChangesetBoundsCalculation.remove(element->getId());
    }
  }
  else if (element->getElementType().getEnum() == ElementType::Way)
  {
    ConstWayPtr way = dynamic_pointer_cast<const Way>(element);
    const vector<long> wayNodeIds = way->getNodeIds();
    for (vector<long>::const_iterator itr = wayNodeIds.begin(); itr != wayNodeIds.end(); itr++)
    {
      const long wayNodeId = *itr;
      //From the way, we only have the node id's and not their coords, so if we haven't already
      //parsed the node's coords, add each way node id to a collection of unparsed nodes.
      if (!_nodeIdsUsedInChangesetBoundsCalculation.contains(wayNodeId))
      {
        _nodeIdsNotUsedInChangesetBoundsCalculation.insert(wayNodeId);
      }
      _wayIdsUsedInChangesetBoundsCalculation.insert(element->getId());
      //If we previously recorded this way as not being used in the changeset calculation when
      //parsing a relation that referenced it, that reference can now be removed.
      _wayIdsNotUsedInChangesetBoundsCalculation.remove(element->getId());
    }
  }
  else
  {
    ConstRelationPtr relation = dynamic_pointer_cast<const Relation>(element);
    const vector<RelationData::Entry> members = relation->getMembers();
    for (vector<RelationData::Entry>::const_iterator itr = members.begin(); itr != members.end(); itr++)
    {
      const RelationData::Entry member = *itr;
      if (member.getElementId().getType().getEnum() == ElementType::Node &&
          !_nodeIdsUsedInChangesetBoundsCalculation.contains(member.getElementId().getId()))
      {
        //From the relation, we only have the node id's and not their coords, so if we haven't
        //already parsed the node's coords, add its id to a collection of unparsed nodes.
        if (!_nodeIdsUsedInChangesetBoundsCalculation.contains(member.getElementId().getId()))
        {
          _nodeIdsNotUsedInChangesetBoundsCalculation.insert(member.getElementId().getId());
        }
      }
      else if (member.getElementId().getType().getEnum() == ElementType::Way)
      {
        //From the relation, we only have the way id's and not their way nodecoords, so if we
        //haven't already parsed the way, add its id to a collection of unparsed ways.
        if (!_wayIdsUsedInChangesetBoundsCalculation.contains(member.getElementId().getId()))
        {
          _wayIdsNotUsedInChangesetBoundsCalculation.insert(member.getElementId().getId());
        }
      }
    }
  }
}

void OsmChangesetSqlFileWriter::_updateChangesetBounds()
{
  LOG_VARD(_nodeIdsUsedInChangesetBoundsCalculation.size());
  LOG_VARD(_changesetBounds.toString());

  //Query against current tables and just take the information from the most recent way/node .  This
  //will be accurate, assuming each feature was referenced only once in the changeset (hopefully
  //that's true).  If for some reason that's not true, then the bounds calculation would be
  //incorrect in that it wouldn't be grabbing the bounds of every change in the changeset.

  QString valuesStr = "";
  QSqlQuery query(_db.getDB());

  LOG_VARD(_wayIdsNotUsedInChangesetBoundsCalculation.size());
  if (_wayIdsNotUsedInChangesetBoundsCalculation.size() > 0)
  {
    //get the node id's of any ways whose node id's couldn't previously be parsed and add them to
    //the unparsed nodes collection

    query.clear();
    valuesStr += "(";
    for (QSet<long>::const_iterator itr = _wayIdsNotUsedInChangesetBoundsCalculation.begin();
         itr != _wayIdsNotUsedInChangesetBoundsCalculation.end(); itr++)
    {
      //LOG_VARD(*itr);
      valuesStr += QString::number(*itr) + ",";
    }
    valuesStr.chop(1);
    valuesStr += ")";
    //LOG_VARD(valuesStr);
    query.prepare("select node_id from current_way_nodes where way_id in " + valuesStr);
    if (query.exec() == false)
    {
      LOG_ERROR(query.executedQuery());
      LOG_ERROR(query.lastError().text());
      throw HootException("Could not execute query: " + query.lastError().text());
    }
    while (query.next())
    {
      //LOG_VARD(query.value(0));
      _nodeIdsNotUsedInChangesetBoundsCalculation.insert(query.value(0).toLongLong());
    }
    query.finish();
  }

  LOG_VARD(_nodeIdsNotUsedInChangesetBoundsCalculation.size());
  if (_nodeIdsNotUsedInChangesetBoundsCalculation.size() > 0)
  {
    //get the bounds for all nodes in the unparsed nodes collection and add that to the changeset
    //bounds calculated from the nodes that were already parsed

    valuesStr += "(";
    for (QSet<long>::const_iterator itr = _nodeIdsNotUsedInChangesetBoundsCalculation.begin();
         itr != _nodeIdsNotUsedInChangesetBoundsCalculation.end(); itr++)
    {
      //LOG_VARD(*itr);
      valuesStr += QString::number(*itr) + ",";
    }
    valuesStr.chop(1);
    valuesStr += ")";
    //LOG_VARD(valuesStr);
    //query.clear();
    query.prepare(
      QString("select min(min_lon), max(max_lon), min(min_lat), max(max_lat) from current_nodes ") +
      QString("where id in " + valuesStr));
    if (query.exec() == false)
    {
      LOG_ERROR(query.executedQuery());
      LOG_ERROR(query.lastError().text());
      throw HootException("Could not execute query: " + query.lastError().text());
    }
    const double minLon = (double)query.value(0).toLongLong() / (double)HootApiDb::COORDINATE_SCALE;
    //LOG_VARD(minLon);
    const double maxLon = (double)query.value(1).toLongLong() / (double)HootApiDb::COORDINATE_SCALE;
    //LOG_VARD(maxLon);
    const double minLat = (double)query.value(2).toLongLong() / (double)HootApiDb::COORDINATE_SCALE;
    //LOG_VARD(minLat);
    const double maxLat = (double)query.value(3).toLongLong() / (double)HootApiDb::COORDINATE_SCALE;
    //LOG_VARD(maxLat);
    query.finish();

    //expand the changeset bounds to include the nodes
    shared_ptr<Envelope> queryEnv(new Envelope(minLon, maxLon, minLat, maxLat));
    _changesetBounds.expandToInclude(queryEnv.get());
  }

  //update the changeset's bounds
  //LOG_VARD(_changesetBounds.toString());
  _outputSql.write(
    QString("UPDATE changesets SET min_lat=%2, max_lat=%3, min_lon=%4, max_lon=%5 WHERE id=%1;\n")
      .arg(_changesetId)
      .arg((qlonglong)HootApiDb::round(_changesetBounds.getMinY() * HootApiDb::COORDINATE_SCALE, 7))
      .arg((qlonglong)HootApiDb::round(_changesetBounds.getMaxY() * HootApiDb::COORDINATE_SCALE, 7))
      .arg((qlonglong)HootApiDb::round(_changesetBounds.getMinX() * HootApiDb::COORDINATE_SCALE, 7))
      .arg((qlonglong)HootApiDb::round(_changesetBounds.getMaxX() * HootApiDb::COORDINATE_SCALE, 7))
    .toUtf8());

  _resetChangesetVars();
}

void OsmChangesetSqlFileWriter::_createChangeSet()
{
  LOG_DEBUG("Creating changeset...");
  _changesetId = _db.getNextId("changesets");
  _outputSql.write(
    QString("INSERT INTO changesets (id, user_id, created_at, closed_at) VALUES "
            "(%1, %2, now(), now());\n")
      .arg(_changesetId)
      .arg(ConfigOptions().getChangesetUserId())
    .toUtf8());
  _outputSql.write(
    QString("INSERT INTO changeset_tags (changeset_id, k, v) VALUES "
            "(%1, '%2', '%3');\n")
      .arg(_changesetId)
      .arg("written_by")
      .arg("Hootenanny")
    .toUtf8());
}

ElementPtr OsmChangesetSqlFileWriter::_getChangeElement(ConstElementPtr element)
{
  ElementPtr changeElement;
  switch (element->getElementType().getEnum())
  {
    case ElementType::Node:
      changeElement.reset(new Node(*dynamic_pointer_cast<const Node>(element)));
      break;
    case ElementType::Way:
      changeElement.reset(new Way(*dynamic_pointer_cast<const Way>(element)));
      break;
   case ElementType::Relation:
      changeElement.reset(new Relation(*dynamic_pointer_cast<const Relation>(element)));
      break;
    default:
      throw HootException("Unknown element type");
  }

  return changeElement;
}

// If osm.changeset.file.writer.generate.new.ids is false, then these create methods assume
// you've already set the ID correctly in terms of the OSM API target db for the element to be
// created.

void OsmChangesetSqlFileWriter::_createNewElement(ConstElementPtr element)
{
  const QString elementTypeStr = element->getElementType().toString().toLower();
  ElementPtr changeElement = _getChangeElement(element);

  long id;
  if (ConfigOptions().getOsmChangesetSqlFileWriterGenerateNewIds())
  {
    id = _db.getNextId(element->getElementType().getEnum());
  }
  else
  {
    id = changeElement->getId();
  }

  changeElement->setId(id);
  changeElement->setVersion(1);
  changeElement->setVisible(true);
  changeElement->setChangeset(_changesetId);

  QString note = "";
  /*if (changeElement->getTags().contains("note"))
  {
    note = changeElement->getTags().get("note");
  }*/
  //LOG_VARD(changeElement->getId());
  //LOG_VARD(note);
  //LOG_VARD(changeElement->getVersion());
  QString commentStr = "/* create " + elementTypeStr + " " + QString::number(changeElement->getId());
  if (!note.isEmpty())
  {
    commentStr += " - note: " + note;
  }
  commentStr += "*/\n";
  _outputSql.write((commentStr).toUtf8());

  const QString values = _getInsertValuesStr(changeElement);
  _outputSql.write(
    ("INSERT INTO " + elementTypeStr + "s (" + elementTypeStr + "_id, " + values).toUtf8());
  _outputSql.write(("INSERT INTO current_" + elementTypeStr + "s (id, " + values).toUtf8());

  _createTags(changeElement);

  switch (changeElement->getElementType().getEnum())
  {
    case ElementType::Way:
      _createWayNodes(dynamic_pointer_cast<const Way>(changeElement));
      break;
    case ElementType::Relation:
      _createRelationMembers(dynamic_pointer_cast<const Relation>(changeElement));
      break;
    default:
      //node
      break;
  }
}

QString OsmChangesetSqlFileWriter::_getUpdateValuesStr(ConstElementPtr element) const
{
  switch (element->getElementType().getEnum())
  {
    case ElementType::Node:
      return _getUpdateValuesNodeStr(dynamic_pointer_cast<const Node>(element));
    case ElementType::Way:
      return _getUpdateValuesWayOrRelationStr(element);
    case ElementType::Relation:
      return _getUpdateValuesWayOrRelationStr(element);
    default:
      throw HootException("Unknown element type");
  }
}

QString OsmChangesetSqlFileWriter::_getUpdateValuesNodeStr(ConstNodePtr node) const
{
  return
    QString("latitude=%2, longitude=%3, changeset_id=%4, visible=%5, \"timestamp\"=now(), tile=%6, version=%7 WHERE id=%1;\n")
      .arg(node->getId())
      .arg((qlonglong)HootApiDb::round(node->getY() * HootApiDb::COORDINATE_SCALE, 7))
      .arg((qlonglong)HootApiDb::round(node->getX() * HootApiDb::COORDINATE_SCALE, 7))
      .arg(node->getChangeset())
      .arg(_getVisibleStr(node->getVisible()))
      .arg(HootApiDb::tileForPoint(node->getY(), node->getX()))
      .arg(node->getVersion());
}

QString OsmChangesetSqlFileWriter::_getUpdateValuesWayOrRelationStr(ConstElementPtr element) const
{
  return
    QString("changeset_id=%2, visible=%3, \"timestamp\"=now(), version=%4 WHERE id=%1;\n")
      .arg(element->getId())
      .arg(element->getChangeset())
      .arg(_getVisibleStr(element->getVisible()))
      .arg(element->getVersion());
}

void OsmChangesetSqlFileWriter::_updateExistingElement(ConstElementPtr element)
{
  const QString elementTypeStr = element->getElementType().toString().toLower();
  ElementPtr changeElement = _getChangeElement(element);

  //if another parsed change previously modified the element with this id, we want to get the
  //modified version
  const long currentVersion = changeElement->getVersion();
  const long newVersion = currentVersion + 1;

  changeElement->setVersion(newVersion);
  changeElement->setChangeset(_changesetId);
  changeElement->setVisible(true);

  QString note = "";
  /*if (changeElement->getTags().contains("note"))
  {
    note = changeElement->getTags().get("note");
  }*/
  //LOG_VARD(changeElement->getId());
  //LOG_VARD(note);
  //LOG_VARD(changeElement->getVersion());
  QString commentStr = "/* modify " + elementTypeStr + " " + QString::number(changeElement->getId());
  if (!note.isEmpty())
  {
    commentStr += " - note: " + note;
  }
  commentStr += "*/\n";
  _outputSql.write((commentStr).toUtf8());

  //<element-name> table contains all version of all elements of that type in a history, so insert
  //into that table.
  _outputSql.write(
    ("INSERT INTO " + elementTypeStr + "s (" + elementTypeStr + "_id, " +
     _getInsertValuesStr(changeElement)).toUtf8());
  //current_<element-name> contains the single latest version of the element, so update that record
  _outputSql.write(
    ("UPDATE current_" + elementTypeStr + "s SET " + _getUpdateValuesStr(changeElement)).toUtf8());

  _deleteCurrentTags(changeElement->getElementId());
  _createTags(changeElement);

  switch (changeElement->getElementType().getEnum())
  {
    case ElementType::Way:
      _deleteAll("current_way_nodes", "way_id", changeElement->getId());
      _deleteAll("way_nodes", "way_id", changeElement->getId());
      _createWayNodes(dynamic_pointer_cast<const Way>(changeElement));
      break;
    case ElementType::Relation:
      _deleteAll("current_relation_members", "relation_id", changeElement->getId());
      _deleteAll("relation_members", "relation_id", changeElement->getId());
      _createRelationMembers(dynamic_pointer_cast<const Relation>(changeElement));
      break;
    default:
      //node
      break;
  }
}

void OsmChangesetSqlFileWriter::_deleteExistingElement(ConstElementPtr element)
{
  const QString elementIdStr = QString::number(element->getId());
  const QString elementTypeStr = element->getElementType().toString().toLower();
  ElementPtr changeElement = _getChangeElement(element);

  const long currentVersion = changeElement->getVersion();
  const long newVersion = currentVersion + 1;

  changeElement->setVersion(newVersion);
  changeElement->setVisible(false);
  changeElement->setChangeset(_changesetId);

  QString note = "";
  /*if (changeElement->getTags().contains("note"))
  {
    note = changeElement->getTags().get("note");
  }*/
  //LOG_VARD(changeElement->getId());
  //LOG_VARD(note);
  //LOG_VARD(changeElement->getVersion());
  QString commentStr = "/* delete " + elementTypeStr + " " + QString::number(changeElement->getId());
  if (!note.isEmpty())
  {
    commentStr += " - note: " + note;
  }
  commentStr += "*/\n";
  _outputSql.write((commentStr).toUtf8());

  //OSM API DB keeps history for all elements, so the existing element in the master table is not
  //modified and a new record is added with the updated version and visible set to false
  _outputSql.write(("INSERT INTO " + elementTypeStr + "s (" + elementTypeStr + "_id, " +
                    _getInsertValuesStr(changeElement)).toUtf8());

  _deleteCurrentTags(changeElement->getElementId());

  switch (changeElement->getElementType().getEnum())
  {
    case ElementType::Node:

      _outputSql.write(
        ("DELETE FROM current_way_nodes WHERE node_id=" + elementIdStr + ";\n").toUtf8());
      _outputSql.write(
        ("DELETE FROM current_relation_members WHERE member_type = 'Node' AND member_id = " +
         elementIdStr + ";\n").toUtf8());

      break;

    case ElementType::Way:

      //all of its entries in current way nodes are removed
      _outputSql.write(
        ("DELETE FROM current_way_nodes WHERE way_id=" + elementIdStr + ";\n").toUtf8());
      _outputSql.write(
        ("DELETE FROM current_relation_members WHERE member_type = 'Way' AND member_id = " +
         elementIdStr + ";\n").toUtf8());

      break;

    case ElementType::Relation:

      _outputSql.write(
        ("DELETE FROM current_relation_members WHERE relation_id=" + elementIdStr + ";\n").toUtf8());
      _outputSql.write(
        ("DELETE FROM current_relation_members WHERE member_type = 'Relation' AND member_id = " +
         elementIdStr + ";\n").toUtf8());

      break;

    default:
      throw HootException("Unknown element type");
  }

  //in the current table, the element isn't deleted but set to be invisible
  const QString values =
    QString("changeset_id=%1, visible=%2, version=%3 WHERE id=%4;\n")
      .arg(changeElement->getChangeset())
      .arg(_getVisibleStr(changeElement->getVisible()))
      .arg(changeElement->getVersion())
      .arg(changeElement->getId());
  _outputSql.write(("UPDATE current_" + elementTypeStr + "s SET " + values).toUtf8());
}

QString OsmChangesetSqlFileWriter::_getInsertValuesStr(ConstElementPtr element) const
{
  switch (element->getElementType().getEnum())
  {
    case ElementType::Node:
      return _getInsertValuesNodeStr(dynamic_pointer_cast<const Node>(element));
    case ElementType::Way:
      return _getInsertValuesWayOrRelationStr(element);
    case ElementType::Relation:
      return _getInsertValuesWayOrRelationStr(element);
    default:
      throw HootException("Unknown element type");
  }
}

QString OsmChangesetSqlFileWriter::_getInsertValuesNodeStr(ConstNodePtr node) const
{
  return
    QString("latitude, longitude, changeset_id, visible, \"timestamp\", "
      "tile, version) VALUES (%1, %2, %3, %4, %5, now(), %6, %7);\n")
      .arg(node->getId())
      .arg((qlonglong)HootApiDb::round(node->getY() * HootApiDb::COORDINATE_SCALE, 7))
      .arg((qlonglong)HootApiDb::round(node->getX() * HootApiDb::COORDINATE_SCALE, 7))
      .arg(node->getChangeset())
      .arg(_getVisibleStr(node->getVisible()))
      .arg(HootApiDb::tileForPoint(node->getY(), node->getX()))
      .arg(node->getVersion());
}

QString OsmChangesetSqlFileWriter::_getInsertValuesWayOrRelationStr(ConstElementPtr element) const
{
  return
    QString("changeset_id, visible, \"timestamp\", "
      "version) VALUES (%1, %2, %3, now(), %4);\n")
      .arg(element->getId())
      .arg(element->getChangeset())
      .arg(_getVisibleStr(element->getVisible()))
      .arg(element->getVersion());
}

void OsmChangesetSqlFileWriter::_createTags(ConstElementPtr element)
{
  QStringList tableNames = _tagTableNamesForElement(element->getElementId());

  Tags tags = element->getTags();
  if (element->getElementType().getEnum() == ElementType::Relation &&
      !tags.contains("type"))
  {
    ConstRelationPtr tmp = dynamic_pointer_cast<const Relation>(element);
    tags.appendValue("type", tmp->getType());
  }

  for (Tags::const_iterator it = tags.begin(); it != tags.end(); ++it)
  {
    QString k = it.key();
    QString v = it.value();

    const QString currentTagValues =
      QString("(%1_id, k, v) VALUES (%2, '%3', '%4');\n")
        .arg(element->getElementId().getType().toString().toLower())
        .arg(element->getElementId().getId())
        .arg(k.replace('\'', "''"))
        .arg(v.replace('\'', "''"));

    const QString tagValues =
      QString("(%1_id, k, v, version) VALUES (%2, '%3', '%4', %5);\n")
        .arg(element->getElementId().getType().toString().toLower())
        .arg(element->getElementId().getId())
        .arg(k.replace('\'', "''"))
        .arg(v.replace('\'', "''"))
        .arg(element->getVersion());

    _outputSql.write((QString("INSERT INTO %1 ").arg(tableNames.at(0)) + currentTagValues).toUtf8());
    _outputSql.write((QString("INSERT INTO %1 ").arg(tableNames.at(1)) + tagValues).toUtf8());
  }
}

QStringList OsmChangesetSqlFileWriter::_tagTableNamesForElement(const ElementId& eid) const
{
  QStringList tableNames;
  const QString tableName1 = "current_" + eid.getType().toString().toLower() + "_tags";
  tableNames.append(tableName1);
  const QString tableName2 = eid.getType().toString().toLower() + "_tags";
  tableNames.append(tableName2);
  return tableNames;
}

void OsmChangesetSqlFileWriter::_createWayNodes(ConstWayPtr way)
{
  //LOG_DEBUG("way nodes create");
  //LOG_VARD(way->getId());
  const std::vector<long> nodeIds = way->getNodeIds();
  for (size_t i = 0; i < nodeIds.size(); i++)
  {
    const long nodeId = nodeIds.at(i);
    //LOG_VARD(nodeId);

    QString values =
      QString("(way_id, node_id, version, sequence_id) VALUES (%1, %2, 1, %3);\n")
        .arg(way->getId())
        .arg(nodeId)
        .arg(i + 1);
    _outputSql.write(("INSERT INTO way_nodes " + values).toUtf8());

    values =
      QString("(way_id, node_id, sequence_id) VALUES (%1, %2, %3);\n")
        .arg(way->getId())
        .arg(nodeId)
        .arg(i + 1);
    _outputSql.write(("INSERT INTO current_way_nodes " + values).toUtf8());
  }
}

void OsmChangesetSqlFileWriter::_createRelationMembers(ConstRelationPtr relation)
{
  //LOG_DEBUG("relation members create");
  //LOG_VARD(relation->getId());
  const vector<RelationData::Entry> members = relation->getMembers();
  for (size_t i = 0; i < members.size(); i++)
  {
    const RelationData::Entry member = members[i];
    //LOG_VARD(member.toString());

    QString values =
      QString(
        "(relation_id, member_type, member_id, member_role, version, sequence_id) VALUES (%1, '%2', %3, '%4', 1, %5);\n")
        .arg(relation->getId())
        .arg(member.getElementId().getType().toString())
        .arg(member.getElementId().getId())
        .arg(member.getRole())
        .arg(i + 1);
    _outputSql.write(("INSERT INTO relation_members " + values).toUtf8());

    values =
      QString(
        "(relation_id, member_type, member_id, member_role, sequence_id) VALUES (%1, '%2', %3, '%4', %5);\n")
        .arg(relation->getId())
        .arg(member.getElementId().getType().toString())
        .arg(member.getElementId().getId())
        .arg(member.getRole())
        .arg(i + 1);
    _outputSql.write(("INSERT INTO current_relation_members " + values).toUtf8());
  }
}

void OsmChangesetSqlFileWriter::_deleteCurrentTags(const ElementId& eid)
{
  QStringList tableNames = _tagTableNamesForElement(eid);
  foreach (QString tableName, tableNames)
  {
    QString idFieldName = tableName;
    idFieldName.replace("current_", "").replace("_tags", "");
    idFieldName += "_id";
    _deleteAll(tableName, idFieldName, eid.getId());
  }
}

void OsmChangesetSqlFileWriter::_deleteAll(const QString tableName, const QString idFieldName,
                                           const long id)
{
  //LOG_DEBUG("delete all" << tableName);
  _outputSql.write(
    (QString("DELETE FROM %1 WHERE %2 = %3;\n")
      .arg(tableName)
      .arg(idFieldName)
      .arg(id))
    .toUtf8());
}

}