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

#include "KeyboardShortcutGridTable.h"

#include "View/ActionManager.h"
#include "View/KeyboardGridCellEditor.h"
#include "View/KeyboardShortcutEntry.h"
#include "View/Menu.h"

#include <set>

namespace TrenchBroom {
    namespace View {
        KeyboardShortcutGridTable::KeyboardShortcutGridTable(EntryList entries) :
        m_entries(std::move(entries)),
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

        QString KeyboardShortcutGridTable::GetValue(const int row, const int col) {
            assert(row >= 0 && row < GetNumberRows());
            assert(col >= 0 && col < GetNumberCols());

            const size_t rowIndex = static_cast<size_t>(row);

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

        void KeyboardShortcutGridTable::SetValue(const int row, const int col, const QString& value) {
            assert(row >= 0 && row < GetNumberRows());
            assert(col == 0);

            int key, modifier1, modifier2, modifier3;
            const bool success = KeyboardShortcut::parseShortcut(value, key, modifier1, modifier2, modifier3);
            assert(success);
            unused(success);

            const size_t rowIndex = static_cast<size_t>(row);
            m_entries[rowIndex]->updateShortcut(KeyboardShortcut(key, modifier1, modifier2, modifier3));

            if (markConflicts()) {
                notifyRowsUpdated(m_entries.size());
            } else {
                notifyRowsUpdated(rowIndex, 1);
        }
        }

        void KeyboardShortcutGridTable::Clear() {
            assert(false);
        }

        bool KeyboardShortcutGridTable::InsertRows(const size_t pos, const size_t numRows) {
            assert(false);
            return false;
        }

        bool KeyboardShortcutGridTable::AppendRows(const size_t numRows) {
            assert(false);
            return false;
        }

        bool KeyboardShortcutGridTable::DeleteRows(const size_t pos, const size_t numRows) {
            assert(false);
            return false;
        }

        QString KeyboardShortcutGridTable::GetColLabelValue(const int col) {
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

        wxGridCellAttr* KeyboardShortcutGridTable::GetAttr(const int row, const int col, const wxGridCellAttr::wxAttrKind kind) {
            wxGridCellAttr* attr = wxGridTableBase::GetAttr(row, col, kind);
            if (row >= 0 && row < GetNumberRows()) {
                const auto& entry = m_entries[static_cast<size_t>(row)];
                if (entry->hasConflicts()) {
                    if (attr == nullptr) {
                        attr = new wxGridCellAttr();
                    }
                    attr->SetTextColour(*wxRED);
                }
                if (col == 0) {
                    if (attr == nullptr) {
                        attr = new wxGridCellAttr();
                    }
                    if (entry->modifiable()) {
                        attr->SetEditor(m_cellEditor);
                        m_cellEditor->IncRef();
                    } else {
                        attr->SetReadOnly(true);
                        attr->SetTextColour(*wxLIGHT_GREY);
                    }
                } else {
                    if (attr == nullptr) {
                        attr = new wxGridCellAttr();
                    }
                    attr->SetReadOnly(true);
                }
            }
            return attr;
        }

        bool KeyboardShortcutGridTable::hasDuplicates() const {
            for (const auto& entry : m_entries) {
                if (entry->hasConflicts()) {
                    return true;
                }
            }
            return false;
        }

        void KeyboardShortcutGridTable::update() {
            if (markConflicts()) {
                notifyRowsUpdated(m_entries.size());
            }
        }

        void KeyboardShortcutGridTable::notifyRowsUpdated(const size_t pos, const size_t numRows) {
            if (GetView() != nullptr) {
                wxGridTableMessage message(this, wxGRIDTABLE_REQUEST_VIEW_GET_VALUES,
                                           static_cast<int>(pos),
                                           static_cast<int>(numRows));
                GetView()->ProcessTableMessage(message);
            }
        }

        void KeyboardShortcutGridTable::notifyRowsInserted(const size_t pos, const size_t numRows) {
            if (GetView() != nullptr) {
                wxGridTableMessage message(this, wxGRIDTABLE_NOTIFY_ROWS_INSERTED,
                                           static_cast<int>(pos),
                                           static_cast<int>(numRows));
                GetView()->ProcessTableMessage(message);
            }
        }

        void KeyboardShortcutGridTable::notifyRowsAppended(const size_t numRows) {
            if (GetView() != nullptr) {
                wxGridTableMessage message(this, wxGRIDTABLE_NOTIFY_ROWS_APPENDED,
                                           static_cast<int>(numRows));
                GetView()->ProcessTableMessage(message);
            }
        }

        void KeyboardShortcutGridTable::notifyRowsDeleted(size_t pos, size_t numRows) {
            if (GetView() != nullptr) {
                wxGridTableMessage message(this, wxGRIDTABLE_NOTIFY_ROWS_DELETED,
                                           static_cast<int>(pos),
                                           static_cast<int>(numRows));
                GetView()->ProcessTableMessage(message);
            }
        }

        bool KeyboardShortcutGridTable::markConflicts() {
            const auto cmp = [](const KeyboardShortcutEntry* lhs, const KeyboardShortcutEntry* rhs){
                if ((lhs->actionContext() & rhs->actionContext()) == 0) {
                    if (lhs->actionContext() < rhs->actionContext()) {
                        return true;
                    } else if (lhs->actionContext() > rhs->actionContext()) {
                        return false;
                    }
                }

                const auto& lhsShortcut = lhs->shortcut();
                const auto& rhsShortcut = rhs->shortcut();
                return lhsShortcut < rhsShortcut;
            };
            std::set<KeyboardShortcutEntry*, decltype(cmp)> entrySet(cmp);

            bool hasConflicts = false;
            for (auto& entry : m_entries) {
                entry->resetConflicts();
                if (entry->shortcut().hasKey()) {
                    auto [it, noConflict] = entrySet.insert(entry.get());
                    if (!noConflict) {
                        // found a duplicate, so there are conflicts
                        (*it)->setHasConflicts();
                        entry->setHasConflicts();
                        hasConflicts = true;
                    }
                }
            }

            return hasConflicts;
        }
    }
}
