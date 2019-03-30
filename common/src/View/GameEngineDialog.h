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

#ifndef GameEngineDialog_h
#define GameEngineDialog_h

#include "StringUtils.h"

#include <wx/dialog.h>

class wxSimplebook;
class wxTextCtrl;

namespace TrenchBroom {
    namespace View {
        class GameEngineProfileManager;

        class GameEngineDialog : public wxDialog {
        private:
            const String m_gameName;
            GameEngineProfileManager* m_profileManager;
        public:
            GameEngineDialog(QWidget* parent, const String& gameName);
        private:
            void createGui();

            void OnUpdateCloseButtonUI();
            void OnCloseButtonClicked();
            void OnClose(wxCloseEvent& event);
        };
    }
}

#endif /* GameEngineDialog_h */
