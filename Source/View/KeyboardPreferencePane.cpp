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

#include "KeyboardPreferencePane.h"

#include "TrenchBroomApp.h"
#include "Controller/PreferenceChangeEvent.h"
#include "View/CommandIds.h"
#include "View/KeyboardShortcutEditor.h"
#include "View/KeyboardShortcutEvent.h"
#include "View/LayoutConstants.h"

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

        KeyboardGridCellEditor::KeyboardGridCellEditor(wxWindow* parent, wxWindowID windowId, wxEvtHandler* evtHandler, int modifierKey1, int modifierKey2, int modifierKey3, int key) :
        wxGridCellEditor(),
        m_editor(NULL),
        m_evtHandler(NULL) {
            Create(parent, windowId, evtHandler);
            m_editor->SetShortcut(key, modifierKey1, modifierKey2, modifierKey3);
        }

        void KeyboardGridCellEditor::Create(wxWindow* parent, wxWindowID windowId, wxEvtHandler* evtHandler) {
            m_evtHandler = evtHandler;
            m_editor = new KeyboardShortcutEditor(parent, wxID_ANY);
            SetControl(m_editor);
            // wxGridCellEditor::Create(parent, windowId, evtHandler);
        }

        wxGridCellEditor* KeyboardGridCellEditor::Clone() const {
            return new KeyboardGridCellEditor(m_editor->GetParent(), wxID_ANY, m_evtHandler,
                                              m_editor->modifierKey1(),
                                              m_editor->modifierKey2(),
                                              m_editor->modifierKey3(),
                                              m_editor->key());
        }

        void KeyboardGridCellEditor::BeginEdit(int row, int col, wxGrid* grid) {
            int modifierKey1, modifierKey2, modifierKey3, key;
            KeyboardShortcut::parseShortcut(grid->GetCellValue(row, col),
                                            modifierKey1,
                                            modifierKey2,
                                            modifierKey3,
                                            key);
            m_editor->SetShortcut(key, modifierKey1, modifierKey2, modifierKey3);
            m_editor->SetFocus();
        }

        bool KeyboardGridCellEditor::EndEdit(int row, int col, const wxGrid* grid, const wxString& oldValue, wxString* newValue) {
            *newValue = KeyboardShortcut::shortcutDisplayText(m_editor->modifierKey1(),
                                                              m_editor->modifierKey2(),
                                                              m_editor->modifierKey3(),
                                                              m_editor->key());
            if (*newValue == oldValue)
                return false;
            return true;
        }

        void KeyboardGridCellEditor::ApplyEdit(int row, int col, wxGrid* grid) {
            wxString newValue = KeyboardShortcut::shortcutDisplayText(m_editor->modifierKey1(),
                                                                      m_editor->modifierKey2(),
                                                                      m_editor->modifierKey3(),
                                                                      m_editor->key());
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
            return KeyboardShortcut::shortcutDisplayText(m_editor->modifierKey1(),
                                                         m_editor->modifierKey2(),
                                                         m_editor->modifierKey3(),
                                                         m_editor->key());
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

        bool KeyboardGridTable::markDuplicates(EntryList& entries) {
            for (size_t i = 0; i < entries.size(); i++)
                entries[i]->setDuplicate(false);

            bool hasDuplicates = false;
            for (size_t i = 0; i < entries.size(); i++) {
                KeyboardShortcutEntry& first = *entries[i];
                if (first.shortcut().key() != WXK_NONE) {
                    for (size_t j = i + 1; j < entries.size(); j++) {
                        KeyboardShortcutEntry& second = *entries[j];
                        if (first.isDuplicateOf(second)) {
                            first.setDuplicate(true);
                            second.setDuplicate(true);
                            hasDuplicates = true;
                        }
                    }
                }
            }
            return hasDuplicates;
        }

        void KeyboardGridTable::addMenu(const Preferences::Menu& menu, EntryList& entries) const {
            using namespace TrenchBroom::Preferences;

            const MenuItem::List& items = menu.items();
            MenuItem::List::const_iterator itemIt, itemEnd;
            for (itemIt = items.begin(), itemEnd = items.end(); itemIt != itemEnd; ++itemIt) {
                const MenuItem& item = **itemIt;
                switch (item.type()) {
                    case MenuItem::MITAction:
                    case MenuItem::MITCheck: {
                        const ShortcutMenuItem& shortcutItem = static_cast<const ShortcutMenuItem&>(item);
                        entries.push_back(KeyboardShortcutEntry::Ptr(new MenuKeyboardShortcutEntry(shortcutItem)));
                        break;
                    }
                    case MenuItem::MITMenu: {
                        const Menu& subMenu = static_cast<const Menu&>(item);
                        addMenu(subMenu, entries);
                        break;
                    }
                    case MenuItem::MITMultiMenu: {
                        const MultiMenu& multiMenu = static_cast<const MultiMenu&>(item);
                        const MenuItem::List& multiItems = multiMenu.items();
                        MenuItem::List::const_iterator multiIt, multiEnd;
                        for (multiIt = multiItems.begin(), multiEnd = multiItems.end(); multiIt != multiEnd; ++multiIt) {
                            const Menu& multiItem = static_cast<const Menu&>(**multiIt);
                            addMenu(multiItem, entries);
                        }
                        break;
                    }
                    default:
                        break;
                }
            }
        }

        void KeyboardGridTable::addShortcut(const Preferences::Preference<KeyboardShortcut>& shortcut, EntryList& entries) const {
            entries.push_back(KeyboardShortcutEntry::Ptr(new SimpleKeyboardShortcutEntry(shortcut)));
        }

        KeyboardShortcutEntry::KeyboardShortcutEntry() :
        m_duplicate(false) {}

        bool KeyboardShortcutEntry::isDuplicateOf(const KeyboardShortcutEntry& entry) const {
            if (shortcut().commandId() == entry.shortcut().commandId())
                return false;
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

        MenuKeyboardShortcutEntry::MenuKeyboardShortcutEntry(const Preferences::ShortcutMenuItem& item) :
        KeyboardShortcutEntry(),
        m_item(item) {}

        const String MenuKeyboardShortcutEntry::caption() const {
            return m_item.longText();
        }

        const KeyboardShortcut& MenuKeyboardShortcutEntry::shortcut() const {
            return m_item.shortcut();
        }

        void MenuKeyboardShortcutEntry::saveShortcut(const KeyboardShortcut& shortcut) const {
            m_item.setShortcut(shortcut);
        }

        SimpleKeyboardShortcutEntry::SimpleKeyboardShortcutEntry(const Preferences::Preference<KeyboardShortcut>& preference) :
        KeyboardShortcutEntry(),
        m_preference(preference) {}

        const String SimpleKeyboardShortcutEntry::caption() const {
            return shortcut().text();
        }

        const KeyboardShortcut& SimpleKeyboardShortcutEntry::shortcut() const {
            Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();
            return prefs.getKeyboardShortcut(m_preference);
        }

        void SimpleKeyboardShortcutEntry::saveShortcut(const KeyboardShortcut& shortcut) const {
            Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();
            prefs.setKeyboardShortcut(m_preference, shortcut);
        }

        KeyboardGridTable::KeyboardGridTable() :
        m_cellEditor(new KeyboardGridCellEditor()) {
            m_cellEditor->IncRef();
        }

        KeyboardGridTable::~KeyboardGridTable() {
            m_cellEditor->DecRef();
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
                    return KeyboardShortcut::contextName(m_entries[rowIndex]->shortcut().context());
                case 2:
                    return m_entries[rowIndex]->shortcut().shortcutDisplayText();
                default:
                    assert(false);
                    break;
            }

            return wxT("");
        }

        void KeyboardGridTable::SetValue(int row, int col, const wxString& value) {
            assert(row >= 0 && row < GetNumberRows());
            assert(col == 2);

            int modifierKey1, modifierKey2, modifierKey3, key;
            bool success = KeyboardShortcut::parseShortcut(value, modifierKey1, modifierKey2, modifierKey3, key);
            assert(success);

            size_t rowIndex = static_cast<size_t>(row);
            const KeyboardShortcut& oldShortcut = m_entries[rowIndex]->shortcut();
            KeyboardShortcut newShortcut = KeyboardShortcut(oldShortcut.commandId(),
                                                            modifierKey1, modifierKey2, modifierKey3, key,
                                                            oldShortcut.context(),
                                                            oldShortcut.text());

            m_entries[rowIndex]->saveShortcut(newShortcut);

            
#ifdef __APPLE__
            Controller::PreferenceChangeEvent preferenceChangeEvent;
            preferenceChangeEvent.setMenuChanged(true);
            static_cast<TrenchBroomApp*>(wxTheApp)->UpdateAllViews(NULL, &preferenceChangeEvent);
#endif

            if (markDuplicates(m_entries))
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

            return wxT("");
        }

        wxGridCellAttr* KeyboardGridTable::GetAttr(int row, int col, wxGridCellAttr::wxAttrKind kind) {
            wxGridCellAttr* attr = wxGridTableBase::GetAttr(row, col, kind);
            if (row >= 0 && row < GetNumberRows()) {
                const KeyboardShortcutEntry& entry = *m_entries[static_cast<size_t>(row)];
                if (entry.duplicate()) {
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
                    attr->SetEditor(m_cellEditor);
                    m_cellEditor->IncRef();
                }
            }
            return attr;
        }

        bool KeyboardGridTable::hasDuplicates() const {
            for (size_t i = 0; i < m_entries.size(); i++)
                if (m_entries[i]->duplicate())
                    return true;
            return false;
        }

        bool KeyboardGridTable::update() {
            using namespace TrenchBroom::Preferences;
            PreferenceManager& prefs = PreferenceManager::preferences();

            EntryList newEntries;

            addMenu(prefs.getMenu(FileMenu), newEntries);
            addMenu(prefs.getMenu(EditMenu), newEntries);
            addMenu(prefs.getMenu(ViewMenu), newEntries);
            addShortcut(Preferences::CameraMoveForward, newEntries);
            addShortcut(Preferences::CameraMoveBackward, newEntries);
            addShortcut(Preferences::CameraMoveLeft, newEntries);
            addShortcut(Preferences::CameraMoveRight, newEntries);

            bool hasDuplicates = markDuplicates(newEntries);

            size_t oldSize = m_entries.size();
            m_entries = newEntries;

            notifyRowsUpdated(0, oldSize);
            if (oldSize < m_entries.size())
                notifyRowsAppended(m_entries.size() - oldSize);
            else if (oldSize > m_entries.size())
                notifyRowsDeleted(oldSize, oldSize - m_entries.size());

            return hasDuplicates;
        }

        wxStaticBox* KeyboardPreferencePane::createMenuShortcutBox() {
            wxStaticBox* box = new wxStaticBox(this, wxID_ANY, wxT("Menu Shortcuts"));
            wxStaticText* infoText = new wxStaticText(box, wxID_ANY, wxT("Click twice on a key combination to edit the shortcut. Press delete or backspace to delete a shortcut."));
#if defined __APPLE__
            infoText->SetFont(*wxSMALL_FONT);
#endif

            m_table = new KeyboardGridTable();
            m_grid = new wxGrid(box, CommandIds::KeyboardPreferencePane::ShortcutEditorId, wxDefaultPosition, wxDefaultSize, wxBORDER_SUNKEN);
            m_grid->Bind(wxEVT_SIZE, &KeyboardPreferencePane::OnGridSize, this);

            m_grid->SetTable(m_table, true, wxGrid::wxGridSelectRows);
            m_grid->SetUseNativeColLabels();
            m_grid->UseNativeColHeader();
            m_grid->SetDefaultCellBackgroundColour(*wxWHITE);
            m_grid->HideRowLabels();
            m_grid->SetCellHighlightPenWidth(0);
            m_grid->SetCellHighlightROPenWidth(0);
            //            m_grid->EnableEditing(false);

            m_grid->DisableColResize(0);
            m_grid->DisableColResize(1);
            m_grid->DisableColResize(2);
            m_grid->DisableDragColMove();
            m_grid->DisableDragCell();
            m_grid->DisableDragColSize();
            m_grid->DisableDragGridSize();
            m_grid->DisableDragRowSize();

            m_table->update();

            wxSizer* innerSizer = new wxBoxSizer(wxVERTICAL);
            innerSizer->AddSpacer(LayoutConstants::StaticBoxTopMargin);
            innerSizer->Add(infoText, 0, wxEXPAND | wxLEFT | wxRIGHT, LayoutConstants::StaticBoxSideMargin);
            innerSizer->AddSpacer(LayoutConstants::ControlVerticalMargin);
            innerSizer->Add(m_grid, 1, wxEXPAND | wxLEFT | wxRIGHT, LayoutConstants::StaticBoxSideMargin);
            innerSizer->AddSpacer(LayoutConstants::StaticBoxBottomMargin);
            box->SetSizer(innerSizer);

            return box;
        }

        KeyboardPreferencePane::KeyboardPreferencePane(wxWindow* parent) :
        PreferencePane(parent),
        m_grid(NULL),
        m_table(NULL) {
            wxStaticBox* menuShortcutBox = createMenuShortcutBox();

            wxSizer* outerSizer = new wxBoxSizer(wxVERTICAL);
            outerSizer->Add(menuShortcutBox, 1, wxEXPAND);
            outerSizer->SetItemMinSize(menuShortcutBox, 700, 550);
            SetSizerAndFit(outerSizer);
        }

        bool KeyboardPreferencePane::validate() {
            m_grid->SaveEditControlValue();
            if (m_table->hasDuplicates()) {
                wxMessageBox(wxT("Please fix all conflicting shortcuts (highlighted in red)."), wxT("Error"), wxOK, this);
                return false;
            }
            return true;
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
    }
}
