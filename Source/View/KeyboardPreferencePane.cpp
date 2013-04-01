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

#include "View/CommandIds.h"
#include "View/KeyboardShortcutEditor.h"
#include "View/KeyboardShortcutEvent.h"
#include "View/LayoutConstants.h"

#include <wx/sizer.h>
#include <wx/statbox.h>
#include <wx/stattext.h>

#include <cassert>

namespace TrenchBroom {
    namespace View {
        KeyboardGridCellEditor::KeyboardGridCellEditor() :
        wxGridCellEditor() {}

        KeyboardGridCellEditor::KeyboardGridCellEditor(wxWindow* parent, wxWindowID windowId, wxEvtHandler* evtHandler, int modifierKey1, int modifierKey2, int modifierKey3, int key) :
        wxGridCellEditor() {
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
                entries[i].setDuplicate(false);

            bool hasDuplicates = false;
            for (size_t i = 0; i < entries.size(); i++) {
                Entry& first = entries[i];
                if (first.shortcut().key() != WXK_NONE) {
                    for (size_t j = i + 1; j < entries.size(); j++) {
                        Entry& second = entries[j];
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
                    return m_entries[rowIndex].shortcut().text();
                case 1:
                    return KeyboardShortcut::contextName(m_entries[rowIndex].shortcut().context());
                case 2:
                    return m_entries[rowIndex].shortcut().shortcutDisplayText();
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
            const KeyboardShortcut& oldShortcut = m_entries[rowIndex].shortcut();
            KeyboardShortcut newShortcut = KeyboardShortcut(oldShortcut.commandId(),
                                                            modifierKey1, modifierKey2, modifierKey3, key,
                                                            oldShortcut.context(),
                                                            oldShortcut.text());

            using namespace TrenchBroom::Preferences;
            PreferenceManager& prefs = PreferenceManager::preferences();
            
            const Preference<KeyboardShortcut>& pref = m_entries[rowIndex].pref();
            prefs.setKeyboardShortcut(pref, newShortcut);
            
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
                const Entry& entry = m_entries[static_cast<size_t>(row)];
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
                if (m_entries[i].duplicate())
                    return true;
            return false;
        }

        bool KeyboardGridTable::update() {
            using namespace TrenchBroom::Preferences;

            EntryList newEntries;

            newEntries.push_back(Entry(FileNew));
            newEntries.push_back(Entry(FileOpen));
            newEntries.push_back(Entry(FileSave));
            newEntries.push_back(Entry(FileSaveAs));
            newEntries.push_back(Entry(FileLoadPointFile));
            newEntries.push_back(Entry(FileUnloadPointFile));
            newEntries.push_back(Entry(FileClose));
            
            newEntries.push_back(Entry(EditUndo));
            newEntries.push_back(Entry(EditRedo));
            newEntries.push_back(Entry(EditCut));
            newEntries.push_back(Entry(EditCopy));
            newEntries.push_back(Entry(EditPaste));
            newEntries.push_back(Entry(EditPasteAtOriginalPosition));
            newEntries.push_back(Entry(EditDelete));
            
            newEntries.push_back(Entry(EditSelectAll));
            newEntries.push_back(Entry(EditSelectSiblings));
            newEntries.push_back(Entry(EditSelectTouching));
            newEntries.push_back(Entry(EditSelectByFilePosition));
            newEntries.push_back(Entry(EditSelectNone));
            newEntries.push_back(Entry(EditHideSelected));
            newEntries.push_back(Entry(EditHideUnselected));
            newEntries.push_back(Entry(EditUnhideAll));
            newEntries.push_back(Entry(EditLockSelected));
            newEntries.push_back(Entry(EditLockUnselected));
            newEntries.push_back(Entry(EditUnlockAll));
            newEntries.push_back(Entry(EditToggleTextureLock));
            newEntries.push_back(Entry(EditShowMapProperties));
            
            newEntries.push_back(Entry(EditToolsToggleClipTool));
            newEntries.push_back(Entry(EditToolsToggleClipSide));
            newEntries.push_back(Entry(EditToolsPerformClip));
            newEntries.push_back(Entry(EditToolsToggleVertexTool));
            newEntries.push_back(Entry(EditToolsToggleRotateTool));
            
            newEntries.push_back(Entry(EditActionsMoveTexturesUp));
            newEntries.push_back(Entry(EditActionsMoveTexturesDown));
            newEntries.push_back(Entry(EditActionsMoveTexturesLeft));
            newEntries.push_back(Entry(EditActionsMoveTexturesRight));
            newEntries.push_back(Entry(EditActionsRotateTexturesCW));
            newEntries.push_back(Entry(EditActionsRotateTexturesCCW));
            newEntries.push_back(Entry(EditActionsMoveTexturesUpFine));
            newEntries.push_back(Entry(EditActionsMoveTexturesDownFine));
            newEntries.push_back(Entry(EditActionsMoveTexturesLeftFine));
            newEntries.push_back(Entry(EditActionsMoveTexturesRightFine));
            newEntries.push_back(Entry(EditActionsRotateTexturesCWFine));
            newEntries.push_back(Entry(EditActionsRotateTexturesCCWFine));
            
            newEntries.push_back(Entry(EditActionsMoveObjectsForward));
            newEntries.push_back(Entry(EditActionsMoveObjectsBackward));
            newEntries.push_back(Entry(EditActionsMoveObjectsLeft));
            newEntries.push_back(Entry(EditActionsMoveObjectsRight));
            newEntries.push_back(Entry(EditActionsMoveObjectsUp));
            newEntries.push_back(Entry(EditActionsMoveObjectsDown));
            newEntries.push_back(Entry(EditActionsDuplicateObjectsForward));
            newEntries.push_back(Entry(EditActionsDuplicateObjectsBackward));
            newEntries.push_back(Entry(EditActionsDuplicateObjectsLeft));
            newEntries.push_back(Entry(EditActionsDuplicateObjectsRight));
            newEntries.push_back(Entry(EditActionsDuplicateObjectsUp));
            newEntries.push_back(Entry(EditActionsDuplicateObjectsDown));
            newEntries.push_back(Entry(EditActionsRollObjectsCW));
            newEntries.push_back(Entry(EditActionsRollObjectsCCW));
            newEntries.push_back(Entry(EditActionsYawObjectsCW));
            newEntries.push_back(Entry(EditActionsYawObjectsCCW));
            newEntries.push_back(Entry(EditActionsPitchObjectsCW));
            newEntries.push_back(Entry(EditActionsPitchObjectsCCW));
            newEntries.push_back(Entry(EditActionsFlipObjectsHorizontally));
            newEntries.push_back(Entry(EditActionsFlipObjectsVertically));
            newEntries.push_back(Entry(EditActionsDuplicateObjects));
            
            newEntries.push_back(Entry(EditActionsMoveVerticesForward));
            newEntries.push_back(Entry(EditActionsMoveVerticesBackward));
            newEntries.push_back(Entry(EditActionsMoveVerticesLeft));
            newEntries.push_back(Entry(EditActionsMoveVerticesRight));
            newEntries.push_back(Entry(EditActionsMoveVerticesUp));
            newEntries.push_back(Entry(EditActionsMoveVerticesDown));
            
            newEntries.push_back(Entry(EditActionsCorrectVertices));
            newEntries.push_back(Entry(EditActionsSnapVertices));
            
            newEntries.push_back(Entry(ViewGridToggleShowGrid));
            newEntries.push_back(Entry(ViewGridToggleSnapToGrid));
            newEntries.push_back(Entry(ViewGridIncGridSize));
            newEntries.push_back(Entry(ViewGridDecGridSize));
            newEntries.push_back(Entry(ViewGridSetSize1));
            newEntries.push_back(Entry(ViewGridSetSize2));
            newEntries.push_back(Entry(ViewGridSetSize4));
            newEntries.push_back(Entry(ViewGridSetSize8));
            newEntries.push_back(Entry(ViewGridSetSize16));
            newEntries.push_back(Entry(ViewGridSetSize32));
            newEntries.push_back(Entry(ViewGridSetSize64));
            newEntries.push_back(Entry(ViewGridSetSize128));
            newEntries.push_back(Entry(ViewGridSetSize256));
            
            newEntries.push_back(Entry(ViewCameraMoveForward));
            newEntries.push_back(Entry(ViewCameraMoveBackward));
            newEntries.push_back(Entry(ViewCameraMoveLeft));
            newEntries.push_back(Entry(ViewCameraMoveRight));
            newEntries.push_back(Entry(ViewCameraMoveUp));
            newEntries.push_back(Entry(ViewCameraMoveDown));
            newEntries.push_back(Entry(ViewCameraMoveToNextPoint));
            newEntries.push_back(Entry(ViewCameraMoveToPreviousPoint));
            newEntries.push_back(Entry(ViewCameraCenterCameraOnSelection));

            newEntries.push_back(Entry(ViewSwitchToEntityTab));
            newEntries.push_back(Entry(ViewSwitchToFaceTab));
            newEntries.push_back(Entry(ViewSwitchToViewTab));

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

        KeyboardPreferencePane::KeyboardPreferencePane(wxWindow* parent) :
        PreferencePane(parent),
        m_grid(NULL),
        m_table(NULL) {
            wxStaticBox* box = new wxStaticBox(this, wxID_ANY, wxT("Keyboard Shortcuts"));
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
            m_grid->DisableDragColMove();
            m_grid->DisableDragCell();
            m_grid->DisableDragColSize();
            m_grid->DisableDragGridSize();
            m_grid->DisableDragRowSize();

            m_table->update();
            m_grid->AutoSize();

            wxSizer* innerSizer = new wxBoxSizer(wxVERTICAL);
            innerSizer->AddSpacer(LayoutConstants::StaticBoxTopMargin);
            innerSizer->Add(infoText, 0, wxEXPAND | wxLEFT | wxRIGHT, LayoutConstants::StaticBoxSideMargin);
            innerSizer->AddSpacer(LayoutConstants::ControlVerticalMargin);
            innerSizer->Add(m_grid, 1, wxEXPAND | wxLEFT | wxRIGHT, LayoutConstants::StaticBoxSideMargin);
            innerSizer->AddSpacer(LayoutConstants::StaticBoxBottomMargin);
            box->SetSizer(innerSizer);
            
            wxSizer* outerSizer = new wxBoxSizer(wxVERTICAL);
            outerSizer->Add(box, 1, wxEXPAND);
            outerSizer->SetItemMinSize(box, wxDefaultSize.x, 500);
            SetSizerAndFit(outerSizer);
        }

        bool KeyboardPreferencePane::validate() {
            if (m_table->hasDuplicates()) {
                wxMessageBox(wxT("Please fix all conflicting shortcuts (highlighted in red)."), wxT("Error"), wxOK, this);
                return false;
            }
            return true;
        }

        void KeyboardPreferencePane::OnGridSize(wxSizeEvent& event) {
            int width = m_grid->GetClientSize().x;
            m_grid->SetColSize(0, width / 3);
            m_grid->SetColSize(1, width / 3);
            m_grid->SetColSize(2, width - m_grid->GetColSize(0) - m_grid->GetColSize(1));
            event.Skip();
        }
    }
}
