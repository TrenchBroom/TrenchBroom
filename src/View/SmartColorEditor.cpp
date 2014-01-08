/*
 Copyright (C) 2010-2014 Kristian Duske
 
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

#include "CollectionUtils.h"
#include "Model/Entity.h"
#include "Model/Map.h"
#include "Model/ModelUtils.h"
#include "View/ColorTable.h"
#include "View/ColorTableSelectedCommand.h"
#include "View/ControllerFacade.h"
#include "View/MapDocument.h"
#include "View/ViewConstants.h"

#include <wx/clrpicker.h>
#include <wx/panel.h>
#include <wx/radiobut.h>
#include <wx/sizer.h>
#include <wx/stattext.h>

#include <iomanip>

namespace TrenchBroom {
    namespace View {
        SmartColorEditor::ColorPtr SmartColorEditor::Color::parseColor(const String& str) {
            const StringList components = StringUtils::splitAndTrim(str, " ");
            if (components.size() != 3)
                return ColorPtr(new ByteColor(0, 0, 0));
            
            ColorRange range = detectRange(components);
            assert(range != Mixed);
            
            if (range == Byte) {
                const int r = std::atoi(components[0].c_str());
                const int g = std::atoi(components[1].c_str());
                const int b = std::atoi(components[2].c_str());
                return ColorPtr(new ByteColor(r, g, b));
            }
            
            const float r = static_cast<float>(std::atof(components[0].c_str()));
            const float g = static_cast<float>(std::atof(components[1].c_str()));
            const float b = static_cast<float>(std::atof(components[2].c_str()));
            return ColorPtr(new FloatColor(r, g, b));
        }
        
        SmartColorEditor::ColorPtr SmartColorEditor::Color::fromWxColor(const wxColor& wxColor, const ColorRange range) {
            ColorPtr byteColor(new ByteColor(wxColor.Red(), wxColor.Green(), wxColor.Blue()));
            return byteColor->toColor(range);
        }

        SmartColorEditor::ColorPtr SmartColorEditor::Color::toColor(const ColorRange range) const {
            if (range == Float)
                return toFloatColor();
            return toByteColor();
        }

        SmartColorEditor::ColorRange SmartColorEditor::Color::detectRange(const String& str) {
            const StringList components = StringUtils::splitAndTrim(str, " ");
            return detectRange(components);
        }

        SmartColorEditor::ColorRange SmartColorEditor::Color::detectRange(const StringList& components) {
            if (components.size() != 3)
                return Byte;
            
            ColorRange range = Byte;
            for (size_t i = 0; i < 3 && range == Byte; ++i)
                if (components[i].find('.') != String::npos)
                    range = Float;
            
            return range;
        }

        SmartColorEditor::FloatColor::FloatColor(const float r, const float g, const float b) {
            assert(r >= 0.0f && r <= 1.0f);
            assert(g >= 0.0f && g <= 1.0f);
            assert(b >= 0.0f && b <= 1.0f);
            
            m_v[0] = r;
            m_v[1] = g;
            m_v[2] = b;
        }
        
        SmartColorEditor::ColorPtr SmartColorEditor::FloatColor::toFloatColor() const {
            return shared_from_this();
        }
        
        SmartColorEditor::ColorPtr SmartColorEditor::FloatColor::toByteColor() const {
            return ColorPtr(new ByteColor(static_cast<int>(Math::round(r() * 255.0f)),
                                          static_cast<int>(Math::round(g() * 255.0f)),
                                          static_cast<int>(Math::round(b() * 255.0f))));
        }
        
        wxColor SmartColorEditor::FloatColor::toWxColor() const {
            return wxColor(static_cast<int>(r() * 255.0f),
                           static_cast<int>(g() * 255.0f),
                           static_cast<int>(b() * 255.0f));
        }

        String SmartColorEditor::FloatColor::asString() const {
            StringStream str;
            print(str, r());
            str << " ";
            print(str, g());
            str << " ";
            print(str, b());
            return str.str();
        }
        
        void SmartColorEditor::FloatColor::print(StringStream& str, const float f) const {
            if (Math::isInteger(f)) {
                str.setf(std::ios::fixed, std::ios::floatfield);
                str.precision(1);
            } else {
                str.unsetf(std::ios::floatfield);
                str.precision(9);
            }
            str << f;
        }
        
        SmartColorEditor::ByteColor::ByteColor(const int r, const int g, const int b) {
            assert(r >= 0 && r <= 255);
            assert(g >= 0 && g <= 255);
            assert(b >= 0 && b <= 255);
            
            m_v[0] = r;
            m_v[1] = g;
            m_v[2] = b;
        }
        
        SmartColorEditor::ColorPtr SmartColorEditor::ByteColor::toFloatColor() const {
            return ColorPtr(new FloatColor(static_cast<float>(r()) / 255.0f,
                                           static_cast<float>(g()) / 255.0f,
                                           static_cast<float>(b()) / 255.0f));
        }
        
        SmartColorEditor::ColorPtr SmartColorEditor::ByteColor::toByteColor() const {
            return shared_from_this();
        }
        
        wxColor SmartColorEditor::ByteColor::toWxColor() const {
            return wxColor(r(), g(), b());
        }
        
        String SmartColorEditor::ByteColor::asString() const {
            StringStream str;
            str << r() << " " << g() << " " << b();
            return str.str();
        }
        
        SmartColorEditor::ColorRange combineColorRanges(const SmartColorEditor::ColorRange oldRange, const SmartColorEditor::ColorRange newRange) {
            if (oldRange == newRange)
                return oldRange;
            return SmartColorEditor::Mixed;
        }
        
        SmartColorEditor::ColorRange detectColorRange(const Model::Entity& entity, const Model::PropertyKey& key) {
            if (!entity.hasProperty(key))
                return SmartColorEditor::Byte;
            const Model::PropertyValue& value = entity.property(key);
            return SmartColorEditor::Color::detectRange(value);
        }
        
        SmartColorEditor::ColorRange detectColorRange(const Model::EntityList& entities, const Model::PropertyKey& key) {
            assert(!entities.empty());
            
            Model::EntityList::const_iterator it = entities.begin();
            Model::EntityList::const_iterator end = entities.end();
            
            SmartColorEditor::ColorRange range = detectColorRange(**it, key);
            while (++it != end)
                range = combineColorRanges(range, detectColorRange(**it, key));
            return range;
        }
        
        SmartColorEditor::SmartColorEditor(View::MapDocumentWPtr document, View::ControllerWPtr controller) :
        SmartPropertyEditor(document, controller),
        m_panel(NULL),
        m_floatRadio(NULL),
        m_byteRadio(NULL),
        m_colorPicker(NULL),
        m_colorHistory(NULL) {}
        
        struct ConvertColorRange {
        private:
            View::ControllerSPtr m_controller;
            Model::PropertyKey m_key;
            SmartColorEditor::ColorRange m_toRange;
        public:
            ConvertColorRange(View::ControllerSPtr controller, const Model::PropertyKey& key, const SmartColorEditor::ColorRange toRange) :
            m_controller(controller),
            m_key(key),
            m_toRange(toRange) {}
            
            void operator()(Model::Entity* entity) const {
                if (entity->hasProperty(m_key)) {
                    const Model::PropertyValue& value = entity->property(m_key);
                    SmartColorEditor::ColorPtr originalColor = SmartColorEditor::Color::parseColor(value);
                    SmartColorEditor::ColorPtr convertedColor = originalColor->toColor(m_toRange);
                    m_controller->setEntityProperty(*entity, m_key, convertedColor->asString());
                }
            }
        };
        
        void SmartColorEditor::OnFloatRangeRadioButton(wxCommandEvent& event) {
            controller()->beginUndoableGroup("Convert " + key() + " Range");
            const Model::EntityList& entities = SmartPropertyEditor::entities();
            Model::each(entities.begin(), entities.end(), ConvertColorRange(controller(), key(), Float), Model::MatchAll());
            controller()->closeGroup();
        }
        
        void SmartColorEditor::OnByteRangeRadioButton(wxCommandEvent& event) {
            controller()->beginUndoableGroup("Convert " + key() + " Range");
            const Model::EntityList& entities = SmartPropertyEditor::entities();
            Model::each(entities.begin(), entities.end(), ConvertColorRange(controller(), key(), Byte), Model::MatchAll());
            controller()->closeGroup();
        }
        
        struct SetColor {
        private:
            View::ControllerSPtr m_controller;
            Model::PropertyKey m_key;
            SmartColorEditor::ColorPtr m_color;
        public:
            SetColor(View::ControllerSPtr controller, const Model::PropertyKey& key, SmartColorEditor::ColorPtr color) :
            m_controller(controller),
            m_key(key),
            m_color(color) {}
            
            void operator()(Model::Entity* entity) const {
                const SmartColorEditor::ColorRange range = detectColorRange(*entity, m_key);
                const SmartColorEditor::ColorPtr color = m_color->toColor(range);
                m_controller->setEntityProperty(*entity, m_key, color->asString());
            }
        };
        
        void SmartColorEditor::OnColorPickerChanged(wxColourPickerEvent& event) {
            setColor(event.GetColour());
        }
        
        void SmartColorEditor::OnColorTableSelected(ColorTableSelectedCommand& event) {
            setColor(event.color());
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
            m_colorHistory->Bind(EVT_COLOR_TABLE_SELECTED_EVENT, EVT_COLOR_TABLE_SELECTED_HANDLER(SmartColorEditor::OnColorTableSelected), this);
            
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
            
            const wxColorList usedColors = collectColors(entities);
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
        
        void SmartColorEditor::updateColorHistory(const wxColorList& selectedColors) {
            const Model::EntityList& entities = document()->map()->entities();
            
            wxColorList allColors = collectColors(entities);
            VectorUtils::sortAndRemoveDuplicates(allColors, ColorCmp());
            m_colorHistory->setColors(allColors);
            m_colorHistory->setSelection(selectedColors);
        }
        
        SmartColorEditor::wxColorList SmartColorEditor::collectColors(const Model::EntityList& entities) const {
            wxColorList colors;
            Model::EntityList::const_iterator it, end;
            for (it = entities.begin(), end = entities.end(); it != end; ++it) {
                const Model::Entity* entity = *it;
                if (entity->hasProperty(key())) {
                    const ColorPtr color = Color::parseColor(entity->property(key()));
                    colors.push_back(color->toWxColor());
                }
            }
            return colors;
        }

        void SmartColorEditor::setColor(const wxColor& wxColor) const {
            ColorPtr color = Color::fromWxColor(wxColor, Byte);
            
            controller()->beginUndoableGroup("Set " + key());
            const Model::EntityList& entities = SmartPropertyEditor::entities();
            Model::each(entities.begin(), entities.end(), SetColor(controller(), key(), color), Model::MatchAll());
            controller()->closeGroup();
        }
    }
}
