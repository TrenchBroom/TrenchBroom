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

#include "KeyboardPreferencePane.h"

#include "Macros.h"
#include "Preferences.h"
#include "View/ActionManager.h"
#include "View/CommandIds.h"
#include "View/KeyboardShortcutEditor.h"
#include "View/KeyboardShortcutEvent.h"
#include "View/ViewConstants.h"
#include "View/Menu.h"

#include <wx/msgdlg.h>
#include <wx/sizer.h>
#include <wx/statbox.h>
#include <wx/stattext.h>
#include <wx/stopwatch.h>

#include <cassert>

namespace TrenchBroom {
    namespace View {
        KeyboardGridCellEditor::KeyboardGridCellEditor() :
        wxGridCellEditor(),
        m_editor(NULL),
        m_evtHandler(NULL) {}
        
        KeyboardGridCellEditor::KeyboardGridCellEditor(wxWindow* parent, wxWindowID windowId, wxEvtHandler* evtHandler, const int key, const int modifier1, const int modifier2, const int modifier3) :
        wxGridCellEditor(),
        m_editor(NULL),
        m_evtHandler(NULL) {
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
            KeyboardGridTable* table = static_cast<KeyboardGridTable*>(grid->GetTable());
            if (!table->isValid(row,
                                m_editor->key(),
                                m_editor->modifier1(),
                                m_editor->modifier2(),
                                m_editor->modifier3())) {
                wxString msg;
                msg << "Shortcuts for menu items must include the ";
                msg << KeyboardShortcut::modifierDisplayString(WXK_CONTROL);
                msg << " key.";
                wxMessageBox(msg, "Error", wxOK, m_editor);
                return false;
            }
            
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
        
        ActionEntry::ActionEntry(Action& action) :
        m_action(action),
        m_conflicts(false) {}
        
        const String ActionEntry::caption() const {
            return m_action.displayName();
        }
        
        const String ActionEntry::contextName() const {
            return m_action.contextName();
            
        }
        
        const wxString ActionEntry::shortcut() const {
            return m_action.shortcutDisplayString();
        }
        
        bool ActionEntry::modifiable() const {
            return m_action.modifiable();
        }

        bool ActionEntry::requiresModifiers() const {
            return m_action.requiresModifiers();
        }

        void ActionEntry::updateShortcut(const KeyboardShortcut& shortcut) {
            m_action.updateShortcut(shortcut);
        }
        
        bool ActionEntry::conflictsWith(const ActionEntry& entry) const {
            return m_action.conflictsWith(entry.m_action);
        }
        
        bool ActionEntry::conflicts() const {
            return m_conflicts;
        }
        
        void ActionEntry::setConflicts(const bool conflicts) {
            m_conflicts = conflicts;
        }
        
        KeyboardGridTable::KeyboardGridTable() :
        m_cellEditor(new KeyboardGridCellEditor()) {
            m_cellEditor->IncRef();
        }
        
        KeyboardGridTable::~KeyboardGridTable() {
            m_cellEditor->DecRef();
        }
        
        bool KeyboardGridTable::isValid(const int row, const int key, const int modifier1, const int modifier2, const int modifier3) const {
            assert(row >= 0);
            const size_t rowIndex = static_cast<size_t>(row);
            assert(rowIndex < m_entries.size());
            
            const ActionEntry::Ptr entry = m_entries[rowIndex];
            if (!entry->modifiable())
                return false;
            if (key == WXK_NONE)
                return true;
            if (entry->requiresModifiers() && modifier1 != WXK_COMMAND && modifier2 != WXK_COMMAND && modifier3 != WXK_COMMAND)
                return false;
            return true;
        }

        int KeyboardGridTable::GetNumberRows() {
            return static_cast<int>(m_entries.size());
        }
        
        int KeyboardGridTable::GetNumberCols() {
            return 3;
        }
        
        wxString KeyboardGridTable::GetValue(int row, int col) {
            assert(row >= 0 && row < GetNumberRows());
            assert(col >= 0 && col < GetNumberCols());
            
            size_t rowIndex = static_cast<size_t>(row);
            
            switch (col) {
                case 0:
                    return m_entries[rowIndex]->caption();
                case 1:
                    return m_entries[rowIndex]->contextName();
                case 2:
                    return m_entries[rowIndex]->shortcut();
                default:
                    assert(false);
                    break;
            }
            
            return "";
        }
        
        void KeyboardGridTable::SetValue(int row, int col, const wxString& value) {
            assert(row >= 0 && row < GetNumberRows());
            assert(col == 2);
            
            int key, modifier1, modifier2, modifier3;
            const bool success = KeyboardShortcut::parseShortcut(value, key, modifier1, modifier2, modifier3);
            assert(success);
            _UNUSED(success);
            
            const size_t rowIndex = static_cast<size_t>(row);
            m_entries[rowIndex]->updateShortcut(KeyboardShortcut(key, modifier1, modifier2, modifier3));
            
            if (markConflicts(m_entries))
                notifyRowsUpdated(m_entries.size());
            else
                notifyRowsUpdated(rowIndex, 1);
        }
        
        void KeyboardGridTable::Clear() {
            assert(false);
        }
        
        bool KeyboardGridTable::InsertRows(size_t pos, size_t numRows) {
            assert(false);
            return false;
        }
        
        bool KeyboardGridTable::AppendRows(size_t numRows) {
            assert(false);
            return false;
        }
        
        bool KeyboardGridTable::DeleteRows(size_t pos, size_t numRows) {
            assert(false);
            return false;
        }
        
        wxString KeyboardGridTable::GetColLabelValue(int col) {
            assert(col >= 0 && col < GetNumberCols());
            switch (col) {
                case 0:
                    return "Command";
                case 1:
                    return "Context";
                case 2:
                    return "Shortcut";
                default:
                    assert(false);
                    break;
            }
            
            return "";
        }
        
        wxGridCellAttr* KeyboardGridTable::GetAttr(int row, int col, wxGridCellAttr::wxAttrKind kind) {
            wxGridCellAttr* attr = wxGridTableBase::GetAttr(row, col, kind);
            if (row >= 0 && row < GetNumberRows()) {
                const ActionEntry& entry = *m_entries[static_cast<size_t>(row)];
                if (entry.conflicts()) {
                    if (attr == NULL)
                        attr = new wxGridCellAttr();
                    attr->SetTextColour(*wxRED);
                }
                if (col < 2) {
                    if (attr == NULL)
                        attr = new wxGridCellAttr();
                    attr->SetReadOnly(true);
                } else if (col == 2) {
                    if (attr == NULL)
                        attr = new wxGridCellAttr();
                    if (entry.modifiable()) {
                        attr->SetEditor(m_cellEditor);
                        m_cellEditor->IncRef();
                    } else {
                        attr->SetReadOnly(true);
                        attr->SetTextColour(*wxLIGHT_GREY);
                    }
                }
            }
            return attr;
        }
        
        bool KeyboardGridTable::hasDuplicates() const {
            for (size_t i = 0; i < m_entries.size(); i++)
                if (m_entries[i]->conflicts())
                    return true;
            return false;
        }
        
        bool KeyboardGridTable::update() {
            EntryList newEntries;

            ActionManager& actionManager = ActionManager::instance();
            addMenu(actionManager.getMenu(), newEntries);
            addActions(actionManager.mapViewActions(), newEntries);
            
            /*
             addShortcut(Preferences::CameraMoveForward, newEntries);
             addShortcut(Preferences::CameraMoveBackward, newEntries);
             addShortcut(Preferences::CameraMoveLeft, newEntries);
             addShortcut(Preferences::CameraMoveRight, newEntries);
             */
            
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
        
        void KeyboardGridTable::notifyRowsUpdated(size_t pos, size_t numRows) {
            if (GetView() != NULL) {
                wxGridTableMessage message(this, wxGRIDTABLE_REQUEST_VIEW_GET_VALUES,
                                           static_cast<int>(pos),
                                           static_cast<int>(numRows));
                GetView()->ProcessTableMessage(message);
            }
        }
        
        void KeyboardGridTable::notifyRowsInserted(size_t pos, size_t numRows) {
            if (GetView() != NULL) {
                wxGridTableMessage message(this, wxGRIDTABLE_NOTIFY_ROWS_INSERTED,
                                           static_cast<int>(pos),
                                           static_cast<int>(numRows));
                GetView()->ProcessTableMessage(message);
            }
        }
        
        void KeyboardGridTable::notifyRowsAppended(size_t numRows) {
            if (GetView() != NULL) {
                wxGridTableMessage message(this, wxGRIDTABLE_NOTIFY_ROWS_APPENDED,
                                           static_cast<int>(numRows));
                GetView()->ProcessTableMessage(message);
            }
        }
        
        void KeyboardGridTable::notifyRowsDeleted(size_t pos, size_t numRows) {
            if (GetView() != NULL) {
                wxGridTableMessage message(this, wxGRIDTABLE_NOTIFY_ROWS_DELETED,
                                           static_cast<int>(pos),
                                           static_cast<int>(numRows));
                GetView()->ProcessTableMessage(message);
            }
        }
        
        bool KeyboardGridTable::markConflicts(EntryList& entries) {
            for (size_t i = 0; i < entries.size(); i++)
                entries[i]->setConflicts(false);
            
            bool hasConflicts = false;
            for (size_t i = 0; i < entries.size(); i++) {
                ActionEntry& first = *entries[i];
                for (size_t j = i + 1; j < entries.size(); j++) {
                    ActionEntry& second = *entries[j];
                    if (first.conflictsWith(second)) {
                        first.setConflicts(true);
                        second.setConflicts(true);
                        hasConflicts = true;
                    }
                }
            }
            return hasConflicts;
        }
        
        void KeyboardGridTable::addMenu(Menu& menu, EntryList& entries) const {
            MenuItem::List& items = menu.items();
            MenuItem::List::iterator itemIt, itemEnd;
            for (itemIt = items.begin(), itemEnd = items.end(); itemIt != itemEnd; ++itemIt) {
                MenuItem& item = **itemIt;
                switch (item.type()) {
                    case MenuItem::Type_Action:
                    case MenuItem::Type_Check: {
                        ActionMenuItem& actionItem = static_cast<ActionMenuItem&>(item);
                        entries.push_back(ActionEntry::Ptr(new ActionEntry(actionItem.action())));
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
        
        void KeyboardGridTable::addActions(Action::List& actions, EntryList& entries) const {
            Action::List::iterator it, end;
            for (it = actions.begin(), end = actions.end(); it != end; ++it) {
                Action& action = *it;
                entries.push_back(ActionEntry::Ptr(new ActionEntry(action)));
            }
        }
        
        void KeyboardGridTable::addShortcut(Preference<KeyboardShortcut>& shortcut, EntryList& entries) const {
            //            entries.push_back(ActionEntry::Ptr(new SimpleKeyboardShortcutEntry(shortcut)));
        }
        
        KeyboardPreferencePane::KeyboardPreferencePane(wxWindow* parent) :
        PreferencePane(parent),
        m_grid(NULL),
        m_table(NULL) {
            wxWindow* menuShortcutGrid = createMenuShortcutGrid();
            
            wxSizer* outerSizer = new wxBoxSizer(wxVERTICAL);
            outerSizer->Add(menuShortcutGrid, 1, wxEXPAND);
            outerSizer->SetItemMinSize(menuShortcutGrid, 900, 550);
            SetSizerAndFit(outerSizer);
            SetBackgroundColour(*wxWHITE);
        }
        
        void KeyboardPreferencePane::OnGridSize(wxSizeEvent& event) {
            int width = m_grid->GetClientSize().x;
            m_grid->AutoSizeColumn(0);
            m_grid->AutoSizeColumn(1);
            int colSize = width - m_grid->GetColSize(0) - m_grid->GetColSize(1);
            if (colSize < -1 || colSize == 0)
                colSize = -1;
            m_grid->SetColSize(2, colSize);
            event.Skip();
        }
        
        
        wxWindow* KeyboardPreferencePane::createMenuShortcutGrid() {
            wxPanel* container = new wxPanel(this);
            container->SetBackgroundColour(*wxWHITE);
            
            m_table = new KeyboardGridTable();
            m_grid = new wxGrid(container, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE);
            m_grid->Bind(wxEVT_SIZE, &KeyboardPreferencePane::OnGridSize, this);
            
            m_grid->SetTable(m_table, true, wxGrid::wxGridSelectRows);
            m_grid->SetColLabelSize(18);
            m_grid->SetDefaultCellBackgroundColour(*wxWHITE);
            m_grid->HideRowLabels();
            m_grid->SetCellHighlightPenWidth(0);
            m_grid->SetCellHighlightROPenWidth(0);
            
            m_grid->DisableColResize(0);
            m_grid->DisableColResize(1);
            m_grid->DisableColResize(2);
            m_grid->DisableDragColMove();
            m_grid->DisableDragCell();
            m_grid->DisableDragColSize();
            m_grid->DisableDragGridSize();
            m_grid->DisableDragRowSize();
            
            m_table->update();
            
            wxStaticText* infoText = new wxStaticText(container, wxID_ANY, "Click twice on a key combination to edit the shortcut. Press delete or backspace to delete a shortcut.");
            infoText->SetBackgroundColour(*wxWHITE);
#if defined __APPLE__
            infoText->SetFont(*wxSMALL_FONT);
#endif
            
            wxSizer* sizer = new wxBoxSizer(wxVERTICAL);
            sizer->Add(m_grid, 1, wxEXPAND);
            sizer->AddSpacer(LayoutConstants::WideVMargin);
            sizer->Add(infoText, 0, wxALIGN_CENTER);
            sizer->AddSpacer(LayoutConstants::NarrowVMargin);
            container->SetSizer(sizer);
            
            return container;
        }
        
        void KeyboardPreferencePane::doUpdateControls() {}
        
        bool KeyboardPreferencePane::doValidate() {
            m_grid->SaveEditControlValue();
            if (m_table->hasDuplicates()) {
                wxMessageBox("Please fix all conflicting shortcuts (highlighted in red).", "Error", wxOK, this);
                return false;
            }
            return true;
        }
    }
}
