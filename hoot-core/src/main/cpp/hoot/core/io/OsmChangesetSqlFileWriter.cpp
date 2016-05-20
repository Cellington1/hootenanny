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
_changesetId(0)
{
  _db.open(url);

  _changeElementIdsToVersionsByElementType[ElementType::Node] = QMap<long, long>();
  _changeElementIdsToVersionsByElementType[ElementType::Way] = QMap<long, long>();
  _changeElementIdsToVersionsByElementType[ElementType::Relation] = QMap<long, long>();
}

void OsmChangesetSqlFileWriter::write(const QString path, ChangeSetProviderPtr changesetProvider)
{
  LOG_DEBUG("Writing changeset to " << path);

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
    changes++;

    if (changes > ConfigOptions().getChangesetMaxSize())
    {
      _createChangeSet();
      changes = 0;
    }
  }

  _outputSql.close();
  _db.close();
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
}

// If osm.changeset.file.writer.generate.new.ids is false, then these create methods assume
// you've already set the ID correctly in terms of the OSM API target db for the element to be
// created.

void OsmChangesetSqlFileWriter::_createNewElement(ConstElementPtr element)
{
  const QString elementTypeStr = element->getElementType().toString().toLower();
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
  //changeElement an element is being created, we're starting fresh so overwriting the map entry is ok
  _changeElementIdsToVersionsByElementType[changeElement->getElementType().getEnum()].insert(
    changeElement->getId(), changeElement->getVersion());

  QString note = "";
  if (changeElement->getTags().contains("note"))
  {
    note = changeElement->getTags().get("note");
  }
  LOG_VARD(changeElement->getId());
  LOG_VARD(note);
  LOG_VARD(changeElement->getVersion());
  QString commentStr = "/* create " + elementTypeStr;
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

void OsmChangesetSqlFileWriter::_updateExistingElement(ConstElementPtr element)
{
  switch (element->getElementType().getEnum())
  {
    case ElementType::Node:
      _modify(dynamic_pointer_cast<const Node>(element));
      break;
    case ElementType::Way:
      _modify(dynamic_pointer_cast<const Way>(element));
      break;
   case ElementType::Relation:
      _modify(dynamic_pointer_cast<const Relation>(element));
      break;
    default:
      throw HootException("Unknown element type");
  }
}

void OsmChangesetSqlFileWriter::_deleteExistingElement(ConstElementPtr element)
{
  const ElementType::Type elementType = element->getElementType().getEnum();
  if (elementType != ElementType::Node && elementType != ElementType::Way &&
      elementType != ElementType::Relation)
  {
    throw HootException("Unknown element type");
  }

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
  const QString elementIdStr = QString::number(changeElement->getId());
  const QString elementTypeStr = changeElement->getElementType().toString().toLower();
  long currentVersion = -1;
  if (_changeElementIdsToVersionsByElementType[elementType].contains(changeElement->getId()))
  {
    currentVersion =
      _changeElementIdsToVersionsByElementType[elementType].value(changeElement->getId());
  }
  else
  {
    currentVersion = changeElement->getVersion();
  }
  //TODO: hack - not sure why the version would ever be less than 1 by this point
  if (currentVersion < 1)
  {
    currentVersion = 2;
  }
  const long newVersion = currentVersion + 1;
  QString note = "";
  if (changeElement->getTags().contains("note"))
  {
    note = changeElement->getTags().get("note");
  }
  changeElement->setVersion(newVersion);
  _changeElementIdsToVersionsByElementType[elementType][changeElement->getId()] = newVersion;
  changeElement->setVisible(false);
  changeElement->setChangeset(_changesetId);
  LOG_VARD(changeElement->getId());
  LOG_VARD(note);
  LOG_VARD(changeElement->getVersion());
  QString commentStr = "/* delete " + elementTypeStr;
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
      return _getInsertValuesStr(dynamic_pointer_cast<const Node>(element));
    case ElementType::Way:
      return _getInsertValuesWayOrRelationStr(element);
    case ElementType::Relation:
      return _getInsertValuesWayOrRelationStr(element);
    default:
      throw HootException("Unknown element type");
  }
}

QString OsmChangesetSqlFileWriter::_getInsertValuesStr(ConstNodePtr node) const
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

/*
 * For modify:
 *
   - <element-name> table contains all version of all elements of that type in a history, so insert
into that table.

   - current_<element-name> contains the single latest version of the element, so update that record.

  //The changeset deriver will start versions at 0 to keep the xml changeset writing happy, but
  //we always want 1 to be the starting point for sql writing.
*/

void OsmChangesetSqlFileWriter::_modify(ConstNodePtr node)
{
  NodePtr changeNode(new Node(*node.get()));
  //if another parsed change previously modified the element with this id, we want to get the
  //modified version
  long currentVersion = -1;
  if (_changeElementIdsToVersionsByElementType[ElementType::Node].contains(changeNode->getId()))
  {
    currentVersion =
      _changeElementIdsToVersionsByElementType[ElementType::Node].value(changeNode->getId());
  }
  else
  {
    currentVersion = changeNode->getVersion();
  }
  const long newVersion = currentVersion + 1;
  QString note = "";
  if (changeNode->getTags().contains("note"))
  {
    note = changeNode->getTags().get("note");
  }
  changeNode->setVersion(newVersion);
  _changeElementIdsToVersionsByElementType[ElementType::Node][changeNode->getId()] = newVersion;
  changeNode->setChangeset(_changesetId);
  LOG_VARD(changeNode->getId());
  LOG_VARD(note);
  LOG_VARD(changeNode->getVersion());
  QString commentStr = "/* modify node ";
  if (!note.isEmpty())
  {
    commentStr += " - note: " + note;
  }
  commentStr += "*/\n";
  _outputSql.write((commentStr).toUtf8());

  QString values =
    QString("latitude, longitude, changeset_id, visible, \"timestamp\", "
      "tile, version) VALUES (%1, %2, %3, %4, true, now(), %5, %6);\n")
      .arg(changeNode->getId())
      .arg((qlonglong)HootApiDb::round(changeNode->getY() * HootApiDb::COORDINATE_SCALE, 7))
      .arg((qlonglong)HootApiDb::round(changeNode->getX() * HootApiDb::COORDINATE_SCALE, 7))
      .arg(changeNode->getChangeset())
      .arg(HootApiDb::tileForPoint(changeNode->getY(), changeNode->getX()))
      .arg(changeNode->getVersion());
  _outputSql.write(("INSERT INTO nodes (node_id, " + values).toUtf8());

  values =
    QString("latitude=%2, longitude=%3, changeset_id=%4, visible=true, \"timestamp\"=now(), tile=%5, version=%6 WHERE id=%1;\n")
      .arg(changeNode->getId())
      .arg((qlonglong)HootApiDb::round(changeNode->getY() * HootApiDb::COORDINATE_SCALE, 7))
      .arg((qlonglong)HootApiDb::round(changeNode->getX() * HootApiDb::COORDINATE_SCALE, 7))
      .arg(changeNode->getChangeset())
      .arg(HootApiDb::tileForPoint(changeNode->getY(), changeNode->getX()))
      .arg(changeNode->getVersion());
  _outputSql.write(("UPDATE current_nodes SET " + values).toUtf8());

  _deleteCurrentTags(changeNode->getElementId());
  _createTags(changeNode);
}

void OsmChangesetSqlFileWriter::_modify(ConstWayPtr way)
{
  WayPtr changeWay(new Way(*way.get()));
  //if another parsed change previously modified the element with this id, we want to get the
  //modified version
  long currentVersion = -1;
  if (_changeElementIdsToVersionsByElementType[ElementType::Way].contains(changeWay->getId()))
  {
    currentVersion =
      _changeElementIdsToVersionsByElementType[ElementType::Way].value(changeWay->getId());
  }
  else
  {
    currentVersion = changeWay->getVersion();
  }
  const long newVersion = currentVersion + 1;
  QString note = "";
  if (changeWay->getTags().contains("note"))
  {
    note = changeWay->getTags().get("note");
  }
  changeWay->setVersion(newVersion);
  _changeElementIdsToVersionsByElementType[ElementType::Way][changeWay->getId()] = newVersion;
  changeWay->setChangeset(_changesetId);
  LOG_VARD(changeWay->getId());
  LOG_VARD(note);
  LOG_VARD(changeWay->getVersion());
  QString commentStr = "/* modify way ";
  if (!note.isEmpty())
  {
    commentStr += " - note: " + note;
  }
  commentStr += "*/\n";
  _outputSql.write((commentStr).toUtf8());

  QString values =
    QString("changeset_id, visible, \"timestamp\", "
      "version) VALUES (%1, %2, true, now(), %3);\n")
      .arg(changeWay->getId())
      .arg(changeWay->getChangeset())
      .arg(changeWay->getVersion());
  _outputSql.write(("INSERT INTO ways (way_id, " + values).toUtf8());

  values =
    QString("changeset_id=%2, visible=true, \"timestamp\"=now(), version=%3 WHERE id=%1;\n")
      .arg(changeWay->getId())
      .arg(changeWay->getChangeset())
      .arg(changeWay->getVersion());
  _outputSql.write(("UPDATE current_ways SET " + values).toUtf8());

  _deleteAll("current_way_nodes", "way_id", changeWay->getId());
  _deleteAll("way_nodes", "way_id", changeWay->getId());
  _createWayNodes(changeWay);

  _deleteCurrentTags(changeWay->getElementId());
  _createTags(changeWay);
}

void OsmChangesetSqlFileWriter::_modify(ConstRelationPtr relation)
{
  RelationPtr changeRelation(new Relation(*relation.get()));
  //if another parsed change previously modified the element with this id, we want to get the
  //modified version
  long currentVersion = -1;
  if (_changeElementIdsToVersionsByElementType[ElementType::Relation].contains(changeRelation->getId()))
  {
    currentVersion =
      _changeElementIdsToVersionsByElementType[ElementType::Relation].value(changeRelation->getId());
  }
  else
  {
    currentVersion = changeRelation->getVersion();
  }
  const long newVersion = currentVersion + 1;
  QString note = "";
  if (changeRelation->getTags().contains("note"))
  {
    note = changeRelation->getTags().get("note");
  }
  changeRelation->setVersion(newVersion);
  _changeElementIdsToVersionsByElementType[ElementType::Relation][changeRelation->getId()] = newVersion;
  changeRelation->setChangeset(_changesetId);
  LOG_VARD(changeRelation->getId());
  LOG_VARD(note);
  LOG_VARD(changeRelation->getVersion());
  QString commentStr = "/* modify relation ";
  if (!note.isEmpty())
  {
    commentStr += " - note: " + note;
  }
  commentStr += "*/\n";
  _outputSql.write((commentStr).toUtf8());

  QString values =
    QString("changeset_id, visible, \"timestamp\", "
      "version) VALUES (%1, %2, true, now(), %3);\n")
      .arg(changeRelation->getId())
      .arg(changeRelation->getChangeset())
      .arg(changeRelation->getVersion());
  _outputSql.write(("INSERT INTO relations (relation_id, " + values).toUtf8());

  values =
    QString("changeset_id=%2, visible=true, \"timestamp\"=now(), version=%3 WHERE id=%1;\n")
      .arg(changeRelation->getId())
      .arg(changeRelation->getChangeset())
      .arg(changeRelation->getVersion());
  _outputSql.write(("UPDATE current_relations SET " + values).toUtf8());

  _deleteAll("current_relation_members", "relation_id", changeRelation->getId());
  _deleteAll("relation_members", "relation_id", changeRelation->getId());
  _createRelationMembers(changeRelation);

  _deleteCurrentTags(changeRelation->getElementId());
  _createTags(changeRelation);
}

void OsmChangesetSqlFileWriter::_createTags(ConstElementPtr element)
{
  QStringList tableNames = _tagTableNamesForElement(element->getElementId());
  const Tags tags = element->getTags();

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