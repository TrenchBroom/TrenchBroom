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

#include "EntityPropertyDataViewModel.h"

#include "Controller/EntityPropertyCommand.h"
#include "Model/EditStateManager.h"
#include "Model/Entity.h"
#include "Model/MapDocument.h"

#include <cassert>

namespace TrenchBroom {
    namespace View {
        void EntityPropertyDataViewModel::clear() {
            wxArrayInt rows;
            for (int i = 0; i < m_properties.size(); i++)
                rows.push_back(i);
            RowsDeleted(rows);
            m_properties.clear();
        }
        
        EntityPropertyDataViewModel::EntityPropertyDataViewModel(Model::MapDocument& document) :
        m_document(document) {}

        unsigned int EntityPropertyDataViewModel::GetColumnCount() const {
            return 2;
        }
        
        wxString EntityPropertyDataViewModel::GetColumnType(unsigned int col) const {
            return wxT("string");
        }
        
        void EntityPropertyDataViewModel::GetValueByRow(wxVariant &variant, unsigned int row, unsigned int col) const {
            assert(row < m_properties.size());
            assert(col < 2);
            
            const EntityProperty& property = m_properties[row];
            if (col == 0)
                variant = property.key;
            else if (property.multi)
                variant = "";
            else
                variant = property.value;
        }
        
        bool EntityPropertyDataViewModel::SetValueByRow(const wxVariant &variant, unsigned int row, unsigned int col) {
            assert(row < m_properties.size());
            assert(col < 2);

            Model::EditStateManager& editStateManager = m_document.editStateManager();
            const Model::EntityList& entities = editStateManager.selectedEntities();

            wxString wxValue;
            if (!variant.Convert(&wxValue))
                return false;

            if (col == 0) {
                Model::PropertyKey newKey = wxValue.ToStdString();
                bool canSetKey = true;
                for (unsigned int i = 0; i < entities.size() && canSetKey; i++) {
                    Model::Entity& entity = *entities[i];
                    canSetKey = (entity.propertyForKey(newKey) == NULL);
                }
                
                if (!canSetKey)
                    return false;
                
                const Model::PropertyKey& oldKey = m_properties[row].key;
                Controller::EntityPropertyCommand* command = Controller::EntityPropertyCommand::setEntityPropertyKey(m_document, oldKey, newKey);
                m_document.GetCommandProcessor()->Submit(command);
            } else {
                const EntityProperty& property = m_properties[row];
                if (property.multi && wxValue.empty())
                    return false;
                
                const Model::PropertyKey& key = property.key;
                Model::PropertyValue newValue = wxValue.ToStdString();
                Controller::EntityPropertyCommand* command = Controller::EntityPropertyCommand::setEntityPropertyValue(m_document, key, newValue);
                m_document.GetCommandProcessor()->Submit(command);
            }

            return true;
        }

        bool EntityPropertyDataViewModel::IsEnabledByRow(unsigned int row, unsigned int col) const {
            assert(row < m_properties.size());
            assert(col < 2);

            return true;
        }

        bool EntityPropertyDataViewModel::GetAttrByRow(unsigned int row, unsigned int col, wxDataViewItemAttr &attr) const {
            assert(row < m_properties.size());
            assert(col < 2);
            
            if (col == 1)
                return false;
            
            const EntityProperty& property = m_properties[row];
            if (!property.multi)
                return false;
            
            attr.SetColour(*wxLIGHT_GREY);
            return true;
        }
        
        unsigned int EntityPropertyDataViewModel::addNewRow() {
            unsigned int index = 1;
            bool freeIndexFound = false;
            StringStream keyStream;
            
            Model::EditStateManager& editStateManager = m_document.editStateManager();
            const Model::EntityList& entities = editStateManager.selectedEntities();
            while (!freeIndexFound) {
                freeIndexFound = true;
                keyStream.str("");
                keyStream << "property" << index;
                String key = keyStream.str();
                for (unsigned int i = 0; i < entities.size() && freeIndexFound; i++) {
                    const Model::Entity& entity = *entities[i];
                    if (entity.propertyForKey(key) != NULL) {
                        freeIndexFound = false;
                        index++;
                    }
                }
            }
            
            Controller::EntityPropertyCommand* command = Controller::EntityPropertyCommand::setEntityPropertyValue(m_document, keyStream.str(), "");
            m_document.GetCommandProcessor()->Submit(command);
            
            return static_cast<unsigned int>(m_properties.size() - 1);
        }

        void EntityPropertyDataViewModel::removeRows(const wxDataViewItemArray& items) {
            Model::PropertyKeyList keys;
            for (unsigned int i = 0; i < items.size(); i++) {
                unsigned int row = GetRow(items[i]);
                keys.push_back(m_properties[row].key);
            }
            
            Controller::EntityPropertyCommand* command = Controller::EntityPropertyCommand::removeEntityProperties(m_document, keys);
            m_document.GetCommandProcessor()->Submit(command);
        }

        void EntityPropertyDataViewModel::update() {
            Model::EditStateManager& editStateManager = m_document.editStateManager();
            const Model::EntityList& entities = editStateManager.selectedEntities();
            
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
                int index = 0;
                for (eProp = m_properties.begin(); eProp != m_properties.end(); ++eProp) {
                    EntityProperty& propertyRow = *eProp;
                    
                    cProp = commonProperties.find(propertyRow.key);
                    if (cProp == commonProperties.end()) {
                        eProp = m_properties.erase(eProp);
                        --eProp;
                        RowDeleted(index);
                    } else {
                        if (multiValueProperties.find(cProp->first) != multiValueProperties.end()) {
                            propertyRow.multi = true;
                            propertyRow.value = "";
                        } else {
                            propertyRow.multi = false;
                            propertyRow.value = cProp->second;
                        }
                        commonProperties.erase(cProp);
                        RowChanged(index);
                        index++;
                    }
                }
                
                for (cProp = commonProperties.begin(); cProp != commonProperties.end(); ++cProp) {
                    if (multiValueProperties.find(cProp->first) != multiValueProperties.end())
                        m_properties.push_back(EntityProperty(cProp->first));
                    else
                        m_properties.push_back(EntityProperty(cProp->first, cProp->second));
                    RowAppended();
                }
            } else {
                clear();
            }
        }
    }
}