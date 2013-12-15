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
#include "CollectionUtils.h"
#include "Model/Entity.h"
#include "Model/Map.h"
#include "View/ColorTable.h"
#include "View/MapDocument.h"
#include "View/LayoutConstants.h"

#include <wx/clrpicker.h>
#include <wx/panel.h>
#include <wx/radiobut.h>
#include <wx/sizer.h>
#include <wx/stattext.h>

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
            wxStaticText* rangeTxt = new wxStaticText(m_panel, wxID_ANY, _("Color range"));
            m_floatRadio = new wxRadioButton(m_panel, wxID_ANY, _("Float [0,1]"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
            m_byteRadio = new wxRadioButton(m_panel, wxID_ANY, _("Byte [0,255]"));
            m_colorPicker = new wxColourPickerCtrl(m_panel, wxID_ANY);
            m_colorHistory = new ColorTable(m_panel, wxID_ANY, ColorHistoryCellSize);

            wxSizer* leftSizer = new wxBoxSizer(wxVERTICAL);
            leftSizer->Add(rangeTxt);
            leftSizer->AddSpacer(LayoutConstants::ControlVerticalMargin);
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
            
            updateColorRange(entities);
            updateColorHistory();
        }

        void SmartColorEditor::updateColorRange(const Model::EntityList& entities) {
            const ColorRange range = detectColorRange(entities);
            switch (range) {
                case Float:
                    m_floatRadio->SetValue(true);
                    m_byteRadio->SetValue(false);
                    break;
                case Byte:
                    m_floatRadio->SetValue(false);
                    m_byteRadio->SetValue(true);
                    break;
                default:
                    m_floatRadio->SetValue(false);
                    m_byteRadio->SetValue(false);
                    break;
            }
        }

        SmartColorEditor::ColorRange SmartColorEditor::detectColorRange(const Model::EntityList& entities) const {
            assert(!entities.empty());
            
            Model::EntityList::const_iterator it = entities.begin();
            Model::EntityList::const_iterator end = entities.end();
            
            ColorRange range = detectColorRange(**it);
            while (++it != end)
                range = combineColorRanges(range, detectColorRange(**it));
            return range;
        }

        SmartColorEditor::ColorRange SmartColorEditor::combineColorRanges(ColorRange oldRange, ColorRange newRange) const {
            if (oldRange == newRange)
                return oldRange;
            return Mixed;
        }

        SmartColorEditor::ColorRange SmartColorEditor::detectColorRange(const Model::Entity& entity) const {
            if (!entity.hasProperty(key()))
                return Byte;
            const Model::PropertyValue& value = entity.property(key());
            const Color color(value);
            if (Color::detectColorRange(color.r(), color.g(), color.b()) == Color::Float)
                return Float;
            return Byte;
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
            
            VectorUtils::sortAndRemoveDuplicates(colors, ColorCmp());
            m_colorHistory->setColors(colors);
        }
    }
}
