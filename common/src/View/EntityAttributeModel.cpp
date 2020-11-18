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

#include "EntityAttributeModel.h"

#include "Macros.h"
#include "Assets/AttributeDefinition.h"
#include "Assets/EntityDefinition.h"
#include "Assets/EntityDefinitionManager.h"
#include "IO/ResourceUtils.h"
#include "Model/AttributableNode.h"
#include "Model/AttributableNodeIndex.h"
#include "Model/Entity.h"
#include "Model/EntityAttributes.h"
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
        bool isAttributeNameMutable(const Model::Entity& entity, const std::string& name) {
            assert(!Model::isGroup(entity.classname(), entity.attributes()));
            assert(!Model::isLayer(entity.classname(), entity.attributes()));

            if (Model::isWorldspawn(entity.classname(), entity.attributes())) {
                return !(name == Model::AttributeNames::Classname
                    || name == Model::AttributeNames::Mods
                    || name == Model::AttributeNames::EntityDefinitions
                    || name == Model::AttributeNames::Wad
                    || name == Model::AttributeNames::Textures
                    || name == Model::AttributeNames::SoftMapBounds
                    || name == Model::AttributeNames::LayerColor
                    || name == Model::AttributeNames::LayerLocked
                    || name == Model::AttributeNames::LayerHidden
                    || name == Model::AttributeNames::LayerOmitFromExport);
            }

            return true;
        }

        bool isAttributeValueMutable(const Model::Entity& entity, const std::string& name) {
            assert(!Model::isGroup(entity.classname(), entity.attributes()));
            assert(!Model::isLayer(entity.classname(), entity.attributes()));

            if (Model::isWorldspawn(entity.classname(), entity.attributes())) {
                return !(name == Model::AttributeNames::Mods
                    || name == Model::AttributeNames::EntityDefinitions
                    || name == Model::AttributeNames::Wad
                    || name == Model::AttributeNames::Textures
                    || name == Model::AttributeNames::SoftMapBounds
                    || name == Model::AttributeNames::LayerColor
                    || name == Model::AttributeNames::LayerLocked
                    || name == Model::AttributeNames::LayerHidden
                    || name == Model::AttributeNames::LayerOmitFromExport);
            }

            return true;
        }

        // AttributeRow

        AttributeRow::AttributeRow() :
        m_valueType(ValueType::Unset),
        m_nameMutable(true),
        m_valueMutable(true) {}

        bool AttributeRow::operator==(const AttributeRow& other) const {
            return m_name == other.m_name
                && m_value == other.m_value
                && m_valueType == other.m_valueType
                && m_nameMutable == other.m_nameMutable
                && m_valueMutable == other.m_valueMutable
                && m_tooltip == other.m_tooltip;
        }

        bool AttributeRow::operator<(const AttributeRow& other) const {
            if (m_name < other.m_name) return true;
            if (m_name > other.m_name) return false;

            if (m_value < other.m_value) return true;
            if (m_value > other.m_value) return false;

            if (m_valueType < other.m_valueType) return true;
            if (m_valueType > other.m_valueType) return false;

            if (m_nameMutable < other.m_nameMutable) return true;
            if (m_nameMutable > other.m_nameMutable) return false;

            if (m_valueMutable < other.m_valueMutable) return true;
            if (m_valueMutable > other.m_valueMutable) return false;

            if (m_tooltip < other.m_tooltip) return true;
            if (m_tooltip > other.m_tooltip) return false;

            return false;
        }

        AttributeRow::AttributeRow(const std::string& name, const Model::AttributableNode* node) :
        m_name(name) {
            const Assets::AttributeDefinition* definition = Model::attributeDefinition(node, name);

            if (const auto* value = node->entity().attribute(name)) {
                m_value = *value;
                m_valueType = ValueType::SingleValue;
            } else if (definition != nullptr) {
                m_value = Assets::AttributeDefinition::defaultValue(*definition);
                m_valueType = ValueType::Unset;
            } else {
                // this is the case when the name is coming from another entity
                m_valueType = ValueType::Unset;
            }

            m_nameMutable = isAttributeNameMutable(node->entity(), name);
            m_valueMutable = isAttributeValueMutable(node->entity(), name);
            m_tooltip = (definition != nullptr ? definition->shortDescription() : "");
            if (m_tooltip.empty()) {
                m_tooltip = "No description found";
            }
        }

        void AttributeRow::merge(const Model::AttributableNode* other) {
            const auto* otherValue = other->entity().attribute(m_name);

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

            m_nameMutable = (m_nameMutable && isAttributeNameMutable(other->entity(), m_name));
            m_valueMutable = (m_valueMutable && isAttributeValueMutable(other->entity(), m_name));
        }

        const std::string& AttributeRow::name() const {
            return m_name;
        }

        std::string AttributeRow::value() const {
            if (m_valueType == ValueType::MultipleValues) {
                return "multi";
            }
            return m_value;
        }

        bool AttributeRow::nameMutable() const {
            return m_nameMutable;
        }

        bool AttributeRow::valueMutable() const {
            return m_valueMutable;
        }

        const std::string& AttributeRow::tooltip() const {
            return m_tooltip;
        }

        bool AttributeRow::isDefault() const {
            return m_valueType == ValueType::Unset;
        }

        bool AttributeRow::multi() const {
            return m_valueType == ValueType::MultipleValues;
        }

        bool AttributeRow::subset() const {
            return m_valueType == ValueType::SingleValueAndUnset;
        }

        AttributeRow AttributeRow::rowForAttributableNodes(const std::string& key, const std::vector<Model::AttributableNode*>& attributables) {
            ensure(attributables.size() > 0, "rowForAttributableNodes requries a non-empty node list");

            std::optional<AttributeRow> result;
            for (const Model::AttributableNode* node : attributables) {
                // this happens at startup when the world is still null
                if (node == nullptr) {
                    continue;
                }

                if (!result.has_value()) {
                    result = AttributeRow(key, node);
                }  else {
                    result->merge(node);
                }
            }

            assert(result.has_value());
            return result.value();
        }

        std::vector<std::string> AttributeRow::allKeys(const std::vector<Model::AttributableNode*>& attributables, const bool showDefaultRows) {
            kdl::vector_set<std::string> result;
            for (const Model::AttributableNode* node : attributables) {
                // this happens at startup when the world is still null
                if (node == nullptr) {
                    continue;
                }

                // Add explicitly set attributes
                for (const Model::EntityAttribute& attribute : node->entity().attributes()) {
                    result.insert(attribute.name());
                }

                // Add default attributes from the entity definition
                if (showDefaultRows) {
                    const Assets::EntityDefinition* entityDefinition = node->entity().definition();
                    if (entityDefinition != nullptr) {
                       for (auto attributeDefinition : entityDefinition->attributeDefinitions()) {
                           result.insert(attributeDefinition->name());
                       }
                    }
                }
            }
            return result.release_data();
        }

        std::map<std::string, AttributeRow> AttributeRow::rowsForAttributableNodes(const std::vector<Model::AttributableNode*>& attributables, const bool showDefaultRows) {
            std::map<std::string, AttributeRow> result;
            for (const std::string& key : allKeys(attributables, showDefaultRows)) {
                result[key] = rowForAttributableNodes(key, attributables);
            }
            return result;
        }

        std::string AttributeRow::newAttributeNameForAttributableNodes(const std::vector<Model::AttributableNode*>& attributables) {
            const std::map<std::string, AttributeRow> rows = rowsForAttributableNodes(attributables, true);

            for (int i = 1; ; ++i) {
                const std::string newName = kdl::str_to_string("property ", i);
                if (rows.find(newName) == rows.end()) {
                    return newName;
                }
            }
            // unreachable
        }

        // EntityAttributeModel

        EntityAttributeModel::EntityAttributeModel(std::weak_ptr<MapDocument> document, QObject* parent) :
        QAbstractTableModel(parent),
        m_showDefaultRows(true),
        m_document(std::move(document)) {
            updateFromMapDocument();
        }

        static auto makeNameToAttributeRowMap(const std::vector<AttributeRow>& rows) {
            std::map<std::string, AttributeRow> result;
            for (const auto& row : rows) {
                result[row.name()] = row;
            }
            return result;
        }

        using AttributeRowMap = std::map<std::string, AttributeRow>;

        struct KeyDiff {
            std::vector<std::string> removed;
            std::vector<std::string> added;
            std::vector<std::string> updated;
            std::vector<std::string> unchanged;
        };

        static KeyDiff compareAttributeMaps(const AttributeRowMap& oldRows, const AttributeRowMap& newRows) {
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

        bool EntityAttributeModel::showDefaultRows() const {
            return m_showDefaultRows;
        }

        void EntityAttributeModel::setShowDefaultRows(const bool showDefaultRows) {
            if (showDefaultRows == m_showDefaultRows) {
                return;
            }
            m_showDefaultRows = showDefaultRows;
            updateFromMapDocument();
        }

        void EntityAttributeModel::setRows(const std::map<std::string, AttributeRow>& newRowMap) {
            auto document = kdl::mem_lock(m_document);
            const auto oldRowMap = makeNameToAttributeRowMap(m_rows);

            if (newRowMap == oldRowMap) {
                MODEL_LOG(qDebug() << "EntityAttributeModel::setRows: no change");
                return;
            }

            const KeyDiff diff = compareAttributeMaps(oldRowMap, newRowMap);

            // If exactly one row was changed
            // we can tell Qt the row was edited instead. This allows the selection/current index
            // to be preserved, whereas removing the row would invalidate the current index.
            //
            // This situation happens when you rename a key and then press Tab to switch
            // to editing the value for the newly renamed key.

            if (diff.removed.size() == 1 && diff.added.size() == 1 && diff.updated.empty()) {
                const AttributeRow& oldDeletion = oldRowMap.at(diff.removed[0]);
                const AttributeRow& newAddition = newRowMap.at(diff.added[0]);

                MODEL_LOG(qDebug() << "EntityAttributeModel::setRows: one row changed: " << mapStringToUnicode(document->encoding(), oldDeletion.name()) << " -> " << mapStringToUnicode(document->encoding(), newAddition.name()));

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

            MODEL_LOG(qDebug() << "EntityAttributeModel::setRows: " << diff.updated.size() << " common keys");
            for (const auto& key : diff.updated) {
                const AttributeRow& oldRow = oldRowMap.at(key);
                const AttributeRow& newRow = newRowMap.at(key);
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
                MODEL_LOG(qDebug() << "EntityAttributeModel::setRows: inserting " << diff.added.size() << " rows");

                const int firstNewRow = static_cast<int>(m_rows.size());
                const int lastNewRow = firstNewRow + static_cast<int>(diff.added.size()) - 1;
                assert(lastNewRow >= firstNewRow);

                beginInsertRows(QModelIndex(), firstNewRow, lastNewRow);
                for (const std::string& key : diff.added) {
                    const AttributeRow& row = newRowMap.at(key);
                    m_rows.push_back(row);
                }
                endInsertRows();
            }

            // Deletions
            if (!diff.removed.empty()) {
                MODEL_LOG(qDebug() << "EntityAttributeModel::setRows: deleting " << diff.removed.size() << " rows");

                for (const std::string& key : diff.removed) {
                    const AttributeRow& row = oldRowMap.at(key);
                    const auto index = kdl::vec_index_of(m_rows, row);
                    assert(index);

                    beginRemoveRows(QModelIndex(), static_cast<int>(*index), static_cast<int>(*index));
                    m_rows.erase(std::next(m_rows.begin(), static_cast<int>(*index)));
                    endRemoveRows();
                }
            }
        }

        const AttributeRow* EntityAttributeModel::dataForModelIndex(const QModelIndex& index) const {
            if (!index.isValid()) {
                return nullptr;
            }
            return &m_rows.at(static_cast<size_t>(index.row()));
        }

        int EntityAttributeModel::rowForAttributeName(const std::string& name) const {
            for (size_t i = 0; i < m_rows.size(); ++i) {
                auto& row = m_rows.at(i);

                if (row.name() == name) {
                    return static_cast<int>(i);
                }
            }
            return -1;
        }

        QStringList EntityAttributeModel::getCompletions(const QModelIndex& index) const {
            const auto name = attributeName(index.row());

            std::vector<std::string> result;
            if (index.column() == 0) {
                result = getAllAttributeNames();
            } else if (index.column() == 1) {
                if (name == Model::AttributeNames::Target ||
                    name == Model::AttributeNames::Killtarget) {
                    result = getAllValuesForAttributeNames({ Model::AttributeNames::Targetname });
                } else if (name == Model::AttributeNames::Targetname) {
                    result = getAllValuesForAttributeNames({ Model::AttributeNames::Target, Model::AttributeNames::Killtarget });
                } else if (name == Model::AttributeNames::Classname) {
                    result = getAllClassnames();
                }
            }

            return toQStringList(std::begin(result), std::end(result));
        }

        std::string EntityAttributeModel::attributeName(const int row) const {
            if (row < 0 || row >= static_cast<int>(m_rows.size())) {
                return "";
            } else {
                return m_rows[static_cast<size_t>(row)].name();
            }
        }

        std::vector<std::string> EntityAttributeModel::attributeNames(const int row, const int count) const {
            std::vector<std::string> result;
            result.reserve(static_cast<std::size_t>(count));

            for (int i = 0; i < count; ++i) {
                result.push_back(this->attributeName(row + i));
            }
            return result;
        }

        std::vector<std::string> EntityAttributeModel::getAllAttributeNames() const {
            auto document = kdl::mem_lock(m_document);
            const auto& index = document->world()->attributableNodeIndex();
            auto result = kdl::vector_set<std::string>(index.allNames());

            // also add keys from all loaded entity definitions
            for (const auto* entityDefinition : document->entityDefinitionManager().definitions()) {
                for (const auto& attributeDefinition : entityDefinition->attributeDefinitions()) {
                    result.insert(attributeDefinition->name());
                }
            }

            // remove the empty string
            result.erase("");
            return result.release_data();
        }

        std::vector<std::string> EntityAttributeModel::getAllValuesForAttributeNames(const std::vector<std::string>& names) const {
            auto document = kdl::mem_lock(m_document);
            const auto& index = document->world()->attributableNodeIndex();

            auto result = std::vector<std::string>();
            auto resultSet = kdl::wrap_set(result);

            for (const auto& name : names) {
                const auto values = index.allValuesForNames(Model::AttributableNodeIndexQuery::numbered(name));
                for (const auto& value : values) {
                    resultSet.insert(value);
                }
            }

            // remove the empty string
            resultSet.erase("");
            return result;
        }

        std::vector<std::string> EntityAttributeModel::getAllClassnames() const {
            auto document = kdl::mem_lock(m_document);

            // start with currently used classnames
            auto result = getAllValuesForAttributeNames({ Model::AttributeNames::Classname });
            auto resultSet = kdl::wrap_set(result);

            // add keys from all loaded entity definitions
            for (const auto* entityDefinition : document->entityDefinitionManager().definitions()) {
                resultSet.insert(entityDefinition->name());
            }

            // remove the empty string
            resultSet.erase("");
            return result;
        }

        void EntityAttributeModel::updateFromMapDocument() {
            MODEL_LOG(qDebug() << "updateFromMapDocument");

            auto document = kdl::mem_lock(m_document);

            const std::map<std::string, AttributeRow> rowsMap =
                AttributeRow::rowsForAttributableNodes(document->allSelectedAttributableNodes(), m_showDefaultRows);

            setRows(rowsMap);
        }

        int EntityAttributeModel::rowCount(const QModelIndex& parent) const {
            if (parent.isValid()) {
                return 0;
            }
            return static_cast<int>(m_rows.size());
        }

        int EntityAttributeModel::columnCount(const QModelIndex& parent) const {
            if (parent.isValid()) {
                return 0;
            }
            return 2;
        }

        Qt::ItemFlags EntityAttributeModel::flags(const QModelIndex &index) const {
            if (!index.isValid()) {
                return Qt::NoItemFlags;
            }

            const AttributeRow& row = m_rows.at(static_cast<size_t>(index.row()));

            Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;

            if (index.column() == 0) {
                if (row.nameMutable()) {
                    flags |= Qt::ItemIsEditable;
                }
            } else {
                if (row.valueMutable()) {
                    flags |= Qt::ItemIsEditable;
                }
            }

            return flags;
        }

        QVariant EntityAttributeModel::data(const QModelIndex& index, int role) const {
            if (!index.isValid()
                || index.row() < 0
                || index.row() >= static_cast<int>(m_rows.size())
                || index.column() < 0
                || index.column() >= 2) {
                return QVariant();
            }

            auto document = kdl::mem_lock(m_document);
            const AttributeRow& row = m_rows.at(static_cast<size_t>(index.row()));

            if (role == Qt::DecorationRole) {
                // lock icon
                if (index.column() == 0) {
                    if (!row.nameMutable()) {
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
                    return QVariant(mapStringToUnicode(document->encoding(), row.name()));
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

        bool EntityAttributeModel::setData(const QModelIndex &index, const QVariant &value, int role) {
            const auto& attributeRow = m_rows.at(static_cast<size_t>(index.row()));
            unused(attributeRow);

            if (role != Qt::EditRole) {
                return false;
            }

            auto document = kdl::mem_lock(m_document);

            const size_t rowIndex = static_cast<size_t>(index.row());
            const std::vector<Model::AttributableNode*> attributables = document->allSelectedAttributableNodes();
            if (attributables.empty()) {
                return false;
            }

            if (index.column() == 0) {
                // rename key
                MODEL_LOG(qDebug() << "tried to rename " << mapStringToUnicode(document->encoding(), attributeRow.name()) << " to " << value.toString());

                const std::string newName = mapStringFromUnicode(document->encoding(), value.toString());
                if (renameAttribute(rowIndex, newName, attributables)) {
                    return true;
                }
            } else if (index.column() == 1) {
                MODEL_LOG(qDebug() << "tried to set " << mapStringToUnicode(document->encoding(), attributeRow.name()) << " to "
                                   << value.toString());

                if (updateAttribute(rowIndex, mapStringFromUnicode(document->encoding(), value.toString()), attributables)) {
                    return true;
                }
            }

            return false;
        }

        QVariant EntityAttributeModel::headerData(int section, Qt::Orientation orientation, int role) const {
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

        int EntityAttributeModel::rowForName(const std::string& name) const {
            for (size_t i = 0; i < m_rows.size(); ++i) {
                if (m_rows[i].name() == name) {
                    return static_cast<int>(i);
                }
            }
            return -1;
        }

        bool EntityAttributeModel::canRemove(const int rowIndexInt) {
            if (rowIndexInt < 0 || static_cast<size_t>(rowIndexInt) >= m_rows.size()) {
                return false;
            }

            const AttributeRow& row = m_rows.at(static_cast<size_t>(rowIndexInt));
            if (row.isDefault()) {
                return false;
            }
            return row.nameMutable() && row.valueMutable();
        }

        bool EntityAttributeModel::hasRowWithAttributeName(const std::string& name) const {
            return rowForAttributeName(name) != -1;
        }

        bool EntityAttributeModel::renameAttribute(const size_t rowIndex, const std::string& newName, const std::vector<Model::AttributableNode*>& /* attributables */) {
            ensure(rowIndex < m_rows.size(), "row index out of bounds");

            auto document = kdl::mem_lock(m_document);
            const AttributeRow& row = m_rows.at(rowIndex);
            const std::string& oldName = row.name();

            if (oldName == newName)
                return true;

            ensure(row.nameMutable(), "tried to rename immutable name"); // EntityAttributeModel::flags prevents us from renaming immutable names

            if (hasRowWithAttributeName(newName)) {
                const AttributeRow& rowToOverwrite = m_rows.at(static_cast<size_t>(rowForAttributeName(newName)));
                if (!rowToOverwrite.valueMutable()) {
                    // Prevent changing an immutable value via a rename
                    // TODO: would this be better checked inside MapDocument::renameAttribute?
                    return false;
                }

                QMessageBox msgBox;
                msgBox.setWindowTitle(tr("Error"));
                msgBox.setText(tr("A property with key '%1' already exists.\n\n Do you wish to overwrite it?")
                    .arg(mapStringToUnicode(document->encoding(), newName)));
                msgBox.setIcon(QMessageBox::Critical);
                msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
                if (msgBox.exec() == QMessageBox::No) {
                    return false;
                }
            }
            
            return document->renameAttribute(oldName, newName);
        }

        bool EntityAttributeModel::updateAttribute(const size_t rowIndex, const std::string& newValue, const std::vector<Model::AttributableNode*>& attributables) {
            ensure(rowIndex < m_rows.size(), "row index out of bounds");

            bool hasChange = false;
            const std::string name = m_rows.at(rowIndex).name();
            for (const Model::AttributableNode* attributable : attributables) {
                if (const auto* oldValue = attributable->entity().attribute(name)) {
                    ensure(isAttributeValueMutable(attributable->entity(), name), "tried to modify immutable attribute value"); // this should be guaranteed by the AttributeRow constructor
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
            return document->setAttribute(name, newValue);
        }

        bool EntityAttributeModel::lessThan(const size_t rowIndexA, const size_t rowIndexB) const {
            const AttributeRow& rowA = m_rows.at(rowIndexA);
            const AttributeRow& rowB = m_rows.at(rowIndexB);

            // 1. non-default sorts before default
            if (!rowA.isDefault() &&  rowB.isDefault()) {
                return true;
            }
            if ( rowA.isDefault() && !rowB.isDefault()) {
                return false;
            }

            // 2. sort by name
            return rowA.name() < rowB.name();
        }
    }
}
