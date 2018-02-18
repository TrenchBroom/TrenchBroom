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

#include "EntityAttributeGridTable.h"

#include "Assets/AttributeDefinition.h"
#include "Assets/EntityDefinition.h"
#include "Assets/EntityDefinitionManager.h"
#include "Model/AttributableNode.h"
#include "Model/Entity.h"
#include "Model/EntityAttributes.h"
#include "Model/World.h"
#include "View/MapDocument.h"
#include "View/ViewUtils.h"

#include <wx/msgdlg.h>

namespace TrenchBroom {
    namespace View {
        EntityAttributeGridTable::AttributeRow::AttributeRow() :
        m_nameMutable(false),
        m_valueMutable(false),
        m_default(false),
        m_maxCount(0),
        m_count(0),
        m_multi(false) {}
        
        EntityAttributeGridTable::AttributeRow::AttributeRow(const String& name, const String& value, const bool nameMutable, const bool valueMutable, const String& tooltip, const bool i_default, const size_t maxCount) :
        m_name(name),
        m_value(value),
        m_nameMutable(nameMutable),
        m_valueMutable(valueMutable),
        m_tooltip(tooltip),
        m_default(i_default),
        m_maxCount(maxCount),
        m_count(1),
        m_multi(false) {
            ensure(!m_default || m_valueMutable, "attribute row cannot be default and immutable");
        }
        
        const String& EntityAttributeGridTable::AttributeRow::name() const {
            return m_name;
        }
        
        const String& EntityAttributeGridTable::AttributeRow::value() const {
            return m_value;
        }
        
        bool EntityAttributeGridTable::AttributeRow::nameMutable() const {
            return m_nameMutable;
        }
        
        bool EntityAttributeGridTable::AttributeRow::valueMutable() const {
            return m_valueMutable;
        }

        const String& EntityAttributeGridTable::AttributeRow::tooltip() const {
            return m_multi ? EmptyString : m_tooltip;
        }

        bool EntityAttributeGridTable::AttributeRow::isDefault() const {
            return m_default;
        }
        
        void EntityAttributeGridTable::AttributeRow::merge(const String& i_value, const bool nameMutable, const bool valueMutable) {
            m_multi |= (m_value != i_value);
            m_nameMutable &= nameMutable;
            m_valueMutable &= valueMutable;
            m_default = false;
            ++m_count;
        }
        
        bool EntityAttributeGridTable::AttributeRow::multi() const {
            return m_multi;
        }
        
        bool EntityAttributeGridTable::AttributeRow::subset() const {
            return m_count < m_maxCount;
        }
        
        void EntityAttributeGridTable::AttributeRow::reset() {
            m_count = m_maxCount;
            m_multi = false;
        }

        size_t EntityAttributeGridTable::RowManager::totalRowCount() const {
            return m_rows.size();
        }

        size_t EntityAttributeGridTable::RowManager::defaultRowCount() const {
            return m_defaultRowCount;
        }

        size_t EntityAttributeGridTable::RowManager::attributeRowCount() const {
            return totalRowCount() - defaultRowCount();
        }
        
        bool EntityAttributeGridTable::RowManager::isAttributeRow(const size_t rowIndex) const {
            return !isDefaultRow(rowIndex);
        }
        
        bool EntityAttributeGridTable::RowManager::isDefaultRow(const size_t rowIndex) const {
            ensure(rowIndex < totalRowCount(), "row index out of bounds");
            return m_rows[rowIndex].isDefault();
        }

        size_t EntityAttributeGridTable::RowManager::indexOf(const String& name) const {
            AttributeRow::List::const_iterator propIt = findRow(m_rows, name);
            if (propIt != std::end(m_rows))
                return static_cast<size_t>(std::distance(std::begin(m_rows), propIt));
            return totalRowCount();
        }

        const String& EntityAttributeGridTable::RowManager::name(const size_t rowIndex) const {
            ensure(rowIndex < totalRowCount(), "row index out of bounds");
            return m_rows[rowIndex].name();
        }
        
        const String& EntityAttributeGridTable::RowManager::value(const size_t rowIndex) const {
            ensure(rowIndex < totalRowCount(), "row index out of bounds");
            const AttributeRow& row = m_rows[rowIndex];
            return row.multi() ? EmptyString : row.value();
        }
        
        bool EntityAttributeGridTable::RowManager::nameMutable(const size_t rowIndex) const {
            ensure(rowIndex < totalRowCount(), "row index out of bounds");
            return m_rows[rowIndex].nameMutable();
        }
        
        bool EntityAttributeGridTable::RowManager::valueMutable(const size_t rowIndex) const {
            ensure(rowIndex < totalRowCount(), "row index out of bounds");
            return m_rows[rowIndex].valueMutable();
        }

        const String& EntityAttributeGridTable::RowManager::tooltip(const size_t rowIndex) const {
            ensure(rowIndex < totalRowCount(), "row index out of bounds");
            return m_rows[rowIndex].tooltip();
        }

        bool EntityAttributeGridTable::RowManager::multi(const size_t rowIndex) const {
            ensure(rowIndex < totalRowCount(), "row index out of bounds");
            return m_rows[rowIndex].multi();
        }

        bool EntityAttributeGridTable::RowManager::subset(const size_t rowIndex) const {
            ensure(rowIndex < totalRowCount(), "row index out of bounds");
            return m_rows[rowIndex].subset();
        }

        StringList EntityAttributeGridTable::RowManager::names(const size_t rowIndex, const size_t count) const {
            ensure(rowIndex + count <= totalRowCount(), "row range exceeds row count");
            
            StringList result(count);
            for (size_t i = 0; i < count; ++i)
                result[i] = m_rows[rowIndex + i].name();
            return result;
        }

        bool EntityAttributeGridTable::RowManager::hasRowWithName(const String& name) const {
            for (size_t i = 0; i < attributeRowCount(); ++i) {
                const auto& row = m_rows[i];
                if (row.name() == name)
                    return true;
            }
            return false;
        }

        void EntityAttributeGridTable::RowManager::updateRows(const Model::AttributableNodeList& attributables, const bool showDefaultRows) {
            m_rows.clear();
            m_defaultRowCount = 0;
            
            for (const Model::AttributableNode* attributable : attributables) {
                for (const Model::EntityAttribute& attribute : attributable->attributes()) {
                    const Model::AttributeName& name = attribute.name();
                    const Model::AttributeValue& value = attribute.value();
                    const Assets::AttributeDefinition* definition = attribute.definition();
                    
                    const bool nameMutable = attributable->isAttributeNameMutable(attribute.name());
                    const bool valueMutable = attributable->isAttributeValueMutable(attribute.value());
                    
                    addAttribute(name, value, definition, nameMutable, valueMutable, false, attributables.size());
                }
            }
            
            if (showDefaultRows) {
                for (const Model::AttributableNode* attributable : attributables) {
                    const Assets::EntityDefinition* entityDefinition = attributable->definition();
                    if (entityDefinition != nullptr) {
                        for (Assets::AttributeDefinitionPtr attributeDefinition : entityDefinition->attributeDefinitions()) {
                            const String& name = attributeDefinition->name();
                            if (findRow(m_rows, name) != std::end(m_rows))
                                continue;
                            
                            const String value = Assets::AttributeDefinition::defaultValue(*attributeDefinition.get());
                            addAttribute(name, value, attributeDefinition.get(), false, true, true, attributables.size());
                            ++m_defaultRowCount;
                        }
                    }
                }
            }
        }
        
        void EntityAttributeGridTable::RowManager::addAttribute(const Model::AttributeName& name, const Model::AttributeValue& value, const Assets::AttributeDefinition* definition, const bool nameMutable, const bool valueMutable, const bool isDefault, const size_t index) {
            AttributeRow::List::iterator rowIt = findRow(m_rows, name);
            if (rowIt != std::end(m_rows)) {
                rowIt->merge(value, nameMutable, valueMutable);
            } else {
                const String tooltip = Assets::AttributeDefinition::safeFullDescription(definition);
                m_rows.push_back(AttributeRow(name, value, nameMutable, valueMutable, tooltip, isDefault, index));
            }
        }

        StringList EntityAttributeGridTable::RowManager::insertRows(const size_t rowIndex, const size_t count, const Model::AttributableNodeList& attributables) {
            ensure(rowIndex <= attributeRowCount(), "row index out of bounds");
            
            const StringList attributeNames = newAttributeNames(count, attributables);
            ensure(attributeNames.size() == count, "invalid number of new attribute names");
            
            AttributeRow::List::iterator entryIt = std::begin(m_rows);
            std::advance(entryIt, static_cast<AttributeRow::List::iterator::difference_type>(rowIndex));
            for (size_t i = 0; i < count; i++) {
                entryIt = m_rows.insert(entryIt, AttributeRow(attributeNames[i], "", true, true, "", false, attributables.size()));
                entryIt->reset();
                std::advance(entryIt, 1);
            }
            
            return attributeNames;
        }

        void EntityAttributeGridTable::RowManager::deleteRows(const size_t rowIndex, const size_t count) {
            ensure(rowIndex + count <= attributeRowCount(), "row range exceeds row count");
            
            AttributeRow::List::iterator first = std::begin(m_rows);
            AttributeRow::List::iterator last = first;
            std::advance(first, static_cast<AttributeRow::List::iterator::difference_type>(rowIndex));
            std::advance(last, static_cast<AttributeRow::List::iterator::difference_type>(rowIndex + count));
            m_rows.erase(first, last);
        }
        
        EntityAttributeGridTable::AttributeRow::List::iterator EntityAttributeGridTable::RowManager::findRow(AttributeRow::List& rows, const String& name) {
            for (auto it = std::begin(rows), end = std::end(rows); it != end; ++it) {
                const AttributeRow& row = *it;
                if (row.name() == name)
                    return it;
            }
            return std::end(rows);
        }

        EntityAttributeGridTable::AttributeRow::List::const_iterator EntityAttributeGridTable::RowManager::findRow(const AttributeRow::List& rows, const String& name) {
            for (auto it = std::begin(rows), end = std::end(rows); it != end; ++it) {
                const AttributeRow& row = *it;
                if (row.name() == name)
                    return it;
            }
            return std::end(rows);
        }
        
        StringList EntityAttributeGridTable::RowManager::newAttributeNames(const size_t count, const Model::AttributableNodeList& attributables) const {
            StringList result;
            result.reserve(count);
            
            size_t index = 1;
            for (size_t i = 0; i < count; ++i) {
                while (true) {
                    StringStream nameStream;
                    nameStream << "property " << index;
                    
                    bool indexIsFree = true;
                    for (auto it = std::begin(attributables), end = std::end(attributables); it != end && indexIsFree; ++it) {
                        const Model::AttributableNode& attributable = **it;
                        indexIsFree = !attributable.hasAttribute(nameStream.str());
                    }
                    
                    if (indexIsFree) {
                        result.push_back(nameStream.str());
                        break;
                    }
                    
                    ++index;
                }
            }
            return result;
        }

        EntityAttributeGridTable::EntityAttributeGridTable(MapDocumentWPtr document) :
        m_document(document),
        m_rows(),
        m_ignoreUpdates(false),
        m_showDefaultRows(true),
        m_readonlyCellColor(wxColor(224, 224, 224)) {}
        
        int EntityAttributeGridTable::GetNumberRows() {
            return static_cast<int>(m_rows.totalRowCount());
        }
        
        int EntityAttributeGridTable::GetNumberAttributeRows() const {
            return static_cast<int>(m_rows.attributeRowCount());
        }

        int EntityAttributeGridTable::GetNumberCols() {
            return 2;
        }
        
        wxString EntityAttributeGridTable::GetValue(const int row, const int col) {
            // Fixes a problem when the user deselects everything while editing an entity property.
            if (row < 0 || col < 0)
                return wxEmptyString;
            
            ensure(row >= 0 && row < GetRowsCount(), "row index out of bounds");
            ensure(col >= 0 && col < GetColsCount(), "column index out of bounds");
            
            const size_t rowIndex = static_cast<size_t>(row);
            if (col == 0)
                return m_rows.name(rowIndex);
            
            if (m_rows.multi(rowIndex))
                return "multi";
            return m_rows.value(rowIndex);
        }
        
        void EntityAttributeGridTable::SetValue(const int row, const int col, const wxString& value) {
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
        
        void EntityAttributeGridTable::Clear() {
            DeleteRows(0, m_rows.totalRowCount());
        }
        
        bool EntityAttributeGridTable::InsertRows(const size_t pos, const size_t numRows) {
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
        
        bool EntityAttributeGridTable::AppendRows(const size_t numRows) {
            return InsertRows(m_rows.totalRowCount(), numRows);
        }
        
        bool EntityAttributeGridTable::DeleteRows(const size_t pos, size_t numRows) {
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
        
        wxString EntityAttributeGridTable::GetColLabelValue(const int col) {
            ensure(col >= 0 && col < GetColsCount(), "column index out of bounds");
            if (col == 0)
                return "Key";
            return "Value";
        }
        
        wxGridCellAttr* EntityAttributeGridTable::GetAttr(const int row, const int col, const wxGridCellAttr::wxAttrKind kind) {
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
                        attr->SetReadOnly(true);
                        attr->SetBackgroundColour(m_readonlyCellColor);
                    }
                }
            } else if (col == 1) {
                if (!m_rows.isDefaultRow(rowIndex)) {
                    attr->SetFont(GetView()->GetFont());
                }
                if (!m_rows.valueMutable(rowIndex)) {
                    attr->SetReadOnly(true);
                    attr->SetBackgroundColour(m_readonlyCellColor);
                }
                if (m_rows.multi(rowIndex)) {
                    attr->SetTextColour(*wxLIGHT_GREY);
                    attr->SetFont(GetView()->GetFont().MakeItalic());
                }
            }
            return attr;
        }
        
        void EntityAttributeGridTable::update() {
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

        String EntityAttributeGridTable::tooltip(const wxGridCellCoords cellCoords) const {
            if (cellCoords.GetRow() < 0 || cellCoords.GetRow() >= GetRowsCount())
                return "";
            
            const size_t rowIndex = static_cast<size_t>(cellCoords.GetRow());
            return m_rows.tooltip(rowIndex);
        }
        
        Model::AttributeName EntityAttributeGridTable::attributeName(const int row) const {
            if (row < 0 || row >= static_cast<int>(m_rows.totalRowCount()))
                return "";
            return m_rows.name(static_cast<size_t>(row));
        }
        
        int EntityAttributeGridTable::rowForName(const Model::AttributeName& name) const {
            const size_t index = m_rows.indexOf(name);
            if (index >= m_rows.totalRowCount())
                return -1;
            return static_cast<int>(index);
        }

        bool EntityAttributeGridTable::canRemove(const int row) {
            if (row < 0 || row >= GetNumberAttributeRows())
                return false;
            const size_t index = static_cast<size_t>(row);
            return m_rows.nameMutable(index) && m_rows.valueMutable(index);
        }

        bool EntityAttributeGridTable::showDefaultRows() const {
            return m_showDefaultRows;
        }
        
        void EntityAttributeGridTable::setShowDefaultRows(const bool showDefaultRows) {
            if (showDefaultRows == m_showDefaultRows)
                return;
            m_showDefaultRows = showDefaultRows;
            update();
        }

        wxArrayString EntityAttributeGridTable::getCompletions(int row, int col) const {
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
                }
            }
            
            return wxArrayString();
        }
        
        StringSet EntityAttributeGridTable::allSortedAttributeNames(MapDocumentSPtr document) {
            const Model::AttributableNodeIndex& index = document->world()->attributableNodeIndex();
            const StringList names = index.allNames();
            
            StringSet keySet = SetUtils::makeSet(names);
            
            // also add keys from all loaded entity definitions
            for (const auto& group : document->entityDefinitionManager().groups()) {
                for (const auto entityDefinition : group.definitions()) {
                    for (const auto& attribute : entityDefinition->attributeDefinitions()) {
                        keySet.insert(attribute->name());
                    }
                }
            }
            
            // an empty string prevents the completion popup from opening on macOS
            keySet.erase("");
            
            return keySet;
        }
        
        StringSet EntityAttributeGridTable::allSortedValuesForAttributeNames(MapDocumentSPtr document, const StringList& names) {
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
        
        wxArrayString EntityAttributeGridTable::arrayString(const StringSet& set) {
            wxArrayString result;
            for (const String& string : set)
                result.Add(wxString(string));
            return result;
        }

        void EntityAttributeGridTable::renameAttribute(const size_t rowIndex, const String& newName, const Model::AttributableNodeList& attributables) {
            ensure(rowIndex < m_rows.attributeRowCount(), "row index out of bounds");
            
            const String& oldName = m_rows.name(rowIndex);
            
            if (oldName == newName)
                return;
            
            if (!m_rows.nameMutable(rowIndex)) {
                wxString msg;
                msg << "Cannot rename property '" << oldName << "' to '" << newName << "'";
                wxMessageBox(msg, "Error", wxOK | wxICON_ERROR | wxCENTRE, GetView());
                return;
            }

            if (m_rows.hasRowWithName(newName)) {
                wxString msg;
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
        
        void EntityAttributeGridTable::updateAttribute(const size_t rowIndex, const String& newValue, const Model::AttributableNodeList& attributables) {
            ensure(rowIndex < m_rows.totalRowCount(), "row index out of bounds");

            bool hasChange = false;
            const String& name = m_rows.name(rowIndex);
            for (const Model::AttributableNode* attributable : attributables) {
                if (attributable->hasAttribute(name)) {
                    if (!attributable->canAddOrUpdateAttribute(name, newValue)) {
                        const Model::AttributeValue& oldValue = attributable->attribute(name);
                        wxString msg;
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
        
        void EntityAttributeGridTable::notifyRowsUpdated(size_t pos, size_t numRows) {
            if (GetView() != nullptr) {
                wxGridTableMessage message(this, wxGRIDTABLE_REQUEST_VIEW_GET_VALUES,
                                           static_cast<int>(pos),
                                           static_cast<int>(numRows));
                GetView()->ProcessTableMessage(message);
            }
        }
        
        void EntityAttributeGridTable::notifyRowsInserted(size_t pos, size_t numRows) {
            if (GetView() != nullptr) {
                wxGridTableMessage message(this, wxGRIDTABLE_NOTIFY_ROWS_INSERTED,
                                           static_cast<int>(pos),
                                           static_cast<int>(numRows));
                GetView()->ProcessTableMessage(message);
            }
        }
        
        void EntityAttributeGridTable::notifyRowsAppended(size_t numRows) {
            if (GetView() != nullptr) {
                wxGridTableMessage message(this, wxGRIDTABLE_NOTIFY_ROWS_APPENDED,
                                           static_cast<int>(numRows));
                GetView()->ProcessTableMessage(message);
            }
        }
        
        void EntityAttributeGridTable::notifyRowsDeleted(size_t pos, size_t numRows) {
            if (GetView() != nullptr) {
                wxGridTableMessage message(this, wxGRIDTABLE_NOTIFY_ROWS_DELETED,
                                           static_cast<int>(pos),
                                           static_cast<int>(numRows));
                GetView()->ProcessTableMessage(message);
            }
        }
    }
}
