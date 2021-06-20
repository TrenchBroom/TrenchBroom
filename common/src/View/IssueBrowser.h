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
#include "View/TabBook.h"

#include <memory>
#include <vector>

class QCheckBox;
class QStackedLayout;
class QWidget;

namespace TrenchBroom {
    namespace Model {
        class BrushFaceHandle;
        class Issue;
        class Node;
    }

    namespace View {
        class FlagsPopupEditor;
        class IssueBrowserView;
        class MapDocument;

        class IssueBrowser : public TabBookPage {
            Q_OBJECT
        private:
            static const int SelectObjectsCommandId = 1;
            static const int ShowIssuesCommandId = 2;
            static const int HideIssuesCommandId = 3;
            static const int FixObjectsBaseId = 4;

            std::weak_ptr<MapDocument> m_document;
            IssueBrowserView* m_view;
            QCheckBox* m_showHiddenIssuesCheckBox;
            FlagsPopupEditor* m_filterEditor;

            NotifierConnection m_notifierConnection;
        public:
            explicit IssueBrowser(std::weak_ptr<MapDocument> document, QWidget* parent = nullptr);

            QWidget* createTabBarPage(QWidget* parent) override;
        private:
            void connectObservers();
            void documentWasNewedOrLoaded(MapDocument* document);
            void documentWasSaved(MapDocument* document);
            void nodesWereAdded(const std::vector<Model::Node*>& nodes);
            void nodesWereRemoved(const std::vector<Model::Node*>& nodes);
            void nodesDidChange(const std::vector<Model::Node*>& nodes);
            void brushFacesDidChange(const std::vector<Model::BrushFaceHandle>& faces);
            void issueIgnoreChanged(Model::Issue* issue);

            void updateFilterFlags();

            void showHiddenIssuesChanged();
            void filterChanged(size_t index, int value, int setFlag, int mixedFlag);
        };
    }
}

