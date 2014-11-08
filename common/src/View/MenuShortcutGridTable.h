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

#ifndef __TrenchBroom__MenuShortcutGridTable__
#define __TrenchBroom__MenuShortcutGridTable__

#include "StringUtils.h"

#include <wx/grid.h>

#include <vector>

namespace TrenchBroom {
    namespace View {
        class KeyboardGridCellEditor;
        class KeyboardShortcut;
        class Menu;
        class MenuAction;
        
        class MenuShortcutGridTable : public wxGridTableBase {
        private:
            class Entry {
            private:
                MenuAction* m_action;
                bool m_conflicts;
            public:
                Entry(MenuAction& action);
                
                const String caption() const;
                const wxString shortcut() const;
                bool modifiable() const;
                
                void updateShortcut(const KeyboardShortcut& shortcut);
                bool conflictsWith(const Entry& entry) const;
                
                bool conflicts() const;
                void setConflicts(bool conflicts);
            };
            
            typedef std::vector<Entry> EntryList;
            
            EntryList m_entries;
            KeyboardGridCellEditor* m_cellEditor;
        public:
            MenuShortcutGridTable();
            ~MenuShortcutGridTable();
            
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
        private:
            void notifyRowsUpdated(size_t pos, size_t numRows = 1);
            void notifyRowsInserted(size_t pos = 0, size_t numRows = 1);
            void notifyRowsAppended(size_t numRows = 1);
            void notifyRowsDeleted(size_t pos = 0, size_t numRows = 1);
            
            bool markConflicts(EntryList& entries);
            void addMenu(Menu& menu, EntryList& entries) const;
        };
    }
}

#endif /* defined(__TrenchBroom__MenuShortcutGridTable__) */
