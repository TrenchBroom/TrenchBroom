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

#include "SmartColorEditor.h"

#include "Color.h"
#include "Model/Entity.h"
#include "Model/Map.h"
#include "View/ColorTable.h"
#include "View/MapDocument.h"
#include "View/LayoutConstants.h"

#include <wx/clrpicker.h>
#include <wx/panel.h>
#include <wx/radiobut.h>
#include <wx/sizer.h>

namespace TrenchBroom {
    namespace View {
        wxColour convertColor(Color color) {
            if (color.r() <= 1.0f && color.g() <= 1.0f && color.b() <= 1.0f)
                color *= 255.0f;
            return wxColour(static_cast<unsigned char>(color.r()),
                            static_cast<unsigned char>(color.g()),
                            static_cast<unsigned char>(color.b()));
        }

        SmartColorEditor::SmartColorEditor(View::MapDocumentPtr document, View::ControllerPtr controller) :
        SmartPropertyEditor(document, controller),
        m_panel(NULL),
        m_floatRadio(NULL),
        m_byteRadio(NULL),
        m_colorPicker(NULL),
        m_colorHistory(NULL) {}

        wxWindow* SmartColorEditor::doCreateVisual(wxWindow* parent) {
            assert(m_panel == NULL);
            assert(m_floatRadio == NULL);
            assert(m_byteRadio == NULL);
            assert(m_colorPicker == NULL);
            assert(m_colorHistory == NULL);
            
            m_panel = new wxPanel(parent);
            m_floatRadio = new wxRadioButton(m_panel, wxID_ANY, _("Color range [0,1]"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
            m_byteRadio = new wxRadioButton(m_panel, wxID_ANY, _("Color range [0,255]"));
            m_colorPicker = new wxColourPickerCtrl(m_panel, wxID_ANY);
            m_colorHistory = new ColorTable(m_panel, wxID_ANY, ColorHistoryCellSize);

            wxSizer* leftSizer = new wxBoxSizer(wxVERTICAL);
            leftSizer->Add(m_floatRadio);
            leftSizer->AddSpacer(LayoutConstants::ControlVerticalMargin);
            leftSizer->Add(m_byteRadio);
            leftSizer->AddSpacer(LayoutConstants::ControlVerticalMargin);
            leftSizer->Add(m_colorPicker);
            leftSizer->AddStretchSpacer();
            
            wxSizer* outerSizer = new wxBoxSizer(wxHORIZONTAL);
            outerSizer->Add(leftSizer);
            outerSizer->AddSpacer(LayoutConstants::ControlHorizontalMargin);
            outerSizer->Add(m_colorHistory, 1, wxEXPAND);
            m_panel->SetSizer(outerSizer);
            
            return m_panel;
        }

        void SmartColorEditor::doDestroyVisual() {
            assert(m_panel != NULL);
            assert(m_floatRadio != NULL);
            assert(m_byteRadio != NULL);
            assert(m_colorPicker != NULL);
            assert(m_colorHistory != NULL);
            
            m_panel->Destroy();
            m_panel = NULL;
            m_floatRadio = NULL;
            m_byteRadio = NULL;
            m_colorPicker = NULL;
            m_colorHistory = NULL;
        }
        
        void SmartColorEditor::doUpdateVisual(const Model::EntityList& entities) {
            assert(m_panel != NULL);
            assert(m_floatRadio != NULL);
            assert(m_byteRadio != NULL);
            assert(m_colorPicker != NULL);
            assert(m_colorHistory != NULL);
            
            updateColorHistory();
        }

        struct ColorCmp {
            bool operator()(const wxColor& lhs, const wxColor& rhs) const {
                const float lr = lhs.Red() / 255.0f;
                const float lg = lhs.Green() / 255.0f;
                const float lb = lhs.Blue() / 255.0f;
                const float rr = rhs.Red() / 255.0f;
                const float rg = rhs.Green() / 255.0f;
                const float rb = rhs.Blue() / 255.0f;
                
                float lh, ls, lbr, rh, rs, rbr;
                Color::rgbToHSB(lr, lg, lb, lh, ls, lbr);
                Color::rgbToHSB(rr, rg, rb, rh, rs, rbr);

                if (Math::lt(lh, rh))
                    return true;
                if (Math::gt(lh, rh))
                    return false;
                if (Math::lt(ls, rs))
                    return true;
                if (Math::gt(ls, rs))
                    return false;
                if (Math::lt(lbr, rbr))
                    return true;
                return false;
            }
        };
        
        void SmartColorEditor::updateColorHistory() {
            const Model::EntityList& entities = document()->map()->entities();
            
            ColorTable::ColorList colors;
            Model::EntityList::const_iterator it, end;
            for (it = entities.begin(), end = entities.end(); it != end; ++it) {
                const Model::Entity* entity = *it;
                if (entity->hasProperty(key())) {
                    const Color color(entity->property(key()));
                    colors.push_back(convertColor(color));
                }
            }
            
            m_colorHistory->setColors(colors, ColorCmp());
        }
    }
}
