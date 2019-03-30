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

#ifndef GameEngineProfileManager_h
#define GameEngineProfileManager_h

#include <wx/panel.h>

namespace TrenchBroom {
    namespace Model {
        class GameEngineConfig;
    }

    namespace View {
        class GameEngineProfileEditor;
        class GameEngineProfileListBox;

        class GameEngineProfileManager : public QWidget {
        private:
            Model::GameEngineConfig& m_config;
            GameEngineProfileListBox* m_profileList;
            GameEngineProfileEditor* m_profileEditor;
        public:
            GameEngineProfileManager(QWidget* parent, Model::GameEngineConfig& config);
        private:
            void OnAddProfile();
            void OnRemoveProfile();
            void OnUpdateAddProfileButtonUI();
            void OnUpdateRemoveProfileButtonUI();

            void OnProfileSelectionChanged();
        };
    }
}

#endif /* GameEngineProfileManager_h */
