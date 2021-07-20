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
#include <vector>

#include <QWidget>

class QPushButton;
class QComboBox;
class QLineEdit;
class QScrollBar;

namespace TrenchBroom {
    namespace IO {
        class Path;
    }
    namespace Model {
        class Node;
    }

    namespace View {
        class EntityBrowserView;
        class GLContextManager;
        class MapDocument;

        class EntityBrowser : public QWidget {
            Q_OBJECT
        private:
            std::weak_ptr<MapDocument> m_document;
            QComboBox* m_sortOrderChoice;
            QPushButton* m_groupButton;
            QPushButton* m_usedButton;
            QLineEdit* m_filterBox;
            QScrollBar* m_scrollBar;
            EntityBrowserView* m_view;

            NotifierConnection m_notifierConnection;
        public:
            EntityBrowser(std::weak_ptr<MapDocument> document, GLContextManager& contextManager, QWidget* parent = nullptr);

            void reload();
        private:
            void createGui(GLContextManager& contextManager);

            void connectObservers();

            void documentWasNewed(MapDocument* document);
            void documentWasLoaded(MapDocument* document);

            void modsDidChange();
            void nodesDidChange(const std::vector<Model::Node*>& nodes);
            void entityDefinitionsDidChange();
            void preferenceDidChange(const IO::Path& path);
        };
    }
}

