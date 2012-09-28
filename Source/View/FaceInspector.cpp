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
#include <wx/spinctrl.h>
#include <wx/splitter.h>
#include <wx/statline.h>
#include <wx/stattext.h>

#include "Controller/SetFaceAttributeCommand.h"
#include "Controller/TextureCollectionCommand.h"
#include "IO/FileManager.h"
#include "Model/Brush.h"
#include "Model/EditStateManager.h"
#include "Model/Face.h"
#include "Model/MapDocument.h"
#include "Model/Texture.h"
#include "View/CommandIds.h"
#include "View/DocumentViewHolder.h"
#include "View/EditorView.h"
#include "View/LayoutConstants.h"
#include "View/SingleTextureViewer.h"
#include "View/TextureBrowser.h"
#include "View/TextureBrowserCanvas.h"

#include "View/SpinControl.h"

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
        EVT_SPINCTRLDOUBLE(CommandIds::FaceInspector::XOffsetEditorId, FaceInspector::OnXOffsetChanged)
        EVT_SPINCTRLDOUBLE(CommandIds::FaceInspector::YOffsetEditorId, FaceInspector::OnYOffsetChanged)
        EVT_SPINCTRLDOUBLE(CommandIds::FaceInspector::XScaleEditorId, FaceInspector::OnXScaleChanged)
        EVT_SPINCTRLDOUBLE(CommandIds::FaceInspector::YScaleEditorId, FaceInspector::OnYScaleChanged)
        EVT_SPINCTRLDOUBLE(CommandIds::FaceInspector::RotationEditorId, FaceInspector::OnRotationChanged)
        EVT_TEXTURE_SELECTED(CommandIds::FaceInspector::TextureBrowserId, FaceInspector::OnTextureSelected)
        EVT_BUTTON(CommandIds::FaceInspector::AddTextureCollectionButtonId, FaceInspector::OnAddTextureCollectionPressed)
        EVT_BUTTON(CommandIds::FaceInspector::RemoveTextureCollectionsButtonId, FaceInspector::OnRemoveTextureCollectionsPressed)
        EVT_UPDATE_UI(CommandIds::FaceInspector::RemoveTextureCollectionsButtonId, FaceInspector::OnUpdateRemoveTextureCollectionsButton)
        END_EVENT_TABLE()

        wxWindow* FaceInspector::createFaceEditor(wxGLContext* sharedContext) {
            wxPanel* faceEditorPanel = new wxPanel(this);
            
            m_textureViewer = new SingleTextureViewer(faceEditorPanel, sharedContext);
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
            m_xOffsetEditor->SetIncrement(1.0);
            m_yOffsetEditor = new SpinControl(faceEditorPanel, CommandIds::FaceInspector::YOffsetEditorId);
            m_yOffsetEditor->SetRange(min, max);
            m_yOffsetEditor->SetIncrement(1.0);
            m_xScaleEditor = new SpinControl(faceEditorPanel, CommandIds::FaceInspector::XScaleEditorId);
            m_xScaleEditor->SetRange(min, max);
            m_xScaleEditor->SetIncrement(0.1);
            m_yScaleEditor = new SpinControl(faceEditorPanel, CommandIds::FaceInspector::YScaleEditorId);
            m_yScaleEditor->SetRange(min, max);
            m_yScaleEditor->SetIncrement(0.1);
            m_rotationEditor = new SpinControl(faceEditorPanel, CommandIds::FaceInspector::RotationEditorId);
            m_rotationEditor->SetRange(min, max);
            m_rotationEditor->SetIncrement(1.0);
            
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
        
        wxWindow* FaceInspector::createTextureBrowser(wxGLContext* sharedContext) {
            wxSplitterWindow* browserSplitter = new wxSplitterWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_3DSASH | wxSP_LIVE_UPDATE);
            browserSplitter->SetSashGravity(1.0f);
            browserSplitter->SetMinimumPaneSize(30);

            m_textureBrowser = new TextureBrowser(browserSplitter, CommandIds::FaceInspector::TextureBrowserId, sharedContext, m_documentViewHolder);
            
            wxPanel* textureCollectionEditor = new wxPanel(browserSplitter);
            m_textureCollectionList = new wxListBox(textureCollectionEditor, CommandIds::FaceInspector::TextureCollectionListId, wxDefaultPosition, wxDefaultSize, 0, NULL, wxLB_MULTIPLE | wxLB_NEEDED_SB);
            m_addTextureCollectionButton = new wxButton(textureCollectionEditor, CommandIds::FaceInspector::AddTextureCollectionButtonId, wxT("+"), wxDefaultPosition, wxDefaultSize, wxBORDER_SUNKEN | wxBU_EXACTFIT);
            m_removeTextureCollectionsButton = new wxButton(textureCollectionEditor, CommandIds::FaceInspector::RemoveTextureCollectionsButtonId, wxT("-"), wxDefaultPosition, wxDefaultSize, wxBORDER_SUNKEN | wxBU_EXACTFIT);

            wxSizer* textureCollectionEditorButtonsSizer = new wxBoxSizer(wxVERTICAL);
            textureCollectionEditorButtonsSizer->Add(m_addTextureCollectionButton, 0, wxEXPAND);
            textureCollectionEditorButtonsSizer->AddSpacer(LayoutConstants::ControlMargin);
            textureCollectionEditorButtonsSizer->Add(m_removeTextureCollectionsButton, 0, wxEXPAND);
            
            wxSizer* textureCollectionEditorSizer = new wxBoxSizer(wxHORIZONTAL);
            textureCollectionEditorSizer->Add(m_textureCollectionList, 1, wxEXPAND);
            textureCollectionEditorSizer->AddSpacer(LayoutConstants::ControlMargin);
            textureCollectionEditorSizer->Add(textureCollectionEditorButtonsSizer);
            textureCollectionEditor->SetSizerAndFit(textureCollectionEditorSizer);

            browserSplitter->SplitHorizontally(m_textureBrowser, textureCollectionEditor);
            return browserSplitter;
        }
        
        FaceInspector::FaceInspector(wxWindow* parent, DocumentViewHolder& documentViewHolder, wxGLContext* sharedContext) :
        wxPanel(parent),
        m_documentViewHolder(documentViewHolder) {
            
            // layout of the contained controls
            wxSizer* innerSizer = new wxBoxSizer(wxVERTICAL);
            innerSizer->Add(createFaceEditor(sharedContext), 0, wxEXPAND);
            innerSizer->AddSpacer(LayoutConstants::DefaultVerticalMargin);
            innerSizer->Add(new wxStaticLine(this), 0, wxEXPAND);
            innerSizer->AddSpacer(LayoutConstants::DefaultVerticalMargin);
            innerSizer->AddSpacer(LayoutConstants::ControlVerticalMargin);
            innerSizer->Add(createTextureBrowser(sharedContext), 1, wxEXPAND | wxBOTTOM, LayoutConstants::NotebookPageExtraBottomMargin);
            
            // creates 5 pixel border inside the page
            wxSizer* outerSizer = new wxBoxSizer(wxVERTICAL);
            outerSizer->Add(innerSizer, 1, wxEXPAND | wxALL, LayoutConstants::NotebookPageInnerMargin);
            SetSizerAndFit(outerSizer);
            
            updateFaceAttributes();
            updateSelectedTexture();
            updateTextureCollectionList();
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

        void FaceInspector::updateTextureBrowser() {
            m_textureBrowser->reload();
        }

        void FaceInspector::updateTextureCollectionList() {
            m_textureCollectionList->Clear();
            
            Model::TextureManager& textureManager = m_documentViewHolder.document().textureManager();
            const Model::TextureCollectionList& collections = textureManager.collections();

            for (unsigned int i = 0; i < collections.size(); i++) {
                Model::TextureCollection* collection = collections[i];
                m_textureCollectionList->Append(collection->name());
            }
        }

        void FaceInspector::OnXOffsetChanged(wxSpinDoubleEvent& event) {
            Model::MapDocument& document = m_documentViewHolder.document();
            Controller::SetFaceAttributeCommand* command = new Controller::SetFaceAttributeCommand(document, "Set X Offset");
            command->setXOffset(static_cast<float>(event.GetValue()));
            document.GetCommandProcessor()->Submit(command);
        }
        
        void FaceInspector::OnYOffsetChanged(wxSpinDoubleEvent& event) {
            Model::MapDocument& document = m_documentViewHolder.document();
            Controller::SetFaceAttributeCommand* command = new Controller::SetFaceAttributeCommand(document, "Set Y Offset");
            command->setYOffset(static_cast<float>(event.GetValue()));
            document.GetCommandProcessor()->Submit(command);
        }
        
        void FaceInspector::OnXScaleChanged(wxSpinDoubleEvent& event) {
            Model::MapDocument& document = m_documentViewHolder.document();
            Controller::SetFaceAttributeCommand* command = new Controller::SetFaceAttributeCommand(document, "Set X Scale");
            command->setXScale(static_cast<float>(event.GetValue()));
            document.GetCommandProcessor()->Submit(command);
        }
        
        void FaceInspector::OnYScaleChanged(wxSpinDoubleEvent& event) {
            Model::MapDocument& document = m_documentViewHolder.document();
            Controller::SetFaceAttributeCommand* command = new Controller::SetFaceAttributeCommand(document, "Set Y Scale");
            command->setYScale(static_cast<float>(event.GetValue()));
            document.GetCommandProcessor()->Submit(command);
        }
        
        void FaceInspector::OnRotationChanged(wxSpinDoubleEvent& event) {
            Model::MapDocument& document = m_documentViewHolder.document();
            Controller::SetFaceAttributeCommand* command = new Controller::SetFaceAttributeCommand(document, "Set Rotation");
            command->setRotation(static_cast<float>(event.GetValue()));
            document.GetCommandProcessor()->Submit(command);
        }

        void FaceInspector::OnTextureSelected(TextureSelectedCommand& event) {
            if (!m_documentViewHolder.valid())
                return;
            
            Model::MapDocument& document = m_documentViewHolder.document();
            Controller::SetFaceAttributeCommand* command = new Controller::SetFaceAttributeCommand(document, "Set Texture");
            command->setTexture(m_textureBrowser->selectedTexture());
            document.GetCommandProcessor()->Submit(command);
        }

        void FaceInspector::OnAddTextureCollectionPressed(wxCommandEvent& event) {
            wxFileDialog addTextureCollectionDialog(NULL, wxT("Choose texture wad"), wxT(""), wxT(""), wxT("*.wad"), wxFD_OPEN | wxFD_FILE_MUST_EXIST);
            if (addTextureCollectionDialog.ShowModal() == wxID_OK) {
                String absWadPath = addTextureCollectionDialog.GetPath().ToStdString();

                Model::MapDocument& document = m_documentViewHolder.document();
                String documentFilename = document.GetFilename().ToStdString();
                if (!documentFilename.empty()) {
                    IO::FileManager fileManager;
                    String relWadPath = fileManager.makeRelative(absWadPath, documentFilename);
                    StringStream message;
                    message << "Would you like to add this texture wad with a path relative to the path of the map file?\n\nAbsolute path: " << absWadPath << "\n\nRelative path: " << relWadPath;
                    wxMessageDialog makeRelativeDialog(NULL, wxString(message.str()), wxT("Add texture wad"), wxYES_NO | wxCANCEL | wxCENTER);
                    makeRelativeDialog.SetYesNoCancelLabels(wxT("Use absolute path"), wxT("Use relative path"), wxT("Cancel"));
                    
                    int result = makeRelativeDialog.ShowModal();
                    if (result == wxID_YES) {
                        Controller::TextureCollectionCommand* command = Controller::TextureCollectionCommand::addTextureWad(document, absWadPath);
                        document.GetCommandProcessor()->Submit(command);
                    } else if (result == wxID_NO) {
                        Controller::TextureCollectionCommand* command = Controller::TextureCollectionCommand::addTextureWad(document, relWadPath);
                        document.GetCommandProcessor()->Submit(command);
                    }
                }
            }
        }
        
        void FaceInspector::OnRemoveTextureCollectionsPressed(wxCommandEvent& event) {
            wxArrayInt selections;
            int selectionCount = m_textureCollectionList->GetSelections(selections);
            if (selectionCount <= 0)
                return;
            
            Controller::TextureCollectionCommand::IndexList indices;
            for (unsigned int i = 0; i < selectionCount; i++)
                indices.push_back(static_cast<size_t>(selections[i]));
            
            Model::MapDocument& document = m_documentViewHolder.document();
            Controller::TextureCollectionCommand* command = Controller::TextureCollectionCommand::removeTextureWads(document, indices);
            document.GetCommandProcessor()->Submit(command);
        }

        void FaceInspector::OnUpdateRemoveTextureCollectionsButton(wxUpdateUIEvent& event) {
            wxArrayInt selections;
            int selectionCount = m_textureCollectionList->GetSelections(selections);
            event.Enable(selectionCount > 0);
        }
    }
}