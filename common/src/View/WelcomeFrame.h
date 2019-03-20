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

#ifndef TrenchBroom_WelcomeFrame
#define TrenchBroom_WelcomeFrame

#include "IO/Path.h"

#include <QMainWindow>

class QPushButton;

namespace TrenchBroom {
    namespace View {
        class RecentDocumentListBox;
        class RecentDocumentSelectedCommand;

        class WelcomeFrame : public QMainWindow {
            Q_OBJECT
        private:
            // FIXME: add
            //RecentDocumentListBox* m_recentDocumentListBox;
            QPushButton* m_createNewDocumentButton;
            QPushButton* m_openOtherDocumentButton;
        public:
            WelcomeFrame();

            void OnCreateNewDocumentClicked();
            void OnOpenOtherDocumentClicked();
            void OnRecentDocumentSelected(RecentDocumentSelectedCommand& event);
        private:
            void createGui();
            QWidget* createAppPanel();
            void bindEvents();
        };
    }
}

#endif /* defined(TrenchBroom_WelcomeFrame) */
