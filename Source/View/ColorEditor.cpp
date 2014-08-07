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
#include "Model/Map.h"
#include "Model/MapDocument.h"
#include "Utility/List.h"
#include "View/LayoutConstants.h"

#include <wx/clrpicker.h>
#include <wx/gbsizer.h>
#include <wx/panel.h>
#include <wx/slider.h>
#include <wx/stattext.h>

#include <cassert>

namespace TrenchBroom {
    namespace View {
        BEGIN_EVENT_TABLE(ColorHistory, wxPanel)
        EVT_PAINT(ColorHistory::OnPaint)
        EVT_LEFT_UP(ColorHistory::OnMouseUp)
        END_EVENT_TABLE()

        ColorHistory::ColorHistory(wxWindow* parent, wxWindowID winId, int rows, int cols, const wxPoint& pos, const wxSize& size, long style) :
        wxPanel(parent, winId, pos, size, style),
        m_rows(rows),
        m_cols(cols),
        m_margin(1),
        m_callback(NULL) {
            assert(rows > 0 && cols > 0);
        }
        
        ColorHistory::~ColorHistory() {
            delete m_callback;
            m_callback = NULL;
        }
        
        void ColorHistory::OnPaint(wxPaintEvent& event) {
            const wxSize clientSize = GetClientSize();
            
            const int cellWidth = (clientSize.x - (m_cols + 1) * m_margin) / m_cols;
            const int cellHeight = (clientSize.y - (m_rows + 1) * m_margin) / m_rows;
            int restWidth = clientSize.x - (m_cols + 1) * m_margin - m_cols * cellWidth;
            int restHeight = clientSize.y - (m_rows + 1) * m_margin - m_rows * cellHeight;
            
            assert(restWidth >= 0);
            assert(restHeight >= 0);
            
            int x = m_margin;
            int y = m_margin;
            
            wxPaintDC dc(this);
            dc.SetPen(*wxTRANSPARENT_PEN);
            dc.SetBrush(*wxWHITE_BRUSH);
            dc.DrawRectangle(0, 0, clientSize.x, clientSize.y);

            Vec3f::List::const_iterator colorIt = m_colors.begin();
            
            for (int row = 0; row < m_rows; row++) {
                const int height = cellHeight + ((restHeight - row) > 0 ? 1 : 0);
                for (int col = 0; col < m_cols; col++) {
                    const int width = cellWidth + ((restWidth - col) > 0 ? 1 : 0);
                    
                    if (colorIt != m_colors.end()) {
                        wxColour wxCol = convertColor(*colorIt);
                        
                        dc.SetPen(wxPen(wxCol));
                        dc.SetBrush(wxBrush(wxCol));
                        dc.DrawRectangle(x, y, width, height);
                        ++colorIt;
                    }
                    x += width + m_margin;
                }
                y += height + m_margin;
                x = m_margin;
            }
        }

        void ColorHistory::OnMouseUp(wxMouseEvent& event) {
            const wxSize clientSize = GetClientSize();

            const int cellWidth = (clientSize.x - (m_cols + 1) * m_margin) / m_cols;
            const int cellHeight = (clientSize.y - (m_rows + 1) * m_margin) / m_rows;
            int restWidth = clientSize.x - (m_cols + 1) * m_margin - m_cols * cellWidth;
            int restHeight = clientSize.y - (m_rows + 1) * m_margin - m_rows * cellHeight;
            
            assert(restWidth >= 0);
            assert(restHeight >= 0);
            
            int x = m_margin;
            int y = m_margin;
            
            for (int row = 0; row < m_rows; row++) {
                const int height = cellHeight + ((restHeight - row) > 0 ? 1 : 0);
                for (int col = 0; col < m_cols; col++) {
                    const int width = cellWidth + ((restWidth - col) > 0 ? 1 : 0);

                    size_t index = static_cast<size_t>(row * m_rows + col);
                    if (event.GetX() >= x && event.GetX() <= x + width &&
                        event.GetY() >= y && event.GetY() <= y + height &&
                        index < m_colors.size()) {

                        if (m_callback != NULL)
                            m_callback->call(m_colors[index]);
                        
                        return;
                    }
                    
                    x += width + m_margin;
                }
                y += height + m_margin;
                x = m_margin;
            }
        }

        void ColorEditor::updateColorHistory() {
            typedef std::set<Vec3f, YIQOrder> YIQSet;
            
            YIQSet colorSet;
            
            const Model::EntityList& entities = document().map().entities();
            Model::EntityList::const_iterator entityIt, entityEnd;
            for (entityIt = entities.begin(), entityEnd = entities.end(); entityIt != entityEnd; ++entityIt) {
                const Model::Entity& entity = **entityIt;
                const Model::PropertyValue* value = entity.propertyForKey(property());
                if (value != NULL)
				{
					Vec3f color = Vec3f(*value);
					
					// YIQOrder requires color values in the range [0..1]
					if (color.x() > 1.0f || color.y() > 1.0f || color.z() > 1.0f)
						color /= 255.0f;

                    colorSet.insert(color);
				}
            }

            m_colorHistory->setColors(Utility::makeList(colorSet));
            m_colorHistory->Refresh();
        }

        wxWindow* ColorEditor::createVisual(wxWindow* parent) {
            assert(m_panel == NULL);
            
            m_panel = new wxPanel(parent);
            
            wxStaticText* redLabel = new wxStaticText(m_panel, wxID_ANY, wxT("Red"));
            wxStaticText* greenLabel = new wxStaticText(m_panel, wxID_ANY, wxT("Green"));
            wxStaticText* blueLabel = new wxStaticText(m_panel, wxID_ANY, wxT("Blue"));
            
            m_redSlider = new wxSlider(m_panel, wxID_ANY, 0, 0, 255);
            m_redSlider->SetMinSize(wxSize(50, wxDefaultSize.y));
            m_redSlider->Bind(wxEVT_SCROLL_THUMBTRACK, &ColorEditor::OnColorSliderChanged, this);
            m_redSlider->Bind(wxEVT_SCROLL_TOP, &ColorEditor::OnColorSliderChanged, this);
            m_redSlider->Bind(wxEVT_SCROLL_BOTTOM, &ColorEditor::OnColorSliderChanged, this);
            m_redSlider->Bind(wxEVT_SCROLL_LINEUP, &ColorEditor::OnColorSliderChanged, this);
            m_redSlider->Bind(wxEVT_SCROLL_LINEDOWN, &ColorEditor::OnColorSliderChanged, this);
            m_redSlider->Bind(wxEVT_SCROLL_PAGEUP, &ColorEditor::OnColorSliderChanged, this);
            m_redSlider->Bind(wxEVT_SCROLL_PAGEDOWN, &ColorEditor::OnColorSliderChanged, this);

            m_greenSlider = new wxSlider(m_panel, wxID_ANY, 0, 0, 255);
            m_greenSlider->SetMinSize(wxSize(50, wxDefaultSize.y));
            m_greenSlider->Bind(wxEVT_SCROLL_THUMBTRACK, &ColorEditor::OnColorSliderChanged, this);
            m_greenSlider->Bind(wxEVT_SCROLL_TOP, &ColorEditor::OnColorSliderChanged, this);
            m_greenSlider->Bind(wxEVT_SCROLL_BOTTOM, &ColorEditor::OnColorSliderChanged, this);
            m_greenSlider->Bind(wxEVT_SCROLL_LINEUP, &ColorEditor::OnColorSliderChanged, this);
            m_greenSlider->Bind(wxEVT_SCROLL_LINEDOWN, &ColorEditor::OnColorSliderChanged, this);
            m_greenSlider->Bind(wxEVT_SCROLL_PAGEUP, &ColorEditor::OnColorSliderChanged, this);
            m_greenSlider->Bind(wxEVT_SCROLL_PAGEDOWN, &ColorEditor::OnColorSliderChanged, this);

            m_blueSlider = new wxSlider(m_panel, wxID_ANY, 0, 0, 255);
            m_blueSlider->SetMinSize(wxSize(50, wxDefaultSize.y));
            m_blueSlider->Bind(wxEVT_SCROLL_THUMBTRACK, &ColorEditor::OnColorSliderChanged, this);
            m_blueSlider->Bind(wxEVT_SCROLL_TOP, &ColorEditor::OnColorSliderChanged, this);
            m_blueSlider->Bind(wxEVT_SCROLL_BOTTOM, &ColorEditor::OnColorSliderChanged, this);
            m_blueSlider->Bind(wxEVT_SCROLL_LINEUP, &ColorEditor::OnColorSliderChanged, this);
            m_blueSlider->Bind(wxEVT_SCROLL_LINEDOWN, &ColorEditor::OnColorSliderChanged, this);
            m_blueSlider->Bind(wxEVT_SCROLL_PAGEUP, &ColorEditor::OnColorSliderChanged, this);
            m_blueSlider->Bind(wxEVT_SCROLL_PAGEDOWN, &ColorEditor::OnColorSliderChanged, this);

            m_colorPicker = new wxColourPickerCtrl(m_panel, wxID_ANY);
            m_colorHistory = new ColorHistory(m_panel, wxID_ANY, ColorHistorySize, ColorHistorySize, wxDefaultPosition, wxSize(parent->GetClientSize().y, parent->GetClientSize().y));
            m_colorHistory->setCallback(this, &ColorEditor::OnColorHistorySelected);

            wxGridBagSizer* sizer = new wxGridBagSizer(LayoutConstants::ControlHorizontalMargin, LayoutConstants::ControlVerticalMargin);
            sizer->Add(redLabel,        wxGBPosition(0, 0), wxDefaultSpan, wxEXPAND);
            sizer->Add(m_redSlider,     wxGBPosition(0, 1), wxDefaultSpan, wxEXPAND);

            sizer->Add(greenLabel,      wxGBPosition(1, 0), wxDefaultSpan, wxEXPAND);
            sizer->Add(m_greenSlider,   wxGBPosition(1, 1), wxDefaultSpan, wxEXPAND);
            
            sizer->Add(blueLabel,       wxGBPosition(2, 0), wxDefaultSpan, wxEXPAND);
            sizer->Add(m_blueSlider,    wxGBPosition(2, 1), wxDefaultSpan, wxEXPAND);
            
            sizer->Add(m_colorPicker,   wxGBPosition(3, 0), wxGBSpan(1, 2), wxALIGN_RIGHT);
            sizer->Add(m_colorHistory,  wxGBPosition(0, 2), wxGBSpan(4, 1));
            
            sizer->AddGrowableCol(1);
            m_panel->SetSizer(sizer);

            wxSizer* outerSizer = new wxBoxSizer(wxHORIZONTAL);
            outerSizer->Add(m_panel, 1, wxEXPAND);
            parent->SetSizer(outerSizer);
            
            m_colorPicker->Bind(wxEVT_COMMAND_COLOURPICKER_CHANGED, &ColorEditor::OnColorPickerChanged, this);
            
            return m_colorPicker;
        }
        
        void ColorEditor::destroyVisual() {
            assert(m_panel != NULL);
            m_panel->Destroy();
            m_panel = NULL;
            m_redSlider = NULL;
            m_greenSlider = NULL;
            m_blueSlider = NULL;
            m_colorPicker = NULL;
            m_colorHistory = NULL;
        }
        
        void ColorEditor::updateVisual() {
            assert(m_colorPicker !=  NULL);
            Vec3f color;
            
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
                            color = Vec3f::Null;
                    } else {
                        color = Vec3f::Null;
                    }
                }
            }

            wxColour wxCol = convertColor(color);
            m_colorPicker->SetColour(wxCol);
            
            m_redSlider->SetValue(wxCol.Red());
            m_greenSlider->SetValue(wxCol.Green());
            m_blueSlider->SetValue(wxCol.Blue());
            
            updateColorHistory();
        }
        
        ColorEditor::ColorEditor(SmartPropertyEditorManager& manager) :
        SmartPropertyEditor(manager),
        m_panel(NULL),
        m_redSlider(NULL),
        m_greenSlider(NULL),
        m_blueSlider(NULL),
        m_colorPicker(NULL),
        m_colorHistory(NULL) {}

        void ColorEditor::OnColorPickerChanged(wxColourPickerEvent& event) {
            wxColour color = event.GetColour();
            StringStream newValue;
            newValue << color.Red() / 255.0f << " " << color.Green() / 255.0f << " " << color.Blue() / 255.0f;
            setPropertyValue(newValue.str(), wxT("Set Color"));
        }

        void ColorEditor::OnColorSliderChanged(wxScrollEvent& event) {
            wxColour color(static_cast<unsigned char>(m_redSlider->GetValue()),
                           static_cast<unsigned char>(m_greenSlider->GetValue()),
                           static_cast<unsigned char>(m_blueSlider->GetValue()));
            StringStream newValue;
            newValue << color.Red() / 255.0f << " " << color.Green() / 255.0f << " " << color.Blue() / 255.0f;
            setPropertyValue(newValue.str(), wxT("Set Color"));
        }
    
        void ColorEditor::OnColorHistorySelected(const Vec3f& color) {
            setPropertyValue(color.asString(), wxT("Set Color"));
        }
    }
}
