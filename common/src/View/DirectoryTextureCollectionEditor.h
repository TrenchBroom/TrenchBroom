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

#include <memory>
#include <vector>

#include <QWidget>

class QListWidget;
class QAbstractButton;

namespace TrenchBroom {
    namespace IO {
        class Path;
    }

    namespace View {
        class MapDocument;

        class DirectoryTextureCollectionEditor : public QWidget {
            Q_OBJECT
        private:
            std::weak_ptr<MapDocument> m_document;

            QListWidget* m_availableCollectionsList;
            QListWidget* m_enabledCollectionsList;

            QAbstractButton* m_addCollectionsButton;
            QAbstractButton* m_removeCollectionsButton;
            QAbstractButton* m_reloadCollectionsButton;
        public:
            explicit DirectoryTextureCollectionEditor(std::weak_ptr<MapDocument> document, QWidget* parent = nullptr);
            ~DirectoryTextureCollectionEditor() override;
        private:
            void addSelectedTextureCollections();
            void removeSelectedTextureCollections();
            void reloadTextureCollections();
            void availableTextureCollectionSelectionChanged();
            void enabledTextureCollectionSelectionChanged();

            bool canAddTextureCollections() const;
            bool canRemoveTextureCollections() const;
            bool canReloadTextureCollections() const;
        private:
            void createGui();
            void updateButtons();

            void bindObservers();
            void unbindObservers();

            void textureCollectionsDidChange();
            void modsDidChange();
            void preferenceDidChange(const IO::Path& path);

            void updateAllTextureCollections();
            void updateAvailableTextureCollections();
            void updateEnabledTextureCollections();
            void updateListBox(QListWidget* box, const std::vector<IO::Path>& paths);

            std::vector<IO::Path> availableTextureCollections() const;
            std::vector<IO::Path> enabledTextureCollections() const;
        };
    }
}

