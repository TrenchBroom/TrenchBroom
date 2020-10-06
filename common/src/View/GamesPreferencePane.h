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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TrenchBroom_GamesPreferencePane
#define TrenchBroom_GamesPreferencePane

#include "View/PreferencePane.h"

class QLineEdit;
class QPushButton;
class QStackedWidget;
class QFormLayout;

namespace TrenchBroom {
    namespace View {
        class GameListBox;

        class GamesPreferencePane : public PreferencePane {
            Q_OBJECT
        private:
            GameListBox* m_gameListBox;
            QStackedWidget* m_stackedWidget;
            QLineEdit* m_gamePathText;
            QPushButton* m_chooseGamePathButton;
            QString m_currentGame;
            QFormLayout* m_compilationToolsLayout;
        public:
            explicit GamesPreferencePane(QWidget* parent = nullptr);
        private:
            void createGui();
            QWidget* createGamePreferencesPage();
        private:
            void currentGameChanged(const QString& gameName);
            void chooseGamePathClicked();
            void updateGamePath(const QString& str);
            void configureEnginesClicked();
        private:
            bool doCanResetToDefaults() override;
            void doResetToDefaults() override;
            void doUpdateControls() override;
            bool doValidate() override;
        };
    }
}

#endif /* defined(TrenchBroom_GamesPreferencePane) */
