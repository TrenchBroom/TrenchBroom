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
#include "Model/AttributableNode.h"
#include "Model/Entity.h"
#include "Model/EntityAttributes.h"
#include "Model/World.h"
#include "View/MapDocument.h"
#include "View/ViewUtils.h"
#include "View/ViewConstants.h"
#include "IO/ResourceUtils.h"

#include <QDebug>
#include <QBrush>
#include <QIcon>

namespace TrenchBroom {
    namespace View {

        // AttributeRow

        AttributeRow::AttributeRow() :
        m_valueType(ValueType::Unset),
        m_nameMutable(true),
        m_valueMutable(true) {}

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

            std::unique_ptr<AttributeRow> result;
            for (const Model::AttributableNode* node : attributables) {
                // this happens at startup when the world is still null
                if (node == nullptr) {
                    continue;
                }

                if (result == nullptr) {
                    result = std::make_unique<AttributeRow>(key, node);
                }  else {
                    result->merge(node);
                }
            }
            return *result;
        }

        std::set<String> AttributeRow::allKeys(const Model::AttributableNodeList& attributables) {
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
                const Assets::EntityDefinition* entityDefinition = node->definition();
                if (entityDefinition != nullptr) {
                   for (Assets::AttributeDefinitionPtr attributeDefinition : entityDefinition->attributeDefinitions()) {
                       result.insert(attributeDefinition->name());
                   }
                }
            }
            return result;
        }

        std::map<String, AttributeRow> AttributeRow::rowsForAttributableNodes(const Model::AttributableNodeList& attributables) {
            std::map<String, AttributeRow> result;
            for (const String& key : allKeys(attributables)) {
                result[key] = rowForAttributableNodes(key, attributables);
            }
            return result;
        }

        String AttributeRow::newAttributeNameForAttributableNodes(const Model::AttributableNodeList& attributables) {
            const std::map<String, AttributeRow> rows = rowsForAttributableNodes(attributables);

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
        m_document(std::move(document)) {
            updateFromMapDocument();
        }

        static auto buildVec(const std::map<String, AttributeRow>& rows) {
            std::vector<AttributeRow> result;
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

        void EntityAttributeModel::setRows(const std::map<String, AttributeRow>& newRowsKeyMap) {
            qDebug() << "EntityAttributeModel::setRows " << newRowsKeyMap.size() << " rows.";

            // Next we're going to update the persistent model indices

            const std::vector<AttributeRow> newRows = buildVec(newRowsKeyMap);
            const std::map<String, int> oldRowIndexMap = buildAttributeToRowIndexMap(m_rows);
            const std::map<String, int> newRowIndexMap = buildAttributeToRowIndexMap(newRows);

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

            // Next tell Qt the data changed for all of the rows
            if (!m_rows.empty()) {
                QModelIndex topLeft = index(0, 0);
                QModelIndex bottomRight = index(static_cast<int>(m_rows.size()) - 1, 1);
                emit dataChanged(topLeft, bottomRight);
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

        void EntityAttributeModel::updateFromMapDocument() {
            qDebug() << "updateFromMapDocument";

            MapDocumentSPtr document = lock(m_document);

            const auto rowsMap = AttributeRow::rowsForAttributableNodes(document->allSelectedAttributableNodes());

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

            if (index.column() == 0) {
                // rename key
                qDebug() << "tried to rename " << QString::fromStdString(attributeRow.name()) << " to " << value.toString();

                MapDocumentSPtr document = lock(m_document);
                if (document->renameAttribute(attributeRow.name(), value.toString().toStdString())) {
                    // TODO: reselect new row
                    return true;
                }


            } else if (index.column() == 1) {
                qDebug() << "tried to set " << QString::fromStdString(attributeRow.name()) << " to "
                         << value.toString();

                MapDocumentSPtr document = lock(m_document);
                if (document->setAttribute(attributeRow.name(), value.toString().toStdString())) {
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

        // Begin old code

#if 0
        void EntityAttributeModel::SetValue(const int row, const int col, const QString& value) {
            ensure(row >= 0 && row < GetRowsCount(), "row index out of bounds");
            ensure(col >= 0 && col < GetColsCount(), "column index out of bounds");

            MapDocumentSPtr document = lock(m_document);

            const size_t rowIndex = static_cast<size_t>(row);
            const Model::AttributableNodeList attributables = document->allSelectedAttributableNodes();
            ensure(!attributables.empty(), "no attributable nodes selected");

            // Ignoring the updates here fails if the user changes the entity classname because in that
            // case, we must really refresh everything from the entity.
            // const TemporarilySetBool ignoreUpdates(m_ignoreUpdates);
            if (col == 0)
                renameAttribute(rowIndex, value.ToStdString(), attributables);
            else
                updateAttribute(rowIndex, value.ToStdString(), attributables);
        }

        void EntityAttributeModel::Clear() {
            DeleteRows(0, m_rows.totalRowCount());
        }

        bool EntityAttributeModel::InsertRows(const size_t pos, const size_t numRows) {
            ensure(pos <= m_rows.totalRowCount(), "insertion position out of bounds");

            MapDocumentSPtr document = lock(m_document);

            const Model::AttributableNodeList attributables = document->allSelectedAttributableNodes();
            ensure(!attributables.empty(), "no attributable nodes selected");

            const StringList newKeys = m_rows.insertRows(pos, numRows, attributables);

            const TemporarilySetBool ignoreUpdates(m_ignoreUpdates);

            const Transaction transaction(document);
            for (const String& name : newKeys)
                document->setAttribute(name, "");

            notifyRowsInserted(pos, numRows);

            return true;
        }

        bool EntityAttributeModel::AppendRows(const size_t numRows) {
            return InsertRows(m_rows.totalRowCount(), numRows);
        }

        bool EntityAttributeModel::DeleteRows(const size_t pos, size_t numRows) {
            if (pos >= m_rows.totalRowCount())
                return false;

            numRows = std::min(m_rows.totalRowCount() - pos, numRows);
            ensure(pos + numRows <= m_rows.totalRowCount(), "row range exceeds row count");

            MapDocumentSPtr document = lock(m_document);

            const Model::AttributableNodeList attributables = document->allSelectedAttributableNodes();
            ensure(!attributables.empty(), "no attributable nodes selected");

            const StringList names = m_rows.names(pos, numRows);
            ensure(names.size() == numRows, "invalid number of row names");

            {
                const TemporarilySetBool ignoreUpdates(m_ignoreUpdates);

                Transaction transaction(document, StringUtils::safePlural(numRows, "Remove Attribute", "Remove Attributes"));

                bool success = true;
                for (size_t i = 0; i < numRows && success; i++)
                    success = document->removeAttribute(names[i]);

                if (!success) {
                    transaction.rollback();
                    return false;
                }

                m_rows.deleteRows(pos, numRows);
                notifyRowsDeleted(pos, numRows);
            }

            // Force an update in case we deleted a property with a default value
            update();

            return true;
        }

        QString EntityAttributeModel::GetColLabelValue(const int col) {
            ensure(col >= 0 && col < GetColsCount(), "column index out of bounds");
            if (col == 0)
                return "Key";
            return "Value";
        }

        wxGridCellAttr* EntityAttributeModel::GetAttr(const int row, const int col, const wxGridCellAttr::wxAttrKind kind) {
            if (row < 0 || row >= GetRowsCount() ||
                col < 0 || col >= GetColsCount())
                return nullptr;

            const size_t rowIndex = static_cast<size_t>(row);
            wxGridCellAttr* attr = wxGridTableBase::GetAttr(row, col, kind);
            if (attr == nullptr)
                attr = new wxGridCellAttr();

            if (m_rows.isDefaultRow(rowIndex) || m_rows.subset(rowIndex)) {
                attr->SetTextColour(*wxLIGHT_GREY);
                attr->SetFont(GetView()->GetFont().MakeItalic());
            }

            if (col == 0) {
                if (m_rows.isDefaultRow(rowIndex)) {
                    attr->SetReadOnly();
                } else {
                    if (!m_rows.nameMutable(rowIndex)) {
                        attr->SetReadOnly();
                        attr->SetRenderer(new LockedGridCellRenderer());
                    }
                }
            } else if (col == 1) {
                if (!m_rows.isDefaultRow(rowIndex)) {
                    attr->SetFont(GetView()->GetFont());
                }
                if (!m_rows.valueMutable(rowIndex)) {
                    attr->SetReadOnly();
                    attr->SetRenderer(new LockedGridCellRenderer());
                }
                if (m_rows.multi(rowIndex)) {
                    attr->SetTextColour(*wxLIGHT_GREY);
                    attr->SetFont(GetView()->GetFont().MakeItalic());
                }
            }
            return attr;
        }

        void EntityAttributeModel::update() {
            if (m_ignoreUpdates)
                return;

            MapDocumentSPtr document = lock(m_document);
            const size_t oldRowCount = m_rows.totalRowCount();
            m_rows.updateRows(document->allSelectedAttributableNodes(), m_showDefaultRows);
            const size_t newRowCount = m_rows.totalRowCount();

            if (oldRowCount < newRowCount)
                notifyRowsAppended(newRowCount - oldRowCount);
            else if (oldRowCount > newRowCount)
                notifyRowsDeleted(oldRowCount - 1, oldRowCount - newRowCount);
            notifyRowsUpdated(0, newRowCount);
        }

        String EntityAttributeModel::tooltip(const wxGridCellCoords& cellCoords) const {
            if (cellCoords.GetRow() < 0 || cellCoords.GetRow() >= GetRowsCount())
                return "";

            const size_t rowIndex = static_cast<size_t>(cellCoords.GetRow());
            return m_rows.tooltip(rowIndex);
        }

        Model::AttributeName EntityAttributeModel::attributeName(const int row) const {
            if (row < 0 || row >= static_cast<int>(m_rows.totalRowCount()))
                return "";
            return m_rows.name(static_cast<size_t>(row));
        }

        int EntityAttributeModel::rowForName(const Model::AttributeName& name) const {
            const size_t index = m_rows.indexOf(name);
            if (index >= m_rows.totalRowCount())
                return -1;
            return static_cast<int>(index);
        }

        bool EntityAttributeModel::canRemove(const int row) {
            if (row < 0 || row >= GetNumberAttributeRows())
                return false;
            const size_t index = static_cast<size_t>(row);
            return m_rows.nameMutable(index) && m_rows.valueMutable(index);
        }

        bool EntityAttributeModel::showDefaultRows() const {
            return m_showDefaultRows;
        }

        void EntityAttributeModel::setShowDefaultRows(const bool showDefaultRows) {
            if (showDefaultRows == m_showDefaultRows)
                return;
            m_showDefaultRows = showDefaultRows;
            update();
        }

        QStringList EntityAttributeModel::getCompletions(int row, int col) const {
            const Model::AttributeName name = attributeName(row);
            MapDocumentSPtr document = lock(m_document);

            if (col == 0) {
                return arrayString(allSortedAttributeNames(document));
            }

            if (col == 1) {
                if (name == Model::AttributeNames::Target
                    || name == Model::AttributeNames::Killtarget) {
                    return arrayString(allSortedValuesForAttributeNames(document, StringList{Model::AttributeNames::Targetname}));
                } else if (name == Model::AttributeNames::Targetname) {
                    return arrayString(allSortedValuesForAttributeNames(document, StringList{Model::AttributeNames::Target, Model::AttributeNames::Killtarget}));
                } else if (name == Model::AttributeNames::Classname) {
                    return arrayString(allSortedClassnames(document));
                }
            }

            return QStringList();
        }

        StringSet EntityAttributeModel::allSortedAttributeNames(MapDocumentSPtr document) {
            const Model::AttributableNodeIndex& index = document->world()->attributableNodeIndex();
            const StringList names = index.allNames();

            StringSet keySet = SetUtils::makeSet(names);

            // also add keys from all loaded entity definitions
            for (const auto entityDefinition : document->entityDefinitionManager().definitions()) {
                for (const auto& attribute : entityDefinition->attributeDefinitions()) {
                    keySet.insert(attribute->name());
                }
            }

            // an empty string prevents the completion popup from opening on macOS
            keySet.erase("");

            return keySet;
        }

        StringSet EntityAttributeModel::allSortedValuesForAttributeNames(MapDocumentSPtr document, const StringList& names) {
            StringSet valueset;
            const Model::AttributableNodeIndex& index = document->world()->attributableNodeIndex();
            for (const auto& name : names) {
                const StringList values = index.allValuesForNames(Model::AttributableNodeIndexQuery::numbered(name));
                for (const auto& value : values) {
                    valueset.insert(value);
                }
            }

            valueset.erase("");

            return valueset;
        }

        StringSet EntityAttributeModel::allSortedClassnames(MapDocumentSPtr document) {
            // Start with classnames in use in the map
            StringSet valueset = allSortedValuesForAttributeNames(document, StringList{ Model::AttributeNames::Classname });

            // Also add keys from all loaded entity definitions
            for (const auto entityDefinition : document->entityDefinitionManager().definitions()) {
                valueset.insert(entityDefinition->name());
            }

            valueset.erase("");

            return valueset;
        }

        wxArrayString EntityAttributeModel::arrayString(const StringSet& set) {
            wxArrayString result;
            for (const String& string : set)
                result.Add(QString(string));
            return result;
        }

        void EntityAttributeModel::renameAttribute(const size_t rowIndex, const String& newName, const Model::AttributableNodeList& attributables) {
            ensure(rowIndex < m_rows.attributeRowCount(), "row index out of bounds");

            const String& oldName = m_rows.name(rowIndex);

            if (oldName == newName)
                return;

            if (!m_rows.nameMutable(rowIndex)) {
                QString msg;
                msg << "Cannot rename property '" << oldName << "' to '" << newName << "'";
                wxMessageBox(msg, "Error", wxOK | wxICON_ERROR | wxCENTRE, GetView());
                return;
            }

            if (m_rows.hasRowWithName(newName)) {
                QString msg;
                msg << "A property with key '" << newName << "' already exists.\n\n Do you wish to overwrite it?";
                if (wxMessageBox(msg, "Error", wxYES_NO | wxICON_ERROR | wxCENTRE, GetView()) == wxNO) {
                    return;
                }
            }

            MapDocumentSPtr document = lock(m_document);
            if (document->renameAttribute(oldName, newName)) {
                m_rows.updateRows(attributables, m_showDefaultRows);
                notifyRowsUpdated(0, m_rows.totalRowCount());
            }
        }

        void EntityAttributeModel::updateAttribute(const size_t rowIndex, const String& newValue, const Model::AttributableNodeList& attributables) {
            ensure(rowIndex < m_rows.totalRowCount(), "row index out of bounds");

            bool hasChange = false;
            const String& name = m_rows.name(rowIndex);
            for (const Model::AttributableNode* attributable : attributables) {
                if (attributable->hasAttribute(name)) {
                    if (!attributable->canAddOrUpdateAttribute(name, newValue)) {
                        const Model::AttributeValue& oldValue = attributable->attribute(name);
                        QString msg;
                        msg << "Cannot change property value '" << oldValue << "' to '" << newValue << "'";
                        wxMessageBox(msg, "Error", wxOK | wxICON_ERROR | wxCENTRE, GetView());
                        return;
                    }
                    if (attributable->attribute(name) != newValue)
                        hasChange = true;
                } else {
                    hasChange = true;
                }
            }

            if (!hasChange)
                return;

            MapDocumentSPtr document = lock(m_document);
            if (document->setAttribute(name, newValue)) {
                m_rows.updateRows(attributables, m_showDefaultRows);
                notifyRowsUpdated(0, m_rows.totalRowCount());
            }
        }

#endif
    }
}
