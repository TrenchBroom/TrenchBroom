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

#pragma once

#include "View/TabBook.h"

#include <memory>

class QSplitter;
class QWidget;

namespace TrenchBroom {
    namespace Assets {
        class Texture;
    }

    namespace View {
        class FaceAttribsEditor;
        class GLContextManager;
        class MapDocument;
        class TextureBrowser;

        class FaceInspector : public TabBookPage {
            Q_OBJECT
        private:
            std::weak_ptr<MapDocument> m_document;
            QSplitter* m_splitter;
            FaceAttribsEditor* m_faceAttribsEditor;
            TextureBrowser* m_textureBrowser;
        public:
            FaceInspector(std::weak_ptr<MapDocument> document, GLContextManager& contextManager, QWidget* parent = nullptr);
            ~FaceInspector() override;

            bool cancelMouseDrag();
            void revealTexture(const Assets::Texture* texture);
        private:
            void createGui(std::weak_ptr<MapDocument> document, GLContextManager& contextManager);
            QWidget* createFaceAttribsEditor(QWidget* parent, std::weak_ptr<MapDocument> document, GLContextManager& contextManager);
            QWidget* createTextureBrowser(QWidget* parent, std::weak_ptr<MapDocument> document, GLContextManager& contextManager);
            QWidget* createTextureCollectionEditor(QWidget* parent, std::weak_ptr<MapDocument> document);

            void textureSelected(const Assets::Texture* texture);
        };
    }
}


