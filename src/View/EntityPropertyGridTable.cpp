/*
 Copyright (C) 2010-2013 Kristian Duske
 
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

#include "EntityPropertyGridTable.h"

#include "Assets/EntityDefinition.h"
#include "Assets/PropertyDefinition.h"
#include "Model/Entity.h"
#include "Model/EntityProperties.h"
#include "View/ControllerFacade.h"
#include "View/MapDocument.h"

namespace TrenchBroom {
    namespace View {
        EntityPropertyGridTable::Entry::Entry() :
        m_maxCount(0),
        m_count(0),
        m_multi(false) {}
        
        EntityPropertyGridTable::Entry::Entry(const String& i_key, const String& i_value, const String& i_tooltip, size_t maxCount) :
        m_maxCount(maxCount),
        m_count(1),
        m_multi(false),
        key(i_key),
        value(i_value),
        tooltip(i_tooltip) {}
        
        void EntityPropertyGridTable::Entry::compareValue(const String& i_value) {
            if (!m_multi && value != i_value)
                m_multi = true;
            ++m_count;
        }
        
        bool EntityPropertyGridTable::Entry::multi() const {
            return m_multi;
        }
        
        bool EntityPropertyGridTable::Entry::subset() const {
            return m_count < m_maxCount;
        }
        
        void EntityPropertyGridTable::Entry::reset() {
            m_count = m_maxCount;
            m_multi = false;
        }

        EntityPropertyGridTable::EntityPropertyGridTable(MapDocumentPtr document, ControllerFacade& controller) :
        m_document(document),
        m_controller(controller),
        m_ignoreUpdates(false),
        m_specialCellColor(wxColor(128, 128, 128)) {}
        
        int EntityPropertyGridTable::GetNumberRows() {
            return static_cast<int>(m_entries.size());
        }
        
        int EntityPropertyGridTable::GetNumberCols() {
            return 2;
        }
        
        wxString EntityPropertyGridTable::GetValue(int row, int col) {
            assert(row >= 0 && row < GetNumberRows());
            assert(col >= 0 && col < GetNumberCols());
            
            size_t rowIndex = static_cast<size_t>(row);
            if (col == 0)
                return m_entries[rowIndex].key;
            return m_entries[rowIndex].multi() ? "" : m_entries[rowIndex].value;
        }
        
        void EntityPropertyGridTable::SetValue(int row, int col, const wxString& value) {
            assert(row >= 0 && row < GetNumberRows());
            assert(col >= 0 && col < GetNumberCols());
            
            size_t rowIndex = static_cast<size_t>(row);
            const Model::EntityList entities = m_document->allSelectedEntities();
            assert(!entities.empty());
            
            m_ignoreUpdates = true;
            Entry oldEntry = m_entries[rowIndex];
            
            if (col == 0) {
                const Model::PropertyKey oldKey = m_entries[rowIndex].key;
                const Model::PropertyKey newKey = value.ToStdString();
                
                m_entries[rowIndex].key = newKey;
                
                if (!m_controller.renameEntityProperty(entities, oldKey, newKey)) {
                    m_entries[rowIndex] = oldEntry;
                } else {
                    const Model::Entity& entity = *entities.front();
                    const Assets::EntityDefinition* entityDefinition = entity.definition();
                    const Assets::PropertyDefinition* propertyDefinition = entityDefinition != NULL ? entityDefinition->propertyDefinition(newKey) : NULL;
                    m_entries[rowIndex].tooltip = propertyDefinition != NULL ? propertyDefinition->description() : "";
                }
            } else {
                const Model::PropertyKey key = m_entries[rowIndex].key;
                const Model::PropertyValue newValue = value.ToStdString();
                
                m_entries[rowIndex].value = newValue;
                m_entries[rowIndex].reset();
                
                if (!m_controller.setEntityProperty(entities, key, newValue))
                    m_entries[rowIndex] = oldEntry;
            }
            m_ignoreUpdates = false;
        }
        
        void EntityPropertyGridTable::Clear() {
            DeleteRows(0, m_entries.size());
        }
        
        bool EntityPropertyGridTable::InsertRows(size_t pos, size_t numRows) {
            assert(pos >= 0 && static_cast<int>(pos) <= GetNumberRows());
            
            const Model::EntityList entities = m_document->allSelectedEntities();
            assert(!entities.empty());
            
            typedef std::vector<Model::PropertyKey> PropertyKeys;
            PropertyKeys keys;
            
            for (size_t i = 0; i < numRows; i++) {
                size_t index = 1;
                while (true) {
                    StringStream keyStream;
                    keyStream << "property " << index;
                    
                    bool indexIsFree = true;
                    Model::EntityList::const_iterator entityIt, entityEnd;
                    for (entityIt = entities.begin(), entityEnd = entities.end(); entityIt != entityEnd && indexIsFree; ++entityIt) {
                        const Model::Entity& entity = **entityIt;
                        indexIsFree = !entity.hasProperty(keyStream.str());
                    }
                    
                    if (indexIsFree) {
                        keys.push_back(keyStream.str());
                        break;
                    }
                    index++;
                }
            }
            
            assert(keys.size() == numRows);
            
            m_ignoreUpdates = true;
            m_controller.beginUndoableGroup(numRows == 1 ? "Add Property" : "Add Properties");
            
            EntryList::iterator entryIt = m_entries.begin();
            std::advance(entryIt, pos);
            for (size_t i = 0; i < numRows; i++) {
                entryIt = m_entries.insert(entryIt, Entry(keys[i], "", "", entities.size()));
                entryIt->reset();
                std::advance(entryIt, 1);
                
                m_controller.setEntityProperty(entities, keys[i], "");
            }
            
            m_controller.closeGroup();
            m_ignoreUpdates = false;
            
            notifyRowsInserted(pos, numRows);
            return true;
        }
        
        bool EntityPropertyGridTable::AppendRows(size_t numRows) {
            return InsertRows(m_entries.size(), numRows);
        }
        
        bool EntityPropertyGridTable::DeleteRows(size_t pos, size_t numRows) {
            assert(pos >= 0 && static_cast<int>(pos + numRows) <= GetNumberRows());
            
            const Model::EntityList entities = m_document->allSelectedEntities();
            assert(!entities.empty());
            
            m_ignoreUpdates = true;
            m_controller.beginUndoableGroup(numRows == 1 ? "Remove Property" : "Remove Properties");
            
            bool success = true;
            for (size_t i = pos; i < pos + numRows && success; i++) {
                const Entry& entry = m_entries[i];
                success = m_controller.removeEntityProperty(entities, entry.key);
            }
            
            if (!success) {
                m_controller.rollbackGroup();
                m_ignoreUpdates = false;
                return false;
            }
            m_ignoreUpdates = false;
            m_controller.closeGroup();
            
            EntryList::iterator first, last;
            std::advance(first = m_entries.begin(), pos);
            std::advance(last = m_entries.begin(), pos + numRows);
            m_entries.erase(first, last);
            notifyRowsDeleted(pos, numRows);
            return true;
        }
        
        wxString EntityPropertyGridTable::GetColLabelValue(int col) {
            assert(col >= 0 && col < GetNumberCols());
            if (col == 0)
                return wxT("Key");
            return wxT("Value");
        }
        
        wxGridCellAttr* EntityPropertyGridTable::getAttr(int row, int col, wxGridCellAttr::wxAttrKind kind) {
            wxGridCellAttr* attr = wxGridTableBase::GetAttr(row, col, kind);
            if (row >= 0 && row < GetNumberRows()) {
                const Entry& entry = m_entries[static_cast<size_t>(row)];
                if (col == 0) {
                    bool readonly = !Model::isPropertyKeyMutable(entry.key) || !Model::isPropertyValueMutable(entry.key);
                    if (readonly) {
                        if (attr == NULL)
                            attr = new wxGridCellAttr();
                        attr->SetReadOnly(true);
                    } else if (entry.subset()) {
                        if (attr == NULL)
                            attr = new wxGridCellAttr();
                        attr->SetTextColour(m_specialCellColor);
                    }
                } else if (col == 1) {
                    bool readonly = !Model::isPropertyValueMutable(entry.key);
                    if (readonly) {
                        if (attr == NULL)
                            attr = new wxGridCellAttr();
                        attr->SetReadOnly(true);
                    }
                    if (entry.multi()) {
                        if (attr == NULL)
                            attr = new wxGridCellAttr();
                        attr->SetTextColour(m_specialCellColor);
                    }
                }
            }
            return attr;
        }
        
        void EntityPropertyGridTable::update() {
            if (m_ignoreUpdates)
                return;
            
            EntryList newEntries;
            const Model::EntityList entities = m_document->allSelectedEntities();
            if (!entities.empty()) {
                Model::EntityList::const_iterator entityIt, entityEnd;
                for (entityIt = entities.begin(), entityEnd = entities.end(); entityIt != entityEnd; ++entityIt) {
                    const Model::Entity& entity = **entityIt;
                    const Assets::EntityDefinition* entityDefinition = entity.definition();
                    
                    const Model::EntityProperty::List& properties = entity.properties();
                    Model::EntityProperty::List::const_iterator propertyIt, propertyEnd;
                    for (propertyIt = properties.begin(), propertyEnd = properties.end(); propertyIt != propertyEnd; ++propertyIt) {
                        const Model::EntityProperty& property = *propertyIt;
                        const Assets::PropertyDefinition* propertyDefinition = entityDefinition != NULL ? entityDefinition->propertyDefinition(property.key) : NULL;
                        
                        EntryList::iterator entryIt = findEntry(newEntries, property.key);
                        if (entryIt != newEntries.end()) {
                            entryIt->compareValue(property.value);
                        } else {
                            const String tooltip = propertyDefinition != NULL ? propertyDefinition->description() : "";
                            newEntries.push_back(Entry(property.key, property.value, tooltip, entities.size()));
                        }
                    }
                }
            }
            
            size_t oldEntryCount = m_entries.size();
            size_t newEntryCount = newEntries.size();
            m_entries = newEntries;
            
            if (oldEntryCount < newEntryCount) {
                notifyRowsAppended(newEntryCount - oldEntryCount);
            } else if (oldEntryCount > newEntryCount) {
                notifyRowsDeleted(oldEntryCount - 1, oldEntryCount - newEntryCount);
            }
            notifyRowsUpdated(0, m_entries.size());
        }
        
        String EntityPropertyGridTable::tooltip(const wxGridCellCoords cellCoords) const {
            if (cellCoords.GetRow() < 0 || cellCoords.GetRow() >= static_cast<int>(m_entries.size()))
                return "";
            
            const Entry& entry = m_entries[static_cast<size_t>(cellCoords.GetRow())];
            if (entry.multi())
                return "";
            
            return entry.tooltip;
        }

        EntityPropertyGridTable::EntryList::iterator EntityPropertyGridTable::findEntry(EntryList& entries, const String& key) const {
            EntryList::iterator entryIt, entryEnd;
            for (entryIt = entries.begin(), entryEnd = entries.end(); entryIt != entryEnd; ++entryIt) {
                const Entry& entry = *entryIt;
                if (entry.key == key)
                    return entryIt;
            }
            return entryEnd;
        }
        
        void EntityPropertyGridTable::notifyRowsUpdated(size_t pos, size_t numRows) {
            if (GetView() != NULL) {
                wxGridTableMessage message(this, wxGRIDTABLE_REQUEST_VIEW_GET_VALUES,
                                           static_cast<int>(pos),
                                           static_cast<int>(numRows));
                GetView()->ProcessTableMessage(message);
            }
        }
        
        void EntityPropertyGridTable::notifyRowsInserted(size_t pos, size_t numRows) {
            if (GetView() != NULL) {
                wxGridTableMessage message(this, wxGRIDTABLE_NOTIFY_ROWS_INSERTED,
                                           static_cast<int>(pos),
                                           static_cast<int>(numRows));
                GetView()->ProcessTableMessage(message);
            }
        }
        
        void EntityPropertyGridTable::notifyRowsAppended(size_t numRows) {
            if (GetView() != NULL) {
                wxGridTableMessage message(this, wxGRIDTABLE_NOTIFY_ROWS_APPENDED,
                                           static_cast<int>(numRows));
                GetView()->ProcessTableMessage(message);
            }
        }
        
        void EntityPropertyGridTable::notifyRowsDeleted(size_t pos, size_t numRows) {
            if (GetView() != NULL) {
                wxGridTableMessage message(this, wxGRIDTABLE_NOTIFY_ROWS_DELETED,
                                           static_cast<int>(pos),
                                           static_cast<int>(numRows));
                GetView()->ProcessTableMessage(message);
            }
        }
    }
}
