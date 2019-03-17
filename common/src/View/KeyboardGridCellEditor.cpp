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

#include "KeyboardGridCellEditor.h"

#include "View/KeyboardShortcut.h"
#include "View/KeyboardShortcutEditor.h"
#include "View/KeyboardShortcutEvent.h"

#include <wx/msgdlg.h>

namespace TrenchBroom {
    namespace View {
        KeyboardGridCellEditor::KeyboardGridCellEditor() :
        wxGridCellEditor(),
        m_editor(nullptr),
        m_evtHandler(nullptr) {}

        KeyboardGridCellEditor::KeyboardGridCellEditor(wxWindow* parent, wxWindowID windowId, wxEvtHandler* evtHandler, const int key, const int modifier1, const int modifier2, const int modifier3) :
        wxGridCellEditor(),
        m_editor(nullptr),
        m_evtHandler(nullptr) {
            Create(parent, windowId, evtHandler);
            m_editor->SetShortcut(key, modifier1, modifier2, modifier3);
        }

        void KeyboardGridCellEditor::Create(wxWindow* parent, wxWindowID windowId, wxEvtHandler* evtHandler) {
            m_evtHandler = evtHandler;
            m_editor = new KeyboardShortcutEditor(parent, wxID_ANY);
            SetControl(m_editor);
            // wxGridCellEditor::Create(parent, windowId, evtHandler);
        }

        wxGridCellEditor* KeyboardGridCellEditor::Clone() const {
            return new KeyboardGridCellEditor(m_editor->GetParent(), wxID_ANY, m_evtHandler,
                                              m_editor->key(),
                                              m_editor->modifier1(),
                                              m_editor->modifier2(),
                                              m_editor->modifier3());
        }

        void KeyboardGridCellEditor::BeginEdit(int row, int col, wxGrid* grid) {
            int modifier1, modifier2, modifier3, key;
            KeyboardShortcut::parseShortcut(grid->GetCellValue(row, col),
                                            key,
                                            modifier1,
                                            modifier2,
                                            modifier3);
            m_editor->SetShortcut(key, modifier1, modifier2, modifier3);
            m_editor->SetFocus();
        }

        bool KeyboardGridCellEditor::EndEdit(int row, int col, const wxGrid* grid, const wxString& oldValue, wxString* newValue) {
            *newValue = KeyboardShortcut::shortcutDisplayString(m_editor->key(),
                                                                m_editor->modifier1(),
                                                                m_editor->modifier2(),
                                                                m_editor->modifier3());
            if (*newValue == oldValue)
                return false;
            return true;
        }

        void KeyboardGridCellEditor::ApplyEdit(int row, int col, wxGrid* grid) {
            wxString newValue = KeyboardShortcut::shortcutDisplayString(m_editor->key(),
                                                                        m_editor->modifier1(),
                                                                        m_editor->modifier2(),
                                                                        m_editor->modifier3());
            grid->SetCellValue(row, col, newValue);
        }

        void KeyboardGridCellEditor::HandleReturn(wxKeyEvent& event) {
            event.Skip();
        }

        void KeyboardGridCellEditor::Reset() {
            m_editor->SetShortcut();
        }

        void KeyboardGridCellEditor::Show(bool show, wxGridCellAttr* attr) {
            m_editor->Show(show);
        }

        wxString KeyboardGridCellEditor::GetValue() const {
            return KeyboardShortcut::shortcutDisplayString(m_editor->key(),
                                                           m_editor->modifier1(),
                                                           m_editor->modifier2(),
                                                           m_editor->modifier3());
        }
    }
}
