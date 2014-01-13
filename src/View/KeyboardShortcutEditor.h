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

#ifndef __TrenchBroom__KeyboardShortcutEditor__
#define __TrenchBroom__KeyboardShortcutEditor__

#include <wx/defs.h>
#include <wx/control.h>

class wxStaticText;

namespace TrenchBroom {
    namespace View {
        class KeyboardShortcut;
        
        class KeyboardShortcutEditor : public wxControl {
        private:
            wxStaticText* m_label;
            int m_modifierKey1;
            int m_modifierKey2;
            int m_modifierKey3;
            int m_key;
            bool m_resetOnNextKey;
            
            void update();
        public:
            KeyboardShortcutEditor(wxWindow* parent, wxWindowID windowId = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = 0, const wxValidator& validator = wxDefaultValidator, const wxString& name = wxPanelNameStr);

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
            
            void SetShortcut(const KeyboardShortcut& shortcut);
            void SetShortcut(int key = WXK_NONE, int modifierKey1 = WXK_NONE, int modifierKey2 = WXK_NONE, int modifierKey3 = WXK_NONE);
            
            void OnPaint(wxPaintEvent& event);
            void OnSetFocus(wxFocusEvent& event);
            void OnKillFocus(wxFocusEvent& event);
            void OnKeyDown(wxKeyEvent& event);
            void OnKeyUp(wxKeyEvent& event);
            
            DECLARE_EVENT_TABLE()
        };
    }
}

#endif /* defined(__TrenchBroom__KeyboardShortcutEditor__) */
