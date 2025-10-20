/*
 Copyright (C) 2010 Kristian Duske

 This file is part of TrenchBroom.

 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 TrenchBroom is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#include "EntityPropertyModel.h"

#include <QBrush>
#include <QByteArray>
#include <QDebug>
#include <QIcon>
#include <QMessageBox>
#include <QString>
#include <QTimer>

#include "Macros.h"
#include "io/ResourceUtils.h"
#include "mdl/Entity.h"
#include "mdl/EntityDefinition.h"
#include "mdl/EntityDefinitionManager.h"
#include "mdl/EntityDefinitionUtils.h"
#include "mdl/EntityNode.h"
#include "mdl/EntityNodeBase.h"
#include "mdl/EntityProperties.h"
#include "mdl/GroupNode.h"
#include "mdl/LayerNode.h"
#include "mdl/Map.h"
#include "mdl/Map_Entities.h"
#include "mdl/ModelUtils.h"
#include "mdl/PropertyDefinition.h"
#include "mdl/WorldNode.h"
#include "ui/MapDocument.h"
#include "ui/QtUtils.h"

#include "kdl/range_utils.h"
#include "kdl/reflection_impl.h"
#include "kdl/string_utils.h"

#include <fmt/format.h>

#include <algorithm>
#include <cassert>
#include <iterator>
#include <map>
#include <optional>
#include <string>
#include <unordered_set>
#include <vector>

#define MODEL_LOG(x)

namespace tb::ui
{
namespace
{

bool isPropertyKeyMutable(const mdl::Entity& entity, const std::string& key)
{
  assert(!mdl::isGroup(entity.classname(), entity.properties()));
  assert(!mdl::isLayer(entity.classname(), entity.properties()));

  if (mdl::isWorldspawn(entity.classname()))
  {
    return !(
      key == mdl::EntityPropertyKeys::Classname || key == mdl::EntityPropertyKeys::Mods
      || key == mdl::EntityPropertyKeys::EntityDefinitions
      || key == mdl::EntityPropertyKeys::Wad
      || key == mdl::EntityPropertyKeys::EnabledMaterialCollections
      || key == mdl::EntityPropertyKeys::SoftMapBounds
      || key == mdl::EntityPropertyKeys::LayerColor
      || key == mdl::EntityPropertyKeys::LayerLocked
      || key == mdl::EntityPropertyKeys::LayerHidden
      || key == mdl::EntityPropertyKeys::LayerOmitFromExport);
  }

  return true;
}

bool isPropertyValueMutable(const mdl::Entity& entity, const std::string& key)
{
  assert(!mdl::isGroup(entity.classname(), entity.properties()));
  assert(!mdl::isLayer(entity.classname(), entity.properties()));

  if (mdl::isWorldspawn(entity.classname()))
  {
    return !(
      key == mdl::EntityPropertyKeys::Classname || key == mdl::EntityPropertyKeys::Mods
      || key == mdl::EntityPropertyKeys::EntityDefinitions
      || key == mdl::EntityPropertyKeys::Wad
      || key == mdl::EntityPropertyKeys::SoftMapBounds
      || key == mdl::EntityPropertyKeys::LayerColor
      || key == mdl::EntityPropertyKeys::LayerLocked
      || key == mdl::EntityPropertyKeys::LayerHidden
      || key == mdl::EntityPropertyKeys::LayerOmitFromExport);
  }

  return true;
}

bool isPropertyProtectable(const mdl::EntityNodeBase& entityNode, const std::string& key)
{
  return mdl::findContainingGroup(&entityNode) && key != mdl::EntityPropertyKeys::Origin;
}

PropertyProtection isPropertyProtected(
  const mdl::EntityNodeBase& entityNode, const std::string& key)
{
  if (isPropertyProtectable(entityNode, key))
  {
    for (const auto& protectedKey : entityNode.entity().protectedProperties())
    {
      if (mdl::isNumberedProperty(protectedKey, key))
      {
        return PropertyProtection::Protected;
      }
    }
    return PropertyProtection::NotProtected;
  }
  return PropertyProtection::NotProtectable;
}

LinkType getLinkType(const mdl::EntityNodeBase& entityNode, const std::string& key)
{
  return mdl::isLinkSourceProperty(entityNode.entity().definition(), key)
           ? LinkType::Source
         : mdl::isLinkTargetProperty(entityNode.entity().definition(), key)
           ? LinkType::Target
           : LinkType::None;
}

PropertyRow rowForEntityNodes(
  const std::string& key, const std::vector<mdl::EntityNodeBase*>& nodes)
{
  ensure(!nodes.empty(), "rowForEntityNodes requries a non-empty node list");

  return std::accumulate(
    std::next(nodes.begin()),
    nodes.end(),
    PropertyRow{key, nodes.front()},
    [](PropertyRow lhs, const mdl::EntityNodeBase* rhs) {
      lhs.merge(rhs);
      return lhs;
    });
}

std::vector<std::string> allKeys(
  const std::vector<mdl::EntityNodeBase*>& nodes,
  const bool showDefaultRows,
  const bool showProtectedProperties)
{
  auto result = kdl::vector_set<std::string>{};
  for (const auto* node : nodes)
  {
    // Add explicitly set properties
    for (const auto& property : node->entity().properties())
    {
      result.insert(property.key());
    }

    // Add default properties from the entity definition
    if (showDefaultRows)
    {
      if (const auto* entityDefinition = node->entity().definition())
      {
        for (const auto& propertyDefinition : entityDefinition->propertyDefinitions)
        {
          result.insert(propertyDefinition.key);
        }
      }
    }
  }

  if (showProtectedProperties)
  {
    for (const auto* node : nodes)
    {
      const auto& protectedProperties = node->entity().protectedProperties();
      result.insert(std::begin(protectedProperties), std::end(protectedProperties));
    }
  }

  return result.release_data();
}

std::map<std::string, PropertyRow> rowsForEntityNodes(
  const std::vector<mdl::EntityNodeBase*>& nodes,
  const bool showDefaultRows,
  const bool showProtectedProperties)
{
  auto result = std::map<std::string, PropertyRow>{};
  for (const auto& key : allKeys(nodes, showDefaultRows, showProtectedProperties))
  {
    result[key] = rowForEntityNodes(key, nodes);
  }
  return result;
}

std::vector<std::string> getAllPropertyKeys(const mdl::Map& map)
{
  auto result = kdl::vector_set<std::string>{};
  auto addEntityKeys = [&](const auto& node) {
    const auto keys =
      node.entity().properties()
      | std::views::transform([](const auto& property) { return property.key(); });

    result.insert(keys.begin(), keys.end());
  };

  map.world()->accept(kdl::overload(
    [&](auto&& thisLambda, const mdl::WorldNode* worldNode) {
      addEntityKeys(*worldNode);
      worldNode->visitChildren(thisLambda);
    },
    [](auto&& thisLambda, const mdl::LayerNode* layerNode) {
      layerNode->visitChildren(thisLambda);
    },
    [](auto&& thisLambda, const mdl::GroupNode* groupNode) {
      groupNode->visitChildren(thisLambda);
    },
    [&](const mdl::EntityNode* entityNode) { addEntityKeys(*entityNode); },
    [](const mdl::BrushNode*) {},
    [](const mdl::PatchNode*) {}));

  // also add keys from all loaded entity definitions
  for (const auto& entityDefinition : map.entityDefinitionManager().definitions())
  {
    for (const auto& propertyDefinition : entityDefinition.propertyDefinitions)
    {
      result.insert(propertyDefinition.key);
    }
  }

  // remove the empty string
  result.erase("");
  return result.release_data();
}

std::vector<std::string> getAllValuesForPropertyKeys(
  const mdl::Map& map, const std::vector<std::string>& propertyKeys)
{
  auto result = kdl::vector_set<std::string>();
  for (const auto& key : propertyKeys)
  {
    for (const auto* entityNode :
         map.findNodes<mdl::EntityNodeBase>(fmt::format("{}%*", key)))
    {
      const auto values =
        entityNode->entity().numberedProperties(key)
        | std::views::transform([](const auto& property) { return property.value(); });

      result.insert(values.begin(), values.end());
    }
  }

  // remove the empty string
  result.erase("");
  return result.release_data();
}

std::vector<std::string> getAllClassnames(const mdl::Map& map)
{
  // start with currently used classnames
  auto result = getAllValuesForPropertyKeys(map, {mdl::EntityPropertyKeys::Classname});
  auto resultSet = kdl::wrap_set(result);

  // add keys from all loaded entity definitions
  for (const auto& entityDefinition : map.entityDefinitionManager().definitions())
  {
    resultSet.insert(entityDefinition.name);
  }

  // remove the empty string
  resultSet.erase("");
  return result;
}

template <typename... ValueType>
std::vector<std::string> getAllValuesForPropertyValueTypes(const mdl::Map& map)
{
  auto result = kdl::vector_set<std::string>();
  map.world()->accept(kdl::overload(
    [](auto&& thisLambda, const mdl::WorldNode* worldNode) {
      worldNode->visitChildren(thisLambda);
    },
    [](auto&& thisLambda, const mdl::LayerNode* layerNode) {
      layerNode->visitChildren(thisLambda);
    },
    [&](auto&& thisLambda, const mdl::GroupNode* groupNode) {
      groupNode->visitChildren(thisLambda);
    },
    [&](const mdl::EntityNode* entityNode) {
      if (const auto* entityDefinition = entityNode->entity().definition())
      {
        auto propertyValues =
          entityDefinition->propertyDefinitions
          | std::views::filter([](const auto& propertyDefinition) {
              return (
                std::holds_alternative<ValueType>(propertyDefinition.valueType) || ...);
            })
          | std::views::transform([&](const auto& propertyDefinition) {
              return entityNode->entity().property(propertyDefinition.key);
            })
          | std::views::filter(
            [](const auto* propertyValue) { return propertyValue != nullptr; })
          | std::views::transform(
            [](const auto& propertyValue) { return *propertyValue; });

        result.insert(propertyValues.begin(), propertyValues.end());
      }
    },
    [&](const mdl::BrushNode*) {},
    [&](const mdl::PatchNode*) {}));

  // remove the empty string
  result.erase("");
  return result.release_data();
}

bool computeShouldShowProtectedProperties(
  const std::vector<mdl::EntityNodeBase*>& entityNodes)
{
  return !entityNodes.empty()
         && std::ranges::all_of(entityNodes, [](const auto* entityNode) {
              return mdl::findContainingGroup(entityNode);
            });
}

} // namespace

std::ostream& operator<<(std::ostream& lhs, const ValueState& rhs)
{
  switch (rhs)
  {
  case ValueState::Unset:
    return lhs << "Unset";
  case ValueState::SingleValue:
    return lhs << "SingleValue";
  case ValueState::SingleValueAndUnset:
    return lhs << "SingleValueAndUnset";
  case ValueState::MultipleValues:
    return lhs << "MultipleValues";
    switchDefault();
  }
}

std::ostream& operator<<(std::ostream& lhs, const LinkType& rhs)
{
  switch (rhs)
  {
  case LinkType::Source:
    return lhs << "Source";
  case LinkType::Target:
    return lhs << "Target";
  case LinkType::None:
    return lhs << "None";
    switchDefault();
  }
}

std::ostream& operator<<(std::ostream& lhs, const PropertyProtection& rhs)
{
  switch (rhs)
  {
  case PropertyProtection::NotProtectable:
    return lhs << "NotProtectable";
  case PropertyProtection::Protected:
    return lhs << "Protected";
  case PropertyProtection::NotProtected:
    return lhs << "NotProtected";
  case PropertyProtection::Mixed:
    return lhs << "Mixed";
    switchDefault();
  }
}

std::string newPropertyKeyForEntityNodes(const std::vector<mdl::EntityNodeBase*>& nodes)
{
  const auto rows = rowsForEntityNodes(nodes, true, false);

  for (int i = 1;; ++i)
  {
    const auto newKey = kdl::str_to_string("property ", i);
    if (rows.find(newKey) == rows.end())
    {
      return newKey;
    }
  }
  // unreachable
}

PropertyRow::PropertyRow()
  : m_valueState{ValueState::Unset}
  , m_keyMutable{true}
  , m_valueMutable{true}
  , m_protected{PropertyProtection::NotProtectable}
  , m_linkType{LinkType::None}
{
}

PropertyRow::PropertyRow(std::string key, const mdl::EntityNodeBase* node)
  : m_key{std::move(key)}
{
  const auto* definition = mdl::propertyDefinition(node, m_key);

  if (const auto* value = node->entity().property(m_key))
  {
    m_value = *value;
    m_valueState = ValueState::SingleValue;
  }
  else if (definition)
  {
    m_value = mdl::PropertyDefinition::defaultValue(*definition).value_or("");
    m_valueState = ValueState::Unset;
  }
  else
  {
    // this is the case when the key is coming from another entity
    m_valueState = ValueState::Unset;
  }

  m_keyMutable = isPropertyKeyMutable(node->entity(), m_key);
  m_valueMutable = isPropertyValueMutable(node->entity(), m_key);
  m_protected = isPropertyProtected(*node, m_key);
  m_linkType = getLinkType(*node, m_key);
  m_tooltip = (definition ? definition->shortDescription : "");
  if (m_tooltip.empty())
  {
    m_tooltip = "No description found";
  }
}

void PropertyRow::merge(const mdl::EntityNodeBase* other)
{
  const auto* otherValue = other->entity().property(m_key);

  // State transitions
  if (m_valueState == ValueState::Unset)
  {
    if (otherValue)
    {
      m_valueState = ValueState::SingleValueAndUnset;
      m_value = *otherValue;
    }
  }
  else if (m_valueState == ValueState::SingleValue)
  {
    if (!otherValue)
    {
      m_valueState = ValueState::SingleValueAndUnset;
    }
    else if (*otherValue != m_value)
    {
      m_valueState = ValueState::MultipleValues;
    }
  }
  else if (m_valueState == ValueState::SingleValueAndUnset)
  {
    if (otherValue && *otherValue != m_value)
    {
      m_valueState = ValueState::MultipleValues;
    }
  }

  m_keyMutable = (m_keyMutable && isPropertyKeyMutable(other->entity(), m_key));
  m_valueMutable = (m_valueMutable && isPropertyValueMutable(other->entity(), m_key));

  const auto otherProtected = isPropertyProtected(*other, m_key);
  if (m_protected != otherProtected)
  {
    if (
      m_protected == PropertyProtection::NotProtectable
      || otherProtected == PropertyProtection::NotProtectable)
    {
      m_protected = PropertyProtection::NotProtectable;
    }
    else
    {
      m_protected = PropertyProtection::Mixed;
    }
  }

  if (m_linkType == LinkType::None)
  {
    m_linkType = getLinkType(*other, m_key);
  }
}

const std::string& PropertyRow::key() const
{
  return m_key;
}

std::string PropertyRow::value() const
{
  if (m_valueState == ValueState::MultipleValues)
  {
    return "multi";
  }
  return m_value;
}

bool PropertyRow::keyMutable() const
{
  return m_keyMutable;
}

bool PropertyRow::valueMutable() const
{
  return m_valueMutable;
}

PropertyProtection PropertyRow::isProtected() const
{
  return m_protected;
}

LinkType PropertyRow::linkType() const
{
  return m_linkType;
}

const std::string& PropertyRow::tooltip() const
{
  return m_tooltip;
}

bool PropertyRow::isDefault() const
{
  return m_valueState == ValueState::Unset;
}

bool PropertyRow::multi() const
{
  return m_valueState == ValueState::MultipleValues;
}

bool PropertyRow::subset() const
{
  return m_valueState == ValueState::SingleValueAndUnset;
}

kdl_reflect_impl(PropertyRow);

// EntityPropertyModel

EntityPropertyModel::EntityPropertyModel(mdl::Map& map, QObject* parent)
  : QAbstractTableModel{parent}
  , m_showDefaultRows{true}
  , m_shouldShowProtectedProperties{false}
  , m_map{map}
{
  updateFromMapDocument();
}

static auto makeKeyToPropertyRowMap(const std::vector<PropertyRow>& rows)
{
  auto result = std::map<std::string, PropertyRow>{};
  for (const auto& row : rows)
  {
    result[row.key()] = row;
  }
  return result;
}

struct KeyDiff
{
  std::vector<std::string> removed;
  std::vector<std::string> added;
  std::vector<std::string> updated;
  std::vector<std::string> unchanged;
};

static KeyDiff comparePropertyMaps(
  const std::map<std::string, PropertyRow>& oldRows,
  const std::map<std::string, PropertyRow>& newRows)
{
  auto result = KeyDiff{};
  result.removed.reserve(oldRows.size());
  result.added.reserve(newRows.size());
  result.updated.reserve(newRows.size());
  result.unchanged.reserve(newRows.size());

  for (const auto& [key, value] : oldRows)
  {
    if (auto it = newRows.find(key); it != std::end(newRows))
    {
      if (it->second == value)
      {
        result.unchanged.push_back(key);
      }
      else
      {
        result.updated.push_back(key);
      }
    }
    else
    {
      result.removed.push_back(key);
    }
  }
  for (const auto& [key, value] : newRows)
  {
    unused(value);
    if (oldRows.find(key) == std::end(oldRows))
    {
      result.added.push_back(key);
    }
  }
  return result;
}

bool EntityPropertyModel::showDefaultRows() const
{
  return m_showDefaultRows;
}

void EntityPropertyModel::setShowDefaultRows(const bool showDefaultRows)
{
  if (showDefaultRows == m_showDefaultRows)
  {
    return;
  }
  m_showDefaultRows = showDefaultRows;
  updateFromMapDocument();
}

bool EntityPropertyModel::shouldShowProtectedProperties() const
{
  return m_shouldShowProtectedProperties;
}

void EntityPropertyModel::setRows(const std::map<std::string, PropertyRow>& newRowMap)
{
  const auto oldRowMap = makeKeyToPropertyRowMap(m_rows);

  if (newRowMap == oldRowMap)
  {
    MODEL_LOG(qDebug() << "EntityPropertyModel::setRows: no change");
    return;
  }

  const auto diff = comparePropertyMaps(oldRowMap, newRowMap);

  // If exactly one row was changed
  // we can tell Qt the row was edited instead. This allows the selection/current
  // index to be preserved, whereas removing the row would invalidate the current
  // index.
  //
  // This situation happens when you rename a key and then press Tab to switch
  // to editing the value for the newly renamed key.

  if (diff.removed.size() == 1 && diff.added.size() == 1 && diff.updated.empty())
  {
    const auto& oldDeletion = oldRowMap.at(diff.removed[0]);
    const auto& newAddition = newRowMap.at(diff.added[0]);

    MODEL_LOG(
      qDebug() << "EntityPropertyModel::setRows: one row changed: "
               << mapStringToUnicode(m_map.encoding(), oldDeletion.key()) << " -> "
               << mapStringToUnicode(m_map.encoding(), newAddition.key()));

    const auto oldIndex = kdl::index_of(m_rows, oldDeletion);
    ensure(oldIndex, "deleted row must be found");

    m_rows.at(*oldIndex) = newAddition;

    // Notify Qt
    const auto topLeft = index(static_cast<int>(*oldIndex), 0);
    const auto bottomRight = index(static_cast<int>(*oldIndex), NumColumns - 1);
    emit dataChanged(topLeft, bottomRight);
    return;
  }

  // Handle edited rows

  MODEL_LOG(
    qDebug() << "EntityPropertyModel::setRows: " << diff.updated.size()
             << " common keys");
  for (const auto& key : diff.updated)
  {
    const auto& oldRow = oldRowMap.at(key);
    const auto& newRow = newRowMap.at(key);
    const auto oldIndex = kdl::index_of(m_rows, oldRow);

    MODEL_LOG(
      qDebug() << "   updating row " << *oldIndex << "(" << QString::fromStdString(key)
               << ")");

    m_rows.at(*oldIndex) = newRow;

    // Notify Qt
    const auto topLeft = index(static_cast<int>(*oldIndex), 0);
    const auto bottomRight = index(static_cast<int>(*oldIndex), NumColumns - 1);
    emit dataChanged(topLeft, bottomRight);
  }

  // Insertions
  if (!diff.added.empty())
  {
    MODEL_LOG(
      qDebug() << "EntityPropertyModel::setRows: inserting " << diff.added.size()
               << " rows");

    const auto firstNewRow = static_cast<int>(m_rows.size());
    const auto lastNewRow = firstNewRow + static_cast<int>(diff.added.size()) - 1;
    assert(lastNewRow >= firstNewRow);

    beginInsertRows(QModelIndex(), firstNewRow, lastNewRow);
    for (const auto& key : diff.added)
    {
      const auto& row = newRowMap.at(key);
      m_rows.push_back(row);
    }
    endInsertRows();
  }

  // Deletions
  if (!diff.removed.empty())
  {
    MODEL_LOG(
      qDebug() << "EntityPropertyModel::setRows: deleting " << diff.removed.size()
               << " rows");

    for (const auto& key : diff.removed)
    {
      const auto& row = oldRowMap.at(key);
      const auto index = kdl::index_of(m_rows, row);
      assert(index);

      beginRemoveRows(QModelIndex{}, static_cast<int>(*index), static_cast<int>(*index));
      m_rows.erase(std::next(m_rows.begin(), static_cast<int>(*index)));
      endRemoveRows();
    }
  }
}

const PropertyRow* EntityPropertyModel::dataForModelIndex(const QModelIndex& index) const
{
  if (!index.isValid())
  {
    return nullptr;
  }
  return &m_rows.at(static_cast<size_t>(index.row()));
}

int EntityPropertyModel::rowForPropertyKey(const std::string& propertyKey) const
{
  const auto it = std::ranges::find_if(
    m_rows, [&](const auto& row) { return row.key() == propertyKey; });
  return it != m_rows.end() ? static_cast<int>(std::distance(m_rows.begin(), it)) : -1;
}

QStringList EntityPropertyModel::getCompletions(const QModelIndex& index) const
{
  if (index.row() < 0 || index.row() >= static_cast<int>(m_rows.size()))
  {
    return {};
  }

  const auto& row = m_rows[static_cast<size_t>(index.row())];
  auto result = std::vector<std::string>{};
  if (index.column() == ColumnKey)
  {
    result = getAllPropertyKeys(m_map);
  }
  else if (index.column() == ColumnValue)
  {
    switch (row.linkType())
    {
    case LinkType::Source:
      result =
        getAllValuesForPropertyValueTypes<mdl::PropertyValueTypes::LinkTarget>(m_map);
      break;
    case LinkType::Target:
      result =
        getAllValuesForPropertyValueTypes<mdl::PropertyValueTypes::LinkSource>(m_map);
      break;
    case LinkType::None:
      if (row.key() == mdl::EntityPropertyKeys::Classname)
      {
        result = getAllClassnames(m_map);
      }
      break;
    }
  }

  return toQStringList(std::begin(result), std::end(result));
}

std::string EntityPropertyModel::propertyKey(const int row) const
{
  if (row < 0 || row >= static_cast<int>(m_rows.size()))
  {
    return "";
  }
  else
  {
    return m_rows[static_cast<size_t>(row)].key();
  }
}

std::vector<std::string> EntityPropertyModel::propertyKeys(
  const int row, const int count) const
{
  auto result = std::vector<std::string>{};
  result.reserve(static_cast<std::size_t>(count));

  for (int i = 0; i < count; ++i)
  {
    result.push_back(this->propertyKey(row + i));
  }
  return result;
}

void EntityPropertyModel::updateFromMapDocument()
{
  MODEL_LOG(qDebug() << "updateFromMapDocument");

  const auto entityNodes = m_map.selection().allEntities();
  const auto rowsMap = rowsForEntityNodes(entityNodes, m_showDefaultRows, true);

  setRows(rowsMap);
  m_shouldShowProtectedProperties = computeShouldShowProtectedProperties(entityNodes);
}

int EntityPropertyModel::rowCount(const QModelIndex& parent) const
{
  if (parent.isValid())
  {
    return 0;
  }
  return static_cast<int>(m_rows.size());
}

int EntityPropertyModel::columnCount(const QModelIndex& parent) const
{
  if (parent.isValid())
  {
    return 0;
  }

  return NumColumns;
}

Qt::ItemFlags EntityPropertyModel::flags(const QModelIndex& index) const
{
  if (!index.isValid())
  {
    return Qt::NoItemFlags;
  }

  const PropertyRow& row = m_rows.at(static_cast<size_t>(index.row()));

  auto flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;

  if (index.column() == ColumnProtected)
  {
    if (row.isProtected() != PropertyProtection::NotProtectable)
    {
      flags |= Qt::ItemIsUserCheckable;
    }
  }
  else if (index.column() == ColumnKey)
  {
    if (row.keyMutable())
    {
      flags |= Qt::ItemIsEditable;
    }
  }
  else if (index.column() == ColumnValue)
  {
    if (row.valueMutable())
    {
      flags |= Qt::ItemIsEditable;
    }
  }

  return flags;
}

QVariant EntityPropertyModel::data(const QModelIndex& index, const int role) const
{
  if (
    !index.isValid() || index.row() < 0 || index.row() >= static_cast<int>(m_rows.size())
    || index.column() < 0 || index.column() >= NumColumns)
  {
    return QVariant{};
  }

  const auto& row = m_rows.at(static_cast<size_t>(index.row()));

  if (role == Qt::DecorationRole)
  {
    // lock icon
    if (index.column() == ColumnKey)
    {
      if (!row.keyMutable())
      {
        return QVariant{io::loadSVGIcon("Locked_small.svg")};
      }
    }
    else if (index.column() == ColumnValue)
    {
      if (!row.valueMutable())
      {
        return QVariant{io::loadSVGIcon("Locked_small.svg")};
      }
    }
    return {};
  }

  if (role == Qt::ForegroundRole)
  {
    const auto disabledCellText = QPalette{}.color(QPalette::Disabled, QPalette::Text);
    if (row.isDefault() || row.subset())
    {
      return QVariant{QBrush{disabledCellText}};
    }
    if (index.column() == ColumnValue)
    {
      if (row.multi())
      {
        return QVariant{QBrush{disabledCellText}};
      }
    }
    return {};
  }

  if (role == Qt::FontRole)
  {
    if (row.isDefault())
    {
      auto italicFont = QFont{};
      italicFont.setItalic(true);
      return QVariant(italicFont);
    }
    if (index.column() == ColumnValue)
    {
      if (row.multi())
      {
        auto italicFont = QFont{};
        italicFont.setItalic(true);
        return QVariant(italicFont);
      }
    }
    return {};
  }

  if (role == Qt::DisplayRole || role == Qt::EditRole)
  {
    if (index.column() == ColumnKey)
    {
      return QVariant{mapStringToUnicode(m_map.encoding(), row.key())};
    }
    else if (index.column() == ColumnValue)
    {
      return QVariant{mapStringToUnicode(m_map.encoding(), row.value())};
    }
  }

  if (role == Qt::CheckStateRole)
  {
    if (index.column() == ColumnProtected)
    {
      if (row.isProtected() == PropertyProtection::Protected)
      {
        return QVariant{Qt::CheckState::Checked};
      }
      else if (row.isProtected() == PropertyProtection::Mixed)
      {
        return QVariant{Qt::CheckState::PartiallyChecked};
      }
      else
      {
        return QVariant{Qt::CheckState::Unchecked};
      }
    }
  }

  if (role == Qt::ToolTipRole)
  {
    if (index.column() == ColumnProtected)
    {
      return QVariant{"Property is protected from changes in linked groups if checked"};
    }
    else
    {
      if (!row.tooltip().empty())
      {
        return QVariant{mapStringToUnicode(m_map.encoding(), row.tooltip())};
      }
    }
  }

  return QVariant{};
}

bool EntityPropertyModel::setData(
  const QModelIndex& index, const QVariant& value, const int role)
{
  const auto& propertyRow = m_rows.at(static_cast<size_t>(index.row()));
  unused(propertyRow);

  if (role != Qt::EditRole && role != Qt::CheckStateRole)
  {
    return false;
  }

  const auto rowIndex = static_cast<size_t>(index.row());
  const auto nodes = m_map.selection().allEntities();
  if (nodes.empty())
  {
    return false;
  }

  if (index.column() == ColumnKey && role == Qt::EditRole)
  {
    // rename key
    MODEL_LOG(
      qDebug() << "tried to rename "
               << mapStringToUnicode(m_map.encoding(), propertyRow.key()) << " to "
               << value.toString());

    const auto newName = mapStringFromUnicode(m_map.encoding(), value.toString());
    if (renameProperty(rowIndex, newName, nodes))
    {
      return true;
    }
  }
  else if (index.column() == ColumnValue && role == Qt::EditRole)
  {
    MODEL_LOG(
      qDebug() << "tried to set "
               << mapStringToUnicode(m_map.encoding(), propertyRow.key()) << " to "
               << value.toString());

    if (updateProperty(
          rowIndex, mapStringFromUnicode(m_map.encoding(), value.toString()), nodes))
    {
      return true;
    }
  }
  else if (index.column() == ColumnProtected && role == Qt::CheckStateRole)
  {
    if (value == Qt::CheckState::Checked)
    {
      MODEL_LOG(
        qDebug() << "tried to set "
                 << mapStringToUnicode(map.encoding(), propertyRow.key())
                 << " to protected");
      setProtectedProperty(rowIndex, true);
    }
    else
    {
      MODEL_LOG(
        qDebug() << "tried to set "
                 << mapStringToUnicode(map.encoding(), propertyRow.key())
                 << " to non protected");
      setProtectedProperty(rowIndex, false);
    }
  }

  return false;
}

QVariant EntityPropertyModel::headerData(
  const int section, const Qt::Orientation orientation, const int role) const
{
  if (role == Qt::DisplayRole)
  {
    if (orientation == Qt::Horizontal)
    {
      if (section == ColumnKey)
      {
        return QVariant{tr("Key")};
      }
      else if (section == ColumnValue)
      {
        return QVariant{tr("Value")};
      }
    }
  }
  else if (role == Qt::DecorationRole)
  {
    if (section == ColumnProtected)
    {
      return QVariant{io::loadSVGIcon("Protected_small.svg")};
    }
  }
  else if (role == Qt::ToolTipRole)
  {
    if (section == ColumnProtected)
    {
      return QVariant{tr("Protect properties from changes in linked groups")};
    }
  }

  return QVariant{};
}

bool EntityPropertyModel::canRemove(const int rowIndexInt)
{
  if (rowIndexInt < 0 || static_cast<size_t>(rowIndexInt) >= m_rows.size())
  {
    return false;
  }

  const auto& row = m_rows.at(static_cast<size_t>(rowIndexInt));
  if (row.isDefault())
  {
    return false;
  }
  return row.keyMutable() && row.valueMutable();
}

bool EntityPropertyModel::hasRowWithPropertyKey(const std::string& propertyKey) const
{
  return rowForPropertyKey(propertyKey) != -1;
}

bool EntityPropertyModel::renameProperty(
  const size_t rowIndex,
  const std::string& newKey,
  const std::vector<mdl::EntityNodeBase*>& /* nodes */)
{
  ensure(rowIndex < m_rows.size(), "row index out of bounds");

  const auto& row = m_rows.at(rowIndex);
  const auto& oldKey = row.key();

  if (oldKey == newKey)
  {
    return true;
  }

  ensure(
    row.keyMutable(),
    "tried to rename immutable name"); // EntityPropertyModel::flags prevents
                                       // us from renaming immutable names

  if (hasRowWithPropertyKey(newKey))
  {
    const auto& rowToOverwrite =
      m_rows.at(static_cast<size_t>(rowForPropertyKey(newKey)));
    if (!rowToOverwrite.valueMutable())
    {
      // Prevent changing an immutable value via a rename
      // TODO: would this be better checked inside MapDocument::renameProperty?
      return false;
    }

    auto msgBox = QMessageBox{};
    msgBox.setWindowTitle(tr("Error"));
    msgBox.setText(
      tr("A property with key '%1' already exists.\n\n Do you wish to overwrite it?")
        .arg(mapStringToUnicode(m_map.encoding(), newKey)));
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    if (msgBox.exec() == QMessageBox::No)
    {
      return false;
    }
  }

  return renameEntityProperty(m_map, oldKey, newKey);
}

bool EntityPropertyModel::updateProperty(
  const size_t rowIndex,
  const std::string& newValue,
  const std::vector<mdl::EntityNodeBase*>& nodes)
{
  ensure(rowIndex < m_rows.size(), "row index out of bounds");

  auto hasChange = false;
  const auto& key = m_rows.at(rowIndex).key();
  for (const auto* node : nodes)
  {
    if (const auto* oldValue = node->entity().property(key))
    {
      ensure(
        isPropertyValueMutable(node->entity(), key),
        "tried to modify immutable property value"); // this should be guaranteed by
                                                     // the PropertyRow constructor
      if (*oldValue != newValue)
      {
        hasChange = true;
      }
    }
    else
    {
      hasChange = true;
    }
  }

  if (!hasChange)
  {
    return true;
  }

  return setEntityProperty(m_map, key, newValue);
}

bool EntityPropertyModel::setProtectedProperty(const size_t rowIndex, const bool newValue)
{
  ensure(rowIndex < m_rows.size(), "row index out of bounds");

  const auto& key = m_rows.at(rowIndex).key();
  return setProtectedEntityProperty(m_map, key, newValue);
}

bool EntityPropertyModel::lessThan(const size_t rowIndexA, const size_t rowIndexB) const
{
  const auto& rowA = m_rows.at(rowIndexA);
  const auto& rowB = m_rows.at(rowIndexB);

  // 1. non-default sorts before default
  if (!rowA.isDefault() && rowB.isDefault())
  {
    return true;
  }
  if (rowA.isDefault() && !rowB.isDefault())
  {
    return false;
  }

  // 2. sort by name
  return rowA.key() < rowB.key();
}

} // namespace tb::ui
