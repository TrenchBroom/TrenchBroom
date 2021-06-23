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

#include "NotifierConnection.h"

#include <memory>

#include <QWidget>

class QListWidget;
class QAbstractButton;
class QDragEnterEvent;
class QDropEvent;

namespace TrenchBroom {
    namespace IO {
        class Path;
    }

    namespace View {
        class MapDocument;

        class FileTextureCollectionEditor : public QWidget {
            Q_OBJECT
        private:
            std::weak_ptr<MapDocument> m_document;

            QListWidget* m_collections;

            QAbstractButton* m_addTextureCollectionsButton;
            QAbstractButton* m_removeTextureCollectionsButton;
            QAbstractButton* m_moveTextureCollectionUpButton;
            QAbstractButton* m_moveTextureCollectionDownButton;
            QAbstractButton* m_reloadTextureCollectionsButton;

            NotifierConnection m_notifierConnection;
        public:
            explicit FileTextureCollectionEditor(std::weak_ptr<MapDocument> document, QWidget* parent = nullptr);

            bool debugUIConsistency() const;
            bool canRemoveTextureCollections() const;
            bool canMoveTextureCollectionsUp() const;
            bool canMoveTextureCollectionsDown() const;
            bool canReloadTextureCollections() const;

            void addTextureCollections();
            void removeSelectedTextureCollections();
            void moveSelectedTextureCollectionsUp();
            void moveSelectedTextureCollectionsDown();
            void reloadTextureCollections();
        private:
            void createGui();
        private slots:
            void updateButtons();
        private:
            void connectObservers();

            void textureCollectionsDidChange();
            void preferenceDidChange(const IO::Path& path);

            void updateControls();
        protected:
            void dragEnterEvent(QDragEnterEvent* event) override;
            void dropEvent(QDropEvent* event) override;
        };
    }
}

