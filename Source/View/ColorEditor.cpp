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

#include "ColorEditor.h"

#include "Model/Entity.h"
#include "Utility/VecMath.h"

#include <wx/clrpicker.h>

using namespace TrenchBroom::Math;

namespace TrenchBroom {
    namespace View {
        wxWindow* ColorEditor::createVisual(wxWindow* parent) {
            assert(m_colorPicker == NULL);
            m_colorPicker = new wxColourPickerCtrl(parent, wxID_ANY);
            m_colorPicker->Bind(wxEVT_COMMAND_COLOURPICKER_CHANGED, &ColorEditor::OnColorPickerChanged, this);
            
            wxSizer* outerSizer = new wxBoxSizer(wxHORIZONTAL);
            outerSizer->AddStretchSpacer();
            outerSizer->Add(m_colorPicker, 0, wxALIGN_CENTER_VERTICAL);
            outerSizer->AddStretchSpacer();
            parent->SetSizer(outerSizer);
            
            return m_colorPicker;
        }
        
        void ColorEditor::destroyVisual() {
            assert(m_colorPicker != NULL);
            m_colorPicker->Destroy();
            m_colorPicker = NULL;
        }
        
        void ColorEditor::updateVisual() {
            assert(m_colorPicker !=  NULL);
            Vec3f color = Vec3f::NaN;
            
            const Model::EntityList entities = selectedEntities();
            if (!entities.empty()) {
                Model::EntityList::const_iterator it = entities.begin();
                Model::EntityList::const_iterator end = entities.end();
                
                if ((*it)->propertyForKey(property()) != NULL)
                    color = Vec3f(*(*it)->propertyForKey(property()));
                
                while (++it != end && !color.nan()) {
                    const Model::Entity& entity = **it;
                    const Model::PropertyValue* value = entity.propertyForKey(property());
                    if (value != NULL) {
                        Vec3f colorValue(*value);
                        if (color != colorValue)
                            color = Vec3f::NaN;
                    } else {
                        color = Vec3f::NaN;
                    }
                }
            }
            
            if (color.nan()) {
                m_colorPicker->SetColour(*wxBLACK);
            } else {
                if (color.x <= 1.0f && color.y <= 1.0f && color.z <= 1.0f)
                    color *= 0xFF;
                m_colorPicker->SetColour(wxColour(static_cast<unsigned char>(color.x),
                                                  static_cast<unsigned char>(color.y),
                                                  static_cast<unsigned char>(color.z)));
            }
        }
        
        ColorEditor::ColorEditor(SmartPropertyEditorManager& manager) :
        SmartPropertyEditor(manager),
        m_colorPicker(NULL) {}

        void ColorEditor::OnColorPickerChanged(wxColourPickerEvent& event) {
            wxColour color = event.GetColour();
            StringStream newValue;
            newValue << color.Red() / 255.0f << " " << color.Green() / 255.0f << " " << color.Blue() / 255.0f;
            setPropertyValue(newValue.str(), wxT("Set Color"));
        }
    }
}
