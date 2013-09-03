/*
 Copyright (C) 2010-2013 Kristian Duske
 
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

#include "FaceAttribsEditor.h"

#include "View/LayoutConstants.h"
#include "View/SpinControl.h"
#include "View/TextureView.h"

#include <wx/gbsizer.h>
#include <wx/sizer.h>
#include <wx/stattext.h>

namespace TrenchBroom {
    namespace View {
        FaceAttribsEditor::FaceAttribsEditor(wxWindow* parent, Renderer::RenderResources& resources) :
        wxPanel(parent) {
            m_textureView = new TextureView(this, wxID_ANY, resources);
            m_textureNameLabel = new wxStaticText(this, wxID_ANY, _T("none"), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER);
            
            const double max = std::numeric_limits<double>::max();
            const double min = -max;
            
            m_xOffsetEditor = new SpinControl(this);
            m_xOffsetEditor->SetRange(min, max);
            m_xOffsetEditor->Bind(EVT_SPINCONTROL_EVENT, &FaceAttribsEditor::OnXOffsetChanged, this);
            
            m_yOffsetEditor = new SpinControl(this);
            m_yOffsetEditor->SetRange(min, max);
            m_yOffsetEditor->Bind(EVT_SPINCONTROL_EVENT, &FaceAttribsEditor::OnYOffsetChanged, this);
            
            m_xScaleEditor = new SpinControl(this);
            m_xScaleEditor->SetRange(min, max);
            m_xScaleEditor->SetIncrements(0.1, 0.25, 0.01);
            m_xScaleEditor->Bind(EVT_SPINCONTROL_EVENT, &FaceAttribsEditor::OnXScaleChanged, this);

            m_yScaleEditor = new SpinControl(this);
            m_yScaleEditor->SetRange(min, max);
            m_yScaleEditor->SetIncrements(0.1, 0.25, 0.01);
            m_yScaleEditor->Bind(EVT_SPINCONTROL_EVENT, &FaceAttribsEditor::OnYScaleChanged, this);
            
            m_rotationEditor = new SpinControl(this);
            m_rotationEditor->SetRange(min, max);
            m_rotationEditor->Bind(EVT_SPINCONTROL_EVENT, &FaceAttribsEditor::OnRotationChanged, this);
            
            wxSizer* textureViewSizer = new wxBoxSizer(wxVERTICAL);
            textureViewSizer->Add(m_textureView, 0, wxEXPAND);
            textureViewSizer->AddSpacer(LayoutConstants::ControlVerticalMargin);
            textureViewSizer->Add(m_textureNameLabel, 1, wxEXPAND | wxALIGN_CENTER);
            textureViewSizer->SetItemMinSize(m_textureView, 128, 128);
            
            wxGridBagSizer* faceAttribsSizer = new wxGridBagSizer(LayoutConstants::FaceAttribsControlMargin, LayoutConstants::FaceAttribsControlMargin);
            faceAttribsSizer->Add(new wxStaticText(this, wxID_ANY, wxT("")), wxGBPosition(0, 0), wxDefaultSpan, wxALIGN_CENTER); // fake
            faceAttribsSizer->Add(new wxStaticText(this, wxID_ANY, wxT("X"), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER), wxGBPosition(0, 1), wxDefaultSpan, wxEXPAND | wxALIGN_CENTER);
            faceAttribsSizer->Add(new wxStaticText(this, wxID_ANY, wxT("Y"), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER), wxGBPosition(0, 2), wxDefaultSpan, wxEXPAND | wxALIGN_CENTER);
            
            faceAttribsSizer->Add(new wxStaticText(this, wxID_ANY, wxT("Offset"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT), wxGBPosition(1, 0), wxDefaultSpan, wxALIGN_RIGHT);
            faceAttribsSizer->Add(m_xOffsetEditor, wxGBPosition(1, 1), wxDefaultSpan, wxEXPAND);
            faceAttribsSizer->Add(m_yOffsetEditor, wxGBPosition(1, 2), wxDefaultSpan, wxEXPAND);
            faceAttribsSizer->Add(new wxStaticText(this, wxID_ANY, wxT("Scale"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT), wxGBPosition(2, 0), wxDefaultSpan, wxALIGN_RIGHT);
            
            faceAttribsSizer->Add(m_xScaleEditor, wxGBPosition(2, 1), wxDefaultSpan, wxEXPAND);
            faceAttribsSizer->Add(m_yScaleEditor, wxGBPosition(2, 2), wxDefaultSpan, wxEXPAND);
            
            faceAttribsSizer->Add(new wxStaticText(this, wxID_ANY, wxT("Rotation"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT), wxGBPosition(3, 0), wxGBSpan(1, 2), wxEXPAND |wxALIGN_RIGHT);
            faceAttribsSizer->Add(m_rotationEditor, wxGBPosition(3, 2), wxDefaultSpan, wxEXPAND);
            
            //            faceAttribsSizer->Add(buttonSizer, wxGBPosition(4, 0), wxGBSpan(1, 3), wxALIGN_RIGHT);
            
            faceAttribsSizer->AddGrowableCol(1);
            faceAttribsSizer->AddGrowableCol(2);
            faceAttribsSizer->SetItemMinSize(m_xOffsetEditor, 50, m_xOffsetEditor->GetSize().y);
            faceAttribsSizer->SetItemMinSize(m_yOffsetEditor, 50, m_yOffsetEditor->GetSize().y);
            faceAttribsSizer->SetItemMinSize(m_xScaleEditor, 50, m_xScaleEditor->GetSize().y);
            faceAttribsSizer->SetItemMinSize(m_yScaleEditor, 50, m_yScaleEditor->GetSize().y);
            faceAttribsSizer->SetItemMinSize(m_rotationEditor, 50, m_rotationEditor->GetSize().y);
            
            wxSizer* faceEditorSizer = new wxBoxSizer(wxHORIZONTAL);
            faceEditorSizer->Add(textureViewSizer);
            faceEditorSizer->AddSpacer(LayoutConstants::ControlHorizontalMargin);
            faceEditorSizer->Add(faceAttribsSizer, 1, wxEXPAND);
            
            SetSizer(faceEditorSizer);
        }

        void FaceAttribsEditor::OnXOffsetChanged(SpinControlEvent& event) {
        }
        
        void FaceAttribsEditor::OnYOffsetChanged(SpinControlEvent& event) {
        }
        
        void FaceAttribsEditor::OnXScaleChanged(SpinControlEvent& event) {
        }
        
        void FaceAttribsEditor::OnYScaleChanged(SpinControlEvent& event) {
        }
        
        void FaceAttribsEditor::OnRotationChanged(SpinControlEvent& event) {
        }
    }
}
