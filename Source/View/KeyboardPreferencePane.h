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

#include "Utility/Preferences.h"
#include "View/KeyboardShortcut.h"

#include <wx/grid.h>
#include <wx/panel.h>

#include <set>
#include <vector>

namespace TrenchBroom {
    namespace View {
        class KeyboardShortcutEditor;
        
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
        
        class KeyboardGridTable : public wxGridTableBase {
        private:
            class Entry {
            private:
                const Preferences::Preference<KeyboardShortcut>* m_pref;
                bool m_duplicate;
                
            public:
                Entry(const Preferences::Preference<KeyboardShortcut>& pref) :
                m_pref(&pref),
                m_duplicate(false) {}
                
                inline const Preferences::Preference<KeyboardShortcut>& pref() const {
                    return *m_pref;
                }
                
                inline const KeyboardShortcut& shortcut() const {
                    return m_pref->value();
                }
                
                inline bool duplicate() const {
                    return m_duplicate;
                }
                
                inline void setDuplicate(bool duplicate) {
                    m_duplicate = duplicate;
                }
                
                inline bool isDuplicateOf(const Entry& entry) const {
                    if (shortcut().modifierKey1() != entry.shortcut().modifierKey1())
                        return false;
                    if (shortcut().modifierKey2() != entry.shortcut().modifierKey2())
                        return false;
                    if (shortcut().modifierKey3() != entry.shortcut().modifierKey3())
                        return false;
                    if (shortcut().key() != entry.shortcut().key())
                        return false;
                    
                    if ((shortcut().context() & entry.shortcut().context()) == 0)
                        return false;
                    return true;
                }
            };
        
            typedef std::vector<Entry> EntryList;
        
            EntryList m_entries;
            KeyboardGridCellEditor* m_cellEditor;

            void notifyRowsUpdated(size_t pos, size_t numRows = 1);
            void notifyRowsInserted(size_t pos = 0, size_t numRows = 1);
            void notifyRowsAppended(size_t numRows = 1);
            void notifyRowsDeleted(size_t pos = 0, size_t numRows = 1);
            
            bool markDuplicates(EntryList& entries);
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
        
            bool update();
        };

        class KeyboardShortcutEvent;
        
        class KeyboardPreferencePane : public wxPanel {
        private:
            wxGrid* m_grid;
            KeyboardGridTable* m_table;
        public:
            KeyboardPreferencePane(wxWindow* parent);

            void OnGridSize(wxSizeEvent& event);
        };
    }
}

#endif /* defined(__TrenchBroom__KeyboardPreferencePane__) */
