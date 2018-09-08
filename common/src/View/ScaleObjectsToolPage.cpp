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

#include "ScaleObjectsToolPage.h"

#include "View/BorderLine.h"
#include "View/Grid.h"
#include "View/MapDocument.h"
#include "View/ScaleObjectsTool.h"
#include "View/SpinControl.h"
#include "View/ViewConstants.h"

#include <wx/button.h>
#include <wx/choice.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/combobox.h>

namespace TrenchBroom {
    namespace View {
        ScaleObjectsToolPage::ScaleObjectsToolPage(wxWindow* parent, MapDocumentWPtr document) :
        wxPanel(parent),
        m_document(document) {
            createGui();
        }
        
        void ScaleObjectsToolPage::createGui() {
            wxStaticText* text = new wxStaticText(this, wxID_ANY, "Scale objects by");
            m_scaleFactors = new wxTextCtrl(this, wxID_ANY, "1.0 1.0 1.0");
            m_button = new wxButton(this, wxID_ANY, "Apply", wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
            
            m_button->Bind(wxEVT_UPDATE_UI, &ScaleObjectsToolPage::OnUpdateButton, this);
            m_button->Bind(wxEVT_BUTTON, &ScaleObjectsToolPage::OnApply, this);
            
            wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
            sizer->Add(text, 0, wxALIGN_CENTER_VERTICAL);
            sizer->AddSpacer(LayoutConstants::NarrowHMargin);
            sizer->Add(m_scaleFactors, 0, wxALIGN_CENTER_VERTICAL);
            sizer->AddSpacer(LayoutConstants::NarrowHMargin);
            sizer->Add(m_button, 0, wxALIGN_CENTER_VERTICAL);
            
            SetSizer(sizer);
        }
        
        void ScaleObjectsToolPage::OnUpdateButton(wxUpdateUIEvent& event) {
            if (IsBeingDeleted()) return;
            
            MapDocumentSPtr document = lock(m_document);
            event.Enable(document->hasSelectedNodes());
        }

        void ScaleObjectsToolPage::OnApply(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;
            
            const auto scaleFactors = vm::vec3::parse(m_scaleFactors->GetValue().ToStdString());

            auto document = lock(m_document);
            const auto box = document->selectionBounds();

            document->scaleObjects(box.center(), scaleFactors);
        }
    }
}
