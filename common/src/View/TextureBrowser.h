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

#ifndef TrenchBroom_TextureBrowser
#define TrenchBroom_TextureBrowser

#include "StringUtils.h"
#include "Assets/TextureManager.h"
#include "View/TextureBrowserView.h"
#include "View/ViewTypes.h"

#include <QWidget>

class QPushButton;
class QComboBox;
class QLineEdit;
class QScrollBar;

namespace TrenchBroom {
    namespace Assets {
        class Texture;
    }

    namespace IO {
        class Path;
    }

    namespace View {
        class GLContextManager;
        class TextureBrowserView;

        class TextureBrowser : public QWidget {
            Q_OBJECT
        private:
            MapDocumentWPtr m_document;
            QComboBox* m_sortOrderChoice;
            QPushButton* m_groupButton;
            QPushButton* m_usedButton;
            QLineEdit* m_filterBox;
            QScrollBar* m_scrollBar;
            TextureBrowserView* m_view;
            QWidget* m_windowContainer;
        public:
            TextureBrowser(QWidget* parent, MapDocumentWPtr document, GLContextManager& contextManager);
            ~TextureBrowser() override;

            Assets::Texture* selectedTexture() const;
            void setSelectedTexture(Assets::Texture* selectedTexture);

            void setSortOrder(TextureBrowserView::SortOrder sortOrder);
            void setGroup(bool group);
            void setHideUnused(bool hideUnused);
            void setFilterText(const String& filterText);
        signals:
            void textureSelected(Assets::Texture* texture);
        private:
            void createGui(GLContextManager& contextManager);
            void bindEvents();

            void bindObservers();
            void unbindObservers();

            void documentWasNewed(MapDocument* document);
            void documentWasLoaded(MapDocument* document);
            void nodesWereAdded(const Model::NodeList& nodes);
            void nodesWereRemoved(const Model::NodeList& nodes);
            void nodesDidChange(const Model::NodeList& nodes);
            void brushFacesDidChange(const Model::BrushFaceList& faces);
            void textureCollectionsDidChange();
            void currentTextureNameDidChange(const String& textureName);
            void preferenceDidChange(const IO::Path& path);

            void reload();
            void updateSelectedTexture();
        };
    }
}

#endif /* defined(TrenchBroom_TextureBrowser) */
