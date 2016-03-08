/*
 Copyright (C) 2010-2014 Kristian Duske
 
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
#include "Model/AttributableNode.h"
#include "Model/Entity.h"
#include "Model/EntityAttributes.h"
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
            assert(!m_default || m_valueMutable);
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
            assert(rowIndex < totalRowCount());
            return m_rows[rowIndex].isDefault();
        }

        size_t EntityAttributeGridTable::RowManager::indexOf(const String& name) const {
            AttributeRow::List::const_iterator propIt = findRow(m_rows, name);
            if (propIt != m_rows.end())
                return static_cast<size_t>(std::distance(m_rows.begin(), propIt));
            return totalRowCount();
        }

        const String& EntityAttributeGridTable::RowManager::name(const size_t rowIndex) const {
            assert(rowIndex < totalRowCount());
            return m_rows[rowIndex].name();
        }
        
        const String& EntityAttributeGridTable::RowManager::value(const size_t rowIndex) const {
            assert(rowIndex < totalRowCount());
            const AttributeRow& row = m_rows[rowIndex];
            return row.multi() ? EmptyString : row.value();
        }
        
        bool EntityAttributeGridTable::RowManager::nameMutable(const size_t rowIndex) const {
            assert(rowIndex < totalRowCount());
            return m_rows[rowIndex].nameMutable();
        }
        
        bool EntityAttributeGridTable::RowManager::valueMutable(const size_t rowIndex) const {
            assert(rowIndex < totalRowCount());
            return m_rows[rowIndex].valueMutable();
        }

        const String& EntityAttributeGridTable::RowManager::tooltip(const size_t rowIndex) const {
            assert(rowIndex < totalRowCount());
            return m_rows[rowIndex].tooltip();
        }

        bool EntityAttributeGridTable::RowManager::multi(const size_t rowIndex) const {
            assert(rowIndex < totalRowCount());
            return m_rows[rowIndex].multi();
        }

        bool EntityAttributeGridTable::RowManager::subset(const size_t rowIndex) const {
            assert(rowIndex < totalRowCount());
            return m_rows[rowIndex].subset();
        }

        const StringList EntityAttributeGridTable::RowManager::names(const size_t rowIndex, const size_t count) const {
            assert(rowIndex + count <= totalRowCount());
            
            StringList result(count);
            for (size_t i = 0; i < count; ++i)
                result[i] = m_rows[rowIndex + i].name();
            return result;
        }

        void EntityAttributeGridTable::RowManager::updateRows(const Model::AttributableNodeList& attributables, const bool showDefaultRows) {
            m_rows.clear();
            m_defaultRowCount = 0;
            
            Model::AttributableNodeList::const_iterator attriutableIt, attributableEnd;
            Model::EntityAttribute::List::const_iterator attributeIt, attributeEnd;
            
            for (attriutableIt = attributables.begin(),
                 attributableEnd = attributables.end();
                 attriutableIt != attributableEnd;
                 ++attriutableIt) {
                
                const Model::AttributableNode* attributable = *attriutableIt;
                const Model::EntityAttribute::List& attributes = attributable->attributes();
                for (attributeIt = attributes.begin(),
                     attributeEnd = attributes.end();
                     attributeIt != attributeEnd;
                     ++attributeIt) {
                    
                    const Model::EntityAttribute& attribute = *attributeIt;
                    const Assets::AttributeDefinition* attributeDefinition = attribute.definition();
                    
                    const bool nameMutable = attributable->isAttributeNameMutable(attribute.name());
                    const bool valueMutable = attributable->isAttributeValueMutable(attribute.value());
                    
                    AttributeRow::List::iterator rowIt = findRow(m_rows, attribute.name());
                    if (rowIt != m_rows.end()) {
                        rowIt->merge(attribute.value(), nameMutable, valueMutable);
                    } else {
                        const String tooltip = Assets::AttributeDefinition::safeFullDescription(attributeDefinition);
                        m_rows.push_back(AttributeRow(attribute.name(), attribute.value(),
                                                      nameMutable, valueMutable,
                                                      tooltip, false, attributables.size()));
                    }
                }
            }
            
            if (showDefaultRows) {
                const Assets::EntityDefinition* definition = Model::AttributableNode::selectEntityDefinition(attributables);
                if (definition != NULL) {
                    const Assets::AttributeDefinitionList& attributeDefs = definition->attributeDefinitions();
                    Assets::AttributeDefinitionList::const_iterator definitionIt, definitionEnd;
                    for (definitionIt = attributeDefs.begin(),
                         definitionEnd = attributeDefs.end();
                         definitionIt != definitionEnd;
                         ++definitionIt) {
                        
                        const Assets::AttributeDefinitionPtr propertyDef = *definitionIt;
                        const String& name = propertyDef->name();
                        
                        if (findRow(m_rows, name) != m_rows.end())
                            continue;
                        
                        const String value = Assets::AttributeDefinition::defaultValue(*propertyDef);
                        const String tooltip = Assets::AttributeDefinition::safeFullDescription(propertyDef.get());
                        m_rows.push_back(AttributeRow(name, value, false, true, tooltip, true, attributables.size()));
                        ++m_defaultRowCount;
                    }
                }
            }
        }

        StringList EntityAttributeGridTable::RowManager::insertRows(const size_t rowIndex, const size_t count, const Model::AttributableNodeList& attributables) {
            assert(rowIndex <= attributeRowCount());
            
            const StringList attributeNames = newAttributeNames(count, attributables);
            assert(attributeNames.size() == count);
            
            AttributeRow::List::iterator entryIt = m_rows.begin();
            std::advance(entryIt, rowIndex);
            for (size_t i = 0; i < count; i++) {
                entryIt = m_rows.insert(entryIt, AttributeRow(attributeNames[i], "", true, true, "", false, attributables.size()));
                entryIt->reset();
                std::advance(entryIt, 1);
            }
            
            return attributeNames;
        }

        void EntityAttributeGridTable::RowManager::deleteRows(const size_t rowIndex, const size_t count) {
            assert(rowIndex + count <= attributeRowCount());
            
            AttributeRow::List::iterator first = m_rows.begin();
            AttributeRow::List::iterator last = first;
            std::advance(first, rowIndex);
            std::advance(last, rowIndex + count);
            m_rows.erase(first, last);
        }
        
        EntityAttributeGridTable::AttributeRow::List::iterator EntityAttributeGridTable::RowManager::findRow(AttributeRow::List& rows, const String& name) {
            AttributeRow::List::iterator it, end;
            for (it = rows.begin(), end = rows.end(); it != end; ++it) {
                const AttributeRow& row = *it;
                if (row.name() == name)
                    return it;
            }
            return end;
        }

        EntityAttributeGridTable::AttributeRow::List::const_iterator EntityAttributeGridTable::RowManager::findRow(const AttributeRow::List& rows, const String& name) {
            AttributeRow::List::const_iterator it, end;
            for (it = rows.begin(), end = rows.end(); it != end; ++it) {
                const AttributeRow& row = *it;
                if (row.name() == name)
                    return it;
            }
            return end;
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
                    Model::AttributableNodeList::const_iterator it, end;
                    for (it = attributables.begin(), end = attributables.end(); it != end && indexIsFree; ++it) {
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
        m_readonlyCellColor(wxColor(224, 224, 224)),
        m_specialCellColor(wxColor(128, 128, 128)) {}
        
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
            
            assert(row >= 0 && row < GetRowsCount());
            assert(col >= 0 && col < GetColsCount());
            
            const size_t rowIndex = static_cast<size_t>(row);
            if (col == 0)
                return m_rows.name(rowIndex);
            return m_rows.value(rowIndex);
        }
        
        void EntityAttributeGridTable::SetValue(const int row, const int col, const wxString& value) {
            assert(row >= 0 && row < GetRowsCount());
            assert(col >= 0 && col < GetColsCount());
            
            MapDocumentSPtr document = lock(m_document);
            
            const size_t rowIndex = static_cast<size_t>(row);
            const Model::AttributableNodeList attributables = document->allSelectedAttributableNodes();
            assert(!attributables.empty());
            
            // Ignoring the updates here fails if the user changes the entity classname because in that
            // case, we must really refresh everything from the entity.
            // const SetBool ignoreUpdates(m_ignoreUpdates);
            if (col == 0)
                renameAttribute(rowIndex, value.ToStdString(), attributables);
            else
                updateAttribute(rowIndex, value.ToStdString(), attributables);
        }
        
        void EntityAttributeGridTable::Clear() {
            DeleteRows(0, m_rows.totalRowCount());
        }
        
        bool EntityAttributeGridTable::InsertRows(const size_t pos, const size_t numRows) {
            assert(pos <= m_rows.totalRowCount());
            
            MapDocumentSPtr document = lock(m_document);

            const Model::AttributableNodeList attributables = document->allSelectedAttributableNodes();
            assert(!attributables.empty());
            
            const StringList newKeys = m_rows.insertRows(pos, numRows, attributables);

            const SetBool ignoreUpdates(m_ignoreUpdates);

            const Transaction transaction(document);
            StringList::const_iterator it, end;
            for (it = newKeys.begin(), end = newKeys.end(); it != end; ++it) {
                const String& name = *it;
                document->setAttribute(name, "");
            }
            
            notifyRowsInserted(pos, numRows);
            
            return true;
        }
        
        bool EntityAttributeGridTable::AppendRows(const size_t numRows) {
            return InsertRows(m_rows.totalRowCount(), numRows);
        }
        
        bool EntityAttributeGridTable::DeleteRows(const size_t pos, size_t numRows) {
            // TODO: when deleting a property that has a default value in the property definition, re-add it to the list
            // of default properties...

            if (pos >= m_rows.totalRowCount())
                return false;
            
            numRows = std::min(m_rows.totalRowCount() - pos, numRows);
            assert(pos + numRows <= m_rows.totalRowCount());
            
            MapDocumentSPtr document = lock(m_document);

            const Model::AttributableNodeList attributables = document->allSelectedAttributableNodes();
            assert(!attributables.empty());
            
            const StringList names = m_rows.names(pos, numRows);
            assert(names.size() == numRows);
            
            const SetBool ignoreUpdates(m_ignoreUpdates);
            
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
            return true;
        }
        
        wxString EntityAttributeGridTable::GetColLabelValue(const int col) {
            assert(col >= 0 && col < GetColsCount());
            if (col == 0)
                return "Key";
            return "Value";
        }
        
        wxGridCellAttr* EntityAttributeGridTable::GetAttr(const int row, const int col, const wxGridCellAttr::wxAttrKind kind) {
            if (row < 0 || row >= GetRowsCount() ||
                col < 0 || col >= GetColsCount())
                return NULL;
            
            const size_t rowIndex = static_cast<size_t>(row);
            wxGridCellAttr* attr = wxGridTableBase::GetAttr(row, col, kind);
            if (attr == NULL)
                attr = new wxGridCellAttr();
            
            if (col == 0) {
                if (m_rows.isDefaultRow(rowIndex)) {
                    attr->SetFont(GetView()->GetFont().MakeItalic());
                    attr->SetReadOnly();
                } else {
                    attr->SetFont(GetView()->GetFont());
                    
                    if (!m_rows.nameMutable(rowIndex)) {
                        attr->SetReadOnly(true);
                        attr->SetBackgroundColour(m_readonlyCellColor);
                    } else if (m_rows.subset(rowIndex)) {
                        attr->SetTextColour(m_specialCellColor);
                    }
                }
            } else if (col == 1) {
                if (m_rows.isDefaultRow(rowIndex)) {
                    attr->SetFont(GetView()->GetFont().MakeItalic());
                } else {
                    attr->SetFont(GetView()->GetFont());

                    if (!m_rows.valueMutable(rowIndex)) {
                        attr->SetReadOnly(true);
                        attr->SetBackgroundColour(m_readonlyCellColor);
                    }
                    if (m_rows.multi(rowIndex))
                        attr->SetTextColour(m_specialCellColor);
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

        void EntityAttributeGridTable::renameAttribute(const size_t rowIndex, const String& newName, const Model::AttributableNodeList& attributables) {
            assert(rowIndex < m_rows.attributeRowCount());
            
            const String& oldName = m_rows.name(rowIndex);
            if (!m_rows.nameMutable(rowIndex)) {
                wxString msg;
                msg << "Cannot rename attribute '" << oldName << "' to '" << newName << "'";
                wxMessageBox(msg, "Error", wxOK | wxICON_ERROR | wxCENTRE, GetView());
                return;
            }

            MapDocumentSPtr document = lock(m_document);
            if (document->renameAttribute(oldName, newName)) {
                m_rows.updateRows(attributables, m_showDefaultRows);
                notifyRowsUpdated(0, m_rows.totalRowCount());
            }
        }
        
        void EntityAttributeGridTable::updateAttribute(const size_t rowIndex, const String& newValue, const Model::AttributableNodeList& attributables) {
            assert(rowIndex < m_rows.totalRowCount());

            const String& name = m_rows.name(rowIndex);
            Model::AttributableNodeList::const_iterator it, end;
            for (it = attributables.begin(), end = attributables.end(); it != end; ++it) {
                const Model::AttributableNode* attributable = *it;
                if (attributable->hasAttribute(name)) {
                    if (!attributable->canAddOrUpdateAttribute(name, newValue)) {
                        const Model::AttributeValue& oldValue = attributable->attribute(name);
                        wxString msg;
                        msg << "Cannot change property value '" << oldValue << "' to '" << newValue << "'";
                        wxMessageBox(msg, "Error", wxOK | wxICON_ERROR | wxCENTRE, GetView());
                        return;
                    }
                }
            }

            MapDocumentSPtr document = lock(m_document);
            if (document->setAttribute(name, newValue)) {
                m_rows.updateRows(attributables, m_showDefaultRows);
                notifyRowsUpdated(0, m_rows.totalRowCount());
            }
        }
        
        void EntityAttributeGridTable::notifyRowsUpdated(size_t pos, size_t numRows) {
            if (GetView() != NULL) {
                wxGridTableMessage message(this, wxGRIDTABLE_REQUEST_VIEW_GET_VALUES,
                                           static_cast<int>(pos),
                                           static_cast<int>(numRows));
                GetView()->ProcessTableMessage(message);
            }
        }
        
        void EntityAttributeGridTable::notifyRowsInserted(size_t pos, size_t numRows) {
            if (GetView() != NULL) {
                wxGridTableMessage message(this, wxGRIDTABLE_NOTIFY_ROWS_INSERTED,
                                           static_cast<int>(pos),
                                           static_cast<int>(numRows));
                GetView()->ProcessTableMessage(message);
            }
        }
        
        void EntityAttributeGridTable::notifyRowsAppended(size_t numRows) {
            if (GetView() != NULL) {
                wxGridTableMessage message(this, wxGRIDTABLE_NOTIFY_ROWS_APPENDED,
                                           static_cast<int>(numRows));
                GetView()->ProcessTableMessage(message);
            }
        }
        
        void EntityAttributeGridTable::notifyRowsDeleted(size_t pos, size_t numRows) {
            if (GetView() != NULL) {
                wxGridTableMessage message(this, wxGRIDTABLE_NOTIFY_ROWS_DELETED,
                                           static_cast<int>(pos),
                                           static_cast<int>(numRows));
                GetView()->ProcessTableMessage(message);
            }
        }
    }
}
