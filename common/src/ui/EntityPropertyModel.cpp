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

#include "kd/contracts.h"
#include "kd/range_utils.h"
#include "kd/reflection_impl.h"
#include "kd/string_utils.h"

#include <fmt/format.h>

#include <algorithm>
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

bool isPropertyReadOnly(const mdl::Entity& entity, const std::string& key)
{
  if (const auto* entityDefinition = entity.definition())
  {
    if (const auto iPropertyDefinition = std::ranges::find_if(
          entityDefinition->propertyDefinitions,
          [&](const auto& propertyDefinition) { return propertyDefinition.key == key; });
        iPropertyDefinition != entityDefinition->propertyDefinitions.end())
    {
      return iPropertyDefinition->readOnly;
    }
  }

  return false;
}

bool isPropertyKeyMutable(const mdl::Entity& entity, const std::string& key)
{
  contract_pre(!mdl::isGroup(entity.classname(), entity.properties()));
  contract_pre(!mdl::isLayer(entity.classname(), entity.properties()));

  if (isPropertyReadOnly(entity, key))
  {
    return false;
  }

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
  contract_pre(!mdl::isGroup(entity.classname(), entity.properties()));
  contract_pre(!mdl::isLayer(entity.classname(), entity.properties()));

  if (isPropertyReadOnly(entity, key))
  {
    return false;
  }

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

PropertyProtection getPropertyProtection(
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

LinkType getLinkType(const mdl::Entity& entity, const std::string& key)
{
  return mdl::isLinkSourceProperty(entity.definition(), key)   ? LinkType::Source
         : mdl::isLinkTargetProperty(entity.definition(), key) ? LinkType::Target
                                                               : LinkType::None;
}

PropertyRow makeRow(std::string key, const mdl::EntityNodeBase& entityNode)
{
  auto row = PropertyRow{};
  row.key = std::move(key);

  const auto& entity = entityNode.entity();
  const auto* definition = mdl::propertyDefinition(&entityNode, row.key);

  if (const auto* value = entity.property(row.key))
  {
    row.value = *value;
    row.valueState = ValueState::SingleValue;
  }
  else if (definition)
  {
    row.value = mdl::PropertyDefinition::defaultValue(*definition).value_or("");
  }

  row.keyMutable = isPropertyKeyMutable(entity, row.key);
  row.valueMutable = isPropertyValueMutable(entity, row.key);
  row.protection = getPropertyProtection(entityNode, row.key);
  row.linkType = getLinkType(entity, row.key);
  row.tooltip = definition ? definition->shortDescription : "No description found";

  return row;
}

PropertyRow mergeRows(PropertyRow row, const mdl::EntityNodeBase& entityNode)
{
  const auto& entity = entityNode.entity();
  const auto* value = entity.property(row.key);

  // State transitions
  if (row.valueState == ValueState::Unset)
  {
    if (value)
    {
      row.valueState = ValueState::SingleValueAndUnset;
      row.value = *value;
    }
  }
  else if (row.valueState == ValueState::SingleValue)
  {
    if (!value)
    {
      row.valueState = ValueState::SingleValueAndUnset;
    }
    else if (*value != row.value)
    {
      row.value = "multi";
      row.valueState = ValueState::MultipleValues;
    }
  }
  else if (row.valueState == ValueState::SingleValueAndUnset)
  {
    if (value && *value != row.value)
    {
      row.value = "multi";
      row.valueState = ValueState::MultipleValues;
    }
  }

  row.keyMutable = row.keyMutable && isPropertyKeyMutable(entity, row.key);
  row.valueMutable = row.valueMutable && isPropertyValueMutable(entity, row.key);

  const auto protection = getPropertyProtection(entityNode, row.key);
  if (row.protection != protection)
  {
    if (
      row.protection == PropertyProtection::NotProtectable
      || protection == PropertyProtection::NotProtectable)
    {
      row.protection = PropertyProtection::NotProtectable;
    }
    else
    {
      row.protection = PropertyProtection::Mixed;
    }
  }

  if (row.linkType == LinkType::None)
  {
    row.linkType = getLinkType(entity, row.key);
  }

  return row;
}

PropertyRow makeRow(std::string key, const std::vector<mdl::EntityNodeBase*>& entityNodes)
{
  contract_pre(!entityNodes.empty());

  return std::accumulate(
    std::next(entityNodes.begin()),
    entityNodes.end(),
    makeRow(std::move(key), *entityNodes.front()),
    [](auto lhs, const auto* rhs) { return mergeRows(std::move(lhs), *rhs); });
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

auto makeKeyToPropertyRowMap(const std::vector<PropertyRow>& rows)
{
  return rows
         | std::views::transform([](const auto& row) { return std::pair{row.key, row}; })
         | kdl::ranges::to<std::map>();
}

struct KeyDiff
{
  std::vector<std::string> removed;
  std::vector<std::string> added;
  std::vector<std::string> updated;
  std::vector<std::string> unchanged;
};

KeyDiff comparePropertyMaps(
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

std::map<std::string, PropertyRow> rowsForEntityNodes(
  const std::vector<mdl::EntityNodeBase*>& entityNodes,
  const bool showDefaultRows,
  const bool showProtectedProperties)
{
  auto result = std::map<std::string, PropertyRow>{};
  for (const auto& key : allKeys(entityNodes, showDefaultRows, showProtectedProperties))
  {
    result[key] = makeRow(key, entityNodes);
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

  map.world().accept(kdl::overload(
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
  map.world().accept(kdl::overload(
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

kdl_reflect_impl(PropertyRow);

// EntityPropertyModel

EntityPropertyModel::EntityPropertyModel(MapDocument& document, QObject* parent)
  : QAbstractTableModel{parent}
  , m_showDefaultRows{true}
  , m_shouldShowProtectedProperties{false}
  , m_document{document}
{
  updateFromMap();
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
  updateFromMap();
}

bool EntityPropertyModel::shouldShowProtectedProperties() const
{
  return m_shouldShowProtectedProperties;
}

const std::vector<PropertyRow>& EntityPropertyModel::rows() const
{
  return m_rows;
}

const PropertyRow* EntityPropertyModel::rowForModelIndex(const QModelIndex& index) const
{
  return index.isValid() ? &m_rows.at(static_cast<size_t>(index.row())) : nullptr;
}

int EntityPropertyModel::rowIndexForPropertyKey(const std::string& propertyKey) const
{
  const auto it =
    std::ranges::find_if(m_rows, [&](const auto& row) { return row.key == propertyKey; });
  return it != m_rows.end() ? static_cast<int>(std::distance(m_rows.begin(), it)) : -1;
}

QStringList EntityPropertyModel::getCompletions(const QModelIndex& index) const
{
  if (index.row() < 0 || index.row() >= static_cast<int>(m_rows.size()))
  {
    return {};
  }

  auto& map = m_document.map();

  const auto& row = m_rows[static_cast<size_t>(index.row())];
  auto result = std::vector<std::string>{};
  if (index.column() == ColumnKey)
  {
    result = getAllPropertyKeys(map);
  }
  else if (index.column() == ColumnValue)
  {
    switch (row.linkType)
    {
    case LinkType::Source:
      result =
        getAllValuesForPropertyValueTypes<mdl::PropertyValueTypes::LinkTarget>(map);
      break;
    case LinkType::Target:
      result =
        getAllValuesForPropertyValueTypes<mdl::PropertyValueTypes::LinkSource>(map);
      break;
    case LinkType::None:
      if (row.key == mdl::EntityPropertyKeys::Classname)
      {
        result = getAllClassnames(map);
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
    return m_rows[static_cast<size_t>(row)].key;
  }
}

void EntityPropertyModel::updateFromMap()
{
  MODEL_LOG(qDebug() << "updateFromMapDocument");

  const auto entityNodes = m_document.map().selection().allEntities();
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
    if (row.protection != PropertyProtection::NotProtectable)
    {
      flags |= Qt::ItemIsUserCheckable;
    }
  }
  else if (index.column() == ColumnKey)
  {
    if (row.keyMutable)
    {
      flags |= Qt::ItemIsEditable;
    }
  }
  else if (index.column() == ColumnValue)
  {
    if (row.valueMutable)
    {
      flags |= Qt::ItemIsEditable;
    }
  }

  return flags;
}

QVariant EntityPropertyModel::data(const QModelIndex& index, const int role) const
{
  auto& map = m_document.map();

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
      if (!row.keyMutable)
      {
        return QVariant{io::loadSVGIcon("Locked_small.svg")};
      }
    }
    else if (index.column() == ColumnValue)
    {
      if (!row.valueMutable)
      {
        return QVariant{io::loadSVGIcon("Locked_small.svg")};
      }
    }
    return {};
  }

  if (role == Qt::ForegroundRole)
  {
    const auto disabledCellText = QPalette{}.color(QPalette::Disabled, QPalette::Text);
    if (
      row.valueState == ValueState::Unset
      || row.valueState == ValueState::SingleValueAndUnset)
    {
      return QVariant{QBrush{disabledCellText}};
    }
    if (index.column() == ColumnValue)
    {
      if (row.valueState == ValueState::MultipleValues)
      {
        return QVariant{QBrush{disabledCellText}};
      }
    }
    return {};
  }

  if (role == Qt::FontRole)
  {
    if (row.valueState == ValueState::Unset)
    {
      auto italicFont = QFont{};
      italicFont.setItalic(true);
      return QVariant(italicFont);
    }
    if (index.column() == ColumnValue)
    {
      if (row.valueState == ValueState::MultipleValues)
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
      return QVariant{mapStringToUnicode(map.encoding(), row.key)};
    }
    else if (index.column() == ColumnValue)
    {
      return QVariant{mapStringToUnicode(map.encoding(), row.value)};
    }
  }

  if (role == Qt::CheckStateRole)
  {
    if (index.column() == ColumnProtected)
    {
      if (row.protection == PropertyProtection::Protected)
      {
        return QVariant{Qt::CheckState::Checked};
      }
      else if (row.protection == PropertyProtection::Mixed)
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
      if (!row.tooltip.empty())
      {
        return QVariant{mapStringToUnicode(map.encoding(), row.tooltip)};
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

  auto& map = m_document.map();

  const auto rowIndex = static_cast<size_t>(index.row());
  const auto nodes = map.selection().allEntities();
  if (nodes.empty())
  {
    return false;
  }

  if (index.column() == ColumnKey && role == Qt::EditRole)
  {
    // rename key
    MODEL_LOG(
      qDebug() << "tried to rename "
               << mapStringToUnicode(m_map.encoding(), propertyRow.key) << " to "
               << value.toString());

    const auto newName = mapStringFromUnicode(map.encoding(), value.toString());
    if (renameProperty(rowIndex, newName, nodes))
    {
      return true;
    }
  }
  else if (index.column() == ColumnValue && role == Qt::EditRole)
  {
    MODEL_LOG(
      qDebug() << "tried to set " << mapStringToUnicode(map.encoding(), propertyRow.key)
               << " to " << value.toString());

    if (updateProperty(
          rowIndex, mapStringFromUnicode(map.encoding(), value.toString()), nodes))
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
                 << mapStringToUnicode(m_map.encoding(), propertyRow.key)
                 << " to protected");
      setProtectedProperty(rowIndex, true);
    }
    else
    {
      MODEL_LOG(
        qDebug() << "tried to set "
                 << mapStringToUnicode(m_map.encoding(), propertyRow.key)
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
  if (row.valueState == ValueState::Unset)
  {
    return false;
  }
  return row.keyMutable && row.valueMutable;
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

void EntityPropertyModel::setRows(const std::map<std::string, PropertyRow>& newRowMap)
{
  const auto oldRowMap = makeKeyToPropertyRowMap(m_rows);

  if (newRowMap == oldRowMap)
  {
    MODEL_LOG(qDebug() << "EntityPropertyModel::setRows: no change");
    return;
  }

  const auto diff = comparePropertyMaps(oldRowMap, newRowMap);

  // If exactly one row was changed we can tell Qt the row was edited instead. This allows
  // the selection/current index to be preserved, whereas removing the row would
  // invalidate the current index.
  //
  // This situation happens when you rename a key and then press Tab to switch to editing
  // the value for the newly renamed key.

  if (diff.removed.size() == 1 && diff.added.size() == 1 && diff.updated.empty())
  {
    const auto& oldDeletion = oldRowMap.at(diff.removed[0]);
    const auto& newAddition = newRowMap.at(diff.added[0]);

    MODEL_LOG(
      qDebug() << "EntityPropertyModel::setRows: one row changed: "
               << mapStringToUnicode(m_map.encoding(), oldDeletion.key) << " -> "
               << mapStringToUnicode(m_map.encoding(), newAddition.key));

    const auto oldIndex = kdl::index_of(m_rows, oldDeletion);
    contract_assert(oldIndex != std::nullopt);

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
    contract_assert(lastNewRow >= firstNewRow);

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
      contract_assert(index);

      beginRemoveRows(QModelIndex{}, static_cast<int>(*index), static_cast<int>(*index));
      m_rows.erase(std::next(m_rows.begin(), static_cast<int>(*index)));
      endRemoveRows();
    }
  }
}

bool EntityPropertyModel::hasRowWithPropertyKey(const std::string& propertyKey) const
{
  return rowIndexForPropertyKey(propertyKey) != -1;
}

bool EntityPropertyModel::renameProperty(
  const size_t rowIndex,
  const std::string& newKey,
  const std::vector<mdl::EntityNodeBase*>& /* nodes */)
{
  contract_pre(rowIndex < m_rows.size());

  const auto& row = m_rows.at(rowIndex);
  const auto& oldKey = row.key;

  if (oldKey == newKey)
  {
    return true;
  }

  // EntityPropertyModel::flags prevents us from renaming immutable names
  contract_assert(row.keyMutable);

  auto& map = m_document.map();
  if (hasRowWithPropertyKey(newKey))
  {
    const auto& rowToOverwrite =
      m_rows.at(static_cast<size_t>(rowIndexForPropertyKey(newKey)));
    if (!rowToOverwrite.valueMutable)
    {
      // Prevent changing an immutable value via a rename
      // TODO: would this be better checked inside MapDocument::renameProperty?
      return false;
    }

    auto msgBox = QMessageBox{};
    msgBox.setWindowTitle(tr("Error"));
    msgBox.setText(
      tr("A property with key '%1' already exists.\n\n Do you wish to overwrite it?")
        .arg(mapStringToUnicode(map.encoding(), newKey)));
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    if (msgBox.exec() == QMessageBox::No)
    {
      return false;
    }
  }

  return renameEntityProperty(map, oldKey, newKey);
}

bool EntityPropertyModel::updateProperty(
  const size_t rowIndex,
  const std::string& newValue,
  const std::vector<mdl::EntityNodeBase*>& nodes)
{
  contract_pre(rowIndex < m_rows.size());

  auto hasChange = false;
  const auto& key = m_rows.at(rowIndex).key;
  for (const auto* node : nodes)
  {
    if (const auto* oldValue = node->entity().property(key))
    {
      // this should be guaranteed by the PropertyRow constructor
      contract_assert(isPropertyValueMutable(node->entity(), key));

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

  return setEntityProperty(m_document.map(), key, newValue);
}

bool EntityPropertyModel::setProtectedProperty(const size_t rowIndex, const bool newValue)
{
  contract_pre(rowIndex < m_rows.size());

  const auto& key = m_rows.at(rowIndex).key;
  return setProtectedEntityProperty(m_document.map(), key, newValue);
}

bool EntityPropertyModel::lessThan(const size_t rowIndexA, const size_t rowIndexB) const
{
  const auto& rowA = m_rows.at(rowIndexA);
  const auto& rowB = m_rows.at(rowIndexB);

  // 1. non-default sorts before default
  if (rowA.valueState != ValueState::Unset && rowB.valueState == ValueState::Unset)
  {
    return true;
  }
  if (rowA.valueState == ValueState::Unset && rowB.valueState != ValueState::Unset)
  {
    return false;
  }

  // 2. sort by name
  return rowA.key < rowB.key;
}

} // namespace tb::ui
