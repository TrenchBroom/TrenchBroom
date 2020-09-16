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

#ifndef GameEngineProfileEditor_h
#define GameEngineProfileEditor_h

#include <QWidget>

class QLineEdit;
class QStackedWidget;

namespace TrenchBroom {
    namespace Model {
        class GameEngineProfile;
    }

    namespace View {
        /**
         * Editor widget for a single game engine profile.
         */
        class GameEngineProfileEditor : public QWidget {
        private:
            Model::GameEngineProfile* m_profile;
            QStackedWidget* m_stackedWidget;
            QLineEdit* m_nameEdit;
            QLineEdit* m_pathEdit;
            bool m_ignoreNotifications;
        public:
            explicit GameEngineProfileEditor(QWidget* parent = nullptr);
        private:
            QWidget* createEditorPage();
            void updatePath(const QString& str);
        public:
            void setProfile(Model::GameEngineProfile* profile);
        private:
            void profileWillBeRemoved();
            void profileDidChange();
            void refresh();

            bool isValidEnginePath(const QString& str) const;
        private slots:
            void nameChanged(const QString& text);
            void pathChanged();
            void changePathClicked();
        };
    }
}

#endif /* GameEngineProfileEditor_h */
