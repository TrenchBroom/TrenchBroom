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

#include "UVEditor.h"
#include "View/UVView.h"
#include "View/ViewConstants.h"

#include <wx/sizer.h>
#include <wx/spinctrl.h>
#include <wx/stattext.h>

namespace TrenchBroom {
    namespace View {
        TexturingEditor::TexturingEditor(wxWindow* parent, GLContextHolder::Ptr sharedContext, MapDocumentWPtr document, ControllerWPtr controller) :
        wxPanel(parent),
        m_document(document),
        m_controller(controller),
        m_uvView(NULL),
        m_xSubDivisionEditor(NULL),
        m_ySubDivisionEditor(NULL) {
            createGui(sharedContext);
            bindEvents();
        }

        void TexturingEditor::OnSubDivisionChanged(wxSpinEvent& event) {
            const int x = m_xSubDivisionEditor->GetValue();
            const int y = m_ySubDivisionEditor->GetValue();
            m_uvView->setSubDivisions(Vec2i(x, y));
        }
        
        void TexturingEditor::createGui(GLContextHolder::Ptr sharedContext) {
            m_uvView = new UVView(this, sharedContext, m_document, m_controller);
            
            wxStaticText* gridLabel = new wxStaticText(this, wxID_ANY, "Texture Grid ");
            gridLabel->SetFont(gridLabel->GetFont().Bold());
            m_xSubDivisionEditor = new wxSpinCtrl(this, wxID_ANY, "1", wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS | wxTE_PROCESS_ENTER | wxALIGN_RIGHT);
            m_xSubDivisionEditor->SetRange(1, 16);
            
            m_ySubDivisionEditor = new wxSpinCtrl(this, wxID_ANY, "1", wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS | wxTE_PROCESS_ENTER | wxALIGN_RIGHT);
            m_ySubDivisionEditor->SetRange(1, 16);
            
            wxSizer* bottomSizer = new wxBoxSizer(wxHORIZONTAL);
            bottomSizer->Add(gridLabel,                              0, wxALIGN_CENTER_VERTICAL);
            bottomSizer->Add(new wxStaticText(this, wxID_ANY, "X:"), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, LayoutConstants::NarrowHMargin);
            bottomSizer->Add(m_xSubDivisionEditor,                   0, wxALIGN_CENTER_VERTICAL | wxRIGHT, LayoutConstants::MediumHMargin);
            bottomSizer->Add(new wxStaticText(this, wxID_ANY, "Y:"), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, LayoutConstants::NarrowHMargin);
            bottomSizer->Add(m_ySubDivisionEditor,                   0, wxALIGN_CENTER_VERTICAL);
            bottomSizer->SetItemMinSize(m_xSubDivisionEditor, 50, m_xSubDivisionEditor->GetSize().y);
            bottomSizer->SetItemMinSize(m_ySubDivisionEditor, 50, m_ySubDivisionEditor->GetSize().y);
            
            wxSizer* outerSizer = new wxBoxSizer(wxVERTICAL);
            outerSizer->Add(m_uvView, 1, wxEXPAND);
            outerSizer->AddSpacer(LayoutConstants::NarrowVMargin);
            outerSizer->Add(bottomSizer, 0, wxALIGN_RIGHT | wxLEFT | wxRIGHT, LayoutConstants::MediumHMargin);
            outerSizer->AddSpacer(LayoutConstants::NarrowVMargin);
            
            SetBackgroundColour(*wxWHITE);
            SetSizer(outerSizer);
        }
        
        void TexturingEditor::bindEvents() {
            m_xSubDivisionEditor->Bind(wxEVT_SPINCTRL, &TexturingEditor::OnSubDivisionChanged, this);
            m_ySubDivisionEditor->Bind(wxEVT_SPINCTRL, &TexturingEditor::OnSubDivisionChanged, this);
        }
    }
}
