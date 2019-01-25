/*
 Copyright (C) 2010-2017 Kristian Duske
 
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
#include "Model/AttributableNode.h"
#include "Model/Entity.h"
#include "Model/EntityColor.h"
#include "Model/World.h"
#include "View/BorderLine.h"
#include "View/ColorTable.h"
#include "View/ColorTableSelectedCommand.h"
#include "View/MapDocument.h"
#include "View/ViewConstants.h"

#include <wx/clrpicker.h>
#include <wx/panel.h>
#include <wx/radiobut.h>
#include <wx/sizer.h>
#include <QLabel>
#include <wx/wupdlock.h>

#include <iomanip>

namespace TrenchBroom {
    namespace View {
        SmartColorEditor::SmartColorEditor(View::MapDocumentWPtr document) :
        SmartAttributeEditor(document),
        m_panel(nullptr),
        m_floatRadio(nullptr),
        m_byteRadio(nullptr),
        m_colorPicker(nullptr),
        m_colorHistory(nullptr) {}
        
        void SmartColorEditor::OnFloatRangeRadioButton(wxCommandEvent& event) {
            if (m_panel->IsBeingDeleted()) return;
            document()->convertEntityColorRange(name(), Assets::ColorRange::Float);
        }
        
        void SmartColorEditor::OnByteRangeRadioButton(wxCommandEvent& event) {
            if (m_panel->IsBeingDeleted()) return;
            document()->convertEntityColorRange(name(), Assets::ColorRange::Byte);
        }
        
        void SmartColorEditor::OnColorPickerChanged(wxColourPickerEvent& event) {
            if (m_panel->IsBeingDeleted()) return;
            setColor(event.GetColour());
        }
        
        void SmartColorEditor::OnColorTableSelected(ColorTableSelectedCommand& event) {
            if (m_panel->IsBeingDeleted()) return;
            setColor(event.color());
        }

        QWidget* SmartColorEditor::doCreateVisual(QWidget* parent) {
            assert(m_panel == nullptr);
            assert(m_floatRadio == nullptr);
            assert(m_byteRadio == nullptr);
            assert(m_colorPicker == nullptr);
            assert(m_colorHistory == nullptr);
            
            m_panel = new QWidget(parent);
            auto* rangeTxt = new QLabel(m_panel, wxID_ANY, "Color range");
            rangeTxt->SetFont(rangeTxt->GetFont().Bold());
            m_floatRadio = new wxRadioButton(m_panel, wxID_ANY, "Float [0,1]", wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
            m_byteRadio = new wxRadioButton(m_panel, wxID_ANY, "Byte [0,255]");
            m_colorPicker = new wxColourPickerCtrl(m_panel, wxID_ANY);
            m_colorHistory = new ColorTable(m_panel, wxID_ANY, ColorHistoryCellSize);
            
            auto* leftSizer = new QVBoxLayout();
            leftSizer->addSpacing(LayoutConstants::WideVMargin);
            leftSizer->Add(rangeTxt);
            leftSizer->addSpacing(LayoutConstants::WideVMargin);
            leftSizer->Add(m_floatRadio);
            leftSizer->addSpacing(LayoutConstants::WideVMargin);
            leftSizer->Add(m_byteRadio);
            leftSizer->addSpacing(LayoutConstants::WideVMargin);
            leftSizer->Add(m_colorPicker);
            leftSizer->AddStretchSpacer();
            
            auto* outerSizer = new QHBoxLayout();
            outerSizer->addSpacing(LayoutConstants::WideHMargin);
            outerSizer->Add(leftSizer);
            outerSizer->addSpacing(LayoutConstants::WideHMargin);
            outerSizer->Add(new BorderLine(m_panel, BorderLine::Direction_Vertical), 0, wxEXPAND);
            outerSizer->Add(m_colorHistory, 1, wxEXPAND);
            m_panel->SetSizer(outerSizer);
            
            m_floatRadio->Bind(wxEVT_RADIOBUTTON, &SmartColorEditor::OnFloatRangeRadioButton, this);
            m_byteRadio->Bind(wxEVT_RADIOBUTTON, &SmartColorEditor::OnByteRangeRadioButton, this);
            m_colorPicker->Bind(wxEVT_COLOURPICKER_CHANGED, &SmartColorEditor::OnColorPickerChanged, this);
            m_colorHistory->Bind(COLOR_TABLE_SELECTED_EVENT, &SmartColorEditor::OnColorTableSelected, this);
            
            return m_panel;
        }
        
        void SmartColorEditor::doDestroyVisual() {
            ensure(m_panel != nullptr, "panel is null");
            ensure(m_floatRadio != nullptr, "floatRadio is null");
            ensure(m_byteRadio != nullptr, "byteRadio is null");
            ensure(m_colorPicker != nullptr, "colorPicker is null");
            ensure(m_colorHistory != nullptr, "colorHistory is null");
            
            m_panel->Destroy();
            m_panel = nullptr;
            m_floatRadio = nullptr;
            m_byteRadio = nullptr;
            m_colorPicker = nullptr;
            m_colorHistory = nullptr;
        }
        
        void SmartColorEditor::doUpdateVisual(const Model::AttributableNodeList& attributables) {
            ensure(m_panel != nullptr, "panel is null");
            ensure(m_floatRadio != nullptr, "floatRadio is null");
            ensure(m_byteRadio != nullptr, "byteRadio is null");
            ensure(m_colorPicker != nullptr, "colorPicker is null");
            ensure(m_colorHistory != nullptr, "colorHistory is null");
            
            wxWindowUpdateLocker locker(m_panel);
            updateColorRange(attributables);
            updateColorHistory();
        }
        
        void SmartColorEditor::updateColorRange(const Model::AttributableNodeList& attributables) {
            const auto range = detectColorRange(name(), attributables);
            if (range == Assets::ColorRange::Float) {
                m_floatRadio->SetValue(true);
                m_byteRadio->SetValue(false);
            } else if (range == Assets::ColorRange::Byte) {
                m_floatRadio->SetValue(false);
                m_byteRadio->SetValue(true);
            } else {
                m_floatRadio->SetValue(false);
                m_byteRadio->SetValue(false);
            }
        }
        
        struct ColorCmp {
            bool operator()(const wxColor& lhs, const wxColor& rhs) const {
                const auto lr = lhs.Red() / 255.0f;
                const auto lg = lhs.Green() / 255.0f;
                const auto lb = lhs.Blue() / 255.0f;
                const auto rr = rhs.Red() / 255.0f;
                const auto rg = rhs.Green() / 255.0f;
                const auto rb = rhs.Blue() / 255.0f;
                
                float lh, ls, lbr, rh, rs, rbr;
                Color::rgbToHSB(lr, lg, lb, lh, ls, lbr);
                Color::rgbToHSB(rr, rg, rb, rh, rs, rbr);
                
                if (lh < rh) {
                    return true;
                } else if (lh > rh) {
                    return false;
                } else if (ls < rs) {
                    return true;
                } else if (ls > rs) {
                    return false;
                } else if (lbr < rbr) {
                    return true;
                } else {
                    return false;
                }
            }
        };
        
        class SmartColorEditor::CollectColorsVisitor : public Model::ConstNodeVisitor {
        private:
            const Model::AttributeName& m_name;
            wxColorList m_colors;
        public:
            CollectColorsVisitor(const Model::AttributeName& name) : m_name(name) {}
            
            const wxColorList& colors() const { return m_colors; }
        private:
            void doVisit(const Model::World* world) override   { visitAttributableNode(world); }
            void doVisit(const Model::Layer* layer) override   {}
            void doVisit(const Model::Group* group) override   {}
            void doVisit(const Model::Entity* entity) override { visitAttributableNode(entity); stopRecursion(); }
            void doVisit(const Model::Brush* brush) override   {}

            void visitAttributableNode(const Model::AttributableNode* attributable) {
                static const auto NullValue("");
                const auto& value = attributable->attribute(m_name, NullValue);
                if (value != NullValue)
                    addColor(Model::parseEntityColor(value));
            }

            void addColor(const wxColor& color) {
                VectorUtils::setInsert(m_colors, color, ColorCmp());
            }
        };
        
        void SmartColorEditor::updateColorHistory() {
            CollectColorsVisitor collectAllColors(name());
            document()->world()->acceptAndRecurse(collectAllColors);
            m_colorHistory->setColors(collectAllColors.colors());
            
            CollectColorsVisitor collectSelectedColors(name());
            const auto nodes = document()->allSelectedAttributableNodes();
            Model::Node::accept(std::begin(nodes), std::end(nodes), collectSelectedColors);
            
            const auto& selectedColors = collectSelectedColors.colors();
            m_colorHistory->setSelection(selectedColors);
            
            const auto& color = !selectedColors.empty() ? selectedColors.back() : *wxBLACK;
            m_colorPicker->SetColour(color);
        }
        
        void SmartColorEditor::setColor(const wxColor& color) const {
            const auto colorRange = m_floatRadio->GetValue() ? Assets::ColorRange::Float : Assets::ColorRange::Byte;
            const auto value = Model::entityColorAsString(color, colorRange);
            document()->setAttribute(name(), value);
        }
    }
}
