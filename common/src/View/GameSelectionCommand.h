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

#ifndef TrenchBroom_GameSelectionCommand
#define TrenchBroom_GameSelectionCommand

#include "StringUtils.h"

#include <wx/event.h>

namespace TrenchBroom {
    namespace View {
        class GameSelectionCommand : public wxCommandEvent {
        protected:
            String m_gameName;
        public:
            GameSelectionCommand();
            GameSelectionCommand(wxEventType type, const String& gameName = "");
            
            const String& gameName() const;
            void setGameName(const String& gameName);
            
            virtual wxEvent* Clone() const override;
            
            wxDECLARE_DYNAMIC_CLASS(GameSelectionCommand);
        };
    }
}

using GameSelectionCommandFunction = void(wxEvtHandler::*)(TrenchBroom::View::GameSelectionCommand&);

wxDECLARE_EVENT(GAME_SELECTION_CHANGE_EVENT, TrenchBroom::View::GameSelectionCommand);
#define GameSelectionChangeHandler(func) wxEVENT_HANDLER_CAST(GameSelectionCommandFunction, func)

wxDECLARE_EVENT(GAME_SELECTION_DBLCLICK_EVENT, TrenchBroom::View::GameSelectionCommand);
#define GameSelectionDblClickHandler(func) wxEVENT_HANDLER_CAST(GameSelectionCommandFunction, func)

#endif /* defined(TrenchBroom_GameSelectionCommand) */
