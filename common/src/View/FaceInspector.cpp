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
#include "View/BorderLine.h"
#include "View/CollapsibleTitledPanel.h"
#include "View/ControllerFacade.h"
#include "View/FaceAttribsEditor.h"
#include "View/ViewConstants.h"
#include "View/MapDocument.h"
#include "View/SplitterWindow.h"
#include "View/TextureBrowser.h"
#include "View/TextureCollectionEditor.h"
#include "View/TextureSelectedCommand.h"
#include "View/TitledPanel.h"

#include <wx/notebook.h>
#include <wx/sizer.h>

namespace TrenchBroom {
    namespace View {
        FaceInspector::FaceInspector(wxWindow* parent, GLContextHolder::Ptr sharedContext, MapDocumentWPtr document, ControllerWPtr controller) :
        TabBookPage(parent),
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

        void FaceInspector::createGui(GLContextHolder::Ptr sharedContext) {
            SplitterWindow* splitter = new SplitterWindow(this);
            splitter->setSashGravity(1.0f);
            splitter->splitHorizontally(createFaceAttribsEditor(splitter, sharedContext),
                                        createTextureBrowser(splitter, sharedContext),
                                        wxSize(100, 250), wxSize(100, 250));
            
            wxSizer* outerSizer = new wxBoxSizer(wxVERTICAL);
            outerSizer->Add(splitter, 1, wxEXPAND);
            outerSizer->Add(new BorderLine(this, BorderLine::Direction_Horizontal), 0, wxEXPAND);
            outerSizer->Add(createTextureCollectionEditor(this), 0, wxEXPAND);
            SetSizer(outerSizer);
        }

        wxWindow* FaceInspector::createFaceAttribsEditor(wxWindow* parent, GLContextHolder::Ptr sharedContext) {
            m_faceAttribsEditor = new FaceAttribsEditor(parent, sharedContext, m_document, m_controller);
            return m_faceAttribsEditor;
        }
        
        wxWindow* FaceInspector::createTextureBrowser(wxWindow* parent, GLContextHolder::Ptr sharedContext) {
            TitledPanel* panel = new TitledPanel(parent, "Texture Browser");
            m_textureBrowser = new TextureBrowser(panel->getPanel(), sharedContext, m_document);

            wxSizer* sizer = new wxBoxSizer(wxVERTICAL);
            sizer->Add(m_textureBrowser, 1, wxEXPAND);
            panel->getPanel()->SetSizer(sizer);
            
            return panel;
        }
        
        wxWindow* FaceInspector::createTextureCollectionEditor(wxWindow* parent) {
            CollapsibleTitledPanel* panel = new CollapsibleTitledPanel(parent, "Texture Collections", false);
            m_textureCollectionEditor = new TextureCollectionEditor(panel->getPanel(), m_document, m_controller);
            
            wxSizer* sizer = new wxBoxSizer(wxVERTICAL);
            sizer->Add(m_textureCollectionEditor, 1, wxEXPAND);
            panel->getPanel()->SetSizer(sizer);
            
            return panel;
        }

        void FaceInspector::bindEvents() {
            m_textureBrowser->Bind(EVT_TEXTURE_SELECTED_EVENT,
                                   EVT_TEXTURE_SELECTED_HANDLER(FaceInspector::OnTextureSelected),
                                   this);
        }
    }
}
