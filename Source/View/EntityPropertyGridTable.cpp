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
        
        void EntityPropertyGridTable::notifyRowsUpdated() {
            if (GetView() != NULL) {
                wxGridTableMessage message(this, wxGRIDTABLE_REQUEST_VIEW_GET_VALUES);
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
        m_document(document) {}

        int EntityPropertyGridTable::GetNumberRows() {
            return static_cast<int>(m_properties.size());
        }
        
        int EntityPropertyGridTable::GetNumberCols() {
            return 2;
        }
        
        wxString EntityPropertyGridTable::GetValue(int row, int col) {
            assert(row >= 0 && row < GetNumberRows());
            assert(col >= 0 && col < GetNumberCols());
            
            size_t rowIndex = static_cast<size_t>(row);
            if (col == 0)
                return m_properties[rowIndex].key;
            return m_properties[rowIndex].value;
        }
        
        void EntityPropertyGridTable::SetValue(int row, int col, const wxString& value) {
            assert(row >= 0 && row < GetNumberRows());
            assert(col >= 0 && col < GetNumberCols());
            
            size_t rowIndex = static_cast<size_t>(row);
            const Model::EntityList entities = selectedEntities();
            assert(!entities.empty());

            if (col == 0) {
                const Model::PropertyKey oldKey = m_properties[rowIndex].key;
                const Model::PropertyKey newKey = value.ToStdString();
                Controller::EntityPropertyCommand* rename = Controller::EntityPropertyCommand::setEntityPropertyKey(m_document, entities, oldKey, newKey);
                m_document.GetCommandProcessor()->Submit(rename);
            } else {
                const Model::PropertyKey key = m_properties[rowIndex].key;
                const Model::PropertyValue newValue = value.ToStdString();
                Controller::EntityPropertyCommand* setValue = Controller::EntityPropertyCommand::setEntityPropertyValue(m_document, entities, key, newValue);
                m_document.GetCommandProcessor()->Submit(setValue);
            }
        }
        
        void EntityPropertyGridTable::Clear() {
            DeleteRows(0, m_properties.size());
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

            CommandProcessor::BeginGroup(m_document.GetCommandProcessor(), numRows == 1 ? wxT("Add Property") : wxT("Add Properties"));
            
            EntityPropertyList::iterator propertyIt = m_properties.begin();
            std::advance(propertyIt, pos);
            for (size_t i = 0; i < numRows; i++) {
                propertyIt = m_properties.insert(propertyIt, EntityProperty(keys[i], ""));
                std::advance(propertyIt, 1);

                Controller::EntityPropertyCommand* addProperty = Controller::EntityPropertyCommand::setEntityPropertyValue(m_document, entities, keys[i], "");
                m_document.GetCommandProcessor()->Submit(addProperty);
            }

            CommandProcessor::EndGroup(m_document.GetCommandProcessor());
            
            notifyRowsInserted(pos, numRows);
            return true;
        }
        
        bool EntityPropertyGridTable::AppendRows(size_t numRows) {
            return InsertRows(m_properties.size(), numRows);
        }
        
        bool EntityPropertyGridTable::DeleteRows(size_t pos, size_t numRows) {
            assert(pos >= 0 && static_cast<int>(pos + numRows) <= GetNumberRows());
            
            const Model::EntityList entities = selectedEntities();
            assert(!entities.empty());

            CommandProcessor::BeginGroup(m_document.GetCommandProcessor(), numRows == 1 ? wxT("Remove Property") : wxT("Remove Properties"));

            bool success = true;
            for (size_t i = pos; i < pos + numRows && success; i++) {
                const EntityProperty& property = m_properties[i];
                Controller::EntityPropertyCommand* removeProperty = Controller::EntityPropertyCommand::removeEntityProperty(m_document, entities, property.key);
                success = m_document.GetCommandProcessor()->Submit(removeProperty);
            }

            if (!success) {
                CommandProcessor::RollbackGroup(m_document.GetCommandProcessor());
                CommandProcessor::EndGroup(m_document.GetCommandProcessor());
                return false;
            }
            CommandProcessor::EndGroup(m_document.GetCommandProcessor());
            
            EntityPropertyList::iterator first, last;
            std::advance(first = m_properties.begin(), pos);
            std::advance(last = m_properties.begin(), pos + numRows);
            m_properties.erase(first, last);
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
            if (col == 0) {
                assert(row >= 0 && row < GetNumberRows());
                const EntityProperty& property = m_properties[static_cast<size_t>(row)];
                bool readonly = !Model::Entity::propertyKeyIsMutable(property.key);
                if (attr == NULL) {
                    if (readonly) {
                        attr = new wxGridCellAttr();
                        attr->SetReadOnly(true);
                    }
                } else {
                    attr->SetReadOnly(readonly);
                }
            }
            return attr;
        }

        void EntityPropertyGridTable::update() {
            const Model::EntityList entities = selectedEntities();
            if (!entities.empty()) {
                std::set<Model::PropertyKey> multiValueProperties;
                Model::Properties commonProperties = entities[0]->properties();
                Model::Properties::iterator cProp;
                for (unsigned int i = 1; i < entities.size(); i++) {
                    Model::Entity* entity = entities[i];
                    const Model::Properties& entityProperties = entity->properties();
                    
                    Model::Properties::const_iterator eProp;
                    cProp = commonProperties.begin();
                    while (cProp != commonProperties.end()) {
                        eProp = entityProperties.find(cProp->first);
                        if (eProp == entityProperties.end()) {
                            commonProperties.erase(cProp++);
                        } else {
                            if (cProp->second != eProp->second) {
                                multiValueProperties.insert(cProp->first);
                                commonProperties[cProp->first] = "";
                            }
                            ++cProp;
                        }
                    }
                }
                
                EntityPropertyList::iterator eProp;
                size_t index = 0;
                for (eProp = m_properties.begin(); eProp != m_properties.end(); ++eProp) {
                    EntityProperty& propertyRow = *eProp;
                    
                    cProp = commonProperties.find(propertyRow.key);
                    if (cProp == commonProperties.end()) {
                        eProp = m_properties.erase(eProp);
                        --eProp;
                        notifyRowsDeleted(index);
                    } else {
                        if (multiValueProperties.find(cProp->first) != multiValueProperties.end()) {
                            propertyRow.multi = true;
                            propertyRow.value = "";
                        } else {
                            propertyRow.multi = false;
                            propertyRow.value = cProp->second;
                        }
                        commonProperties.erase(cProp);
                        index++;
                    }
                }
                
                for (cProp = commonProperties.begin(); cProp != commonProperties.end(); ++cProp) {
                    if (multiValueProperties.find(cProp->first) != multiValueProperties.end())
                        m_properties.push_back(EntityProperty(cProp->first));
                    else
                        m_properties.push_back(EntityProperty(cProp->first, cProp->second));
                    notifyRowsAppended();
                }

                notifyRowsUpdated();
            } else {
                notifyRowsDeleted(0, m_properties.size());
                m_properties.clear();
            }
        }
    }
}
