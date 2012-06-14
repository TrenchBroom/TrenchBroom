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
#include <string>

#include "Gwen/Controls/Properties.h"
#include "Gwen/Controls/ScrollControl.h"

#include "Controller/Editor.h"
#include "Model/Map/Entity.h"
#include "Model/Map/Map.h"
#include "Model/Selection.h"

namespace TrenchBroom {
    namespace Gui {
        void EntityPropertyTableControl::updateProperties() {
            if (!m_entities.empty()) {
                Model::Properties commonProperties = m_entities[0]->properties();
                Model::Properties::iterator cProp;
                for (unsigned int i = 1; i < m_entities.size(); i++) {
                    Model::Entity* entity = m_entities[i];
                    const Model::Properties& entityProperties = entity->properties();
                    
                    Model::Properties::const_iterator eProp;
                    for (cProp = commonProperties.begin(); cProp != commonProperties.end(); ++cProp) {
                        eProp = entityProperties.find(cProp->first);
                        if (eProp == entityProperties.end()) {
                            commonProperties.erase(cProp);
                        } else {
                            if (cProp->second != eProp->second)
                                commonProperties[cProp->first] = "";
                        }
                    }
                }

                std::vector<Gwen::Controls::PropertyRow*>::iterator pRow;
                for (pRow = m_propertyRows.begin(); pRow != m_propertyRows.end(); ++pRow) {
                    Gwen::Controls::PropertyRow* propertyRow = *pRow;

                    std::string key = Gwen::Utility::UnicodeToString(propertyRow->GetLabel()->GetText());
                    std::string value = propertyRow->GetProperty()->GetPropertyValueAnsi();
                    
                    cProp = commonProperties.find(key);
                    if (cProp == commonProperties.end()) {
                        propertyRow->DelayedDelete();
                        pRow = m_propertyRows.erase(pRow);
                        --pRow;
                    } else {
                        propertyRow->GetProperty()->SetPropertyValue(cProp->second);
                        commonProperties.erase(cProp);
                    }
                }

                for (cProp = commonProperties.begin(); cProp != commonProperties.end(); ++cProp) {
                    Gwen::Controls::PropertyRow* propertyRow = m_properties->Add(cProp->first, cProp->second);
                    propertyRow->GetProperty()->SetPlaceholderString("multiple");
                    propertyRow->onChange.Add(this, &EntityPropertyTableControl::propertyChanged);
                    m_propertyRows.push_back(propertyRow);
                }
            } else {
                m_propertyRows.clear();
                m_properties->Clear();
            }
        }

        void EntityPropertyTableControl::propertyChanged(Gwen::Controls::Base* control) {
            Gwen::Controls::PropertyRow* propertyRow = static_cast<Gwen::Controls::PropertyRow*>(control);
            
            std::string key = Gwen::Utility::UnicodeToString(propertyRow->GetLabel()->GetText());
            std::string value = propertyRow->GetProperty()->GetPropertyValueAnsi();
            
            m_editor.map().setEntityProperty(key, &value);
        }

        EntityPropertyTableControl::EntityPropertyTableControl(Gwen::Controls::Base* parent, Controller::Editor& editor) : Base(parent), m_editor(editor) {
            m_scroller = new Gwen::Controls::ScrollControl(this);
            m_scroller->Dock(Gwen::Pos::Fill);
            m_scroller->SetScroll(false, true);
            
            m_properties = new Gwen::Controls::Properties(m_scroller);
            m_properties->Dock(Gwen::Pos::Top);
            m_properties->SetSorted(true);
            
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