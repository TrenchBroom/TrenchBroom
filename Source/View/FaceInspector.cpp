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

#include <wx/gbsizer.h>
#include <wx/sizer.h>
#include <wx/spinctrl.h>
#include <wx/statline.h>
#include <wx/stattext.h>

#include "Model/Brush.h"
#include "Model/Face.h"
#include "Model/MapDocument.h"
#include "Model/Texture.h"
#include "View/DocumentViewHolder.h"
#include "View/EditorView.h"
#include "View/LayoutConstants.h"
#include "View/SingleTextureViewer.h"
#include "View/TextureBrowser.h"

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
            
            m_xOffsetEditor = new wxSpinCtrlDouble(faceEditorPanel);
            m_xOffsetEditor->SetRange(min, max);
            m_xOffsetEditor->SetIncrement(1.0);
            m_yOffsetEditor = new wxSpinCtrlDouble(faceEditorPanel);
            m_yOffsetEditor->SetRange(min, max);
            m_yOffsetEditor->SetIncrement(1.0);
            m_xScaleEditor = new wxSpinCtrlDouble(faceEditorPanel);
            m_xScaleEditor->SetRange(min, max);
            m_xScaleEditor->SetIncrement(0.1);
            m_yScaleEditor = new wxSpinCtrlDouble(faceEditorPanel);
            m_yScaleEditor->SetRange(min, max);
            m_yScaleEditor->SetIncrement(0.1);
            m_rotationEditor = new wxSpinCtrlDouble(faceEditorPanel);
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
            wxPanel* textureBrowserPanel = new wxPanel(this);

            Utility::Console& console = m_documentViewHolder.view().console();
            Model::TextureManager& textureManager = m_documentViewHolder.document().textureManager();
            m_textureBrowser = new TextureBrowser(textureBrowserPanel, sharedContext, console, textureManager);
            
            wxSizer* textureBrowserSizer = new wxBoxSizer(wxVERTICAL);
            textureBrowserSizer->Add(m_textureBrowser, 1, wxEXPAND);
            
            textureBrowserPanel->SetSizerAndFit(textureBrowserSizer);
            return textureBrowserPanel;
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
            
            update(Model::EmptyFaceList);
        }

        void FaceInspector::update(const Model::FaceList& faces) {
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

                if (xOffsetMulti)
                    m_xOffsetEditor->SetValue(wxT("multi"));
                else
                    m_xOffsetEditor->SetValue(xOffset);
                if (yOffsetMulti)
                    m_yOffsetEditor->SetValue(wxT("multi"));
                else
                    m_yOffsetEditor->SetValue(yOffset);
                if (xScaleMulti)
                    m_xScaleEditor->SetValue(wxT("multi"));
                else
                    m_xScaleEditor->SetValue(xScale);
                if (yScaleMulti)
                    m_yScaleEditor->SetValue(wxT("multi"));
                else
                    m_yScaleEditor->SetValue(yScale);
                if (rotationMulti)
                    m_rotationEditor->SetValue(wxT("multi"));
                else
                    m_rotationEditor->SetValue(rotation);
                if (textureMulti) {
                    m_textureViewer->setTexture(NULL);
                    m_textureNameLabel->SetLabel(wxT("multi"));
                } else {
                    m_textureViewer->setTexture(texture);
                    m_textureNameLabel->SetLabel(texture->name());
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
        
        void FaceInspector::update(const Model::BrushList& brushes) {
            Model::FaceList faces;
            for (unsigned int i = 0; i < brushes.size(); i++) {
                Model::Brush* brush = brushes[i];
                faces.insert(faces.end(), brush->faces().begin(), brush->faces().end());
            }
            update(faces);
        }

        void FaceInspector::updateSelectedTexture(Model::Texture* texture) {
            m_textureBrowser->setSelectedTexture(texture);
        }

        void FaceInspector::updateTextureBrowser() {
            m_textureBrowser->reload();
        }
    }
}