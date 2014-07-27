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

#include "View/ControllerFacade.h"
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
        RotateObjectsToolPage::RotateObjectsToolPage(wxWindow* parent, MapDocumentWPtr document, ControllerWPtr controller, RotateObjectsTool* tool) :
        wxPanel(parent),
        m_document(document),
        m_controller(controller),
        m_tool(tool) {
            createGui();
        }
        
        void RotateObjectsToolPage::createGui() {
            wxStaticText* text1 = new wxStaticText(this, wxID_ANY, "Rotate objects");
            wxStaticText* text2 = new wxStaticText(this, wxID_ANY, "degs about");
            wxStaticText* text3 = new wxStaticText(this, wxID_ANY, "axis");
            m_angle = new SpinControl(this);
            m_angle->SetRange(-360.0, 360.0);
            
            wxString axes[] = { "X", "Y", "Z" };
            m_axis = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 3, axes);
            m_button = new wxButton(this, wxID_ANY, "Apply", wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
            
            Bind(wxEVT_IDLE, &RotateObjectsToolPage::OnIdle, this);
            m_button->Bind(wxEVT_UPDATE_UI, &RotateObjectsToolPage::OnUpdateButton, this);
            m_button->Bind(wxEVT_BUTTON, &RotateObjectsToolPage::OnApply, this);
            
            wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
            sizer->Add(text1, 0, wxALIGN_CENTER_VERTICAL);
            sizer->AddSpacer(LayoutConstants::NarrowHMargin);
            sizer->Add(m_angle);
            sizer->AddSpacer(LayoutConstants::NarrowHMargin);
            sizer->Add(text2, 0, wxALIGN_CENTER_VERTICAL);
            sizer->AddSpacer(LayoutConstants::NarrowHMargin);
            sizer->Add(m_axis);
            sizer->AddSpacer(LayoutConstants::NarrowHMargin);
            sizer->Add(text3, 0, wxALIGN_CENTER_VERTICAL);
            sizer->AddSpacer(LayoutConstants::WideHMargin);
            sizer->Add(m_button, 0);
            sizer->SetItemMinSize(m_angle, 50, wxDefaultCoord);

            SetSizer(sizer);
        }
        
        void RotateObjectsToolPage::OnIdle(wxIdleEvent& event) {
            const Grid& grid = lock(m_document)->grid();
            m_angle->SetIncrements(Math::degrees(grid.angle()), 90.0, 1.0);
        }

        void RotateObjectsToolPage::OnUpdateButton(wxUpdateUIEvent& event) {
            MapDocumentSPtr document = lock(m_document);
            event.Enable(document->hasSelectedObjects());
        }
        
        void RotateObjectsToolPage::OnApply(wxCommandEvent& event) {
            MapDocumentSPtr document = lock(m_document);
            ControllerSPtr controller = lock(m_controller);
            
            const Model::ObjectList& objects = document->selectedObjects();
            const Vec3 center = m_tool->center();
            const Vec3 axis = getAxis();
            const FloatType angle = Math::radians(m_angle->GetValue());
            const bool lockTextures = document->textureLock();
            controller->rotateObjects(objects, center, axis, angle, lockTextures);
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
