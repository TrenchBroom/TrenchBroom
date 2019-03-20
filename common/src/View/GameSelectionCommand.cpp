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

#include "GameSelectionCommand.h"

wxDEFINE_EVENT(GAME_SELECTION_CHANGE_EVENT, TrenchBroom::View::GameSelectionCommand);
wxDEFINE_EVENT(GAME_SELECTION_DBLCLICK_EVENT, TrenchBroom::View::GameSelectionCommand);

namespace TrenchBroom {
    namespace View {
        wxIMPLEMENT_DYNAMIC_CLASS(GameSelectionCommand, wxCommandEvent)

        GameSelectionCommand::GameSelectionCommand() :
        wxCommandEvent(),
        m_gameName("") {}

        GameSelectionCommand::GameSelectionCommand(wxEventType type, const String& gameName) :
        wxCommandEvent(type),
        m_gameName(gameName) {}

        const String& GameSelectionCommand::gameName() const {
            return m_gameName;
        }

        void GameSelectionCommand::setGameName(const String& gameName) {
            m_gameName = gameName;
        }

        wxEvent* GameSelectionCommand::Clone() const {
            return new GameSelectionCommand(*this);
        }

    }
}
