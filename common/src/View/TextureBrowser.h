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

#include <memory>
#include <string>
#include <vector>

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

    namespace Model {
        class BrushFaceHandle;
        class Node;
    }

    namespace View {
        class GLContextManager;
        class MapDocument;
        class TextureBrowserView;
        enum class TextureSortOrder;

        class TextureBrowser : public QWidget {
            Q_OBJECT
        private:
            std::weak_ptr<MapDocument> m_document;
            QComboBox* m_sortOrderChoice;
            QPushButton* m_groupButton;
            QPushButton* m_usedButton;
            QLineEdit* m_filterBox;
            QScrollBar* m_scrollBar;
            TextureBrowserView* m_view;
        public:
            TextureBrowser(std::weak_ptr<MapDocument> document, GLContextManager& contextManager, QWidget* parent = nullptr);
            ~TextureBrowser() override;

            Assets::Texture* selectedTexture() const;
            void setSelectedTexture(Assets::Texture* selectedTexture);
            void revealTexture(Assets::Texture* texture);

            void setSortOrder(TextureSortOrder sortOrder);
            void setGroup(bool group);
            void setHideUnused(bool hideUnused);
            void setFilterText(const std::string& filterText);
        signals:
            void textureSelected(Assets::Texture* texture);
        private:
            void createGui(GLContextManager& contextManager);
            void bindEvents();

            void bindObservers();
            void unbindObservers();

            void documentWasNewed(MapDocument* document);
            void documentWasLoaded(MapDocument* document);
            void nodesWereAdded(const std::vector<Model::Node*>& nodes);
            void nodesWereRemoved(const std::vector<Model::Node*>& nodes);
            void nodesDidChange(const std::vector<Model::Node*>& nodes);
            void brushFacesDidChange(const std::vector<Model::BrushFaceHandle>& faces);
            void textureCollectionsDidChange();
            void currentTextureNameDidChange(const std::string& textureName);
            void preferenceDidChange(const IO::Path& path);

            void reload();
            void updateSelectedTexture();
        };
    }
}

#endif /* defined(TrenchBroom_TextureBrowser) */
