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
#include "Model/ChangeBrushFaceAttributesRequest.h"
#include "View/MapDocument.h"
#include "View/UVView.h"
#include "View/ViewConstants.h"
#include "View/wxUtils.h"

#include <wx/bmpbuttn.h>
#include <wx/sizer.h>
#include <wx/spinctrl.h>
#include <wx/stattext.h>

namespace TrenchBroom {
    namespace View {
        UVEditor::UVEditor(wxWindow* parent, MapDocumentWPtr document, GLContextManager& contextManager) :
        wxPanel(parent),
        m_document(document),
        m_uvView(NULL),
        m_xSubDivisionEditor(NULL),
        m_ySubDivisionEditor(NULL) {
            createGui(contextManager);
        }

        void UVEditor::OnResetTexture(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            Model::ChangeBrushFaceAttributesRequest request;
            request.resetTextureAxes();
            request.setOffset(Vec2f::Null);
            request.setRotation(0.0f);
            request.setScale(Vec2f::One);
            
            MapDocumentSPtr document = lock(m_document);
            document->setFaceAttributes(request);
        }
        
        void UVEditor::OnFlipTextureH(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            Model::ChangeBrushFaceAttributesRequest request;
            request.mulXScale(-1.0f);
            
            MapDocumentSPtr document = lock(m_document);
            document->setFaceAttributes(request);
        }
        
        void UVEditor::OnFlipTextureV(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            Model::ChangeBrushFaceAttributesRequest request;
            request.mulYScale(-1.0f);
            
            MapDocumentSPtr document = lock(m_document);
            document->setFaceAttributes(request);
        }
        
        void UVEditor::OnRotateTextureCCW(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            Model::ChangeBrushFaceAttributesRequest request;
            request.addRotation(90.0f);
            
            MapDocumentSPtr document = lock(m_document);
            document->setFaceAttributes(request);
        }
        
        void UVEditor::OnRotateTextureCW(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            Model::ChangeBrushFaceAttributesRequest request;
            request.addRotation(-90.0f);
            
            MapDocumentSPtr document = lock(m_document);
            document->setFaceAttributes(request);
        }
        
        void UVEditor::OnUpdateButtonUI(wxUpdateUIEvent& event) {
            if (IsBeingDeleted()) return;

            MapDocumentSPtr document = lock(m_document);
            event.Enable(!document->allSelectedBrushFaces().empty());
        }

        void UVEditor::OnSubDivisionChanged(wxSpinEvent& event) {
            if (IsBeingDeleted()) return;

            const int x = m_xSubDivisionEditor->GetValue();
            const int y = m_ySubDivisionEditor->GetValue();
            m_uvView->setSubDivisions(Vec2i(x, y));
        }
        
        void UVEditor::createGui(GLContextManager& contextManager) {
            m_uvView = new UVView(this, m_document, contextManager);
            
            wxWindow* resetTextureButton = createBitmapButton(this, "ResetTexture.png", "Reset texture alignment");
            wxWindow* flipTextureHButton = createBitmapButton(this, "FlipTextureH.png", "Flip texture X axis");
            wxWindow* flipTextureVButton = createBitmapButton(this, "FlipTextureV.png", "Flip texture Y axis");
            wxWindow* rotateTextureCCWButton = createBitmapButton(this, "RotateTextureCCW.png", "Rotate texture 90° counter-clockwise");
            wxWindow* rotateTextureCWButton = createBitmapButton(this, "RotateTextureCW.png", "Rotate texture 90° clockwise");
            
            resetTextureButton->Bind(wxEVT_BUTTON, &UVEditor::OnResetTexture, this);
            resetTextureButton->Bind(wxEVT_UPDATE_UI, &UVEditor::OnUpdateButtonUI, this);
            flipTextureHButton->Bind(wxEVT_BUTTON, &UVEditor::OnFlipTextureH, this);
            flipTextureHButton->Bind(wxEVT_UPDATE_UI, &UVEditor::OnUpdateButtonUI, this);
            flipTextureVButton->Bind(wxEVT_BUTTON, &UVEditor::OnFlipTextureV, this);
            flipTextureVButton->Bind(wxEVT_UPDATE_UI, &UVEditor::OnUpdateButtonUI, this);
            rotateTextureCCWButton->Bind(wxEVT_BUTTON, &UVEditor::OnRotateTextureCCW, this);
            rotateTextureCCWButton->Bind(wxEVT_UPDATE_UI, &UVEditor::OnUpdateButtonUI, this);
            rotateTextureCWButton->Bind(wxEVT_BUTTON, &UVEditor::OnRotateTextureCW, this);
            rotateTextureCWButton->Bind(wxEVT_UPDATE_UI, &UVEditor::OnUpdateButtonUI, this);

            wxStaticText* gridLabel = new wxStaticText(this, wxID_ANY, "Grid ");
            gridLabel->SetFont(gridLabel->GetFont().Bold());
            m_xSubDivisionEditor = new wxSpinCtrl(this, wxID_ANY, "1", wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS | wxTE_PROCESS_ENTER | wxALIGN_RIGHT);
            m_xSubDivisionEditor->SetRange(1, 16);
            
            m_ySubDivisionEditor = new wxSpinCtrl(this, wxID_ANY, "1", wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS | wxTE_PROCESS_ENTER | wxALIGN_RIGHT);
            m_ySubDivisionEditor->SetRange(1, 16);
            
            m_xSubDivisionEditor->Bind(wxEVT_SPINCTRL, &UVEditor::OnSubDivisionChanged, this);
            m_ySubDivisionEditor->Bind(wxEVT_SPINCTRL, &UVEditor::OnSubDivisionChanged, this);

            wxSizer* bottomSizer = new wxBoxSizer(wxHORIZONTAL);
            bottomSizer->Add(resetTextureButton,                   0, wxALIGN_CENTER_VERTICAL | wxRIGHT, LayoutConstants::NarrowHMargin);
            bottomSizer->Add(flipTextureHButton,                   0, wxALIGN_CENTER_VERTICAL | wxRIGHT, LayoutConstants::NarrowHMargin);
            bottomSizer->Add(flipTextureVButton,                   0, wxALIGN_CENTER_VERTICAL | wxRIGHT, LayoutConstants::NarrowHMargin);
            bottomSizer->Add(rotateTextureCCWButton,               0, wxALIGN_CENTER_VERTICAL | wxRIGHT, LayoutConstants::NarrowHMargin);
            bottomSizer->Add(rotateTextureCWButton,                0, wxALIGN_CENTER_VERTICAL | wxRIGHT, LayoutConstants::NarrowHMargin);
            bottomSizer->AddStretchSpacer();
            bottomSizer->Add(gridLabel,                              0, wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT);
            bottomSizer->Add(new wxStaticText(this, wxID_ANY, "X:"), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, LayoutConstants::NarrowHMargin);
            bottomSizer->Add(m_xSubDivisionEditor,                   0, wxALIGN_CENTER_VERTICAL | wxRIGHT, LayoutConstants::MediumHMargin);
            bottomSizer->Add(new wxStaticText(this, wxID_ANY, "Y:"), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, LayoutConstants::NarrowHMargin);
            bottomSizer->Add(m_ySubDivisionEditor,                   0, wxALIGN_CENTER_VERTICAL);
            bottomSizer->SetItemMinSize(m_xSubDivisionEditor, 50, m_xSubDivisionEditor->GetSize().y);
            bottomSizer->SetItemMinSize(m_ySubDivisionEditor, 50, m_ySubDivisionEditor->GetSize().y);
            
            wxSizer* outerSizer = new wxBoxSizer(wxVERTICAL);
            outerSizer->Add(m_uvView, 1, wxEXPAND);
            outerSizer->AddSpacer(LayoutConstants::NarrowVMargin);
            outerSizer->Add(bottomSizer, 0, wxLEFT | wxRIGHT | wxEXPAND, LayoutConstants::MediumHMargin);
            outerSizer->AddSpacer(LayoutConstants::NarrowVMargin);
            
            SetBackgroundColour(*wxWHITE);
            SetSizer(outerSizer);
        }
    }
}
