/*
 Copyright (C) 2010-2012 Kristian Duske
 
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

#ifndef __TrenchBroom__KeyboardShortcutEditor__
#define __TrenchBroom__KeyboardShortcutEditor__

#include <wx/panel.h>

class wxStaticText;

namespace TrenchBroom {
    namespace View {
        class KeyboardShortcutEvent : public wxEvent {
        protected:
            int m_modifierKey1;
            int m_modifierKey2;
            int m_modifierKey3;
            int m_key;
        public:
            KeyboardShortcutEvent();
            KeyboardShortcutEvent(int modifierKey1, int modifierKey2, int modifierKey3, int key);

            inline int modifierKey1() const {
                return m_modifierKey1;
            }
            
            inline int modifierKey2() const {
                return m_modifierKey2;
            }

            inline int modifierKey3() const {
                return m_modifierKey3;
            }

            inline int key() const {
                return m_key;
            }
            
            virtual wxEvent* Clone() const;
            
            DECLARE_DYNAMIC_CLASS(KeyboardShortcutEvent)
        };

        class KeyboardShortcutEditor : public wxPanel {
        private:
            wxStaticText* m_label;
            int m_modifierKey1;
            int m_modifierKey2;
            int m_modifierKey3;
            int m_key;
            
            void update();
        public:
            KeyboardShortcutEditor(wxWindow* parent, wxWindowID windowId = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = 0, const wxString& name = wxPanelNameStr);
            
            void OnSetFocus(wxFocusEvent& event);
            void OnKillFocus(wxFocusEvent& event);
            void OnKeyDown(wxKeyEvent& event);
            void OnKeyUp(wxKeyEvent& event);
            
            DECLARE_EVENT_TABLE()
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

#endif /* defined(__TrenchBroom__KeyboardShortcutEditor__) */
