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

#include "RotateObjectsToolPage.h"

#include "View/BorderLine.h"
#include "View/Grid.h"
#include "View/MapDocument.h"
#include "View/RotateObjectsTool.h"
#include "View/SpinControl.h"
#include "View/ViewConstants.h"

#include <wx/button.h>
#include <wx/choice.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>

namespace TrenchBroom {
    namespace View {
        RotateObjectsToolPage::RotateObjectsToolPage(wxWindow* parent, MapDocumentWPtr document, RotateObjectsTool* tool) :
        wxPanel(parent),
        m_document(document),
        m_tool(tool) {
            createGui();
        }
        
        void RotateObjectsToolPage::setAxis(const Math::Axis::Type axis) {
            m_axis->SetSelection(static_cast<int>(axis));
        }

        void RotateObjectsToolPage::createGui() {
            wxStaticText* text1 = new wxStaticText(this, wxID_ANY, "Rotate objects");
            wxStaticText* text2 = new wxStaticText(this, wxID_ANY, "degs about");
            wxStaticText* text3 = new wxStaticText(this, wxID_ANY, "axis");
            m_angle = new SpinControl(this);
            m_angle->SetRange(-360.0, 360.0);
            m_angle->SetValue(Math::degrees(m_tool->angle()));
            
            wxString axes[] = { "X", "Y", "Z" };
            m_axis = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 3, axes);
            m_axis->SetSelection(2);
            
            m_rotateButton = new wxButton(this, wxID_ANY, "Apply", wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
            m_resetButton = new wxButton(this, wxID_ANY, "Reset Center", wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
            m_resetButton->SetToolTip("Reset the position of the rotate handle to the center of the current selection.");
            
            Bind(wxEVT_IDLE, &RotateObjectsToolPage::OnIdle, this);
            m_angle->Bind(SPIN_CONTROL_EVENT, &RotateObjectsToolPage::OnAngleChanged, this);
            m_rotateButton->Bind(wxEVT_UPDATE_UI, &RotateObjectsToolPage::OnUpdateRotateButton, this);
            m_rotateButton->Bind(wxEVT_BUTTON, &RotateObjectsToolPage::OnRotate, this);
            m_resetButton->Bind(wxEVT_BUTTON, &RotateObjectsToolPage::OnReset, this);
            
            BorderLine* separator = new BorderLine(this, BorderLine::Direction_Vertical);
            separator->SetForegroundColour(Colors::separatorColor());
            
            wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
            sizer->Add(m_resetButton);
            sizer->AddSpacer(LayoutConstants::MediumHMargin);
            sizer->Add(separator, 0, wxEXPAND | wxTOP | wxBOTTOM, 2);
            sizer->AddSpacer(LayoutConstants::NarrowHMargin);
            sizer->Add(text1, 0, wxALIGN_CENTER_VERTICAL);
            sizer->AddSpacer(LayoutConstants::NarrowHMargin);
            sizer->Add(m_angle);
            sizer->AddSpacer(LayoutConstants::NarrowHMargin);
            sizer->Add(text2, 0, wxALIGN_CENTER_VERTICAL);
            sizer->AddSpacer(LayoutConstants::NarrowHMargin);
            sizer->Add(m_axis, 0, wxTOP, LayoutConstants::ChoiceTopMargin);
            sizer->AddSpacer(LayoutConstants::NarrowHMargin);
            sizer->Add(text3, 0, wxALIGN_CENTER_VERTICAL);
            sizer->AddSpacer(LayoutConstants::NarrowHMargin);
            sizer->Add(m_rotateButton);
            sizer->SetItemMinSize(m_angle, 80, wxDefaultCoord);

            SetSizer(sizer);
        }
        
        void RotateObjectsToolPage::OnIdle(wxIdleEvent& event) {
            const Grid& grid = lock(m_document)->grid();
            m_angle->SetIncrements(Math::degrees(grid.angle()), 90.0, 1.0);
            
            if (!Math::eq(m_tool->angle(), Math::degrees(m_angle->GetValue())))
                m_angle->SetValue(Math::degrees(m_tool->angle()));
        }

        void RotateObjectsToolPage::OnAngleChanged(SpinControlEvent& event) {
            m_tool->setAngle(Math::radians(event.GetValue()));
        }

        void RotateObjectsToolPage::OnUpdateRotateButton(wxUpdateUIEvent& event) {
            MapDocumentSPtr document = lock(m_document);
            event.Enable(document->hasSelectedNodes());
        }
        
        void RotateObjectsToolPage::OnRotate(wxCommandEvent& event) {
            const Vec3 center = m_tool->center();
            const Vec3 axis = getAxis();
            const FloatType angle = Math::radians(m_angle->GetValue());
            
            MapDocumentSPtr document = lock(m_document);
            document->rotateObjects(center, axis, angle);
        }
        
        void RotateObjectsToolPage::OnReset(wxCommandEvent& event) {
            m_tool->resetHandlePosition();
        }

        Vec3 RotateObjectsToolPage::getAxis() const {
            switch (m_axis->GetSelection()) {
                case 0:
                    return Vec3::PosX;
                case 1:
                    return Vec3::PosY;
                default:
                    return Vec3::PosZ;
            }
        }
    }
}
