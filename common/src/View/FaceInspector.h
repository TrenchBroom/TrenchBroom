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

#ifndef TrenchBroom_FaceInspector
#define TrenchBroom_FaceInspector

#include "View/TabBook.h"
#include "View/ViewTypes.h"

class QWidget;

namespace TrenchBroom {
    namespace Model {
        class BrushFace;
        class Object;
        class SelectionResult;
    }

    namespace View {
        class FaceAttribsEditor;
        class GLContextManager;
        class TextureBrowser;
        class FileTextureCollectionEditor;
        class TextureSelectedCommand;

        class FaceInspector : public TabBookPage {
        private:
            MapDocumentWPtr m_document;

            FaceAttribsEditor* m_faceAttribsEditor;
            TextureBrowser* m_textureBrowser;
        public:
            FaceInspector(QWidget* parent, MapDocumentWPtr document, GLContextManager& contextManager);

            bool cancelMouseDrag();
        private:
            void OnTextureSelected(TextureSelectedCommand& event);
        private:
            void createGui(MapDocumentWPtr document, GLContextManager& contextManager);
            QWidget* createFaceAttribsEditor(QWidget* parent, MapDocumentWPtr document, GLContextManager& contextManager);
            QWidget* createTextureBrowser(QWidget* parent, MapDocumentWPtr document, GLContextManager& contextManager);
            QWidget* createTextureCollectionEditor(QWidget* parent, MapDocumentWPtr document);

            void bindEvents();
        };
    }
}

#endif /* defined(TrenchBroom_FaceInspector) */
