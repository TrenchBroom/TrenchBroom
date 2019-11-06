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
#include "Model/EntityAttributes.h"
#include "Model/World.h"
#include "View/MapDocument.h"
#include "View/ViewConstants.h"
#include "View/wxUtils.h"

#include <optional-lite/optional.hpp>

#include <QDebug>
#include <QBrush>
#include <QMessageBox>
#include <QTimer>

namespace TrenchBroom {
    namespace View {

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

        AttributeRow::AttributeRow(const String& name, const Model::AttributableNode* node) :
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
            const String otherValue = other->attribute(m_name);

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

        const String& AttributeRow::name() const {
            return m_name;
        }

        String AttributeRow::value() const {
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

        const String& AttributeRow::tooltip() const {
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

        AttributeRow AttributeRow::rowForAttributableNodes(const String& key, const Model::AttributableNodeList& attributables) {
            ensure(attributables.size() > 0, "rowForAttributableNodes requries a non-empty node list");

            nonstd::optional<AttributeRow> result;
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

        std::set<String> AttributeRow::allKeys(const Model::AttributableNodeList& attributables, const bool showDefaultRows) {
            std::set<String> result;
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
                       for (Assets::AttributeDefinitionPtr attributeDefinition : entityDefinition->attributeDefinitions()) {
                           result.insert(attributeDefinition->name());
                       }
                    }
                }
            }
            return result;
        }

        std::map<String, AttributeRow> AttributeRow::rowsForAttributableNodes(const Model::AttributableNodeList& attributables, const bool showDefaultRows) {
            std::map<String, AttributeRow> result;
            for (const String& key : allKeys(attributables, showDefaultRows)) {
                result[key] = rowForAttributableNodes(key, attributables);
            }
            return result;
        }

        String AttributeRow::newAttributeNameForAttributableNodes(const Model::AttributableNodeList& attributables) {
            const std::map<String, AttributeRow> rows = rowsForAttributableNodes(attributables, true);

            for (int i = 1; ; ++i) {
                StringStream ss;
                ss << "property " << i;

                const String newName = ss.str();
                if (rows.find(newName) == rows.end()) {
                    return newName;
                }
            }
            // unreachable
        }

        // EntityAttributeModel

        EntityAttributeModel::EntityAttributeModel(MapDocumentWPtr document, QObject* parent) :
        QAbstractTableModel(parent),
        m_showDefaultRows(true),
        m_document(std::move(document)) {
            updateFromMapDocument();
        }

        static std::vector<AttributeRow> buildVec(const std::map<String, AttributeRow>& rows) {
            std::vector<AttributeRow> result;
            result.reserve(rows.size());
            for (auto& [key, row] : rows) {
                unused(key);
                result.push_back(row);
            }
            return result;
        }

        static auto buildAttributeToRowIndexMap(const std::vector<AttributeRow>& rows) {
            std::map<String, int> result;
            for (size_t i = 0; i < rows.size(); ++i) {
                const AttributeRow& row = rows[i];
                result[row.name()] = static_cast<int>(i);
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

        void EntityAttributeModel::setRows(const std::map<String, AttributeRow>& newRowsKeyMap) {
            const std::vector<AttributeRow> oldRows = m_rows;
            const std::vector<AttributeRow> newRows = buildVec(newRowsKeyMap);
            if (newRows == m_rows) {
                // Fast path: nothing in the viewmodel changed.
                return;
            }

            qDebug() << "EntityAttributeModel::setRows " << newRowsKeyMap.size() << " rows.";

            const std::map<String, int> oldRowIndexMap = buildAttributeToRowIndexMap(m_rows);
            const std::map<String, int> newRowIndexMap = buildAttributeToRowIndexMap(newRows);

            // If the key list changes, report it to Qt
            if (oldRowIndexMap != newRowIndexMap) {
                // see: http://doc.qt.io/qt-5/model-view-programming.html#resizable-models
                // and: http://doc.qt.io/qt-5/qabstractitemmodel.html#layoutChanged

                emit layoutAboutToBeChanged();

                // Figure out the mapping from old to new indices
                const QModelIndexList oldPersistentIndices = persistentIndexList();
                QModelIndexList newPersistentIndices;

                for (const auto& oldPersistentIndex : oldPersistentIndices) {
                    if (!oldPersistentIndex.isValid()) {
                        // Shouldn't ever happen, but handle it anyway
                        newPersistentIndices.push_back(QModelIndex());
                        continue;
                    }

                    const int oldRow = oldPersistentIndex.row();
                    const int oldColumn = oldPersistentIndex.column();

                    const String oldKey = m_rows.at(static_cast<size_t>(oldRow)).name();

                    // see if there is a corresponding new row
                    auto it = newRowIndexMap.find(oldKey);
                    if (it != newRowIndexMap.end()) {
                        const int newRow = it->second;
                        newPersistentIndices.push_back(index(newRow, oldColumn));
                    } else {
                        newPersistentIndices.push_back(QModelIndex());
                    }
                }

                m_rows = newRows;
                changePersistentIndexList(oldPersistentIndices, newPersistentIndices);
                emit layoutChanged();
            } else {
                m_rows = newRows;
            }

            // If any values changed, report them to Qt
            for (size_t i = 0; i < m_rows.size(); ++i) {
                const AttributeRow& newRow = m_rows[i];
                const String& key = newRow.name();

                bool rowChanged = true;

                auto it = oldRowIndexMap.find(key);
                if (it != oldRowIndexMap.end()) {
                    const int oldRowIndex = it->second;
                    const AttributeRow oldRow = oldRows.at(static_cast<size_t>(oldRowIndex));

                    if (oldRow == newRow) {
                        rowChanged = false;
                    }
                }

                if (rowChanged) {
                    const QModelIndex topLeft = index(static_cast<int>(i), 0);
                    const QModelIndex bottomRight = index(static_cast<int>(i), 1);
                    emit dataChanged(topLeft, bottomRight);
                }
            }
        }

        const AttributeRow* EntityAttributeModel::dataForModelIndex(const QModelIndex& index) const {
            if (!index.isValid()) {
                return nullptr;
            }
            return &m_rows.at(static_cast<size_t>(index.row()));
        }

        int EntityAttributeModel::rowForAttributeName(const String& name) const {
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

            StringList result;
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

        Model::AttributeName EntityAttributeModel::attributeName(const int row) const {
            if (row < 0 || row >= static_cast<int>(m_rows.size())) {
                return "";
            } else {
                return m_rows[static_cast<size_t>(row)].name();
            }
        }

        Model::AttributeNameList EntityAttributeModel::attributeNames(const int row, const int count) const {
            Model::AttributeNameList result;
            for (int i = 0; i < count; ++i) {
                result.push_back(this->attributeName(row + i));
            }
            return result;
        }

        StringList EntityAttributeModel::getAllAttributeNames() const {
            auto document = lock(m_document);
            const auto& index = document->world()->attributableNodeIndex();
            auto result = index.allNames();

            // remove duplicates and sort
            VectorUtils::setCreate(result);

            // also add keys from all loaded entity definitions
            for (const auto* entityDefinition : document->entityDefinitionManager().definitions()) {
                for (const auto& attributeDefinition : entityDefinition->attributeDefinitions()) {
                    VectorUtils::setInsert(result, attributeDefinition->name());
                }
            }

            // remove the empty string
            VectorUtils::setRemove(result, "");
            return result;
        }

        StringList EntityAttributeModel::getAllValuesForAttributeNames(const StringList& names) const {
            auto document = lock(m_document);
            const auto& index = document->world()->attributableNodeIndex();

            StringList result;
            for (const auto& name : names) {
                const auto values = index.allValuesForNames(Model::AttributableNodeIndexQuery::numbered(name));
                for (const auto& value : values) {
                    VectorUtils::setInsert(result, value);
                }
            }

            // remove the empty string
            VectorUtils::setRemove(result, "");
            return result;
        }

        StringList EntityAttributeModel::getAllClassnames() const {
            auto document = lock(m_document);

            // start with currently used classnames
            auto result = getAllValuesForAttributeNames({ Model::AttributeNames::Classname });

            // add keys from all loaded entity definitions
            for (const auto* entityDefinition : document->entityDefinitionManager().definitions()) {
                VectorUtils::setInsert(result, entityDefinition->name());
            }

            // remove the empty string
            VectorUtils::setRemove(result, "");
            return result;
        }

        void EntityAttributeModel::updateFromMapDocument() {
            qDebug() << "updateFromMapDocument";

            MapDocumentSPtr document = lock(m_document);

            const std::map<String, AttributeRow> rowsMap =
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
            const AttributeRow& issue = m_rows.at(static_cast<size_t>(index.row()));

            Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;

            if (index.column() == 0) {
                if (issue.nameMutable()) {
                    flags |= Qt::ItemIsEditable;
                }
            } else {
                if (issue.valueMutable()) {
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
                    return QVariant(QBrush(Colors::disabledText()));
                }
                if (index.column() == 1) {
                    if (row.multi()) {
                        return QVariant(QBrush(Colors::disabledText()));
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
                    return QVariant(QString::fromStdString(row.name()));
                } else {
                    return QVariant(QString::fromStdString(row.value()));
                }
            }

            if (role == Qt::ToolTipRole) {
                if (!row.tooltip().empty()) {
                    return QVariant(QString::fromStdString(row.tooltip()));
                }
            }

            return QVariant();
        }

        bool EntityAttributeModel::setData(const QModelIndex &index, const QVariant &value, int role) {
            const auto& attributeRow = m_rows.at(static_cast<size_t>(index.row()));

            if (role != Qt::EditRole) {
                return false;
            }

            MapDocumentSPtr document = lock(m_document);

            const size_t rowIndex = static_cast<size_t>(index.row());
            const Model::AttributableNodeList attributables = document->allSelectedAttributableNodes();
            if (attributables.empty()) {
                return false;
            }

            if (index.column() == 0) {
                // rename key
                qDebug() << "tried to rename " << QString::fromStdString(attributeRow.name()) << " to " << value.toString();

                const String newName = value.toString().toStdString();
                if (renameAttribute(rowIndex, newName, attributables)) {
                    // Queue selection of the renamed key.
                    // Not executed immediately because we need to wait for EntityAttributeGrid::updateControls() to
                    // call EntityAttributeModel::setRows().
                    QTimer::singleShot(0, this, [this, newName]() {
                        const int row = this->rowForAttributeName(newName);
                        if (row != -1) {
                            emit currentItemChangeRequestedByModel(this->index(row, 1));
                        }
                    });
                    return true;
                }
            } else if (index.column() == 1) {
                qDebug() << "tried to set " << QString::fromStdString(attributeRow.name()) << " to "
                         << value.toString();

                if (updateAttribute(rowIndex, value.toString().toStdString(), attributables)) {
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

            MapDocumentSPtr document = lock(m_document);

            const Model::AttributableNodeList attributables = document->allSelectedAttributableNodes();
            ensure(!attributables.empty(), "no attributable nodes selected");

            const String newKey = AttributeRow::newAttributeNameForAttributableNodes(attributables);

            const Transaction transaction(document);
            document->setAttribute(newKey, "");

            return true;
        }

        bool EntityAttributeModel::AppendRow() {
            return InsertRow(m_rows.size());
        }

        bool EntityAttributeModel::DeleteRows(const size_t pos, size_t numRows) {
            if (pos >= m_rows.size())
                return false;

            // FIXME: dangerous use of size_t, convert all of this to int
            numRows = std::min(m_rows.size() - pos, numRows);
            ensure(pos + numRows <= m_rows.size(), "row range exceeds row count");

            MapDocumentSPtr document = lock(m_document);

            const Model::AttributableNodeList attributables = document->allSelectedAttributableNodes();
            ensure(!attributables.empty(), "no attributable nodes selected");

            const StringList names = attributeNames(static_cast<int>(pos), static_cast<int>(numRows));
            ensure(names.size() == numRows, "invalid number of row names");

            {
                Transaction transaction(document, StringUtils::safePlural(numRows, "Remove Attribute", "Remove Attributes"));

                bool success = true;
                for (size_t i = 0; i < numRows && success; i++)
                    success = document->removeAttribute(names[i]);

                if (!success) {
                    transaction.rollback();
                    return false;
                }
            }

            return true;
        }

        int EntityAttributeModel::rowForName(const Model::AttributeName& name) const {
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

        bool EntityAttributeModel::hasRowWithAttributeName(const Model::AttributeName& name) const {
            return rowForAttributeName(name) != -1;
        }

        bool EntityAttributeModel::renameAttribute(const size_t rowIndex, const String& newName, const Model::AttributableNodeList& attributables) {
            ensure(rowIndex < m_rows.size(), "row index out of bounds");

            const AttributeRow& row = m_rows.at(rowIndex);
            const String& oldName = row.name();

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
                    .arg(QString::fromStdString(newName)));
                msgBox.setIcon(QMessageBox::Critical);
                msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
                if (msgBox.exec() == QMessageBox::No) {
                    return false;
                }
            }

            MapDocumentSPtr document = lock(m_document);
            return document->renameAttribute(oldName, newName);
        }

        bool EntityAttributeModel::updateAttribute(const size_t rowIndex, const String& newValue, const Model::AttributableNodeList& attributables) {
            ensure(rowIndex < m_rows.size(), "row index out of bounds");

            bool hasChange = false;
            const String name = m_rows.at(rowIndex).name();
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

            MapDocumentSPtr document = lock(m_document);
            return document->setAttribute(name, newValue);
        }
    }
}
