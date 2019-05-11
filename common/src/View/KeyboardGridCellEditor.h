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

#ifndef TrenchBroom_KeyboardGridCellEditor
#define TrenchBroom_KeyboardGridCellEditor

#include <wx/grid.h>

namespace TrenchBroom {
    namespace View {
        class KeyboardShortcutEditor;

        class KeyboardGridCellEditor : public wxGridCellEditor {
        private:
            KeyboardShortcutEditor* m_editor;
            wxEvtHandler* m_evtHandler;
        public:
            KeyboardGridCellEditor();
            KeyboardGridCellEditor(wxWindow* parent, wxWindowID windowId, wxEvtHandler* evtHandler, int key, int modifier1, int modifier2, int modifier3);

            void Create(wxWindow* parent, wxWindowID windowId, wxEvtHandler* evtHandler) override;
            wxGridCellEditor* Clone() const override;

            void BeginEdit(int row, int col, wxGrid* grid) override;
            bool EndEdit(int row, int col, const wxGrid* grid, const wxString& oldValue, wxString* newValue) override;
            void ApplyEdit(int row, int col, wxGrid* grid) override;
            void HandleReturn(wxKeyEvent& event) override;

            void Reset() override;
            void Show(bool show, wxGridCellAttr* attr = nullptr) override;

            wxString GetValue() const override;
        };
    }
}

#endif /* defined(TrenchBroom_KeyboardGridCellEditor) */
