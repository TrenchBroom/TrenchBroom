/*
 Copyright (C) 2010-2017 Kristian Duske

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
#include "View/MapDocument.h"
#include "View/SplitterWindow2.h"
#include "View/TextureBrowser.h"
#include "View/TextureCollectionEditor.h"
#include "View/TextureSelectedCommand.h"
#include "View/TitledPanel.h"
#include "View/ViewConstants.h"

#include <wx/notebook.h>
#include <wx/persist.h>
#include <wx/sizer.h>

namespace TrenchBroom {
    namespace View {
        FaceInspector::FaceInspector(QWidget* parent, MapDocumentWPtr document, GLContextManager& contextManager) :
        TabBookPage(parent),
        m_document(document) {
#if defined __APPLE__
            SetWindowVariant(wxWINDOW_VARIANT_SMALL);
#endif
            createGui(document, contextManager);
            bindEvents();
        }

        bool FaceInspector::cancelMouseDrag() {
            return m_faceAttribsEditor->cancelMouseDrag();
        }

        void FaceInspector::OnTextureSelected(TextureSelectedCommand& event) {
            if (IsBeingDeleted()) return;

            MapDocumentSPtr document = lock(m_document);
            document->setTexture(event.texture());
        }

        void FaceInspector::createGui(MapDocumentWPtr document, GLContextManager& contextManager) {
            SplitterWindow2* splitter = new SplitterWindow2(this);
            splitter->setSashGravity(0.0);
            splitter->SetName("FaceInspectorSplitter");

            splitter->splitHorizontally(createFaceAttribsEditor(splitter, document, contextManager),
                                        createTextureBrowser(splitter, document, contextManager),
                                        wxSize(100, 200), wxSize(100, 200));

            auto* outerSizer = new QVBoxLayout();
            outerSizer->Add(splitter, 1, wxEXPAND);
            outerSizer->Add(new BorderLine(this, BorderLine::Direction_Horizontal), 0, wxEXPAND);
            outerSizer->Add(createTextureCollectionEditor(this, document), 0, wxEXPAND);
            SetSizer(outerSizer);

            wxPersistenceManager::Get().RegisterAndRestore(splitter);
        }

        QWidget* FaceInspector::createFaceAttribsEditor(QWidget* parent, MapDocumentWPtr document, GLContextManager& contextManager) {
            m_faceAttribsEditor = new FaceAttribsEditor(parent, document, contextManager);
            return m_faceAttribsEditor;
        }

        QWidget* FaceInspector::createTextureBrowser(QWidget* parent, MapDocumentWPtr document, GLContextManager& contextManager) {
            TitledPanel* panel = new TitledPanel(parent, "Texture Browser");
            m_textureBrowser = new TextureBrowser(panel->getPanel(), document, contextManager);

            auto* sizer = new QVBoxLayout();
            sizer->addWidget(m_textureBrowser, 1, wxEXPAND);
            panel->getPanel()->SetSizer(sizer);

            return panel;
        }

        QWidget* FaceInspector::createTextureCollectionEditor(QWidget* parent, MapDocumentWPtr document) {
            CollapsibleTitledPanel* panel = new CollapsibleTitledPanel(parent, "Texture Collections", false);
            QWidget* collectionEditor = new TextureCollectionEditor(panel->getPanel(), document);

            auto* sizer = new QVBoxLayout();
            sizer->addWidget(collectionEditor, 1, wxEXPAND);
            panel->getPanel()->SetSizer(sizer);

            return panel;
        }

        void FaceInspector::bindEvents() {
            m_textureBrowser->Bind(TEXTURE_SELECTED_EVENT, &FaceInspector::OnTextureSelected, this);
        }
    }
}
