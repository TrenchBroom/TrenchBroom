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
#include "Model/EntityAttributes.h"
#include "Model/World.h"
#include "View/MapDocument.h"
#include "View/ViewConstants.h"
#include "View/QtUtils.h"

#include <kdl/map_utils.h>
#include <kdl/memory_utils.h>
#include <kdl/string_utils.h>
#include <kdl/vector_utils.h>
#include <kdl/vector_set.h>

#include <iterator>
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

namespace TrenchBroom {
    namespace View {
        /**
         * The rationale for not using UTF-8 here is Quake maps often want to use
         * the bytes 128-255 which are characters in Quake's bitmap font (gold text, etc.)
         * UTF-8 is not suitable since we need a single-byte encoding.
         *
         * See: https://github.com/kduske/TrenchBroom/issues/3122 
         */
        static QString entityStringToUnicode(const std::string& string) {
            return QString::fromLocal8Bit(QByteArray::fromStdString(string));
        }

        static std::string entityStringFromUnicode(const QString& string) {
            return string.toLocal8Bit().toStdString();
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
            const Assets::AttributeDefinition* definition = node->attributeDefinition(name);

            if (node->hasAttribute(name)) {
                m_value = node->attribute(name);
                m_valueType = ValueType::SingleValue;
            } else if (definition != nullptr) {
                m_value = Assets::AttributeDefinition::defaultValue(*definition);
                m_valueType = ValueType::Unset;
            } else {
                // this is the case when the name is coming from another entity
                m_valueType = ValueType::Unset;
            }

            m_nameMutable = node->isAttributeNameMutable(name);
            m_valueMutable = node->isAttributeValueMutable(name);
            m_tooltip = (definition != nullptr ? definition->shortDescription() : "");
            if (m_tooltip.empty()) {
                m_tooltip = "No description found";
            }
        }

        void AttributeRow::merge(const Model::AttributableNode* other) {
            const bool otherHasAttribute = other->hasAttribute(m_name);
            const std::string otherValue = other->attribute(m_name);

            // State transitions
            if (m_valueType == ValueType::Unset) {
                if (otherHasAttribute) {
                     m_valueType = ValueType::SingleValueAndUnset;
                     m_value = otherValue;
                }
            } else if (m_valueType == ValueType::SingleValue) {
                if (!otherHasAttribute) {
                    m_valueType = ValueType::SingleValueAndUnset;
                } else if (otherValue != m_value) {
                    m_valueType = ValueType::MultipleValues;
                }
            } else if (m_valueType == ValueType::SingleValueAndUnset) {
                if (otherHasAttribute && otherValue != m_value) {
                    m_valueType = ValueType::MultipleValues;
                }
            }

            m_nameMutable = (m_nameMutable && other->isAttributeNameMutable(m_name));
            m_valueMutable = (m_valueMutable && other->isAttributeValueMutable(m_name));
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
                for (const Model::EntityAttribute& attribute : node->attributes()) {
                    result.insert(attribute.name());
                }

                // Add default attributes from the entity definition
                if (showDefaultRows) {
                    const Assets::EntityDefinition* entityDefinition = node->definition();
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

        static std::vector<AttributeRow> buildVec(const std::map<std::string, AttributeRow>& rows) {
            std::vector<AttributeRow> result;
            result.reserve(rows.size());
            for (auto& [key, row] : rows) {
                unused(key);
                result.push_back(row);
            }
            return result;
        }

        /* FIXME: remove unused code
        static auto buildAttributeToRowIndexMap(const std::vector<AttributeRow>& rows) {
            std::map<std::string, int> result;
            for (size_t i = 0; i < rows.size(); ++i) {
                const AttributeRow& row = rows[i];
                result[row.name()] = static_cast<int>(i);
            }
            return result;
        }

        static std::set<std::string> attributeRowKeySet(const std::vector<AttributeRow>& rows) {
            std::set<std::string> result;
            for (const auto& row : rows) {
                result.insert(row.name());
            }
            return result;
        }
        */

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

        void EntityAttributeModel::setRows(const std::map<std::string, AttributeRow>& newRowsKeyMap) {
            const auto newRowSet = kdl::vector_set(kdl::map_values(newRowsKeyMap));
            const auto oldRowSet = kdl::vector_set(std::begin(m_rows), std::end(m_rows));

            if (newRowSet == oldRowSet) {
                qDebug() << "EntityAttributeModel::setRows: no change";
                return;
            }

            // If exactly one row was changed
            // we can tell Qt the row was edited instead. This allows the selection/current index
            // to be preserved, whereas removing the row would invalidate the current index.
            //
            // This situation happens when you rename a key and then press Tab to switch
            // to editing the value for the newly renamed key.
            const auto newMinusOld = kdl::set_difference(newRowSet, oldRowSet);
            const auto oldMinusNew = kdl::set_difference(oldRowSet, newRowSet);

            if (newMinusOld.size() == 1 && oldMinusNew.size() == 1) {
                const AttributeRow oldDeletion = *oldMinusNew.begin();
                const AttributeRow newAddition = *newMinusOld.begin();

                qDebug() << "EntityAttributeModel::setRows: one row changed: " << entityStringToUnicode(oldDeletion.name()) << " -> " << entityStringToUnicode(newAddition.name());

                const size_t oldIndex = kdl::vec_index_of(m_rows, oldDeletion);
                m_rows.at(oldIndex) = newAddition;

                // Notify Qt
                const QModelIndex topLeft = index(static_cast<int>(oldIndex), 0);
                const QModelIndex bottomRight = index(static_cast<int>(oldIndex), 1);
                emit dataChanged(topLeft, bottomRight);

                return;
            }

            // Insertions
            if (!newMinusOld.empty() && oldMinusNew.empty()) {
                qDebug() << "EntityAttributeModel::setRows: inserting " << newMinusOld.size() << " rows";

                const int firstNewRow = static_cast<int>(m_rows.size());
                const int lastNewRow = firstNewRow + static_cast<int>(newMinusOld.size()) - 1;
                assert(lastNewRow >= firstNewRow);

                beginInsertRows(QModelIndex(), firstNewRow, lastNewRow);
                for (const AttributeRow& row : newMinusOld) {
                    m_rows.push_back(row);
                }
                endInsertRows();
                return;
            }

            // Deletions
            if (newMinusOld.empty() && !oldMinusNew.empty()) {
                qDebug() << "EntityAttributeModel::setRows: deleting " << oldMinusNew.size() << " rows";

                for (const AttributeRow& row : oldMinusNew) {
                    const int index = static_cast<int>(kdl::vec_index_of(m_rows, row));
                    assert(index < static_cast<int>(m_rows.size()));

                    beginRemoveRows(QModelIndex(), index, index);
                    m_rows.erase(std::next(m_rows.begin(), index));
                    endRemoveRows();
                }
                return;
            }

            // Fallback case: this will reset selections
            qDebug() << "EntityAttributeModel::setRows. resetting model (selections will be lost)";

            beginResetModel();
            m_rows = buildVec(newRowsKeyMap);
            endResetModel();
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
            qDebug() << "updateFromMapDocument";

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

            const AttributeRow& row = m_rows.at(static_cast<size_t>(index.row()));

            if (role == Qt::DecorationRole) {
                // lock icon
                if (index.column() == 0) {
                    if (!row.nameMutable()) {
                        return QVariant(IO::loadIconResourceQt(IO::Path("Locked_small.png")));
                    }
                } else if (index.column() == 1) {
                    if (!row.valueMutable()) {
                        return QVariant(IO::loadIconResourceQt(IO::Path("Locked_small.png")));
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
                    return QVariant(entityStringToUnicode(row.name()));
                } else {
                    return QVariant(entityStringToUnicode(row.value()));
                }
            }

            if (role == Qt::ToolTipRole) {
                if (!row.tooltip().empty()) {
                    return QVariant(entityStringToUnicode(row.tooltip()));
                }
            }

            return QVariant();
        }

        bool EntityAttributeModel::setData(const QModelIndex &index, const QVariant &value, int role) {
            const auto& attributeRow = m_rows.at(static_cast<size_t>(index.row()));

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
                qDebug() << "tried to rename " << entityStringToUnicode(attributeRow.name()) << " to " << value.toString();

                const std::string newName = entityStringFromUnicode(value.toString());
                if (renameAttribute(rowIndex, newName, attributables)) {
                    return true;
                }
            } else if (index.column() == 1) {
                qDebug() << "tried to set " << entityStringToUnicode(attributeRow.name()) << " to "
                         << value.toString();

                if (updateAttribute(rowIndex, entityStringFromUnicode(value.toString()), attributables)) {
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

        bool EntityAttributeModel::InsertRow(const size_t pos) {
            ensure(pos <= m_rows.size(), "insertion position out of bounds");

            auto document = kdl::mem_lock(m_document);

            const std::vector<Model::AttributableNode*> attributables = document->allSelectedAttributableNodes();
            ensure(!attributables.empty(), "no attributable nodes selected");

            const std::string newKey = AttributeRow::newAttributeNameForAttributableNodes(attributables);

            const Transaction transaction(document);
            document->setAttribute(newKey, "");

            return true;
        }

        bool EntityAttributeModel::AppendRow() {
            return InsertRow(m_rows.size());
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
            if (rowIndexInt < 0 || static_cast<size_t>(rowIndexInt) >= m_rows.size())
                return false;

            const AttributeRow& row = m_rows.at(static_cast<size_t>(rowIndexInt));
            return row.nameMutable() && row.valueMutable();
        }

        bool EntityAttributeModel::hasRowWithAttributeName(const std::string& name) const {
            return rowForAttributeName(name) != -1;
        }

        bool EntityAttributeModel::renameAttribute(const size_t rowIndex, const std::string& newName, const std::vector<Model::AttributableNode*>& /* attributables */) {
            ensure(rowIndex < m_rows.size(), "row index out of bounds");

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
                    .arg(entityStringToUnicode(newName)));
                msgBox.setIcon(QMessageBox::Critical);
                msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
                if (msgBox.exec() == QMessageBox::No) {
                    return false;
                }
            }

            auto document = kdl::mem_lock(m_document);
            return document->renameAttribute(oldName, newName);
        }

        bool EntityAttributeModel::updateAttribute(const size_t rowIndex, const std::string& newValue, const std::vector<Model::AttributableNode*>& attributables) {
            ensure(rowIndex < m_rows.size(), "row index out of bounds");

            bool hasChange = false;
            const std::string name = m_rows.at(rowIndex).name();
            for (const Model::AttributableNode* attributable : attributables) {
                if (attributable->hasAttribute(name)) {
                    ensure(attributable->canAddOrUpdateAttribute(name, newValue), "tried to modify immutable attribute value"); // this should be guaranteed by the AttributeRow constructor
                    if (attributable->attribute(name) != newValue) {
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
