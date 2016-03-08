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

#include <wx/panel.h>
#include <wx/settings.h>
#include <wx/sizer.h>
#include <wx/stattext.h>

namespace TrenchBroom {
    namespace View {
        void KeyboardShortcutEditor::update() {
            if (!KeyboardShortcut::isShortcutValid(m_key, m_modifiers[0], m_modifiers[1], m_modifiers[2])) {
                resetKey();
                resetModifiers();
            }
            
            KeyboardShortcut::sortModifierKeys(m_modifiers[0], m_modifiers[1], m_modifiers[2]);
            wxString label = KeyboardShortcut::shortcutDisplayString(m_key, m_modifiers[0], m_modifiers[1], m_modifiers[2]);
            m_label->SetLabel(label);
            Refresh();
        }
        
        KeyboardShortcutEditor::KeyboardShortcutEditor(wxWindow* parent, wxWindowID windowId, const wxPoint& pos, const wxSize& size, long style, const wxValidator& validator, const wxString& name) :
        wxControl(parent, windowId, pos, size, style | wxTAB_TRAVERSAL | wxWANTS_CHARS, validator, name),
        m_panel(new wxPanel(this)),
        m_label(new wxStaticText(m_panel, wxID_ANY, "")),
        m_resetOnNextKey(false) {
            resetKey();
            resetModifiers();
            
            wxSizer* panelSizer = new wxBoxSizer(wxVERTICAL);
            panelSizer->Add(m_label, 0, wxEXPAND | wxALIGN_CENTRE_VERTICAL);
            m_panel->SetSizer(panelSizer);

            wxSizer* sizer = new wxBoxSizer(wxVERTICAL);
            sizer->Add(m_panel, 1, wxEXPAND);
            SetSizer(sizer);
            
            Bind(wxEVT_PAINT, &KeyboardShortcutEditor::OnPaint, this);
            Bind(wxEVT_SET_FOCUS, &KeyboardShortcutEditor::OnSetFocus, this);
            Bind(wxEVT_KILL_FOCUS, &KeyboardShortcutEditor::OnKillFocus, this);
            Bind(wxEVT_KEY_DOWN, &KeyboardShortcutEditor::OnKeyDown, this);
            Bind(wxEVT_KEY_UP, &KeyboardShortcutEditor::OnKeyUp, this);
            
            Bind(wxEVT_LEFT_DOWN, &KeyboardShortcutEditor::OnMouseDown, this);
            Bind(wxEVT_RIGHT_DOWN, &KeyboardShortcutEditor::OnMouseDown, this);
            Bind(wxEVT_LEFT_DCLICK, &KeyboardShortcutEditor::OnMouseDown, this);
            
            m_panel->Bind(wxEVT_LEFT_DOWN, &KeyboardShortcutEditor::OnMouseDown, this);
            m_panel->Bind(wxEVT_RIGHT_DOWN, &KeyboardShortcutEditor::OnMouseDown, this);
            m_panel->Bind(wxEVT_LEFT_DCLICK, &KeyboardShortcutEditor::OnMouseDown, this);
            
            m_label->Bind(wxEVT_LEFT_DOWN, &KeyboardShortcutEditor::OnMouseDown, this);
            m_label->Bind(wxEVT_RIGHT_DOWN, &KeyboardShortcutEditor::OnMouseDown, this);
            m_label->Bind(wxEVT_LEFT_DCLICK, &KeyboardShortcutEditor::OnMouseDown, this);
        }

        int KeyboardShortcutEditor::key() const {
            return m_key;
        }
        
        int KeyboardShortcutEditor::modifier1() const {
            return m_modifiers[0];
        }
        
        int KeyboardShortcutEditor::modifier2() const {
            return m_modifiers[1];
        }
        
        int KeyboardShortcutEditor::modifier3() const {
            return m_modifiers[2];
        }
        
        void KeyboardShortcutEditor::SetShortcut(const KeyboardShortcut& shortcut) {
            SetShortcut(shortcut.key(), shortcut.modifier1(), shortcut.modifier2(), shortcut.modifier3());
        }

        void KeyboardShortcutEditor::SetShortcut(const int key, const int modifier1, const int modifier2, const int modifier3) {
            m_key = key;
            m_modifiers[0] = modifier1;
            m_modifiers[1] = modifier2;
            m_modifiers[2] = modifier3;
            m_resetOnNextKey = true;
            update();
        }

        void KeyboardShortcutEditor::OnPaint(wxPaintEvent& event) {
            if (IsBeingDeleted()) return;

            /*
            if (HasFocus()) {
                wxDelegateRendererNative renderer;
                
                wxPaintDC dc(this);
                renderer.DrawFocusRect(this, dc, GetClientRect());
            }
             */
        }

        void KeyboardShortcutEditor::OnSetFocus(wxFocusEvent& event) {
            if (IsBeingDeleted()) return;

            m_panel->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT));
            m_label->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHTTEXT));
            Refresh();
            event.Skip();
        }
        
        void KeyboardShortcutEditor::OnKillFocus(wxFocusEvent& event) {
            if (IsBeingDeleted()) return;

            m_panel->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOX));
            m_label->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOXTEXT));
            Refresh();
            event.Skip();
        }

        void KeyboardShortcutEditor::OnKeyDown(wxKeyEvent& event) {
            if (IsBeingDeleted()) return;

            bool wasReset = false;
            if (m_resetOnNextKey) {
                wasReset = m_key != WXK_NONE || m_modifiers[0] != WXK_NONE || m_modifiers[1] != WXK_NONE || m_modifiers[2] != WXK_NONE;
                SetShortcut();
                m_resetOnNextKey = false;
            }
            
            const int key = event.GetKeyCode();
            switch (key) {
                case WXK_SHIFT:
                case WXK_ALT:
                case WXK_CONTROL:
                    updateModifiers(event);
#if defined __APPLE__
                case WXK_RAW_CONTROL:
#endif
                    break;
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
                    KeyboardShortcutEvent shortcutEvent(key, m_modifiers[0], m_modifiers[1], m_modifiers[2]);
                    shortcutEvent.SetEventType(KEYBOARD_SHORTCUT_EVENT);
                    shortcutEvent.SetEventObject(this);
                    shortcutEvent.SetId(GetId());
                    ProcessEvent(shortcutEvent);
                    if (shortcutEvent.IsAllowed())
                        m_key = key;
                    break;
            }
            update();
        }
        
        void KeyboardShortcutEditor::OnKeyUp(wxKeyEvent& event) {
            if (IsBeingDeleted()) return;

            const int key = event.GetKeyCode();
            switch (key) {
                case WXK_SHIFT:
                case WXK_ALT:
                case WXK_CONTROL:
                    if (m_key == WXK_NONE)
                        updateModifiers(event);
#if defined __APPLE__
                case WXK_RAW_CONTROL:
#endif
                    break;
                default:
                    m_resetOnNextKey = true;
                    break;
            }
            update();
        }
        
        void KeyboardShortcutEditor::OnMouseDown(wxMouseEvent& event) {
            if (IsBeingDeleted()) return;

            SetFocus();
        }

        void KeyboardShortcutEditor::updateModifiers(wxKeyEvent& event) {
            resetModifiers();

            size_t count = 0;
            if (event.ShiftDown())
                m_modifiers[count++] = WXK_SHIFT;
            if (event.AltDown())
                m_modifiers[count++] = WXK_ALT;
            if (event.ControlDown())
                m_modifiers[count++] = WXK_CONTROL;
        }

        void KeyboardShortcutEditor::resetKey() {
            m_key = WXK_NONE;
        }

        void KeyboardShortcutEditor::resetModifiers() {
            for (size_t i = 0; i < 3; ++i)
                m_modifiers[i] = WXK_NONE;
        }
    }
}
