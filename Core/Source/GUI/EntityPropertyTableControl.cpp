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

#include "EntityPropertyTableControl.h"

#include <map>
#include <set>
#include <string>

#include "Gwen/Controls/Properties.h"
#include "Gwen/Controls/ScrollControl.h"

#include "Controller/Editor.h"
#include "Model/Map/Entity.h"
#include "Model/Map/Map.h"
#include "Model/Selection.h"
#include "Model/Undo/UndoManager.h"

namespace TrenchBroom {
    namespace Gui {
        void EntityPropertyTableControl::updateProperties() {
            if (!m_entities.empty()) {
                std::set<Model::PropertyKey> multiValueProperties;
                Model::Properties commonProperties = m_entities[0]->properties();
                Model::Properties::iterator cProp;
                for (unsigned int i = 1; i < m_entities.size(); i++) {
                    Model::Entity* entity = m_entities[i];
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

                std::vector<Gwen::Controls::PropertyRow*>::iterator pRow;
                for (pRow = m_propertyRows.begin(); pRow != m_propertyRows.end(); ++pRow) {
                    Gwen::Controls::PropertyRow* propertyRow = *pRow;

                    Model::PropertyKey key = propertyRow->GetKey()->GetContentAnsi();
                    Model::PropertyValue value = propertyRow->GetValue()->GetContentAnsi();
                    
                    cProp = commonProperties.find(key);
                    if (cProp == commonProperties.end()) {
                        propertyRow->DelayedDelete();
                        pRow = m_propertyRows.erase(pRow);
                        --pRow;
                    } else {
                        propertyRow->GetValue()->SetContent(cProp->second);
                        if (multiValueProperties.find(cProp->first) != multiValueProperties.end())
                            propertyRow->GetValue()->SetPlaceholderString("multiple");
                        commonProperties.erase(cProp);
                    }
                }

                for (cProp = commonProperties.begin(); cProp != commonProperties.end(); ++cProp) {
                    Gwen::Controls::PropertyRow* propertyRow = m_properties->Add(cProp->first, cProp->second);
                    if (multiValueProperties.find(cProp->first) != multiValueProperties.end())
                        propertyRow->GetValue()->SetPlaceholderString("multiple");
                    propertyRow->onKeyChange.Add(this, &EntityPropertyTableControl::propertyKeyChanged);
                    propertyRow->onValueChange.Add(this, &EntityPropertyTableControl::propertyValueChanged);
                    m_propertyRows.push_back(propertyRow);
                }
                
                m_properties->SetShowEmptyRow(true);
            } else {
                m_properties->SetShowEmptyRow(false);
                m_propertyRows.clear();
                m_properties->Clear();
            }
        }

        void EntityPropertyTableControl::propertyKeyChanged(Gwen::Controls::Base* control) {
            Gwen::Controls::PropertyRow* propertyRow = static_cast<Gwen::Controls::PropertyRow*>(control);
            
            Model::PropertyKey oldKey = propertyRow->GetOldKey().Get();
            Model::PropertyKey newKey = propertyRow->GetKey()->GetContentAnsi();
            
            m_editor.map().renameEntityProperty(oldKey, newKey);
        }
        
        void EntityPropertyTableControl::propertyValueChanged(Gwen::Controls::Base* control) {
            Gwen::Controls::PropertyRow* propertyRow = static_cast<Gwen::Controls::PropertyRow*>(control);
            
            Model::PropertyKey key = propertyRow->GetKey()->GetContentAnsi();
            Model::PropertyValue value = propertyRow->GetValue()->GetContentAnsi();
            
            m_editor.map().setEntityProperty(key, &value);
        }

        void EntityPropertyTableControl::propertyRowAdded(Gwen::Controls::Base* control) {
            Gwen::Controls::PropertyRow* propertyRow = static_cast<Gwen::Controls::PropertyRow*>(control);
            propertyRow->onKeyChange.Add(this, &EntityPropertyTableControl::propertyKeyChanged);
            propertyRow->onValueChange.Add(this, &EntityPropertyTableControl::propertyValueChanged);
            m_propertyRows.push_back(propertyRow);
            
            Model::PropertyKey key = propertyRow->GetKey()->GetContentAnsi();
            Model::PropertyValue value = propertyRow->GetValue()->GetContentAnsi();
            
            m_editor.map().setEntityProperty(key, &value);
        }

        EntityPropertyTableControl::EntityPropertyTableControl(Gwen::Controls::Base* parent, Controller::Editor& editor) : Base(parent), m_editor(editor) {
            m_scroller = new Gwen::Controls::ScrollControl(this);
            m_scroller->Dock(Gwen::Pos::Fill);
            m_scroller->SetScroll(false, true);
            
            m_properties = new Gwen::Controls::Properties(m_scroller);
            m_properties->Dock(Gwen::Pos::Top);
//            m_properties->SetSorted(true);
            m_properties->onRowAdd.Add(this, &EntityPropertyTableControl::propertyRowAdded);
            
            updateProperties();
        }
        
        EntityPropertyTableControl::~EntityPropertyTableControl() {
        }
        
        void EntityPropertyTableControl::Render(Gwen::Skin::Base* skin) {
            skin->DrawBox(this);
        }

        void EntityPropertyTableControl::setEntities(const std::vector<Model::Entity*>& entities) {
            m_entities = entities;
            updateProperties();
        }
    }
}