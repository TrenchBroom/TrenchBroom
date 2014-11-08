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

#include "MenuShortcutGridTable.h"

#include "View/ActionManager.h"
#include "View/KeyboardGridCellEditor.h"
#include "View/Menu.h"
#include "View/MenuAction.h"

namespace TrenchBroom {
    namespace View {
        MenuShortcutGridTable::Entry::Entry(MenuAction& action) :
        m_action(&action),
        m_conflicts(false) {}
        
        const String MenuShortcutGridTable::Entry::caption() const {
            return m_action->displayName();
        }
        
        const wxString MenuShortcutGridTable::Entry::shortcut() const {
            return m_action->shortcutDisplayString();
        }
        
        bool MenuShortcutGridTable::Entry::modifiable() const {
            return m_action->modifiable();
        }
        
        void MenuShortcutGridTable::Entry::updateShortcut(const KeyboardShortcut& shortcut) {
            m_action->updateShortcut(shortcut);
        }
        
        bool MenuShortcutGridTable::Entry::conflictsWith(const Entry& entry) const {
            return m_action->conflictsWith(*entry.m_action);
        }
        
        bool MenuShortcutGridTable::Entry::conflicts() const {
            return m_conflicts;
        }
        
        void MenuShortcutGridTable::Entry::setConflicts(const bool conflicts) {
            m_conflicts = conflicts;
        }
        
        MenuShortcutGridTable::MenuShortcutGridTable() :
        m_cellEditor(new KeyboardGridCellEditor()) {
            m_cellEditor->IncRef();
        }
        
        MenuShortcutGridTable::~MenuShortcutGridTable() {
            m_cellEditor->DecRef();
        }
        
        int MenuShortcutGridTable::GetNumberRows() {
            return static_cast<int>(m_entries.size());
        }
        
        int MenuShortcutGridTable::GetNumberCols() {
            return 2;
        }
        
        wxString MenuShortcutGridTable::GetValue(int row, int col) {
            assert(row >= 0 && row < GetNumberRows());
            assert(col >= 0 && col < GetNumberCols());
            
            size_t rowIndex = static_cast<size_t>(row);
            
            switch (col) {
                case 0:
                    return m_entries[rowIndex].shortcut();
                case 1:
                    return m_entries[rowIndex].caption();
                default:
                    assert(false);
                    break;
            }
            
            return "";
        }
        
        void MenuShortcutGridTable::SetValue(int row, int col, const wxString& value) {
            assert(row >= 0 && row < GetNumberRows());
            assert(col == 0);
            
            int key, modifier1, modifier2, modifier3;
            const bool success = KeyboardShortcut::parseShortcut(value, key, modifier1, modifier2, modifier3);
            assert(success);
            _UNUSED(success);
            
            const size_t rowIndex = static_cast<size_t>(row);
            m_entries[rowIndex].updateShortcut(KeyboardShortcut(key, modifier1, modifier2, modifier3));
            
            if (markConflicts(m_entries))
                notifyRowsUpdated(m_entries.size());
            else
                notifyRowsUpdated(rowIndex, 1);
        }
        
        void MenuShortcutGridTable::Clear() {
            assert(false);
        }
        
        bool MenuShortcutGridTable::InsertRows(size_t pos, size_t numRows) {
            assert(false);
            return false;
        }
        
        bool MenuShortcutGridTable::AppendRows(size_t numRows) {
            assert(false);
            return false;
        }
        
        bool MenuShortcutGridTable::DeleteRows(size_t pos, size_t numRows) {
            assert(false);
            return false;
        }
        
        wxString MenuShortcutGridTable::GetColLabelValue(int col) {
            assert(col >= 0 && col < GetNumberCols());
            switch (col) {
                case 0:
                    return "Shortcut";
                case 1:
                    return "Menu Item";
                default:
                    assert(false);
                    break;
            }
            
            return "";
        }
        
        wxGridCellAttr* MenuShortcutGridTable::GetAttr(int row, int col, wxGridCellAttr::wxAttrKind kind) {
            wxGridCellAttr* attr = wxGridTableBase::GetAttr(row, col, kind);
            if (row >= 0 && row < GetNumberRows()) {
                const Entry& entry = m_entries[static_cast<size_t>(row)];
                if (entry.conflicts()) {
                    if (attr == NULL)
                        attr = new wxGridCellAttr();
                    attr->SetTextColour(*wxRED);
                }
                if (col == 0) {
                    if (attr == NULL)
                        attr = new wxGridCellAttr();
                    if (entry.modifiable()) {
                        attr->SetEditor(m_cellEditor);
                        m_cellEditor->IncRef();
                    } else {
                        attr->SetReadOnly(true);
                        attr->SetTextColour(*wxLIGHT_GREY);
                    }
                } else if (col == 1) {
                    if (attr == NULL)
                        attr = new wxGridCellAttr();
                    attr->SetReadOnly(true);
                }
            }
            return attr;
        }
        
        bool MenuShortcutGridTable::hasDuplicates() const {
            for (size_t i = 0; i < m_entries.size(); i++)
                if (m_entries[i].conflicts())
                    return true;
            return false;
        }
        
        bool MenuShortcutGridTable::update() {
            EntryList newEntries;
            
            ActionManager& actionManager = ActionManager::instance();
            addMenu(actionManager.getMenu(), newEntries);
            
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
        
        void MenuShortcutGridTable::notifyRowsUpdated(size_t pos, size_t numRows) {
            if (GetView() != NULL) {
                wxGridTableMessage message(this, wxGRIDTABLE_REQUEST_VIEW_GET_VALUES,
                                           static_cast<int>(pos),
                                           static_cast<int>(numRows));
                GetView()->ProcessTableMessage(message);
            }
        }
        
        void MenuShortcutGridTable::notifyRowsInserted(size_t pos, size_t numRows) {
            if (GetView() != NULL) {
                wxGridTableMessage message(this, wxGRIDTABLE_NOTIFY_ROWS_INSERTED,
                                           static_cast<int>(pos),
                                           static_cast<int>(numRows));
                GetView()->ProcessTableMessage(message);
            }
        }
        
        void MenuShortcutGridTable::notifyRowsAppended(size_t numRows) {
            if (GetView() != NULL) {
                wxGridTableMessage message(this, wxGRIDTABLE_NOTIFY_ROWS_APPENDED,
                                           static_cast<int>(numRows));
                GetView()->ProcessTableMessage(message);
            }
        }
        
        void MenuShortcutGridTable::notifyRowsDeleted(size_t pos, size_t numRows) {
            if (GetView() != NULL) {
                wxGridTableMessage message(this, wxGRIDTABLE_NOTIFY_ROWS_DELETED,
                                           static_cast<int>(pos),
                                           static_cast<int>(numRows));
                GetView()->ProcessTableMessage(message);
            }
        }
        
        bool MenuShortcutGridTable::markConflicts(EntryList& entries) {
            for (size_t i = 0; i < entries.size(); i++)
                entries[i].setConflicts(false);
            
            bool hasConflicts = false;
            for (size_t i = 0; i < entries.size(); i++) {
                Entry& first = entries[i];
                for (size_t j = i + 1; j < entries.size(); j++) {
                    Entry& second = entries[j];
                    if (first.conflictsWith(second)) {
                        first.setConflicts(true);
                        second.setConflicts(true);
                        hasConflicts = true;
                    }
                }
            }
            return hasConflicts;
        }
        
        void MenuShortcutGridTable::addMenu(Menu& menu, EntryList& entries) const {
            MenuItem::List& items = menu.items();
            MenuItem::List::iterator itemIt, itemEnd;
            for (itemIt = items.begin(), itemEnd = items.end(); itemIt != itemEnd; ++itemIt) {
                MenuItem& item = **itemIt;
                switch (item.type()) {
                    case MenuItem::Type_Action:
                    case MenuItem::Type_Check: {
                        ActionMenuItem& actionItem = static_cast<ActionMenuItem&>(item);
                        entries.push_back(Entry(actionItem.action()));
                        break;
                    }
                    case MenuItem::Type_Menu: {
                        Menu& subMenu = static_cast<Menu&>(item);
                        addMenu(subMenu, entries);
                        break;
                    }
                    case MenuItem::Type_Separator:
                        break;
                }
            }
        }
    }
}
