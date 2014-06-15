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

#include "KeyboardShortcutEditor.h"

#include "View/KeyboardShortcut.h"
#include "View/KeyboardShortcutEvent.h"

#include <wx/dcclient.h>
#include <wx/renderer.h>
#include <wx/settings.h>
#include <wx/sizer.h>
#include <wx/stattext.h>

namespace TrenchBroom {
    namespace View {
        BEGIN_EVENT_TABLE(KeyboardShortcutEditor, wxControl)
        EVT_PAINT(KeyboardShortcutEditor::OnPaint)
        EVT_SET_FOCUS(KeyboardShortcutEditor::OnSetFocus)
        EVT_KILL_FOCUS(KeyboardShortcutEditor::OnKillFocus)
        EVT_KEY_DOWN(KeyboardShortcutEditor::OnKeyDown)
        EVT_KEY_UP(KeyboardShortcutEditor::OnKeyUp)
        END_EVENT_TABLE()
        
        void KeyboardShortcutEditor::update() {
            if (!KeyboardShortcut::isShortcutValid(m_key, m_modifier1, m_modifier2, m_modifier3)) {
                m_key = WXK_NONE;
                m_modifier1 = WXK_NONE;
                m_modifier2 = WXK_NONE;
                m_modifier3 = WXK_NONE;
            }
            
            KeyboardShortcut::sortModifierKeys(m_modifier1, m_modifier2, m_modifier3);
            wxString label = KeyboardShortcut::shortcutDisplayString(m_key, m_modifier1, m_modifier2, m_modifier3);
            m_label->SetLabel(label);
            Refresh();
        }
        
        KeyboardShortcutEditor::KeyboardShortcutEditor(wxWindow* parent, wxWindowID windowId, const wxPoint& pos, const wxSize& size, long style, const wxValidator& validator, const wxString& name) :
        wxControl(parent, windowId, pos, size, style | wxTAB_TRAVERSAL | wxWANTS_CHARS, validator, name),
        m_label(NULL),
        m_key(WXK_NONE),
        m_modifier1(WXK_NONE),
        m_modifier2(WXK_NONE),
        m_modifier3(WXK_NONE),
        m_resetOnNextKey(false) {
            SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT));
            m_label = new wxStaticText(this, wxID_ANY, "");
            m_label->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHTTEXT));
            wxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
            sizer->Add(m_label, 1, wxEXPAND);
            SetSizer(sizer);
        }

        int KeyboardShortcutEditor::key() const {
            return m_key;
        }
        
        int KeyboardShortcutEditor::modifier1() const {
            return m_modifier1;
        }
        
        int KeyboardShortcutEditor::modifier2() const {
            return m_modifier2;
        }
        
        int KeyboardShortcutEditor::modifier3() const {
            return m_modifier3;
        }
        
        void KeyboardShortcutEditor::SetShortcut(const KeyboardShortcut& shortcut) {
            SetShortcut(shortcut.key(), shortcut.modifier1(), shortcut.modifier2(), shortcut.modifier3());
        }

        void KeyboardShortcutEditor::SetShortcut(const int key, const int modifier1, const int modifier2, const int modifier3) {
            m_key = key;
            m_modifier1 = modifier1;
            m_modifier2 = modifier2;
            m_modifier3 = modifier3;
            m_resetOnNextKey = true;
            update();
        }

        void KeyboardShortcutEditor::OnPaint(wxPaintEvent& event) {
            wxDelegateRendererNative renderer;
            
            wxPaintDC dc(this);
            renderer.DrawFocusRect(this, dc, GetClientRect());
        }

        void KeyboardShortcutEditor::OnSetFocus(wxFocusEvent& event) {
            event.Skip();
        }
        
        void KeyboardShortcutEditor::OnKillFocus(wxFocusEvent& event) {
            event.Skip();
        }

        void KeyboardShortcutEditor::OnKeyDown(wxKeyEvent& event) {
            bool wasReset = false;
            if (m_resetOnNextKey) {
                wasReset = m_key != WXK_NONE || m_modifier1 != WXK_NONE || m_modifier2 != WXK_NONE || m_modifier3 != WXK_NONE;
                SetShortcut();
                m_resetOnNextKey = false;
            }
            
            const int key = event.GetKeyCode();
            switch (key) {
                case WXK_SHIFT:
                case WXK_ALT:
                case WXK_CONTROL:
                    if (m_modifier1 == WXK_NONE)
                        m_modifier1 = key;
                    else if (m_modifier2 == WXK_NONE)
                        m_modifier2 = key;
                    else if (m_modifier3 == WXK_NONE)
                        m_modifier3 = key;
                    break;
#if defined __APPLE__
                case WXK_RAW_CONTROL:
                    // not supported
                    break;
#endif
                case WXK_BACK:
                case WXK_DELETE:
                    if (m_key != WXK_NONE) {
                        SetShortcut();
                        m_resetOnNextKey = false;
                    } else if (!wasReset) {
                        m_key = key;
                    }
                    break;
                default:
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
                        if (m_modifier1 == key)
                            m_modifier1 = WXK_NONE;
                        else if (m_modifier2 == key)
                            m_modifier2 = WXK_NONE;
                        else if (m_modifier3 == key)
                            m_modifier3 = WXK_NONE;
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
                KeyboardShortcutEvent shortcutEvent(m_key, m_modifier1, m_modifier2, m_modifier3);
                shortcutEvent.SetEventType(EVT_KEYBOARD_SHORTCUT_EVENT);
                shortcutEvent.SetEventObject(this);
                shortcutEvent.SetId(GetId());
                ProcessEvent(shortcutEvent);
            }
        }
    }
}
