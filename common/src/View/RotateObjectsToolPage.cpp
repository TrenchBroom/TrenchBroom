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

#include "RotateObjectsToolPage.h"

#include "TrenchBroom.h"
#include "View/BorderLine.h"
#include "View/Grid.h"
#include "View/MapDocument.h"
#include "View/RotateObjectsTool.h"
#include "View/SpinControl.h"
#include "View/ViewConstants.h"

#include <vecmath/vec.h>
#include <vecmath/util.h>

#include <wx/button.h>
#include <wx/choice.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/combobox.h>

namespace TrenchBroom {
    namespace View {
        RotateObjectsToolPage::RotateObjectsToolPage(wxWindow* parent, MapDocumentWPtr document, RotateObjectsTool* tool) :
        wxPanel(parent),
        m_document(document),
        m_tool(tool) {
            createGui();
            m_angle->SetValue(vm::toDegrees(m_tool->angle()));
        }

        void RotateObjectsToolPage::setAxis(const vm::axis::type axis) {
            m_axis->SetSelection(static_cast<int>(axis));
        }

        void RotateObjectsToolPage::setRecentlyUsedCenters(const std::vector<vm::vec3>& centers) {
            m_recentlyUsedCentersList->Clear();

            for (auto it = centers.rbegin(), end = centers.rend(); it != end; ++it) {
                const auto& center = *it;
                m_recentlyUsedCentersList->Append(StringUtils::toString(center));
            }

            if (m_recentlyUsedCentersList->GetCount() > 0)
                m_recentlyUsedCentersList->SetSelection(0);
        }

        void RotateObjectsToolPage::setCurrentCenter(const vm::vec3& center) {
            m_recentlyUsedCentersList->SetValue(StringUtils::toString(center));
        }

        void RotateObjectsToolPage::createGui() {
            auto* centerText = new wxStaticText(this, wxID_ANY, "Center");
            m_recentlyUsedCentersList = new wxComboBox(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, nullptr, wxTE_PROCESS_ENTER);

            m_resetCenterButton = new wxButton(this, wxID_ANY, "Reset", wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
            m_resetCenterButton->SetToolTip("Reset the position of the rotate handle to the center of the current selection.");

            auto* text1 = new wxStaticText(this, wxID_ANY, "Rotate objects");
            auto* text2 = new wxStaticText(this, wxID_ANY, "degs about");
            auto* text3 = new wxStaticText(this, wxID_ANY, "axis");
            m_angle = new SpinControl(this);
            m_angle->SetRange(-360.0, 360.0);
            m_angle->SetValue(vm::toDegrees(m_tool->angle()));
            m_angle->SetDigits(0, 4);

            wxString axes[] = { "X", "Y", "Z" };
            m_axis = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 3, axes);
            m_axis->SetSelection(2);

            m_rotateButton = new wxButton(this, wxID_ANY, "Apply", wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);

            Bind(wxEVT_IDLE, &RotateObjectsToolPage::OnIdle, this);
            m_recentlyUsedCentersList->Bind(wxEVT_TEXT_ENTER, &RotateObjectsToolPage::OnCenterChanged, this);
            m_recentlyUsedCentersList->Bind(wxEVT_COMBOBOX, &RotateObjectsToolPage::OnCenterChanged, this);
            m_resetCenterButton->Bind(wxEVT_BUTTON, &RotateObjectsToolPage::OnResetCenter, this);
            m_angle->Bind(SPIN_CONTROL_EVENT, &RotateObjectsToolPage::OnAngleChanged, this);
            m_rotateButton->Bind(wxEVT_UPDATE_UI, &RotateObjectsToolPage::OnUpdateRotateButton, this);
            m_rotateButton->Bind(wxEVT_BUTTON, &RotateObjectsToolPage::OnRotate, this);

            auto* separator = new BorderLine(this, BorderLine::Direction_Vertical);
            separator->SetForegroundColour(Colors::separatorColor());

            auto* sizer = new wxBoxSizer(wxHORIZONTAL);
            sizer->Add(centerText, 0, wxALIGN_CENTER_VERTICAL);
            sizer->AddSpacer(LayoutConstants::NarrowHMargin);
            sizer->Add(m_recentlyUsedCentersList, 0, wxALIGN_CENTER_VERTICAL);
            sizer->AddSpacer(LayoutConstants::NarrowHMargin);
            sizer->Add(m_resetCenterButton, 0, wxALIGN_CENTER_VERTICAL);
            sizer->AddSpacer(LayoutConstants::MediumHMargin);
            sizer->Add(separator, 0, wxEXPAND | wxTOP | wxBOTTOM, 2);
            sizer->AddSpacer(LayoutConstants::NarrowHMargin);
            sizer->Add(text1, 0, wxALIGN_CENTER_VERTICAL);
            sizer->AddSpacer(LayoutConstants::NarrowHMargin);
            sizer->Add(m_angle, 0, wxALIGN_CENTER_VERTICAL);
            sizer->AddSpacer(LayoutConstants::NarrowHMargin);
            sizer->Add(text2, 0, wxALIGN_CENTER_VERTICAL);
            sizer->AddSpacer(LayoutConstants::NarrowHMargin);
            sizer->Add(m_axis, 0, wxTOP, LayoutConstants::ChoiceTopMargin);
            sizer->AddSpacer(LayoutConstants::NarrowHMargin);
            sizer->Add(text3, 0, wxALIGN_CENTER_VERTICAL);
            sizer->AddSpacer(LayoutConstants::NarrowHMargin);
            sizer->Add(m_rotateButton, 0, wxALIGN_CENTER_VERTICAL);
            sizer->SetItemMinSize(m_angle, 80, wxDefaultCoord);

            SetSizer(sizer);
        }

        void RotateObjectsToolPage::OnIdle(wxIdleEvent& event) {
            if (IsBeingDeleted()) return;

            const auto& grid = lock(m_document)->grid();
            m_angle->SetIncrements(vm::toDegrees(grid.angle()), 90.0, 1.0);
        }

        void RotateObjectsToolPage::OnCenterChanged(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            const auto center = vm::vec3::parse(m_recentlyUsedCentersList->GetValue().ToStdString());
            m_tool->setRotationCenter(center);
        }

        void RotateObjectsToolPage::OnResetCenter(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            m_tool->resetRotationCenter();
        }

        void RotateObjectsToolPage::OnAngleChanged(SpinControlEvent& event) {
            if (IsBeingDeleted()) return;

            const auto newAngleDegs = vm::correct(event.IsSpin() ? m_angle->GetValue() + event.GetValue() : event.GetValue());
            m_angle->SetValue(newAngleDegs);
            m_tool->setAngle(vm::toRadians(newAngleDegs));
        }

        void RotateObjectsToolPage::OnUpdateRotateButton(wxUpdateUIEvent& event) {
            if (IsBeingDeleted()) return;

            auto document = lock(m_document);
            event.Enable(document->hasSelectedNodes());
        }

        void RotateObjectsToolPage::OnRotate(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            const auto center = m_tool->rotationCenter();
            const auto axis = getAxis();
            const auto angle = vm::toRadians(m_angle->GetValue());

            auto document = lock(m_document);
            document->rotateObjects(center, axis, angle);
        }

        vm::vec3 RotateObjectsToolPage::getAxis() const {
            switch (m_axis->GetSelection()) {
                case 0:
                    return vm::vec3::pos_x;
                case 1:
                    return vm::vec3::pos_y;
                default:
                    return vm::vec3::pos_z;
            }
        }
    }
}
