/*
 Copyright (C) 2010-2012 Kristian Duske
 
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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "EntityPropertyGridTable.h"

#include "Controller/EntityPropertyCommand.h"
#include "Model/BrushTypes.h"
#include "Model/EditStateManager.h"
#include "Model/Entity.h"
#include "Model/MapDocument.h"
#include "Utility/CommandProcessor.h"

#include <set>

namespace TrenchBroom {
    namespace View {
        EntityPropertyGridTable::EntryList::iterator EntityPropertyGridTable::findEntry(EntryList& entries, const String& key) const {
            EntryList::iterator entryIt, entryEnd;
            for (entryIt = entries.begin(), entryEnd = entries.end(); entryIt != entryEnd; ++entryIt) {
                const Entry& entry = *entryIt;
                if (entry.key == key)
                    return entryIt;
            }
            return entryEnd;
        }

        Model::EntityList EntityPropertyGridTable::selectedEntities() {
            Model::EditStateManager& editStateManager = m_document.editStateManager();
            Model::EntityList entities = editStateManager.selectedEntities();
            const Model::BrushList& selectedBrushes = editStateManager.selectedBrushes();
            if (!selectedBrushes.empty()) {
                Model::EntitySet brushEntities;
                Model::BrushList::const_iterator brushIt, brushEnd;
                for (brushIt = selectedBrushes.begin(), brushEnd = selectedBrushes.end(); brushIt != brushEnd; ++brushIt) {
                    Model::Brush* brush = *brushIt;
                    Model::Entity* entity = brush->entity();
                    brushEntities.insert(entity);
                }
                
                entities.insert(entities.end(), brushEntities.begin(), brushEntities.end());
            }
            return entities;
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
        
        EntityPropertyGridTable::EntityPropertyGridTable(Model::MapDocument& document) :
        m_document(document),
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
            const Model::EntityList entities = selectedEntities();
            assert(!entities.empty());

            m_ignoreUpdates = true;
            Entry oldEntry = m_entries[rowIndex];
            
            if (col == 0) {
                const Model::PropertyKey oldKey = m_entries[rowIndex].key;
                const Model::PropertyKey newKey = value.ToStdString();

                m_entries[rowIndex].key = newKey;
                
                Controller::EntityPropertyCommand* rename = Controller::EntityPropertyCommand::setEntityPropertyKey(m_document, entities, oldKey, newKey);
                if (!m_document.GetCommandProcessor()->Submit(rename))
                    m_entries[rowIndex] = oldEntry;
            } else {
                const Model::PropertyKey key = m_entries[rowIndex].key;
                const Model::PropertyValue newValue = value.ToStdString();
                
                m_entries[rowIndex].value = newValue;
                m_entries[rowIndex].reset();

                Controller::EntityPropertyCommand* setValue = Controller::EntityPropertyCommand::setEntityPropertyValue(m_document, entities, key, newValue);
                if (!m_document.GetCommandProcessor()->Submit(setValue))
                    m_entries[rowIndex] = oldEntry;
            }
            m_ignoreUpdates = false;
        }
        
        void EntityPropertyGridTable::Clear() {
            DeleteRows(0, m_entries.size());
        }
        
        bool EntityPropertyGridTable::InsertRows(size_t pos, size_t numRows) {
            assert(pos >= 0 && static_cast<int>(pos) <= GetNumberRows());
            
            const Model::EntityList entities = selectedEntities();
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
                        indexIsFree = entity.propertyForKey(keyStream.str()) == NULL;
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
            CommandProcessor::BeginGroup(m_document.GetCommandProcessor(), numRows == 1 ? wxT("Add Property") : wxT("Add Properties"));
            
            EntryList::iterator entryIt = m_entries.begin();
            std::advance(entryIt, pos);
            for (size_t i = 0; i < numRows; i++) {
                entryIt = m_entries.insert(entryIt, Entry(keys[i], "", entities.size()));
                entryIt->reset();
                std::advance(entryIt, 1);

                Controller::EntityPropertyCommand* addProperty = Controller::EntityPropertyCommand::setEntityPropertyValue(m_document, entities, keys[i], "");
                m_document.GetCommandProcessor()->Submit(addProperty);
            }

            CommandProcessor::EndGroup(m_document.GetCommandProcessor());
            m_ignoreUpdates = false;
            
            notifyRowsInserted(pos, numRows);
            return true;
        }
        
        bool EntityPropertyGridTable::AppendRows(size_t numRows) {
            return InsertRows(m_entries.size(), numRows);
        }
        
        bool EntityPropertyGridTable::DeleteRows(size_t pos, size_t numRows) {
            assert(pos >= 0 && static_cast<int>(pos + numRows) <= GetNumberRows());
            
            const Model::EntityList entities = selectedEntities();
            assert(!entities.empty());

            m_ignoreUpdates = true;
            CommandProcessor::BeginGroup(m_document.GetCommandProcessor(), numRows == 1 ? wxT("Remove Property") : wxT("Remove Properties"));

            bool success = true;
            for (size_t i = pos; i < pos + numRows && success; i++) {
                const Entry& entry = m_entries[i];
                Controller::EntityPropertyCommand* removeProperty = Controller::EntityPropertyCommand::removeEntityProperty(m_document, entities, entry.key);
                success = m_document.GetCommandProcessor()->Submit(removeProperty);
            }

            if (!success) {
                CommandProcessor::RollbackGroup(m_document.GetCommandProcessor());
                CommandProcessor::EndGroup(m_document.GetCommandProcessor());
                m_ignoreUpdates = false;
                return false;
            }
            m_ignoreUpdates = false;
            CommandProcessor::EndGroup(m_document.GetCommandProcessor());
            
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
        
        wxGridCellAttr* EntityPropertyGridTable::GetAttr(int row, int col, wxGridCellAttr::wxAttrKind kind) {
            wxGridCellAttr* attr = wxGridTableBase::GetAttr(row, col, kind);
            if (row >= 0 && row < GetNumberRows()) {
                const Entry& entry = m_entries[static_cast<size_t>(row)];
                if (col == 0) {
                    bool readonly = !Model::Entity::propertyKeyIsMutable(entry.key);
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
            const Model::EntityList entities = selectedEntities();
            if (!entities.empty()) {
                Model::EntityList::const_iterator entityIt, entityEnd;
                for (entityIt = entities.begin(), entityEnd = entities.end(); entityIt != entityEnd; ++entityIt) {
                    const Model::Entity& entity = **entityIt;
                    
                    const Model::PropertyList& properties = entity.properties();
                    Model::PropertyList::const_iterator propertyIt, propertyEnd;
                    for (propertyIt = properties.begin(), propertyEnd = properties.end(); propertyIt != propertyEnd; ++propertyIt) {
                        const Model::Property& property = *propertyIt;
                        
                        EntryList::iterator entryIt = findEntry(newEntries, property.key());
                        if (entryIt != newEntries.end())
                            entryIt->compareValue(property.value());
                        else
                            newEntries.push_back(Entry(property.key(), property.value(), entities.size()));
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
    }
}
