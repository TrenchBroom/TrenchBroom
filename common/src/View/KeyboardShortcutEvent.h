/*
 Copyright (C) 2010-2014 Kristian Duske
 
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

#ifndef TrenchBroom_KeyboardShortcutEvent
#define TrenchBroom_KeyboardShortcutEvent

#include <wx/event.h>

namespace TrenchBroom {
    namespace View {
        class KeyboardShortcutEvent : public wxNotifyEvent {
        protected:
            int m_key;
            int m_modifier1;
            int m_modifier2;
            int m_modifier3;
        public:
            KeyboardShortcutEvent();
            KeyboardShortcutEvent(int key, int modifier1, int modifier2, int modifier3);
            
            int key() const;
            int modifier1() const;
            int modifier2() const;
            int modifier3() const;
            
            virtual wxEvent* Clone() const;
            
            DECLARE_DYNAMIC_CLASS(KeyboardShortcutEvent)
        };
    }
}

typedef void (wxEvtHandler::*KeyboardShortcutEventFunction)(TrenchBroom::View::KeyboardShortcutEvent&);

wxDECLARE_EVENT(KEYBOARD_SHORTCUT_EVENT, TrenchBroom::View::KeyboardShortcutEvent);
#define KeyboardShortcutHandler(func) wxEVENT_HANDLER_CAST(KeyboardShortcutEventFunction, func)

#endif /* defined(TrenchBroom_KeyboardShortcutEvent) */
