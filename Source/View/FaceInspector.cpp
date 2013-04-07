/*
 Copyright (C) 2010-2012 Kristian Duske
 
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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */


#include "FaceInspector.h"

#include <wx/button.h>
#include <wx/filedlg.h>
#include <wx/gbsizer.h>
#include <wx/listbox.h>
#include <wx/sizer.h>
#include <wx/splitter.h>
#include <wx/statline.h>
#include <wx/stattext.h>

#include "Controller/Command.h"
#include "Controller/SetFaceAttributesCommand.h"
#include "Controller/TextureCollectionCommand.h"
#include "IO/FileManager.h"
#include "Model/Brush.h"
#include "Model/EditStateManager.h"
#include "Model/Face.h"
#include "Model/MapDocument.h"
#include "Model/Texture.h"
#include "Renderer/SharedResources.h"
#include "Utility/Grid.h"
#include "View/CommandIds.h"
#include "View/DocumentViewHolder.h"
#include "View/EditorView.h"
#include "View/LayoutConstants.h"
#include "View/SingleTextureViewer.h"
#include "View/TextureBrowser.h"
#include "View/TextureBrowserCanvas.h"
#include "View/TextureSelectedCommand.h"

#include <limits>

namespace TrenchBroom {
    namespace View {
        namespace LayoutConstants {
#if defined _WIN32
            static const int TextureAttribsControlMargin    = 5;
#elif defined __APPLE__
            static const int TextureAttribsControlMargin    = 5;
#elif defined __linux__
            static const int TextureAttribsControlMargin    = 5;
#endif
        }

        BEGIN_EVENT_TABLE(FaceInspector, wxPanel)
        EVT_SPINCONTROL(CommandIds::FaceInspector::XOffsetEditorId, FaceInspector::OnXOffsetChanged)
        EVT_SPINCONTROL(CommandIds::FaceInspector::YOffsetEditorId, FaceInspector::OnYOffsetChanged)
        EVT_SPINCONTROL(CommandIds::FaceInspector::XScaleEditorId, FaceInspector::OnXScaleChanged)
        EVT_SPINCONTROL(CommandIds::FaceInspector::YScaleEditorId, FaceInspector::OnYScaleChanged)
        EVT_SPINCONTROL(CommandIds::FaceInspector::RotationEditorId, FaceInspector::OnRotationChanged)
        EVT_BUTTON(CommandIds::FaceInspector::ResetFaceAttribsId, FaceInspector::OnResetFaceAttribsPressed)
        EVT_BUTTON(CommandIds::FaceInspector::AlignTextureId, FaceInspector::OnAlignTexturePressed)
        EVT_BUTTON(CommandIds::FaceInspector::FitTextureId, FaceInspector::OnFitTexturePressed)
        EVT_UPDATE_UI(CommandIds::FaceInspector::ResetFaceAttribsId, FaceInspector::OnUpdateFaceButtons)
        EVT_UPDATE_UI(CommandIds::FaceInspector::AlignTextureId, FaceInspector::OnUpdateFaceButtons)
        EVT_UPDATE_UI(CommandIds::FaceInspector::FitTextureId, FaceInspector::OnUpdateFaceButtons)
        EVT_TEXTURE_SELECTED(CommandIds::FaceInspector::TextureBrowserId, FaceInspector::OnTextureSelected)
        EVT_IDLE(FaceInspector::OnIdle)
        END_EVENT_TABLE()

        wxWindow* FaceInspector::createFaceEditor() {
            wxPanel* faceEditorPanel = new wxPanel(this);
            
            m_textureViewer = new SingleTextureViewer(faceEditorPanel, m_documentViewHolder.document().sharedResources());
            m_textureNameLabel = new wxStaticText(faceEditorPanel, wxID_ANY, wxT("none"), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER);
            
            wxSizer* textureViewerSizer = new wxBoxSizer(wxVERTICAL);
            textureViewerSizer->Add(m_textureViewer, 0, wxEXPAND);
            textureViewerSizer->AddSpacer(LayoutConstants::ControlVerticalMargin);
            textureViewerSizer->Add(m_textureNameLabel, 1, wxEXPAND | wxALIGN_CENTER);
            textureViewerSizer->SetItemMinSize(m_textureViewer, 128, 128);

            double max = std::numeric_limits<double>::max();
            double min = -max;
            
            m_xOffsetEditor = new SpinControl(faceEditorPanel, CommandIds::FaceInspector::XOffsetEditorId);
            m_xOffsetEditor->SetRange(min, max);
            m_yOffsetEditor = new SpinControl(faceEditorPanel, CommandIds::FaceInspector::YOffsetEditorId);
            m_yOffsetEditor->SetRange(min, max);
            m_xScaleEditor = new SpinControl(faceEditorPanel, CommandIds::FaceInspector::XScaleEditorId);
            m_xScaleEditor->SetRange(min, max);
            m_xScaleEditor->SetIncrements(0.1, 0.25, 0.01);
            m_yScaleEditor = new SpinControl(faceEditorPanel, CommandIds::FaceInspector::YScaleEditorId);
            m_yScaleEditor->SetRange(min, max);
            m_yScaleEditor->SetIncrements(0.1, 0.25, 0.01);
            m_rotationEditor = new SpinControl(faceEditorPanel, CommandIds::FaceInspector::RotationEditorId);
            m_rotationEditor->SetRange(min, max);

            wxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
            buttonSizer->Add(new wxButton(faceEditorPanel, CommandIds::FaceInspector::ResetFaceAttribsId, wxT("Reset"), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT));
            
            /*
            buttonSizer->AddSpacer(LayoutConstants::ControlHorizontalMargin);
            buttonSizer->Add(new wxButton(faceEditorPanel, CommandIds::FaceInspector::AlignTextureId, wxT("A"), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT));
            buttonSizer->AddSpacer(LayoutConstants::ControlHorizontalMargin);
            buttonSizer->Add(new wxButton(faceEditorPanel, CommandIds::FaceInspector::FitTextureId, wxT("F"), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT));
             */

            wxGridBagSizer* textureAttribsSizer = new wxGridBagSizer(LayoutConstants::TextureAttribsControlMargin, LayoutConstants::TextureAttribsControlMargin);
            textureAttribsSizer->Add(new wxStaticText(faceEditorPanel, wxID_ANY, wxT("")), wxGBPosition(0, 0), wxDefaultSpan, wxALIGN_CENTER); // fake
            textureAttribsSizer->Add(new wxStaticText(faceEditorPanel, wxID_ANY, wxT("X"), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER), wxGBPosition(0, 1), wxDefaultSpan, wxEXPAND | wxALIGN_CENTER);
            textureAttribsSizer->Add(new wxStaticText(faceEditorPanel, wxID_ANY, wxT("Y"), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER), wxGBPosition(0, 2), wxDefaultSpan, wxEXPAND | wxALIGN_CENTER);

            textureAttribsSizer->Add(new wxStaticText(faceEditorPanel, wxID_ANY, wxT("Offset"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT), wxGBPosition(1, 0), wxDefaultSpan, wxALIGN_RIGHT);
            textureAttribsSizer->Add(m_xOffsetEditor, wxGBPosition(1, 1), wxDefaultSpan, wxEXPAND);
            textureAttribsSizer->Add(m_yOffsetEditor, wxGBPosition(1, 2), wxDefaultSpan, wxEXPAND);
            textureAttribsSizer->Add(new wxStaticText(faceEditorPanel, wxID_ANY, wxT("Scale"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT), wxGBPosition(2, 0), wxDefaultSpan, wxALIGN_RIGHT);
            
            textureAttribsSizer->Add(m_xScaleEditor, wxGBPosition(2, 1), wxDefaultSpan, wxEXPAND);
            textureAttribsSizer->Add(m_yScaleEditor, wxGBPosition(2, 2), wxDefaultSpan, wxEXPAND);
            
            textureAttribsSizer->Add(new wxStaticText(faceEditorPanel, wxID_ANY, wxT("Rotation"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT), wxGBPosition(3, 0), wxGBSpan(1, 2), wxEXPAND |wxALIGN_RIGHT);
            textureAttribsSizer->Add(m_rotationEditor, wxGBPosition(3, 2), wxDefaultSpan, wxEXPAND);
            
            textureAttribsSizer->Add(new wxStaticText(faceEditorPanel, wxID_ANY, wxT(""), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT), wxGBPosition(4, 0), wxDefaultSpan, wxALIGN_RIGHT);
            textureAttribsSizer->Add(new wxStaticText(faceEditorPanel, wxID_ANY, wxT(""), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT), wxGBPosition(4, 1), wxDefaultSpan, wxALIGN_LEFT);
            textureAttribsSizer->Add(buttonSizer, wxGBPosition(4, 2), wxDefaultSpan, wxALIGN_LEFT);
            
            textureAttribsSizer->AddGrowableCol(1);
            textureAttribsSizer->AddGrowableCol(2);
            textureAttribsSizer->SetItemMinSize(m_xOffsetEditor, 50, m_xOffsetEditor->GetSize().y);
            textureAttribsSizer->SetItemMinSize(m_yOffsetEditor, 50, m_yOffsetEditor->GetSize().y);
            textureAttribsSizer->SetItemMinSize(m_xScaleEditor, 50, m_xScaleEditor->GetSize().y);
            textureAttribsSizer->SetItemMinSize(m_yScaleEditor, 50, m_yScaleEditor->GetSize().y);
            textureAttribsSizer->SetItemMinSize(m_rotationEditor, 50, m_rotationEditor->GetSize().y);
            
            wxSizer* outerSizer = new wxBoxSizer(wxHORIZONTAL);
            outerSizer->Add(textureViewerSizer);
            outerSizer->AddSpacer(LayoutConstants::ControlHorizontalMargin);
            outerSizer->Add(textureAttribsSizer, 1, wxEXPAND);
            
            faceEditorPanel->SetSizerAndFit(outerSizer);
            return faceEditorPanel;
        }
        
        wxWindow* FaceInspector::createTextureBrowser() {
            m_textureBrowser = new TextureBrowser(this, CommandIds::FaceInspector::TextureBrowserId, m_documentViewHolder);
            return m_textureBrowser;
        }
        
        void FaceInspector::updateFaceAttributes() {
            Model::FaceList faces;
            if (m_documentViewHolder.valid()) {
                Model::EditStateManager& editStateManager = m_documentViewHolder.document().editStateManager();
                faces = editStateManager.allSelectedFaces();
            }
            
            if (!faces.empty()) {
                bool xOffsetMulti = false;
                bool yOffsetMulti = false;
                bool xScaleMulti = false;
                bool yScaleMulti = false;
                bool rotationMulti = false;
                bool textureMulti = false;
                
                const float xOffset = faces[0]->xOffset();
                const float yOffset = faces[0]->yOffset();
                const float xScale = faces[0]->xScale();
                const float yScale = faces[0]->yScale();
                const float rotation = faces[0]->rotation();
                Model::Texture* const texture = faces[0]->texture();
                const String& textureName = faces[0]->textureName();
                
                for (unsigned int i = 1; i < faces.size(); i++) {
                    Model::Face* face = faces[i];
                    xOffsetMulti |= (xOffset != face->xOffset());
                    yOffsetMulti |= (yOffset != face->yOffset());
                    xScaleMulti |= (xScale != face->xScale());
                    yScaleMulti |= (yScale != face->yScale());
                    rotationMulti |= (rotation != face->rotation());
                    textureMulti |= (texture != face->texture());
                }
                
                m_xOffsetEditor->Enable();
                m_yOffsetEditor->Enable();
                m_xScaleEditor->Enable();
                m_yScaleEditor->Enable();
                m_rotationEditor->Enable();
                
                if (xOffsetMulti) {
                    m_xOffsetEditor->SetHint(wxT("multi"));
                    m_xOffsetEditor->SetValue(wxT(""));
                } else {
                    m_xOffsetEditor->SetHint(wxT(""));
                    m_xOffsetEditor->SetValue(xOffset);
                }
                if (yOffsetMulti) {
                    m_yOffsetEditor->SetHint(wxT("multi"));
                    m_yOffsetEditor->SetValue(wxT(""));
                } else {
                    m_yOffsetEditor->SetHint(wxT(""));
                    m_yOffsetEditor->SetValue(yOffset);
                }
                if (xScaleMulti){
                    m_xScaleEditor->SetHint(wxT("multi"));
                    m_xScaleEditor->SetValue(wxT(""));
                } else {
                    m_xScaleEditor->SetHint(wxT(""));
                    m_xScaleEditor->SetValue(xScale);
                }
                if (yScaleMulti) {
                    m_yScaleEditor->SetHint(wxT("multi"));
                    m_yScaleEditor->SetValue(wxT(""));
                } else {
                    m_yScaleEditor->SetHint(wxT(""));
                    m_yScaleEditor->SetValue(yScale);
                }
                if (rotationMulti) {
                    m_rotationEditor->SetHint(wxT("multi"));
                    m_rotationEditor->SetValue(wxT(""));
                } else {
                    m_rotationEditor->SetHint(wxT(""));
                    m_rotationEditor->SetValue(rotation);
                }
                if (textureMulti) {
                    m_textureViewer->setTexture(NULL);
                    m_textureNameLabel->SetLabel(wxT("multi"));
                } else {
                    m_textureViewer->setTexture(texture);
                    m_textureNameLabel->SetLabel(textureName);
                }
            } else {
                m_xOffsetEditor->SetValue(wxT("n/a"));
                m_xOffsetEditor->Disable();
                m_yOffsetEditor->SetValue(wxT("n/a"));
                m_yOffsetEditor->Disable();
                m_xScaleEditor->SetValue(wxT("n/a"));
                m_xScaleEditor->Disable();
                m_yScaleEditor->SetValue(wxT("n/a"));
                m_yScaleEditor->Disable();
                m_rotationEditor->SetValue(wxT("n/a"));
                m_rotationEditor->Disable();
                m_textureViewer->setTexture(NULL);
                m_textureNameLabel->SetLabel("n/a");
            }
        }
        
        void FaceInspector::updateSelectedTexture() {
            if (m_documentViewHolder.valid())
                m_textureBrowser->setSelectedTexture(m_documentViewHolder.document().mruTexture());
            else
                m_textureBrowser->setSelectedTexture(NULL);
        }
        
        void FaceInspector::updateTextureBrowser(bool reloadTextures) {
            m_textureBrowser->reload(reloadTextures);
        }
        
        FaceInspector::FaceInspector(wxWindow* parent, DocumentViewHolder& documentViewHolder) :
        wxPanel(parent),
        m_documentViewHolder(documentViewHolder) {
            
            // layout of the contained controls
            wxSizer* innerSizer = new wxBoxSizer(wxVERTICAL);
            innerSizer->Add(createFaceEditor(), 0, wxEXPAND);
            innerSizer->AddSpacer(LayoutConstants::DefaultVerticalMargin);
            innerSizer->Add(new wxStaticLine(this), 0, wxEXPAND);
            innerSizer->AddSpacer(LayoutConstants::ControlVerticalMargin);
            innerSizer->Add(createTextureBrowser(), 1, wxEXPAND | wxBOTTOM, LayoutConstants::NotebookPageExtraBottomMargin);
            
            // creates 5 pixel border inside the page
            wxSizer* outerSizer = new wxBoxSizer(wxVERTICAL);
            outerSizer->Add(innerSizer, 1, wxEXPAND | wxALL, LayoutConstants::NotebookPageInnerMargin);
            SetSizerAndFit(outerSizer);
            
            updateFaceAttributes();
            updateSelectedTexture();
        }

        void FaceInspector::update(const Controller::Command& command) {
            switch (command.type()) {
                case Controller::Command::LoadMap:
                case Controller::Command::ClearMap:
                case Controller::Command::RemoveTextureCollection:
                case Controller::Command::MoveTextureCollectionUp:
                case Controller::Command::MoveTextureCollectionDown:
                case Controller::Command::AddTextureCollection:
                    updateFaceAttributes();
                    updateSelectedTexture();
                    updateTextureBrowser(true);
                    break;
                case Controller::Command::ChangeEditState:
                    updateFaceAttributes();
                    updateSelectedTexture();
                    break;
                case Controller::Command::SetFaceAttributes:
                case Controller::Command::MoveTextures:
                case Controller::Command::RotateTextures:
                    updateFaceAttributes();
                    updateSelectedTexture();
                    updateTextureBrowser(false);
                    break;
                case Controller::Command::AddObjects:
                case Controller::Command::RemoveObjects:
                    updateTextureBrowser(false);
                default:
                    break;
            }
        }

        void FaceInspector::OnXOffsetChanged(SpinControlEvent& event) {
            if (!m_documentViewHolder.valid())
                return;
            Model::MapDocument& document = m_documentViewHolder.document();
            const Model::FaceList& faces = document.editStateManager().allSelectedFaces();
            Controller::SetFaceAttributesCommand* command = new Controller::SetFaceAttributesCommand(document, faces, "Set X Offset");
            command->setXOffset(static_cast<float>(event.GetValue()));
            document.GetCommandProcessor()->Submit(command);
        }
        
        void FaceInspector::OnYOffsetChanged(SpinControlEvent& event) {
            if (!m_documentViewHolder.valid())
                return;
            Model::MapDocument& document = m_documentViewHolder.document();
            const Model::FaceList& faces = document.editStateManager().allSelectedFaces();
            Controller::SetFaceAttributesCommand* command = new Controller::SetFaceAttributesCommand(document, faces, "Set Y Offset");
            command->setYOffset(static_cast<float>(event.GetValue()));
            document.GetCommandProcessor()->Submit(command);
        }
        
        void FaceInspector::OnXScaleChanged(SpinControlEvent& event) {
            if (!m_documentViewHolder.valid())
                return;
            Model::MapDocument& document = m_documentViewHolder.document();
            const Model::FaceList& faces = document.editStateManager().allSelectedFaces();
            Controller::SetFaceAttributesCommand* command = new Controller::SetFaceAttributesCommand(document, faces, "Set X Scale");
            command->setXScale(static_cast<float>(event.GetValue()));
            document.GetCommandProcessor()->Submit(command);
        }
        
        void FaceInspector::OnYScaleChanged(SpinControlEvent& event) {
            if (!m_documentViewHolder.valid())
                return;
            Model::MapDocument& document = m_documentViewHolder.document();
            const Model::FaceList& faces = document.editStateManager().allSelectedFaces();
            Controller::SetFaceAttributesCommand* command = new Controller::SetFaceAttributesCommand(document, faces, "Set Y Scale");
            command->setYScale(static_cast<float>(event.GetValue()));
            document.GetCommandProcessor()->Submit(command);
        }
        
        void FaceInspector::OnRotationChanged(SpinControlEvent& event) {
            if (!m_documentViewHolder.valid())
                return;
            Model::MapDocument& document = m_documentViewHolder.document();
            const Model::FaceList& faces = document.editStateManager().allSelectedFaces();
            Controller::SetFaceAttributesCommand* command = new Controller::SetFaceAttributesCommand(document, faces, "Set Rotation");
            command->setRotation(static_cast<float>(event.GetValue()));
            document.GetCommandProcessor()->Submit(command);
        }

        void FaceInspector::OnResetFaceAttribsPressed(wxCommandEvent& event) {
            if (!m_documentViewHolder.valid())
                return;
            Model::MapDocument& document = m_documentViewHolder.document();
            const Model::FaceList& faces = document.editStateManager().allSelectedFaces();
            Controller::SetFaceAttributesCommand* command = new Controller::SetFaceAttributesCommand(document, faces, "Set X Scale");
            command->setXOffset(0.0f);
            command->setYOffset(0.0f);
            command->setXScale(1.0f);
            command->setYScale(1.0f);
            command->setRotation(0.0f);
            document.GetCommandProcessor()->Submit(command);
        }
        
        void FaceInspector::OnAlignTexturePressed(wxCommandEvent& event) {
        }
        
        void FaceInspector::OnFitTexturePressed(wxCommandEvent& event) {
        }
        
        void FaceInspector::OnUpdateFaceButtons(wxUpdateUIEvent& event) {
            Model::MapDocument& document = m_documentViewHolder.document();
            const Model::FaceList& faces = document.editStateManager().allSelectedFaces();
            event.Enable(!faces.empty());
        }

        void FaceInspector::OnTextureSelected(TextureSelectedCommand& event) {
            if (!m_documentViewHolder.valid())
                return;
            Model::MapDocument& document = m_documentViewHolder.document();
            const Model::FaceList& faces = document.editStateManager().allSelectedFaces();
            Controller::SetFaceAttributesCommand* command = new Controller::SetFaceAttributesCommand(document, faces, "Set Texture");
            command->setTexture(m_textureBrowser->selectedTexture());
            document.GetCommandProcessor()->Submit(command);
        }

        void FaceInspector::OnIdle(wxIdleEvent& event) {
            if (!m_documentViewHolder.valid())
                return;
            
            Utility::Grid& grid = m_documentViewHolder.document().grid();
            m_xOffsetEditor->SetIncrements(grid.actualSize(), 2.0 * grid.actualSize(), 1.0);
            m_yOffsetEditor->SetIncrements(grid.actualSize(), 2.0 * grid.actualSize(), 1.0);
            m_rotationEditor->SetIncrements(grid.angle(), 90.0, 1.0);
        }
    }
}
