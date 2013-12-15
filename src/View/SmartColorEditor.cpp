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
#include "Model/ModelUtils.h"
#include "View/ColorTable.h"
#include "View/ControllerFacade.h"
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

        typedef enum {
            Float,
            Byte,
            Mixed
        } ColorRange;

        ColorRange combineColorRanges(const ColorRange oldRange, const Color::Range newRange) {
            if (oldRange == newRange)
                return oldRange;
            return Mixed;
        }

        Color::Range detectColorRange(const Model::Entity& entity, const Model::PropertyKey& key) {
            if (!entity.hasProperty(key))
                return Color::Byte;
            const Model::PropertyValue& value = entity.property(key);
            const Color color(value);
            if (Color::detectColorRange(color.r(), color.g(), color.b()) == Color::Float)
                return Color::Float;
            return Color::Byte;
        }

        ColorRange detectColorRange(const Model::EntityList& entities, const Model::PropertyKey& key) {
            assert(!entities.empty());
            
            Model::EntityList::const_iterator it = entities.begin();
            Model::EntityList::const_iterator end = entities.end();
            
            ColorRange range = detectColorRange(**it, key) == Color::Float ? Float : Byte;
            while (++it != end)
                range = combineColorRanges(range, detectColorRange(**it, key));
            return range;
        }
        
        SmartColorEditor::SmartColorEditor(View::MapDocumentPtr document, View::ControllerPtr controller) :
        SmartPropertyEditor(document, controller),
        m_panel(NULL),
        m_floatRadio(NULL),
        m_byteRadio(NULL),
        m_colorPicker(NULL),
        m_colorHistory(NULL) {}

        struct ConvertColorRange {
        private:
            View::ControllerPtr m_controller;
            Model::PropertyKey m_key;
            Color::Range m_toRange;
        public:
            ConvertColorRange(View::ControllerPtr controller, const Model::PropertyKey& key, const Color::Range toRange) :
            m_controller(controller),
            m_key(key),
            m_toRange(toRange) {}
            
            void operator()(Model::Entity* entity) const {
                if (entity->hasProperty(m_key)) {
                    const Model::PropertyValue& value = entity->property(m_key);
                    Color color(value);
                    color.convertToRange(m_toRange);
                    m_controller->setEntityProperty(*entity, m_key, color.asString(3));
                }
            }
        };
        
        void SmartColorEditor::OnFloatRangeRadioButton(wxCommandEvent& event) {
            controller()->beginUndoableGroup("Convert " + key() + " Range");
            const Model::EntityList& entities = SmartPropertyEditor::entities();
            Model::each(entities.begin(), entities.end(), ConvertColorRange(controller(), key(), Color::Float), Model::MatchAll());
            controller()->closeGroup();
        }
        
        void SmartColorEditor::OnByteRangeRadioButton(wxCommandEvent& event) {
            controller()->beginUndoableGroup("Convert " + key() + " Range");
            const Model::EntityList& entities = SmartPropertyEditor::entities();
            Model::each(entities.begin(), entities.end(), ConvertColorRange(controller(), key(), Color::Byte), Model::MatchAll());
            controller()->closeGroup();
        }
        
        struct SetColor {
        private:
            View::ControllerPtr m_controller;
            Model::PropertyKey m_key;
            Color m_color;
        public:
            SetColor(View::ControllerPtr controller, const Model::PropertyKey& key, const Color& color) :
            m_controller(controller),
            m_key(key),
            m_color(color) {}
            
            void operator()(Model::Entity* entity) const {
                const Color::Range range = detectColorRange(*entity, m_key);
                Color actualColor(m_color);
                actualColor.convertToRange(range);
                m_controller->setEntityProperty(*entity, m_key, actualColor.asString(3));
            }
        };

        void SmartColorEditor::OnColorPickerChanged(wxColourPickerEvent& event) {
            const wxColor wxColor = event.GetColour();
            const Color color(static_cast<float>(wxColor.Red()),
                              static_cast<float>(wxColor.Green()),
                              static_cast<float>(wxColor.Blue()),
                              255.0f);
            
            controller()->beginUndoableGroup("Set " + key());
            const Model::EntityList& entities = SmartPropertyEditor::entities();
            Model::each(entities.begin(), entities.end(), SetColor(controller(), key(), color), Model::MatchAll());
            controller()->closeGroup();
        }

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
            
            m_floatRadio->Bind(wxEVT_RADIOBUTTON, &SmartColorEditor::OnFloatRangeRadioButton, this);
            m_byteRadio->Bind(wxEVT_RADIOBUTTON, &SmartColorEditor::OnByteRangeRadioButton, this);
            m_colorPicker->Bind(wxEVT_COLOURPICKER_CHANGED, &SmartColorEditor::OnColorPickerChanged, this);
            
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
            
            const ColorList usedColors = collectColors(entities);
            assert(!usedColors.empty());
            const wxColor pickerColor = usedColors.back();

            updateColorRange(entities);
            updateColorPicker(pickerColor);
            updateColorHistory(usedColors);
        }

        void SmartColorEditor::updateColorRange(const Model::EntityList& entities) {
            const ColorRange range = detectColorRange(entities, key());
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

        
        void SmartColorEditor::updateColorPicker(const wxColor& color) {
            m_colorPicker->SetColour(color);
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

        void SmartColorEditor::updateColorHistory(const ColorList& selectedColors) {
            const Model::EntityList& entities = document()->map()->entities();
            
            ColorList allColors = collectColors(entities);
            VectorUtils::sortAndRemoveDuplicates(allColors, ColorCmp());
            m_colorHistory->setColors(allColors);
            m_colorHistory->setSelection(selectedColors);
        }

        SmartColorEditor::ColorList SmartColorEditor::collectColors(const Model::EntityList& entities) const {
            ColorList colors;
            Model::EntityList::const_iterator it, end;
            for (it = entities.begin(), end = entities.end(); it != end; ++it) {
                const Model::Entity* entity = *it;
                if (entity->hasProperty(key())) {
                    const Color color(entity->property(key()));
                    colors.push_back(convertColor(color));
                }
            }
            return colors;
        }
    }
}
