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

#include "Model/Entity.h"
#include "Model/Object.h"
#include "View/BorderLine.h"
#include "View/CollapsibleTitledPanel.h"
#include "View/FaceAttribsEditor.h"
#include "View/ViewConstants.h"
#include "View/MapDocument.h"
#include "View/SplitterWindow2.h"
#include "View/TextureBrowser.h"
#include "View/TextureCollectionEditor.h"
#include "View/TextureSelectedCommand.h"
#include "View/TitledPanel.h"

#include <wx/notebook.h>
#include <wx/persist.h>
#include <wx/sizer.h>

namespace TrenchBroom {
    namespace View {
        FaceInspector::FaceInspector(wxWindow* parent, MapDocumentWPtr document, GLContextManager& contextManager) :
        TabBookPage(parent),
        m_document(document) {
#if defined __APPLE__
            SetWindowVariant(wxWINDOW_VARIANT_SMALL);
#endif
            createGui(document, contextManager);
            bindEvents();
        }
        
        void FaceInspector::OnTextureSelected(TextureSelectedCommand& event) {
            if (IsBeingDeleted()) return;

            MapDocumentSPtr document = lock(m_document);
            if (!document->setTexture(event.texture()))
                event.Veto();
        }

        void FaceInspector::createGui(MapDocumentWPtr document, GLContextManager& contextManager) {
            SplitterWindow2* splitter = new SplitterWindow2(this);
            splitter->setSashGravity(1.0f);
            splitter->SetName("FaceInspectorSplitter");

            splitter->splitHorizontally(createFaceAttribsEditor(splitter, document, contextManager),
                                        createTextureBrowser(splitter, document, contextManager),
                                        wxSize(100, 100), wxSize(100, 100));
            
            wxSizer* outerSizer = new wxBoxSizer(wxVERTICAL);
            outerSizer->Add(splitter, 1, wxEXPAND);
            outerSizer->Add(new BorderLine(this, BorderLine::Direction_Horizontal), 0, wxEXPAND);
            outerSizer->Add(createTextureCollectionEditor(this, document), 0, wxEXPAND);
            SetSizer(outerSizer);

            wxPersistenceManager::Get().RegisterAndRestore(splitter);
        }

        wxWindow* FaceInspector::createFaceAttribsEditor(wxWindow* parent, MapDocumentWPtr document, GLContextManager& contextManager) {
            m_faceAttribsEditor = new FaceAttribsEditor(parent, document, contextManager);
            return m_faceAttribsEditor;
        }
        
        wxWindow* FaceInspector::createTextureBrowser(wxWindow* parent, MapDocumentWPtr document, GLContextManager& contextManager) {
            TitledPanel* panel = new TitledPanel(parent, "Texture Browser");
            m_textureBrowser = new TextureBrowser(panel->getPanel(), document, contextManager);

            wxSizer* sizer = new wxBoxSizer(wxVERTICAL);
            sizer->Add(m_textureBrowser, 1, wxEXPAND);
            panel->getPanel()->SetSizer(sizer);
            
            return panel;
        }
        
        wxWindow* FaceInspector::createTextureCollectionEditor(wxWindow* parent, MapDocumentWPtr document) {
            CollapsibleTitledPanel* panel = new CollapsibleTitledPanel(parent, "Texture Collections", false);
            m_textureCollectionEditor = new TextureCollectionEditor(panel->getPanel(), document);
            
            wxSizer* sizer = new wxBoxSizer(wxVERTICAL);
            sizer->Add(m_textureCollectionEditor, 1, wxEXPAND);
            panel->getPanel()->SetSizer(sizer);
            
            return panel;
        }

        void FaceInspector::bindEvents() {
            m_textureBrowser->Bind(TEXTURE_SELECTED_EVENT, &FaceInspector::OnTextureSelected, this);
        }
    }
}
