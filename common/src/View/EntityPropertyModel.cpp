/*
 Copyright (C) 2010-2017 Kristian Duske

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

#include "Macros.h"
#include "Assets/EntityDefinition.h"
#include "Assets/EntityDefinitionManager.h"
#include "Assets/PropertyDefinition.h"
#include "IO/ResourceUtils.h"
#include "Model/Entity.h"
#include "Model/EntityNodeBase.h"
#include "Model/EntityNodeIndex.h"
#include "Model/EntityProperties.h"
#include "Model/WorldNode.h"
#include "View/MapDocument.h"
#include "View/ViewConstants.h"
#include "View/QtUtils.h"

#include <kdl/map_utils.h>
#include <kdl/memory_utils.h>
#include <kdl/string_utils.h>
#include <kdl/vector_utils.h>
#include <kdl/vector_set.h>

#include <cassert>
#include <iterator>
#include <map>
#include <optional>
#include <string>
#include <vector>

#include <QBrush>
#include <QByteArray>
#include <QDebug>
#include <QIcon>
#include <QMessageBox>
#include <QString>
#include <QTimer>

#define MODEL_LOG(x)

namespace TrenchBroom {
    namespace View {
        // helper functions
        static bool isPropertyKeyMutable(const Model::Entity& entity, const std::string& key) {
            assert(!Model::isGroup(entity.classname(), entity.properties()));
            assert(!Model::isLayer(entity.classname(), entity.properties()));

            if (Model::isWorldspawn(entity.classname(), entity.properties())) {
                return !(key == Model::PropertyKeys::Classname
                    || key == Model::PropertyKeys::Mods
                    || key == Model::PropertyKeys::EntityDefinitions
                    || key == Model::PropertyKeys::Wad
                    || key == Model::PropertyKeys::Textures
                    || key == Model::PropertyKeys::SoftMapBounds
                    || key == Model::PropertyKeys::LayerColor
                    || key == Model::PropertyKeys::LayerLocked
                    || key == Model::PropertyKeys::LayerHidden
                    || key == Model::PropertyKeys::LayerOmitFromExport);
            }

            return true;
        }

        static bool isPropertyValueMutable(const Model::Entity& entity, const std::string& key) {
            assert(!Model::isGroup(entity.classname(), entity.properties()));
            assert(!Model::isLayer(entity.classname(), entity.properties()));

            if (Model::isWorldspawn(entity.classname(), entity.properties())) {
                return !(key == Model::PropertyKeys::Mods
                    || key == Model::PropertyKeys::EntityDefinitions
                    || key == Model::PropertyKeys::Wad
                    || key == Model::PropertyKeys::Textures
                    || key == Model::PropertyKeys::SoftMapBounds
                    || key == Model::PropertyKeys::LayerColor
                    || key == Model::PropertyKeys::LayerLocked
                    || key == Model::PropertyKeys::LayerHidden
                    || key == Model::PropertyKeys::LayerOmitFromExport);
            }

            return true;
        }

        // AttributeRow

        PropertyRow::PropertyRow() :
        m_valueType(ValueType::Unset),
        m_keyMutable(true),
        m_valueMutable(true) {}

        bool PropertyRow::operator==(const PropertyRow& other) const {
            return m_key == other.m_key
                && m_value == other.m_value
                && m_valueType == other.m_valueType
                && m_keyMutable == other.m_keyMutable
                && m_valueMutable == other.m_valueMutable
                   && m_tooltip == other.m_tooltip;
        }

        bool PropertyRow::operator<(const PropertyRow& other) const {
            if (m_key < other.m_key) return true;
            if (m_key > other.m_key) return false;

            if (m_value < other.m_value) return true;
            if (m_value > other.m_value) return false;

            if (m_valueType < other.m_valueType) return true;
            if (m_valueType > other.m_valueType) return false;

            if (m_keyMutable < other.m_keyMutable) return true;
            if (m_keyMutable > other.m_keyMutable) return false;

            if (m_valueMutable < other.m_valueMutable) return true;
            if (m_valueMutable > other.m_valueMutable) return false;

            if (m_tooltip < other.m_tooltip) return true;
            if (m_tooltip > other.m_tooltip) return false;

            return false;
        }

        PropertyRow::PropertyRow(const std::string& key, const Model::EntityNodeBase* node) :
            m_key(key) {
            const Assets::PropertyDefinition* definition = Model::propertyDefinition(node, key);

            if (const auto* value = node->entity().property(key)) {
                m_value = *value;
                m_valueType = ValueType::SingleValue;
            } else if (definition != nullptr) {
                m_value = Assets::PropertyDefinition::defaultValue(*definition);
                m_valueType = ValueType::Unset;
            } else {
                // this is the case when the key is coming from another entity
                m_valueType = ValueType::Unset;
            }

            m_keyMutable = isPropertyKeyMutable(node->entity(), key);
            m_valueMutable = isPropertyValueMutable(node->entity(), key);
            m_tooltip = (definition != nullptr ? definition->shortDescription() : "");
            if (m_tooltip.empty()) {
                m_tooltip = "No description found";
            }
        }

        void PropertyRow::merge(const Model::EntityNodeBase* other) {
            const auto* otherValue = other->entity().property(m_key);

            // State transitions
            if (m_valueType == ValueType::Unset) {
                if (otherValue) {
                     m_valueType = ValueType::SingleValueAndUnset;
                     m_value = *otherValue;
                }
            } else if (m_valueType == ValueType::SingleValue) {
                if (!otherValue) {
                    m_valueType = ValueType::SingleValueAndUnset;
                } else if (*otherValue != m_value) {
                    m_valueType = ValueType::MultipleValues;
                }
            } else if (m_valueType == ValueType::SingleValueAndUnset) {
                if (otherValue && *otherValue != m_value) {
                    m_valueType = ValueType::MultipleValues;
                }
            }

            m_keyMutable = (m_keyMutable && isPropertyKeyMutable(other->entity(), m_key));
            m_valueMutable = (m_valueMutable && isPropertyValueMutable(other->entity(), m_key));
        }

        const std::string& PropertyRow::key() const {
            return m_key;
        }

        std::string PropertyRow::value() const {
            if (m_valueType == ValueType::MultipleValues) {
                return "multi";
            }
            return m_value;
        }

        bool PropertyRow::keyMutable() const {
            return m_keyMutable;
        }

        bool PropertyRow::valueMutable() const {
            return m_valueMutable;
        }

        const std::string& PropertyRow::tooltip() const {
            return m_tooltip;
        }

        bool PropertyRow::isDefault() const {
            return m_valueType == ValueType::Unset;
        }

        bool PropertyRow::multi() const {
            return m_valueType == ValueType::MultipleValues;
        }

        bool PropertyRow::subset() const {
            return m_valueType == ValueType::SingleValueAndUnset;
        }

        PropertyRow PropertyRow::rowForEntityNodes(const std::string& key, const std::vector<Model::EntityNodeBase*>& nodes) {
            ensure(nodes.size() > 0, "rowForEntityNodes requries a non-empty node list");

            std::optional<PropertyRow> result;
            for (const Model::EntityNodeBase* node : nodes) {
                // this happens at startup when the world is still null
                if (node == nullptr) {
                    continue;
                }

                if (!result.has_value()) {
                    result = PropertyRow(key, node);
                }  else {
                    result->merge(node);
                }
            }

            assert(result.has_value());
            return result.value();
        }

        std::vector<std::string> PropertyRow::allKeys(const std::vector<Model::EntityNodeBase*>& nodes, const bool showDefaultRows) {
            kdl::vector_set<std::string> result;
            for (const Model::EntityNodeBase* node : nodes) {
                // this happens at startup when the world is still null
                if (node == nullptr) {
                    continue;
                }

                // Add explicitly set properties
                for (const Model::EntityProperty& property : node->entity().properties()) {
                    result.insert(property.key());
                }

                // Add default properties from the entity definition
                if (showDefaultRows) {
                    const Assets::EntityDefinition* entityDefinition = node->entity().definition();
                    if (entityDefinition != nullptr) {
                       for (auto propertyDefinition : entityDefinition->propertyDefinitions()) {
                           result.insert(propertyDefinition->key());
                       }
                    }
                }
            }
            return result.release_data();
        }

        std::map<std::string, PropertyRow> PropertyRow::rowsForEntityNodes(const std::vector<Model::EntityNodeBase*>& nodes, const bool showDefaultRows) {
            std::map<std::string, PropertyRow> result;
            for (const std::string& key : allKeys(nodes, showDefaultRows)) {
                result[key] = rowForEntityNodes(key, nodes);
            }
            return result;
        }

        std::string PropertyRow::newPropertyKeyForEntityNodes(const std::vector<Model::EntityNodeBase*>& nodes) {
            const std::map<std::string, PropertyRow> rows = rowsForEntityNodes(nodes, true);

            for (int i = 1; ; ++i) {
                const std::string newKey = kdl::str_to_string("property ", i);
                if (rows.find(newKey) == rows.end()) {
                    return newKey;
                }
            }
            // unreachable
        }

        // EntityPropertyModel

        EntityPropertyModel::EntityPropertyModel(std::weak_ptr<MapDocument> document, QObject* parent) :
        QAbstractTableModel(parent),
        m_showDefaultRows(true),
        m_document(std::move(document)) {
            updateFromMapDocument();
        }

        static auto makeKeyToPropertyRowMap(const std::vector<PropertyRow>& rows) {
            std::map<std::string, PropertyRow> result;
            for (const auto& row : rows) {
                result[row.key()] = row;
            }
            return result;
        }

        using PropertyRowMap = std::map<std::string, PropertyRow>;

        struct KeyDiff {
            std::vector<std::string> removed;
            std::vector<std::string> added;
            std::vector<std::string> updated;
            std::vector<std::string> unchanged;
        };

        static KeyDiff comparePropertyMaps(const PropertyRowMap& oldRows, const PropertyRowMap& newRows) {
            KeyDiff result;
            result.removed.reserve(oldRows.size());
            result.added.reserve(newRows.size());
            result.updated.reserve(newRows.size());
            result.unchanged.reserve(newRows.size());

            for (const auto& [key, value] : oldRows) {
                if (auto it = newRows.find(key); it != std::end(newRows)) {
                    if (it->second == value) {
                        result.unchanged.push_back(key);
                    } else {
                        result.updated.push_back(key);
                    }
                } else {
                    result.removed.push_back(key);
                }
            }
            for (const auto& [key, value] : newRows) {
                unused(value);
                if (oldRows.find(key) == std::end(oldRows)) {
                    result.added.push_back(key);
                }
            }
            return result;
        }

        bool EntityPropertyModel::showDefaultRows() const {
            return m_showDefaultRows;
        }

        void EntityPropertyModel::setShowDefaultRows(const bool showDefaultRows) {
            if (showDefaultRows == m_showDefaultRows) {
                return;
            }
            m_showDefaultRows = showDefaultRows;
            updateFromMapDocument();
        }

        void EntityPropertyModel::setRows(const std::map<std::string, PropertyRow>& newRowMap) {
            auto document = kdl::mem_lock(m_document);
            const auto oldRowMap = makeKeyToPropertyRowMap(m_rows);

            if (newRowMap == oldRowMap) {
                MODEL_LOG(qDebug() << "EntityPropertyModel::setRows: no change");
                return;
            }

            const KeyDiff diff = comparePropertyMaps(oldRowMap, newRowMap);

            // If exactly one row was changed
            // we can tell Qt the row was edited instead. This allows the selection/current index
            // to be preserved, whereas removing the row would invalidate the current index.
            //
            // This situation happens when you rename a key and then press Tab to switch
            // to editing the value for the newly renamed key.

            if (diff.removed.size() == 1 && diff.added.size() == 1 && diff.updated.empty()) {
                const PropertyRow& oldDeletion = oldRowMap.at(diff.removed[0]);
                const PropertyRow& newAddition = newRowMap.at(diff.added[0]);

                MODEL_LOG(qDebug() << "EntityPropertyModel::setRows: one row changed: " << mapStringToUnicode(document->encoding(),
                    oldDeletion.key()) << " -> " << mapStringToUnicode(document->encoding(),
                    newAddition.key()));

                const auto oldIndex = kdl::vec_index_of(m_rows, oldDeletion);
                ensure(oldIndex, "deleted row must be found");

                m_rows.at(*oldIndex) = newAddition;

                // Notify Qt
                const QModelIndex topLeft = index(static_cast<int>(*oldIndex), 0);
                const QModelIndex bottomRight = index(static_cast<int>(*oldIndex), 1);
                emit dataChanged(topLeft, bottomRight);
                return;
            }

            // Handle edited rows

            MODEL_LOG(qDebug() << "EntityPropertyModel::setRows: " << diff.updated.size() << " common keys");
            for (const auto& key : diff.updated) {
                const PropertyRow& oldRow = oldRowMap.at(key);
                const PropertyRow& newRow = newRowMap.at(key);
                const auto oldIndex = kdl::vec_index_of(m_rows, oldRow);

                MODEL_LOG(qDebug() << "   updating row " << *oldIndex << "(" << QString::fromStdString(key) << ")");

                m_rows.at(*oldIndex) = newRow;

                // Notify Qt
                const QModelIndex topLeft = index(static_cast<int>(*oldIndex), 0);
                const QModelIndex bottomRight = index(static_cast<int>(*oldIndex), 1);
                emit dataChanged(topLeft, bottomRight);
            }

            // Insertions
            if (!diff.added.empty()) {
                MODEL_LOG(qDebug() << "EntityPropertyModel::setRows: inserting " << diff.added.size() << " rows");

                const int firstNewRow = static_cast<int>(m_rows.size());
                const int lastNewRow = firstNewRow + static_cast<int>(diff.added.size()) - 1;
                assert(lastNewRow >= firstNewRow);

                beginInsertRows(QModelIndex(), firstNewRow, lastNewRow);
                for (const std::string& key : diff.added) {
                    const PropertyRow& row = newRowMap.at(key);
                    m_rows.push_back(row);
                }
                endInsertRows();
            }

            // Deletions
            if (!diff.removed.empty()) {
                MODEL_LOG(qDebug() << "EntityPropertyModel::setRows: deleting " << diff.removed.size() << " rows");

                for (const std::string& key : diff.removed) {
                    const PropertyRow& row = oldRowMap.at(key);
                    const auto index = kdl::vec_index_of(m_rows, row);
                    assert(index);

                    beginRemoveRows(QModelIndex(), static_cast<int>(*index), static_cast<int>(*index));
                    m_rows.erase(std::next(m_rows.begin(), static_cast<int>(*index)));
                    endRemoveRows();
                }
            }
        }

        const PropertyRow* EntityPropertyModel::dataForModelIndex(const QModelIndex& index) const {
            if (!index.isValid()) {
                return nullptr;
            }
            return &m_rows.at(static_cast<size_t>(index.row()));
        }

        int EntityPropertyModel::rowForPropertyKey(const std::string& propertyKey) const {
            for (size_t i = 0; i < m_rows.size(); ++i) {
                auto& row = m_rows.at(i);

                if (row.key() == propertyKey) {
                    return static_cast<int>(i);
                }
            }
            return -1;
        }

        QStringList EntityPropertyModel::getCompletions(const QModelIndex& index) const {
            const auto key = propertyKey(index.row());

            std::vector<std::string> result;
            if (index.column() == 0) {
                result = getAllPropertyKeys();
            } else if (index.column() == 1) {
                if (key == Model::PropertyKeys::Target ||
                    key == Model::PropertyKeys::Killtarget) {
                    result = getAllValuesForPropertyKeys({ Model::PropertyKeys::Targetname });
                } else if (key == Model::PropertyKeys::Targetname) {
                    result = getAllValuesForPropertyKeys(
                        { Model::PropertyKeys::Target, Model::PropertyKeys::Killtarget });
                } else if (key == Model::PropertyKeys::Classname) {
                    result = getAllClassnames();
                }
            }

            return toQStringList(std::begin(result), std::end(result));
        }

        std::string EntityPropertyModel::propertyKey(const int row) const {
            if (row < 0 || row >= static_cast<int>(m_rows.size())) {
                return "";
            } else {
                return m_rows[static_cast<size_t>(row)].key();
            }
        }

        std::vector<std::string> EntityPropertyModel::propertyKeys(const int row, const int count) const {
            std::vector<std::string> result;
            result.reserve(static_cast<std::size_t>(count));

            for (int i = 0; i < count; ++i) {
                result.push_back(this->propertyKey(row + i));
            }
            return result;
        }

        std::vector<std::string> EntityPropertyModel::getAllPropertyKeys() const {
            auto document = kdl::mem_lock(m_document);
            const auto& index = document->world()->entityNodeIndex();
            auto result = kdl::vector_set<std::string>(index.allKeys());

            // also add keys from all loaded entity definitions
            for (const auto* entityDefinition : document->entityDefinitionManager().definitions()) {
                for (const auto& attributeDefinition : entityDefinition->propertyDefinitions()) {
                    result.insert(attributeDefinition->key());
                }
            }

            // remove the empty string
            result.erase("");
            return result.release_data();
        }

        std::vector<std::string> EntityPropertyModel::getAllValuesForPropertyKeys(const std::vector<std::string>& propertyKeys) const {
            auto document = kdl::mem_lock(m_document);
            const auto& index = document->world()->entityNodeIndex();

            auto result = std::vector<std::string>();
            auto resultSet = kdl::wrap_set(result);

            for (const auto& key : propertyKeys) {
                const auto values = index.allValuesForKeys(Model::EntityNodeIndexQuery::numbered(key));
                for (const auto& value : values) {
                    resultSet.insert(value);
                }
            }

            // remove the empty string
            resultSet.erase("");
            return result;
        }

        std::vector<std::string> EntityPropertyModel::getAllClassnames() const {
            auto document = kdl::mem_lock(m_document);

            // start with currently used classnames
            auto result = getAllValuesForPropertyKeys({ Model::PropertyKeys::Classname });
            auto resultSet = kdl::wrap_set(result);

            // add keys from all loaded entity definitions
            for (const auto* entityDefinition : document->entityDefinitionManager().definitions()) {
                resultSet.insert(entityDefinition->name());
            }

            // remove the empty string
            resultSet.erase("");
            return result;
        }

        void EntityPropertyModel::updateFromMapDocument() {
            MODEL_LOG(qDebug() << "updateFromMapDocument");

            auto document = kdl::mem_lock(m_document);

            const std::map<std::string, PropertyRow> rowsMap =
                PropertyRow::rowsForEntityNodes(document->allSelectedEntityNodes(), m_showDefaultRows);

            setRows(rowsMap);
        }

        int EntityPropertyModel::rowCount(const QModelIndex& parent) const {
            if (parent.isValid()) {
                return 0;
            }
            return static_cast<int>(m_rows.size());
        }

        int EntityPropertyModel::columnCount(const QModelIndex& parent) const {
            if (parent.isValid()) {
                return 0;
            }
            return 2;
        }

        Qt::ItemFlags EntityPropertyModel::flags(const QModelIndex &index) const {
            if (!index.isValid()) {
                return Qt::NoItemFlags;
            }

            const PropertyRow& row = m_rows.at(static_cast<size_t>(index.row()));

            Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;

            if (index.column() == 0) {
                if (row.keyMutable()) {
                    flags |= Qt::ItemIsEditable;
                }
            } else {
                if (row.valueMutable()) {
                    flags |= Qt::ItemIsEditable;
                }
            }

            return flags;
        }

        QVariant EntityPropertyModel::data(const QModelIndex& index, const int role) const {
            if (!index.isValid()
                || index.row() < 0
                || index.row() >= static_cast<int>(m_rows.size())
                || index.column() < 0
                || index.column() >= 2) {
                return QVariant();
            }

            auto document = kdl::mem_lock(m_document);
            const PropertyRow& row = m_rows.at(static_cast<size_t>(index.row()));

            if (role == Qt::DecorationRole) {
                // lock icon
                if (index.column() == 0) {
                    if (!row.keyMutable()) {
                        return QVariant(IO::loadSVGIcon(IO::Path("Locked_small.svg")));
                    }
                } else if (index.column() == 1) {
                    if (!row.valueMutable()) {
                        return QVariant(IO::loadSVGIcon(IO::Path("Locked_small.svg")));
                    }
                }
                return QVariant();
            }


            if (role == Qt::ForegroundRole) {
                if (row.isDefault() || row.subset()) {
                    return QVariant(QBrush(Colors::disabledCellText()));
                }
                if (index.column() == 1) {
                    if (row.multi()) {
                        return QVariant(QBrush(Colors::disabledCellText()));
                    }
                }
                return QVariant();
            }

            if (role == Qt::FontRole) {
                if (row.isDefault()) {
                    QFont italicFont;
                    italicFont.setItalic(true);
                    return QVariant(italicFont);
                }
                if (index.column() == 1) {
                    if (row.multi()) {
                        QFont italicFont;
                        italicFont.setItalic(true);
                        return QVariant(italicFont);
                    }
                }
                return QVariant();
            }

            if (role == Qt::DisplayRole || role == Qt::EditRole) {
                if (index.column() == 0) {
                    return QVariant(mapStringToUnicode(document->encoding(), row.key()));
                } else {
                    return QVariant(mapStringToUnicode(document->encoding(), row.value()));
                }
            }

            if (role == Qt::ToolTipRole) {
                if (!row.tooltip().empty()) {
                    return QVariant(mapStringToUnicode(document->encoding(), row.tooltip()));
                }
            }

            return QVariant();
        }

        bool EntityPropertyModel::setData(const QModelIndex& index, const QVariant& value, const int role) {
            const auto& propertyRow = m_rows.at(static_cast<size_t>(index.row()));
            unused(propertyRow);

            if (role != Qt::EditRole) {
                return false;
            }

            auto document = kdl::mem_lock(m_document);

            const size_t rowIndex = static_cast<size_t>(index.row());
            const std::vector<Model::EntityNodeBase*> nodes = document->allSelectedEntityNodes();
            if (nodes.empty()) {
                return false;
            }

            if (index.column() == 0) {
                // rename key
                MODEL_LOG(qDebug() << "tried to rename " << mapStringToUnicode(document->encoding(), propertyRow.key()) << " to " << value.toString());

                const std::string newName = mapStringFromUnicode(document->encoding(), value.toString());
                if (renameProperty(rowIndex, newName, nodes)) {
                    return true;
                }
            } else if (index.column() == 1) {
                MODEL_LOG(qDebug() << "tried to set " << mapStringToUnicode(document->encoding(), propertyRow.key()) << " to "
                                   << value.toString());

                if (updateProperty(rowIndex, mapStringFromUnicode(document->encoding(), value.toString()), nodes)) {
                    return true;
                }
            }

            return false;
        }

        QVariant EntityPropertyModel::headerData(const int section, const Qt::Orientation orientation, const int role) const {
            if (role != Qt::DisplayRole) {
                return QVariant();
            }

            if (orientation == Qt::Horizontal) {
                if (section == 0) {
                    return QVariant(tr("Key"));
                } else if (section == 1) {
                    return QVariant(tr("Value"));
                }
            }
            return QVariant();
        }

        bool EntityPropertyModel::canRemove(const int rowIndexInt) {
            if (rowIndexInt < 0 || static_cast<size_t>(rowIndexInt) >= m_rows.size()) {
                return false;
            }

            const PropertyRow& row = m_rows.at(static_cast<size_t>(rowIndexInt));
            if (row.isDefault()) {
                return false;
            }
            return row.keyMutable() && row.valueMutable();
        }

        bool EntityPropertyModel::hasRowWithPropertyKey(const std::string& propertyKey) const {
            return rowForPropertyKey(propertyKey) != -1;
        }

        bool EntityPropertyModel::renameProperty(const size_t rowIndex, const std::string& newKey, const std::vector<Model::EntityNodeBase*>& /* nodes */) {
            ensure(rowIndex < m_rows.size(), "row index out of bounds");

            auto document = kdl::mem_lock(m_document);
            const PropertyRow& row = m_rows.at(rowIndex);
            const std::string& oldKey = row.key();

            if (oldKey == newKey)
                return true;

            ensure(row.keyMutable(), "tried to rename immutable name"); // EntityPropertyModel::flags prevents us from renaming immutable names

            if (hasRowWithPropertyKey(newKey)) {
                const PropertyRow& rowToOverwrite = m_rows.at(static_cast<size_t>(rowForPropertyKey(newKey)));
                if (!rowToOverwrite.valueMutable()) {
                    // Prevent changing an immutable value via a rename
                    // TODO: would this be better checked inside MapDocument::renameProperty?
                    return false;
                }

                QMessageBox msgBox;
                msgBox.setWindowTitle(tr("Error"));
                msgBox.setText(tr("A property with key '%1' already exists.\n\n Do you wish to overwrite it?")
                    .arg(mapStringToUnicode(document->encoding(), newKey)));
                msgBox.setIcon(QMessageBox::Critical);
                msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
                if (msgBox.exec() == QMessageBox::No) {
                    return false;
                }
            }
            
            return document->renameProperty(oldKey, newKey);
        }

        bool EntityPropertyModel::updateProperty(const size_t rowIndex, const std::string& newValue, const std::vector<Model::EntityNodeBase*>& nodes) {
            ensure(rowIndex < m_rows.size(), "row index out of bounds");

            bool hasChange = false;
            const std::string& name = m_rows.at(rowIndex).key();
            for (const Model::EntityNodeBase* node : nodes) {
                if (const auto* oldValue = node->entity().property(name)) {
                    ensure(isPropertyValueMutable(node->entity(), name), "tried to modify immutable property value"); // this should be guaranteed by the PropertyRow constructor
                    if (*oldValue != newValue) {
                        hasChange = true;
                    }
                } else {
                    hasChange = true;
                }
            }

            if (!hasChange)
                return true;

            auto document = kdl::mem_lock(m_document);
            return document->setProperty(name, newValue);
        }

        bool EntityPropertyModel::lessThan(const size_t rowIndexA, const size_t rowIndexB) const {
            const PropertyRow& rowA = m_rows.at(rowIndexA);
            const PropertyRow& rowB = m_rows.at(rowIndexB);

            // 1. non-default sorts before default
            if (!rowA.isDefault() &&  rowB.isDefault()) {
                return true;
            }
            if ( rowA.isDefault() && !rowB.isDefault()) {
                return false;
            }

            // 2. sort by name
            return rowA.key() < rowB.key();
        }
    }
}
