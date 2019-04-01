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

#include "Assets/Texture.h"
#include "Model/Entity.h"
#include "Model/Object.h"
#include "View/BorderLine.h"
#include "View/CollapsibleTitledPanel.h"
#include "View/FaceAttribsEditor.h"
#include "View/MapDocument.h"
#include "View/TextureBrowser.h"
#include "View/TextureCollectionEditor.h"
#include "View/TitledPanel.h"
#include "View/ViewConstants.h"

#include <QSplitter>
#include <QVBoxLayout>

namespace TrenchBroom {
    namespace View {
        FaceInspector::FaceInspector(QWidget* parent, MapDocumentWPtr document, GLContextManager& contextManager) :
        TabBookPage(parent),
        m_document(document) {
            createGui(document, contextManager);
            bindEvents();
        }

        bool FaceInspector::cancelMouseDrag() {
            return m_faceAttribsEditor->cancelMouseDrag();
        }

        void FaceInspector::OnTextureSelected(Assets::Texture* texture) {
            MapDocumentSPtr document = lock(m_document);
            document->setTexture(texture);
        }

        void FaceInspector::createGui(MapDocumentWPtr document, GLContextManager& contextManager) {
            auto* splitter = new QSplitter(Qt::Vertical);
//            splitter->setSashGravity(0.0);
//            splitter->SetName("FaceInspectorSplitter");

            splitter->addWidget(createFaceAttribsEditor(splitter, document, contextManager));
            splitter->addWidget(createTextureBrowser(splitter, document, contextManager));

            // FIXME: size limit
            //wxSize(100, 200), wxSize(100, 200));

            auto* outerSizer = new QVBoxLayout();
            outerSizer->setContentsMargins(0, 0, 0, 0);
            outerSizer->setSpacing(0);
            outerSizer->addWidget(splitter, 1);
            outerSizer->addWidget(new BorderLine(nullptr, BorderLine::Direction_Horizontal));
            outerSizer->addWidget(createTextureCollectionEditor(this, document));
            setLayout(outerSizer);

            // FIXME:
            //wxPersistenceManager::Get().RegisterAndRestore(splitter);
        }

        QWidget* FaceInspector::createFaceAttribsEditor(QWidget* parent, MapDocumentWPtr document, GLContextManager& contextManager) {
            m_faceAttribsEditor = new FaceAttribsEditor(parent, document, contextManager);
            return m_faceAttribsEditor;
        }

        QWidget* FaceInspector::createTextureBrowser(QWidget* parent, MapDocumentWPtr document, GLContextManager& contextManager) {
            TitledPanel* panel = new TitledPanel(parent, "Texture Browser");
            m_textureBrowser = new TextureBrowser(panel->getPanel(), document, contextManager);

            auto* sizer = new QVBoxLayout();
            sizer->setContentsMargins(0, 0, 0, 0);
            sizer->setSpacing(0);
            sizer->addWidget(m_textureBrowser, 1);
            panel->getPanel()->setLayout(sizer);

            return panel;
        }

        QWidget* FaceInspector::createTextureCollectionEditor(QWidget* parent, MapDocumentWPtr document) {
            CollapsibleTitledPanel* panel = new CollapsibleTitledPanel(parent, "Texture Collections", false);
            QWidget* collectionEditor = new TextureCollectionEditor(panel->getPanel(), document);

            auto* sizer = new QVBoxLayout();
            sizer->setContentsMargins(0, 0, 0, 0);
            sizer->addWidget(collectionEditor, 1);
            panel->getPanel()->setLayout(sizer);

            return panel;
        }

        void FaceInspector::bindEvents() {
            connect(m_textureBrowser, &TextureBrowser::textureSelected, this, &FaceInspector::OnTextureSelected);
        }
    }
}
