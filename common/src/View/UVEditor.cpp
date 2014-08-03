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
#include "View/ControllerFacade.h"
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
        UVEditor::UVEditor(wxWindow* parent, GLContextHolder::Ptr sharedContext, MapDocumentWPtr document, ControllerWPtr controller) :
        wxPanel(parent),
        m_document(document),
        m_controller(controller),
        m_uvView(NULL),
        m_resetTextureButton(NULL),
        m_flipTextureHButton(NULL),
        m_flipTextureVButton(NULL),
        m_rotateTextureCCWButton(NULL),
        m_rotateTextureCWButton(NULL),
        m_xSubDivisionEditor(NULL),
        m_ySubDivisionEditor(NULL) {
            createGui(sharedContext);
            bindEvents();
        }

        void UVEditor::OnResetTexture(wxCommandEvent& event) {
            MapDocumentSPtr document = lock(m_document);
            ControllerSPtr controller = lock(m_controller);
            const Model::BrushFaceList& faces = document->allSelectedFaces();
            
            const UndoableCommandGroup group(controller, "Reset texture alignment");
            controller->setFaceOffset(faces, Vec2f::Null, false);
            controller->setFaceRotation(faces, 0.0f, false);
            controller->setFaceScale(faces, Vec2f::One, false);
        }
        
        void UVEditor::OnFlipTextureH(wxCommandEvent& event) {
            MapDocumentSPtr document = lock(m_document);
            ControllerSPtr controller = lock(m_controller);
            const Model::BrushFaceList& faces = document->allSelectedFaces();
            controller->invertFaceXScale(faces);
        }
        
        void UVEditor::OnFlipTextureV(wxCommandEvent& event) {
            MapDocumentSPtr document = lock(m_document);
            ControllerSPtr controller = lock(m_controller);
            const Model::BrushFaceList& faces = document->allSelectedFaces();
            controller->invertFaceYScale(faces);
        }
        
        void UVEditor::OnRotateTextureCCW(wxCommandEvent& event) {
            MapDocumentSPtr document = lock(m_document);
            ControllerSPtr controller = lock(m_controller);
            const Model::BrushFaceList& faces = document->allSelectedFaces();
            controller->setFaceRotation(faces, 90.0f, true);
        }
        
        void UVEditor::OnRotateTextureCW(wxCommandEvent& event) {
            MapDocumentSPtr document = lock(m_document);
            ControllerSPtr controller = lock(m_controller);
            const Model::BrushFaceList& faces = document->allSelectedFaces();
            controller->setFaceRotation(faces, -90.0f, true);
        }
        
        void UVEditor::OnUpdateButtonUI(wxUpdateUIEvent& event) {
            MapDocumentSPtr document = lock(m_document);
            event.Enable(!document->allSelectedFaces().empty());
        }

        void UVEditor::OnSubDivisionChanged(wxSpinEvent& event) {
            const int x = m_xSubDivisionEditor->GetValue();
            const int y = m_ySubDivisionEditor->GetValue();
            m_uvView->setSubDivisions(Vec2i(x, y));
        }
        
        void UVEditor::createGui(GLContextHolder::Ptr sharedContext) {
            m_uvView = new UVView(this, sharedContext, m_document, m_controller);
            
            m_resetTextureButton = createBitmapButton(this, "ResetTexture.png", "Reset texture alignment");
            m_flipTextureHButton = createBitmapButton(this, "FlipTextureH.png", "Flip texture X axis");
            m_flipTextureVButton = createBitmapButton(this, "FlipTextureV.png", "Flip texture Y axis");
            m_rotateTextureCCWButton = createBitmapButton(this, "RotateTextureCCW.png", "Rotate texture 90° counter-clockwise");
            m_rotateTextureCWButton = createBitmapButton(this, "RotateTextureCW.png", "Rotate texture 90° clockwise");
            
            wxStaticText* gridLabel = new wxStaticText(this, wxID_ANY, "Grid ");
            gridLabel->SetFont(gridLabel->GetFont().Bold());
            m_xSubDivisionEditor = new wxSpinCtrl(this, wxID_ANY, "1", wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS | wxTE_PROCESS_ENTER | wxALIGN_RIGHT);
            m_xSubDivisionEditor->SetRange(1, 16);
            
            m_ySubDivisionEditor = new wxSpinCtrl(this, wxID_ANY, "1", wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS | wxTE_PROCESS_ENTER | wxALIGN_RIGHT);
            m_ySubDivisionEditor->SetRange(1, 16);
            
            wxSizer* bottomSizer = new wxBoxSizer(wxHORIZONTAL);
            bottomSizer->Add(m_resetTextureButton,                   0, wxALIGN_CENTER_VERTICAL | wxRIGHT, LayoutConstants::NarrowHMargin);
            bottomSizer->Add(m_flipTextureHButton,                   0, wxALIGN_CENTER_VERTICAL | wxRIGHT, LayoutConstants::NarrowHMargin);
            bottomSizer->Add(m_flipTextureVButton,                   0, wxALIGN_CENTER_VERTICAL | wxRIGHT, LayoutConstants::NarrowHMargin);
            bottomSizer->Add(m_rotateTextureCCWButton,               0, wxALIGN_CENTER_VERTICAL | wxRIGHT, LayoutConstants::NarrowHMargin);
            bottomSizer->Add(m_rotateTextureCWButton,                0, wxALIGN_CENTER_VERTICAL | wxRIGHT, LayoutConstants::NarrowHMargin);
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
        
        void UVEditor::bindEvents() {
            m_resetTextureButton->Bind(wxEVT_BUTTON, &UVEditor::OnResetTexture, this);
            m_resetTextureButton->Bind(wxEVT_UPDATE_UI, &UVEditor::OnUpdateButtonUI, this);
            m_flipTextureHButton->Bind(wxEVT_BUTTON, &UVEditor::OnFlipTextureH, this);
            m_flipTextureHButton->Bind(wxEVT_UPDATE_UI, &UVEditor::OnUpdateButtonUI, this);
            m_flipTextureVButton->Bind(wxEVT_BUTTON, &UVEditor::OnFlipTextureV, this);
            m_flipTextureVButton->Bind(wxEVT_UPDATE_UI, &UVEditor::OnUpdateButtonUI, this);
            m_rotateTextureCCWButton->Bind(wxEVT_BUTTON, &UVEditor::OnRotateTextureCCW, this);
            m_rotateTextureCCWButton->Bind(wxEVT_UPDATE_UI, &UVEditor::OnUpdateButtonUI, this);
            m_rotateTextureCWButton->Bind(wxEVT_BUTTON, &UVEditor::OnRotateTextureCW, this);
            m_rotateTextureCWButton->Bind(wxEVT_UPDATE_UI, &UVEditor::OnUpdateButtonUI, this);
            
            m_xSubDivisionEditor->Bind(wxEVT_SPINCTRL, &UVEditor::OnSubDivisionChanged, this);
            m_ySubDivisionEditor->Bind(wxEVT_SPINCTRL, &UVEditor::OnSubDivisionChanged, this);
        }
    }
}
