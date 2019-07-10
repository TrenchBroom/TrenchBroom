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

#include "TextureCollectionEditor.h"

#include "Model/Game.h"
#include "View/DirectoryTextureCollectionEditor.h"
#include "View/FileTextureCollectionEditor.h"
#include "View/MapDocument.h"

#include <QVBoxLayout>

namespace TrenchBroom {
    namespace View {
        TextureCollectionEditor::TextureCollectionEditor(QWidget* parent, MapDocumentWPtr document) :
        QWidget(parent),
        m_document(std::move(document)) {
            auto doc = lock(m_document);
            doc->documentWasNewedNotifier.addObserver(this, &TextureCollectionEditor::documentWasNewedOrLoaded);
            doc->documentWasLoadedNotifier.addObserver(this, &TextureCollectionEditor::documentWasNewedOrLoaded);
        }

        TextureCollectionEditor::~TextureCollectionEditor() {
            if (!expired(m_document)) {
                auto document = lock(m_document);
                document->documentWasNewedNotifier.removeObserver(this, &TextureCollectionEditor::documentWasNewedOrLoaded);
                document->documentWasLoadedNotifier.removeObserver(this, &TextureCollectionEditor::documentWasNewedOrLoaded);
            }
        }

        void TextureCollectionEditor::documentWasNewedOrLoaded(MapDocument* document) {
            qDeleteAll(children());
            delete layout();
            createGui();
        }

        void TextureCollectionEditor::createGui() {
            QWidget* collectionEditor = nullptr;

            auto document = lock(m_document);
            const auto type = document->game()->texturePackageType();
            switch (type) {
                case Model::Game::TexturePackageType::File:
                    collectionEditor = new FileTextureCollectionEditor(m_document);
                    break;
                case Model::Game::TexturePackageType::Directory:
                    collectionEditor = new DirectoryTextureCollectionEditor(m_document);
                    break;
            }

            auto* layout = new QVBoxLayout();
            layout->setContentsMargins(0, 0, 0, 0);
            layout->setSpacing(0);
            layout->addWidget(collectionEditor, 1);
            setLayout(layout);
        }
    }
}
