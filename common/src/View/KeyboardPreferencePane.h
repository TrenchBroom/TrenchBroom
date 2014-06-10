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

#ifndef __TrenchBroom__KeyboardPreferencePane__
#define __TrenchBroom__KeyboardPreferencePane__

#include "Preference.h"
#include "SharedPointer.h"
#include "View/KeyboardShortcut.h"
#include "View/PreferencePane.h"

#include <wx/grid.h>
#include <wx/panel.h>

#include <set>
#include <vector>

class wxStaticBox;

namespace TrenchBroom {
    namespace View {
        class Menu;
        class KeyboardShortcutEditor;
        class KeyboardShortcutEvent;
        class ShortcutMenuItem;

        class KeyboardGridCellEditor : public wxGridCellEditor {
        private:
            KeyboardShortcutEditor* m_editor;
            wxEvtHandler* m_evtHandler;
        public:
            KeyboardGridCellEditor();
            KeyboardGridCellEditor(wxWindow* parent, wxWindowID windowId, wxEvtHandler* evtHandler, int modifierKey1, int modifierKey2, int modifierKey3, int key);

            void Create(wxWindow* parent, wxWindowID windowId, wxEvtHandler* evtHandler);
            wxGridCellEditor* Clone() const;

            void BeginEdit(int row, int col, wxGrid* grid);
            bool EndEdit(int row, int col, const wxGrid* grid, const wxString& oldValue, wxString* newValue);
            void ApplyEdit(int row, int col, wxGrid* grid);
            void HandleReturn(wxKeyEvent& event);

            void Reset();
            void Show(bool show, wxGridCellAttr* attr = NULL);

            wxString GetValue() const;
        };

        class KeyboardShortcutEntry {
        public:
            typedef std::tr1::shared_ptr<KeyboardShortcutEntry> Ptr;
        private:
            bool m_duplicate;
        public:
            KeyboardShortcutEntry();
            
            virtual ~KeyboardShortcutEntry() {}
            
            virtual const String caption() const = 0;
            virtual const KeyboardShortcut& shortcut() const = 0;
            virtual void saveShortcut(const KeyboardShortcut& shortcut) const = 0;
            bool isDuplicateOf(const KeyboardShortcutEntry& entry) const;
            
            inline bool duplicate() const {
                return m_duplicate;
            }
            
            inline void setDuplicate(bool duplicate) {
                m_duplicate = duplicate;
            }
        };
        
        class MenuKeyboardShortcutEntry : public KeyboardShortcutEntry {
        private:
            const ShortcutMenuItem& m_item;
        public:
            MenuKeyboardShortcutEntry(const ShortcutMenuItem& item);
            
            const String caption() const;
            const KeyboardShortcut& shortcut() const;
            void saveShortcut(const KeyboardShortcut& shortcut) const;
        };
        
        class SimpleKeyboardShortcutEntry : public KeyboardShortcutEntry {
        private:
            Preference<KeyboardShortcut>& m_preference;
        public:
            SimpleKeyboardShortcutEntry(Preference<KeyboardShortcut>& preference);
            
            const String caption() const;
            inline const KeyboardShortcut& shortcut() const;
            inline void saveShortcut(const KeyboardShortcut& shortcut) const;
        };

        class KeyboardGridTable : public wxGridTableBase {
        private:
            typedef std::vector<KeyboardShortcutEntry::Ptr> EntryList;

            EntryList m_entries;
            KeyboardGridCellEditor* m_cellEditor;

            void notifyRowsUpdated(size_t pos, size_t numRows = 1);
            void notifyRowsInserted(size_t pos = 0, size_t numRows = 1);
            void notifyRowsAppended(size_t numRows = 1);
            void notifyRowsDeleted(size_t pos = 0, size_t numRows = 1);

            bool markDuplicates(EntryList& entries);
            void addMenu(const Menu& menu, EntryList& entries) const;
            void addShortcut(Preference<KeyboardShortcut>& shortcut, EntryList& entries) const;
        public:
            KeyboardGridTable();
            ~KeyboardGridTable();

            int GetNumberRows();
            int GetNumberCols();

            wxString GetValue(int row, int col);
            void SetValue(int row, int col, const wxString& value);

            void Clear();
            bool InsertRows(size_t pos = 0, size_t numRows = 1);
            bool AppendRows(size_t numRows = 1);
            bool DeleteRows(size_t pos = 0, size_t numRows = 1);

            wxString GetColLabelValue(int col);
            wxGridCellAttr* GetAttr(int row, int col, wxGridCellAttr::wxAttrKind kind);

            bool hasDuplicates() const;
            bool update();
        };


        class KeyboardPreferencePane : public PreferencePane {
        private:
            wxGrid* m_grid;
            KeyboardGridTable* m_table;
            
        public:
            KeyboardPreferencePane(wxWindow* parent);
            void OnGridSize(wxSizeEvent& event);
        private:
            wxWindow* createMenuShortcutGrid();
            
            void doUpdateControls();
            bool doValidate();
        };
    }
}

#endif /* defined(__TrenchBroom__KeyboardPreferencePane__) */
