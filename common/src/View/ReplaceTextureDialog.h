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

#ifndef TrenchBroom_ReplaceTextureDialog
#define TrenchBroom_ReplaceTextureDialog

#include "Model/ModelTypes.h"
#include "View/ViewTypes.h"

#include <QDialog>

class QPushButton;

namespace TrenchBroom {
    namespace Assets {
        class Texture;
    }

    namespace View {
        class GLContextManager;
        class TextureBrowser;

        class ReplaceTextureDialog : public QDialog {
            Q_OBJECT
        private:
            MapDocumentWPtr m_document;

            TextureBrowser* m_subjectBrowser;
            TextureBrowser* m_replacementBrowser;
            QPushButton* m_replaceButton;
        public:
            ReplaceTextureDialog(MapDocumentWPtr document, GLContextManager& contextManager, QWidget* parent = nullptr);
        private:
            virtual void accept() override;
            Model::BrushFaceList getApplicableFaces() const;
            void createGui(GLContextManager& contextManager);
        private slots:
            void subjectSelected(Assets::Texture* subject);
            void replacementSelected(Assets::Texture* replacement);
            void updateReplaceButton();
        };
    }
}

#endif /* defined(TrenchBroom_ReplaceTextureDialog) */
