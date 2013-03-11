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

#include "KeyboardShortcutEditor.h"

#include "View/KeyboardShortcut.h"
#include "View/KeyboardShortcutEvent.h"

#include <wx/defs.h>
#include <wx/sizer.h>
#include <wx/stattext.h>

namespace TrenchBroom {
    namespace View {
        BEGIN_EVENT_TABLE(KeyboardShortcutEditor, wxPanel)
        EVT_SET_FOCUS(KeyboardShortcutEditor::OnSetFocus)
        EVT_KILL_FOCUS(KeyboardShortcutEditor::OnKillFocus)
        EVT_KEY_DOWN(KeyboardShortcutEditor::OnKeyDown)
        EVT_KEY_UP(KeyboardShortcutEditor::OnKeyUp)
        END_EVENT_TABLE()
        
        void KeyboardShortcutEditor::update() {
            KeyboardShortcut::sortModifierKeys(m_modifierKey1, m_modifierKey2, m_modifierKey3);
            wxString label = KeyboardShortcut::shortcutDisplayText(m_modifierKey1, m_modifierKey2, m_modifierKey3, m_key);
            m_label->SetLabel(label);
            Refresh();
        }
        
        KeyboardShortcutEditor::KeyboardShortcutEditor(wxWindow* parent, wxWindowID windowId, const wxPoint& pos, const wxSize& size, long style, const wxString& name) :
        wxPanel(parent, windowId, pos, size, style | wxTAB_TRAVERSAL | wxWANTS_CHARS, name),
        m_label(NULL),
        m_modifierKey1(WXK_NONE),
        m_modifierKey2(WXK_NONE),
        m_modifierKey3(WXK_NONE),
        m_key(WXK_NONE) {
            SetBackgroundColour(*wxWHITE);
            m_label = new wxStaticText(this, wxID_ANY, wxT(""));
            wxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
            sizer->Add(m_label, 1, wxEXPAND);
            SetSizer(sizer);
        }

        void KeyboardShortcutEditor::OnSetFocus(wxFocusEvent& event) {
            event.Skip();
        }
        
        void KeyboardShortcutEditor::OnKillFocus(wxFocusEvent& event) {
            event.Skip();
        }

        void KeyboardShortcutEditor::OnKeyDown(wxKeyEvent& event) {
            const int key = event.GetKeyCode();
            switch (key) {
                case WXK_SHIFT:
                case WXK_ALT:
                case WXK_CONTROL:
                    if (m_modifierKey1 == WXK_NONE)
                        m_modifierKey1 = key;
                    else if (m_modifierKey2 == WXK_NONE)
                        m_modifierKey2 = key;
                    else if (m_modifierKey3 == WXK_NONE)
                        m_modifierKey3 = key;
                    break;
#if defined __APPLE__
                case WXK_RAW_CONTROL:
                    // not supported
                    break;
#endif
                default:
                    if (m_key == WXK_NONE)
                        m_key = key;
                    break;
            }
            update();
        }
        
        void KeyboardShortcutEditor::OnKeyUp(wxKeyEvent& event) {
            if (m_key == WXK_NONE) {
                const int key = event.GetKeyCode();
                switch (key) {
                    case WXK_SHIFT:
                    case WXK_ALT:
                    case WXK_CONTROL:
                        if (m_modifierKey1 == key)
                            m_modifierKey1 = WXK_NONE;
                        else if (m_modifierKey2 == key)
                            m_modifierKey2 = WXK_NONE;
                        else if (m_modifierKey3 == key)
                            m_modifierKey3 = WXK_NONE;
                        break;
#if defined __APPLE__
                    case WXK_RAW_CONTROL:
                        // not supported
                        break;
#endif
                    default:
                        break;
                }
                update();
            } else {
                KeyboardShortcutEvent shortcutEvent(m_modifierKey1, m_modifierKey2, m_modifierKey3, m_key);
                shortcutEvent.SetEventType(EVT_KEYBOARD_SHORTCUT_EVENT);
                shortcutEvent.SetEventObject(this);
                shortcutEvent.SetId(GetId());
                ProcessEvent(shortcutEvent);
            }
        }
    }
}
