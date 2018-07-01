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

#include "ShearObjectsToolPage.h"

#include "View/BorderLine.h"
#include "View/Grid.h"
#include "View/MapDocument.h"
#include "View/ShearObjectsTool.h"
#include "View/SpinControl.h"
#include "View/ViewConstants.h"

#include <wx/button.h>
#include <wx/choice.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/combobox.h>

namespace TrenchBroom {
    namespace View {
        ShearObjectsToolPage::ShearObjectsToolPage(wxWindow* parent, MapDocumentWPtr document) :
        wxPanel(parent),
        m_document(document) {
            createGui();
        }
        
        void ShearObjectsToolPage::createGui() {
            wxStaticText* text = new wxStaticText(this, wxID_ANY, "Scale objects by");
            m_scaleFactors = new wxTextCtrl(this, wxID_ANY, "1.0 1.0 1.0");
            m_button = new wxButton(this, wxID_ANY, "Apply", wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
            
            m_button->Bind(wxEVT_UPDATE_UI, &ShearObjectsToolPage::OnUpdateButton, this);
            m_button->Bind(wxEVT_BUTTON, &ShearObjectsToolPage::OnApply, this);
            
            wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
            sizer->Add(text, 0, wxALIGN_CENTER_VERTICAL);
            sizer->AddSpacer(LayoutConstants::NarrowHMargin);
            sizer->Add(m_scaleFactors, 0, wxALIGN_CENTER_VERTICAL);
            sizer->AddSpacer(LayoutConstants::NarrowHMargin);
            sizer->Add(m_button, 0, wxALIGN_CENTER_VERTICAL);
            
            SetSizer(sizer);
        }
        
        void ShearObjectsToolPage::OnUpdateButton(wxUpdateUIEvent& event) {
            if (IsBeingDeleted()) return;
            
            MapDocumentSPtr document = lock(m_document);
            event.Enable(document->hasSelectedNodes());
        }

        void ShearObjectsToolPage::OnApply(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;
            
            const Vec3 scaleFactors = Vec3::parse(m_scaleFactors->GetValue().ToStdString());
            
            MapDocumentSPtr document = lock(m_document);
            const auto box = document->selectionBounds();
            document->scaleObjects(box, scaleBBoxFromCenter(box, scaleFactors));
        }

        BBox3 ShearObjectsToolPage::scaleBBoxFromCenter(const BBox3& box, const Vec3& scaleFactors) {
            Vec3 newSize;
            for (size_t i = 0; i < 3; i++) {
                newSize[i] = box.size()[i] * scaleFactors[i];
            }

            return BBox3(box.min - (newSize * 0.5), box.max + (newSize * 0.5));
        }
    }
}
