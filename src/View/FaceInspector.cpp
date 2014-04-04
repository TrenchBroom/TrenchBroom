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

#include "FaceInspector.h"

#include "Controller/EntityPropertyCommand.h"
#include "Model/Entity.h"
#include "Model/Object.h"
#include "View/ControllerFacade.h"
#include "View/FaceAttribsEditor.h"
#include "View/ViewConstants.h"
#include "View/MapDocument.h"
#include "View/TextureBrowser.h"
#include "View/TextureCollectionEditor.h"
#include "View/TextureSelectedCommand.h"

#include <wx/collpane.h>
#include <wx/notebook.h>
#include <wx/sizer.h>
#include <wx/splitter.h>

namespace TrenchBroom {
    namespace View {
        FaceInspector::FaceInspector(wxWindow* parent, GLContextHolder::Ptr sharedContext, MapDocumentWPtr document, ControllerWPtr controller) :
        wxPanel(parent),
        m_document(document),
        m_controller(controller) {
            createGui(sharedContext);
            bindEvents();
        }
        
        void FaceInspector::OnTextureSelected(TextureSelectedCommand& event) {
            MapDocumentSPtr document = lock(m_document);
            ControllerSPtr controller = lock(m_controller);
            if (!controller->setTexture(document->allSelectedFaces(), event.texture()))
                event.Veto();
        }

        void FaceInspector::OnTextureCollectionEditorPaneChanged(wxCollapsiblePaneEvent& event) {
            Layout();
        }

        void FaceInspector::createGui(GLContextHolder::Ptr sharedContext) {
            wxSplitterWindow* splitter = new wxSplitterWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_LIVE_UPDATE | wxSP_3DSASH);
            wxWindow* faceAttribsEditor = createFaceAttribsEditor(splitter, sharedContext);
            wxWindow* texturePanel = createTexturePanel(splitter, sharedContext);

            splitter->SetSashGravity(0.0f);
            splitter->SetMinimumPaneSize(250);
            splitter->SplitHorizontally(faceAttribsEditor, texturePanel);

            wxSizer* outerSizer = new wxBoxSizer(wxVERTICAL);
            outerSizer->Add(splitter, 1, wxEXPAND);
            SetSizer(outerSizer);
        }
        
        wxWindow* FaceInspector::createTexturePanel(wxWindow* parent, GLContextHolder::Ptr sharedContext) {
            wxPanel* browserPanel = new wxPanel(parent);
            wxWindow* textureBrowser = createTextureBrowser(browserPanel, sharedContext);
            wxWindow* collectionEditor = createTextureCollectionEditor(browserPanel);
            
            wxSizer* browserPanelSizer = new wxBoxSizer(wxVERTICAL);
            browserPanelSizer->Add(textureBrowser, 1, wxEXPAND | wxLEFT | wxRIGHT, LayoutConstants::NotebookPageInnerMargin);
            browserPanelSizer->AddSpacer(LayoutConstants::ControlVerticalMargin);
            browserPanelSizer->Add(collectionEditor, 0, wxEXPAND | wxLEFT | wxBOTTOM | wxRIGHT, LayoutConstants::NotebookPageInnerMargin);
            browserPanel->SetSizer(browserPanelSizer);
            
            return browserPanel;
        }

        wxWindow* FaceInspector::createFaceAttribsEditor(wxWindow* parent, GLContextHolder::Ptr sharedContext) {
            m_faceAttribsEditor = new FaceAttribsEditor(parent, sharedContext, m_document, m_controller);
            return m_faceAttribsEditor;
        }
        
        wxWindow* FaceInspector::createTextureBrowser(wxWindow* parent, GLContextHolder::Ptr sharedContext) {
            m_textureBrowser = new TextureBrowser(parent, sharedContext, m_document);
            return m_textureBrowser;
        }
        
        wxWindow* FaceInspector::createTextureCollectionEditor(wxWindow* parent) {
            wxCollapsiblePane* collPane = new wxCollapsiblePane(parent, wxID_ANY, "Texture Collections", wxDefaultPosition, wxDefaultSize, wxCP_NO_TLW_RESIZE | wxTAB_TRAVERSAL | wxBORDER_NONE);

#if defined _WIN32
            // this is a hack to prevent the pane having the wrong background color on Windows 7
            wxNotebook* book = static_cast<wxNotebook*>(GetParent());
            wxColour col = book->GetThemeBackgroundColour();
            if (col.IsOk()) {
                collPane->SetBackgroundColour(col);
                collPane->GetPane()->SetBackgroundColour(col);
            }
#endif

            m_textureCollectionEditor = new TextureCollectionEditor(collPane->GetPane(), m_document, m_controller);
            
            wxSizer* sizer = new wxBoxSizer(wxVERTICAL);
            sizer->Add(m_textureCollectionEditor, 1, wxEXPAND);
            collPane->GetPane()->SetSizerAndFit(sizer);
            
            collPane->Bind(wxEVT_COLLAPSIBLEPANE_CHANGED, &FaceInspector::OnTextureCollectionEditorPaneChanged, this);
            return collPane;
        }

        void FaceInspector::bindEvents() {
            m_textureBrowser->Bind(EVT_TEXTURE_SELECTED_EVENT,
                                   EVT_TEXTURE_SELECTED_HANDLER(FaceInspector::OnTextureSelected),
                                   this);
        }
    }
}
