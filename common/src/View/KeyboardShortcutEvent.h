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

#ifndef __TrenchBroom__KeyboardShortcutEvent__
#define __TrenchBroom__KeyboardShortcutEvent__

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


#define WXDLLIMPEXP_CUSTOM_EVENT

BEGIN_DECLARE_EVENT_TYPES()
DECLARE_EXPORTED_EVENT_TYPE(WXDLLIMPEXP_CUSTOM_EVENT, EVT_KEYBOARD_SHORTCUT_EVENT, 1)
END_DECLARE_EVENT_TYPES()

typedef void (wxEvtHandler::*keyboardShortcutEventFunction)(TrenchBroom::View::KeyboardShortcutEvent&);

#define EVT_KEYBOARD_SHORTCUT_HANDLER(func) \
(wxObjectEventFunction) \
(keyboardShortcutEventFunction) & func

#define EVT_KEYBOARD_SHORTCUT(id,func) \
DECLARE_EVENT_TABLE_ENTRY( EVT_KEYBOARD_SHORTCUT_EVENT, \
id, \
wxID_ANY, \
(wxObjectEventFunction) \
(keyboardShortcutEventFunction) & func, \
(wxObject *) NULL),

#endif /* defined(__TrenchBroom__KeyboardShortcutEvent__) */
