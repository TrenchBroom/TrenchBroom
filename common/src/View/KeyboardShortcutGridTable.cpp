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

#include "KeyboardShortcutGridTable.h"

#include "View/ActionManager.h"
#include "View/KeyboardGridCellEditor.h"
#include "View/KeyboardShortcutEntry.h"
#include "View/Menu.h"

namespace TrenchBroom {
    namespace View {
        KeyboardShortcutGridTable::KeyboardShortcutGridTable() :
        m_cellEditor(new KeyboardGridCellEditor()) {
            m_cellEditor->IncRef();
        }
        
        KeyboardShortcutGridTable::~KeyboardShortcutGridTable() {
            m_cellEditor->DecRef();
        }
        
        int KeyboardShortcutGridTable::GetNumberRows() {
            return static_cast<int>(m_entries.size());
        }
        
        int KeyboardShortcutGridTable::GetNumberCols() {
            return 3;
        }
        
        wxString KeyboardShortcutGridTable::GetValue(int row, int col) {
            assert(row >= 0 && row < GetNumberRows());
            assert(col >= 0 && col < GetNumberCols());
            
            size_t rowIndex = static_cast<size_t>(row);
            
            switch (col) {
                case 0:
                    return m_entries[rowIndex]->shortcutDescription();
                case 1:
                    return m_entries[rowIndex]->actionContextDescription();
                case 2:
                    return m_entries[rowIndex]->actionDescription();
                default:
                    assert(false);
                    break;
            }
            
            return "";
        }
        
        void KeyboardShortcutGridTable::SetValue(int row, int col, const wxString& value) {
            assert(row >= 0 && row < GetNumberRows());
            assert(col == 0);
            
            int key, modifier1, modifier2, modifier3;
            const bool success = KeyboardShortcut::parseShortcut(value, key, modifier1, modifier2, modifier3);
            assert(success);
            unused(success);
            
            const size_t rowIndex = static_cast<size_t>(row);
            m_entries[rowIndex]->updateShortcut(KeyboardShortcut(key, modifier1, modifier2, modifier3));
            
            if (markConflicts(m_entries))
                notifyRowsUpdated(m_entries.size());
            else
                notifyRowsUpdated(rowIndex, 1);
        }
        
        void KeyboardShortcutGridTable::Clear() {
            assert(false);
        }
        
        bool KeyboardShortcutGridTable::InsertRows(size_t pos, size_t numRows) {
            assert(false);
            return false;
        }
        
        bool KeyboardShortcutGridTable::AppendRows(size_t numRows) {
            assert(false);
            return false;
        }
        
        bool KeyboardShortcutGridTable::DeleteRows(size_t pos, size_t numRows) {
            assert(false);
            return false;
        }
        
        wxString KeyboardShortcutGridTable::GetColLabelValue(int col) {
            assert(col >= 0 && col < GetNumberCols());
            switch (col) {
                case 0:
                    return "Shortcut";
                case 1:
                    return "Context";
                case 2:
                    return "Description";
                default:
                    assert(false);
                    break;
            }
            
            return "";
        }
        
        wxGridCellAttr* KeyboardShortcutGridTable::GetAttr(int row, int col, wxGridCellAttr::wxAttrKind kind) {
            wxGridCellAttr* attr = wxGridTableBase::GetAttr(row, col, kind);
            if (row >= 0 && row < GetNumberRows()) {
                const KeyboardShortcutEntry* entry = m_entries[static_cast<size_t>(row)];
                if (entry->hasConflicts()) {
                    if (attr == NULL)
                        attr = new wxGridCellAttr();
                    attr->SetTextColour(*wxRED);
                }
                if (col == 0) {
                    if (attr == NULL)
                        attr = new wxGridCellAttr();
                    if (entry->modifiable()) {
                        attr->SetEditor(m_cellEditor);
                        m_cellEditor->IncRef();
                    } else {
                        attr->SetReadOnly(true);
                        attr->SetTextColour(*wxLIGHT_GREY);
                    }
                } else {
                    if (attr == NULL)
                        attr = new wxGridCellAttr();
                    attr->SetReadOnly(true);
                }
            }
            return attr;
        }
        
        bool KeyboardShortcutGridTable::hasDuplicates() const {
            for (size_t i = 0; i < m_entries.size(); i++)
                if (m_entries[i]->hasConflicts())
                    return true;
            return false;
        }
        
        bool KeyboardShortcutGridTable::update() {
            EntryList newEntries;
            
            ActionManager& actionManager = ActionManager::instance();
            actionManager.getShortcutEntries(newEntries);
            
            const bool hasConflicts = markConflicts(newEntries);
            
            size_t oldSize = m_entries.size();
            m_entries = newEntries;
            
            notifyRowsUpdated(0, oldSize);
            if (oldSize < m_entries.size())
                notifyRowsAppended(m_entries.size() - oldSize);
            else if (oldSize > m_entries.size())
                notifyRowsDeleted(oldSize, oldSize - m_entries.size());
            
            return hasConflicts;
        }
        
        void KeyboardShortcutGridTable::notifyRowsUpdated(size_t pos, size_t numRows) {
            if (GetView() != NULL) {
                wxGridTableMessage message(this, wxGRIDTABLE_REQUEST_VIEW_GET_VALUES,
                                           static_cast<int>(pos),
                                           static_cast<int>(numRows));
                GetView()->ProcessTableMessage(message);
            }
        }
        
        void KeyboardShortcutGridTable::notifyRowsInserted(size_t pos, size_t numRows) {
            if (GetView() != NULL) {
                wxGridTableMessage message(this, wxGRIDTABLE_NOTIFY_ROWS_INSERTED,
                                           static_cast<int>(pos),
                                           static_cast<int>(numRows));
                GetView()->ProcessTableMessage(message);
            }
        }
        
        void KeyboardShortcutGridTable::notifyRowsAppended(size_t numRows) {
            if (GetView() != NULL) {
                wxGridTableMessage message(this, wxGRIDTABLE_NOTIFY_ROWS_APPENDED,
                                           static_cast<int>(numRows));
                GetView()->ProcessTableMessage(message);
            }
        }
        
        void KeyboardShortcutGridTable::notifyRowsDeleted(size_t pos, size_t numRows) {
            if (GetView() != NULL) {
                wxGridTableMessage message(this, wxGRIDTABLE_NOTIFY_ROWS_DELETED,
                                           static_cast<int>(pos),
                                           static_cast<int>(numRows));
                GetView()->ProcessTableMessage(message);
            }
        }
        
        bool KeyboardShortcutGridTable::markConflicts(EntryList& entries) {
            for (size_t i = 0; i < entries.size(); i++)
                entries[i]->resetConflicts();
            
            bool hasConflicts = false;
            for (size_t i = 0; i < entries.size(); i++) {
                KeyboardShortcutEntry* first = entries[i];
                for (size_t j = i + 1; j < entries.size(); j++) {
                    KeyboardShortcutEntry* second = entries[j];
                    if (first->updateConflicts(second)) {
                        second->updateConflicts(first);
                        hasConflicts = true;
                    }
                }
            }
            return hasConflicts;
        }
    }
}
