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

#include "MoveObjectsToolPage.h"

#include "View/MapDocument.h"
#include "View/ViewConstants.h"

#include <wx/button.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>

namespace TrenchBroom {
    namespace View {
        MoveObjectsToolPage::MoveObjectsToolPage(wxWindow* parent, MapDocumentWPtr document) :
        wxPanel(parent),
        m_document(document) {
            createGui();
        }

        void MoveObjectsToolPage::createGui() {
            wxStaticText* text = new wxStaticText(this, wxID_ANY, "Move objects by");
            m_offset = new wxTextCtrl(this, wxID_ANY, "0.0 0.0 0.0");
            m_button = new wxButton(this, wxID_ANY, "Apply", wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
         
            m_button->Bind(wxEVT_UPDATE_UI, &MoveObjectsToolPage::OnUpdateButton, this);
            m_button->Bind(wxEVT_BUTTON, &MoveObjectsToolPage::OnApply, this);
            
            wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
            sizer->Add(text, 0, wxALIGN_CENTER_VERTICAL);
            sizer->AddSpacer(LayoutConstants::NarrowHMargin);
            sizer->Add(m_offset, 0, wxALIGN_CENTER_VERTICAL);
            sizer->AddSpacer(LayoutConstants::NarrowHMargin);
            sizer->Add(m_button, 0, wxALIGN_CENTER_VERTICAL);
            
            SetSizer(sizer);
        }

        void MoveObjectsToolPage::OnUpdateButton(wxUpdateUIEvent& event) {
            if (IsBeingDeleted()) return;

            MapDocumentSPtr document = lock(m_document);
            event.Enable(document->hasSelectedNodes());
        }

        void MoveObjectsToolPage::OnApply(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            const Vec3 delta = Vec3::parse(m_offset->GetValue().ToStdString());

            MapDocumentSPtr document = lock(m_document);
            document->translateObjects(delta);
        }
    }
}
