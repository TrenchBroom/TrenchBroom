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

#ifndef __TrenchBroom__KeyboardPreferencePane__
#define __TrenchBroom__KeyboardPreferencePane__

#include "View/KeyboardShortcut.h"

#include <wx/grid.h>
#include <wx/panel.h>

#include <set>
#include <vector>

namespace TrenchBroom {
    namespace View {
        class KeyboardGridTable : public wxGridTableBase {
        private:
            class Entry {
            private:
                const KeyboardShortcut* m_shortcut;
                bool m_duplicate;
                
            public:
                Entry(const KeyboardShortcut& shortcut) :
                m_shortcut(&shortcut),
                m_duplicate(false) {}
                
                inline const KeyboardShortcut& shortcut() const {
                    return *m_shortcut;
                }
                
                inline bool duplicate() const {
                    return m_duplicate;
                }
                
                inline void setDuplicate(bool duplicate) {
                    m_duplicate = duplicate;
                }
                
                inline bool isDuplicateOf(const Entry& entry) const {
                    if (m_shortcut->modifierKey1() != entry.m_shortcut->modifierKey1())
                        return false;
                    if (m_shortcut->modifierKey2() != entry.m_shortcut->modifierKey2())
                        return false;
                    if (m_shortcut->modifierKey3() != entry.m_shortcut->modifierKey3())
                        return false;
                    if (m_shortcut->key() != entry.m_shortcut->key())
                        return false;
                    
                    if ((m_shortcut->context() & entry.m_shortcut->context()) == 0)
                        return false;
                    return true;
                }
            };
        
            typedef std::vector<Entry> EntryList;
        
            EntryList m_entries;

            void notifyRowsUpdated(size_t pos, size_t numRows = 1);
            void notifyRowsInserted(size_t pos = 0, size_t numRows = 1);
            void notifyRowsAppended(size_t numRows = 1);
            void notifyRowsDeleted(size_t pos = 0, size_t numRows = 1);
        public:
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
        
            bool update();
        };

        class KeyboardShortcutEvent;
        
        class KeyboardPreferencePane : public wxPanel {
        private:
            wxGrid* m_grid;
            KeyboardGridTable* m_table;
        public:
            KeyboardPreferencePane(wxWindow* parent);

            void OnKeyboardShortcut(KeyboardShortcutEvent& event);
            void OnGridSize(wxSizeEvent& event);
        };
    }
}

#endif /* defined(__TrenchBroom__KeyboardPreferencePane__) */
