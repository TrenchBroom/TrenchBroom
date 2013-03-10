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

#include "Utility/Preferences.h"
#include "View/CommandIds.h"
#include "View/LayoutConstants.h"

#include <wx/sizer.h>
#include <wx/statbox.h>
#include <wx/stattext.h>

#include <cassert>

namespace TrenchBroom {
    namespace View {
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
                    return m_entries[rowIndex].shortcut().shortcutMenuText();
                default:
                    assert(false);
                    break;
            }

            return wxT("");
        }

        void KeyboardGridTable::SetValue(int row, int col, const wxString& value) {
            assert(row >= 0 && row < GetNumberRows());
            assert(col == 2);

            // TODO: implement
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
                }
            }
            return attr;
        }

        bool KeyboardGridTable::update() {
            using namespace TrenchBroom::Preferences;
            PreferenceManager& prefs = PreferenceManager::preferences();

            EntryList newEntries;

            newEntries.push_back(Entry(prefs.getKeyboardShortcut(FileNew)));
            newEntries.push_back(Entry(prefs.getKeyboardShortcut(FileOpen)));
            newEntries.push_back(Entry(prefs.getKeyboardShortcut(FileSave)));
            newEntries.push_back(Entry(prefs.getKeyboardShortcut(FileSaveAs)));
            newEntries.push_back(Entry(prefs.getKeyboardShortcut(FileLoadPointFile)));
            newEntries.push_back(Entry(prefs.getKeyboardShortcut(FileUnloadPointFile)));
            newEntries.push_back(Entry(prefs.getKeyboardShortcut(FileClose)));

            newEntries.push_back(Entry(prefs.getKeyboardShortcut(EditUndo)));
            newEntries.push_back(Entry(prefs.getKeyboardShortcut(EditRedo)));
            newEntries.push_back(Entry(prefs.getKeyboardShortcut(EditCut)));
            newEntries.push_back(Entry(prefs.getKeyboardShortcut(EditCopy)));
            newEntries.push_back(Entry(prefs.getKeyboardShortcut(EditPaste)));
            newEntries.push_back(Entry(prefs.getKeyboardShortcut(EditPasteAtOriginalPosition)));
            newEntries.push_back(Entry(prefs.getKeyboardShortcut(EditDelete)));

            newEntries.push_back(Entry(prefs.getKeyboardShortcut(EditSelectAll)));
            newEntries.push_back(Entry(prefs.getKeyboardShortcut(EditSelectSiblings)));
            newEntries.push_back(Entry(prefs.getKeyboardShortcut(EditSelectTouching)));
            newEntries.push_back(Entry(prefs.getKeyboardShortcut(EditSelectNone)));
            newEntries.push_back(Entry(prefs.getKeyboardShortcut(EditHideSelected)));
            newEntries.push_back(Entry(prefs.getKeyboardShortcut(EditHideUnselected)));
            newEntries.push_back(Entry(prefs.getKeyboardShortcut(EditUnhideAll)));
            newEntries.push_back(Entry(prefs.getKeyboardShortcut(EditLockSelected)));
            newEntries.push_back(Entry(prefs.getKeyboardShortcut(EditLockUnselected)));
            newEntries.push_back(Entry(prefs.getKeyboardShortcut(EditUnlockAll)));
            newEntries.push_back(Entry(prefs.getKeyboardShortcut(EditToggleTextureLock)));
            newEntries.push_back(Entry(prefs.getKeyboardShortcut(EditShowMapProperties)));

            newEntries.push_back(Entry(prefs.getKeyboardShortcut(EditToolsToggleClipTool)));
            newEntries.push_back(Entry(prefs.getKeyboardShortcut(EditToolsToggleClipSide)));
            newEntries.push_back(Entry(prefs.getKeyboardShortcut(EditToolsPerformClip)));
            newEntries.push_back(Entry(prefs.getKeyboardShortcut(EditToolsToggleVertexTool)));
            newEntries.push_back(Entry(prefs.getKeyboardShortcut(EditToolsToggleRotateTool)));

            newEntries.push_back(Entry(prefs.getKeyboardShortcut(EditActionsMoveTexturesUp)));
            newEntries.push_back(Entry(prefs.getKeyboardShortcut(EditActionsMoveTexturesDown)));
            newEntries.push_back(Entry(prefs.getKeyboardShortcut(EditActionsMoveTexturesLeft)));
            newEntries.push_back(Entry(prefs.getKeyboardShortcut(EditActionsMoveTexturesRight)));
            newEntries.push_back(Entry(prefs.getKeyboardShortcut(EditActionsRotateTexturesCW)));
            newEntries.push_back(Entry(prefs.getKeyboardShortcut(EditActionsRotateTexturesCCW)));
            newEntries.push_back(Entry(prefs.getKeyboardShortcut(EditActionsMoveTexturesUpFine)));
            newEntries.push_back(Entry(prefs.getKeyboardShortcut(EditActionsMoveTexturesDownFine)));
            newEntries.push_back(Entry(prefs.getKeyboardShortcut(EditActionsMoveTexturesLeftFine)));
            newEntries.push_back(Entry(prefs.getKeyboardShortcut(EditActionsMoveTexturesRightFine)));
            newEntries.push_back(Entry(prefs.getKeyboardShortcut(EditActionsRotateTexturesCWFine)));
            newEntries.push_back(Entry(prefs.getKeyboardShortcut(EditActionsRotateTexturesCCWFine)));

            newEntries.push_back(Entry(prefs.getKeyboardShortcut(EditActionsMoveObjectsForward)));
            newEntries.push_back(Entry(prefs.getKeyboardShortcut(EditActionsMoveObjectsBackward)));
            newEntries.push_back(Entry(prefs.getKeyboardShortcut(EditActionsMoveObjectsLeft)));
            newEntries.push_back(Entry(prefs.getKeyboardShortcut(EditActionsMoveObjectsRight)));
            newEntries.push_back(Entry(prefs.getKeyboardShortcut(EditActionsMoveObjectsUp)));
            newEntries.push_back(Entry(prefs.getKeyboardShortcut(EditActionsMoveObjectsDown)));
            newEntries.push_back(Entry(prefs.getKeyboardShortcut(EditActionsRollObjectsCW)));
            newEntries.push_back(Entry(prefs.getKeyboardShortcut(EditActionsRollObjectsCCW)));
            newEntries.push_back(Entry(prefs.getKeyboardShortcut(EditActionsYawObjectsCW)));
            newEntries.push_back(Entry(prefs.getKeyboardShortcut(EditActionsYawObjectsCCW)));
            newEntries.push_back(Entry(prefs.getKeyboardShortcut(EditActionsPitchObjectsCW)));
            newEntries.push_back(Entry(prefs.getKeyboardShortcut(EditActionsPitchObjectsCCW)));
            newEntries.push_back(Entry(prefs.getKeyboardShortcut(EditActionsFlipObjectsHorizontally)));
            newEntries.push_back(Entry(prefs.getKeyboardShortcut(EditActionsFlipObjectsVertically)));
            newEntries.push_back(Entry(prefs.getKeyboardShortcut(EditActionsDuplicateObjects)));

            newEntries.push_back(Entry(prefs.getKeyboardShortcut(EditActionsMoveVerticesForward)));
            newEntries.push_back(Entry(prefs.getKeyboardShortcut(EditActionsMoveVerticesBackward)));
            newEntries.push_back(Entry(prefs.getKeyboardShortcut(EditActionsMoveVerticesLeft)));
            newEntries.push_back(Entry(prefs.getKeyboardShortcut(EditActionsMoveVerticesRight)));
            newEntries.push_back(Entry(prefs.getKeyboardShortcut(EditActionsMoveVerticesUp)));
            newEntries.push_back(Entry(prefs.getKeyboardShortcut(EditActionsMoveVerticesDown)));

            newEntries.push_back(Entry(prefs.getKeyboardShortcut(EditActionsCorrectVertices)));
            newEntries.push_back(Entry(prefs.getKeyboardShortcut(EditActionsSnapVertices)));

            newEntries.push_back(Entry(prefs.getKeyboardShortcut(ViewGridToggleShowGrid)));
            newEntries.push_back(Entry(prefs.getKeyboardShortcut(ViewGridToggleSnapToGrid)));
            newEntries.push_back(Entry(prefs.getKeyboardShortcut(ViewGridIncGridSize)));
            newEntries.push_back(Entry(prefs.getKeyboardShortcut(ViewGridDecGridSize)));
            newEntries.push_back(Entry(prefs.getKeyboardShortcut(ViewGridSetSize1)));
            newEntries.push_back(Entry(prefs.getKeyboardShortcut(ViewGridSetSize2)));
            newEntries.push_back(Entry(prefs.getKeyboardShortcut(ViewGridSetSize4)));
            newEntries.push_back(Entry(prefs.getKeyboardShortcut(ViewGridSetSize8)));
            newEntries.push_back(Entry(prefs.getKeyboardShortcut(ViewGridSetSize16)));
            newEntries.push_back(Entry(prefs.getKeyboardShortcut(ViewGridSetSize32)));
            newEntries.push_back(Entry(prefs.getKeyboardShortcut(ViewGridSetSize64)));
            newEntries.push_back(Entry(prefs.getKeyboardShortcut(ViewGridSetSize128)));
            newEntries.push_back(Entry(prefs.getKeyboardShortcut(ViewGridSetSize256)));

            newEntries.push_back(Entry(prefs.getKeyboardShortcut(ViewCameraMoveForward)));
            newEntries.push_back(Entry(prefs.getKeyboardShortcut(ViewCameraMoveBackward)));
            newEntries.push_back(Entry(prefs.getKeyboardShortcut(ViewCameraMoveLeft)));
            newEntries.push_back(Entry(prefs.getKeyboardShortcut(ViewCameraMoveRight)));
            newEntries.push_back(Entry(prefs.getKeyboardShortcut(ViewCameraMoveUp)));
            newEntries.push_back(Entry(prefs.getKeyboardShortcut(ViewCameraMoveDown)));
            newEntries.push_back(Entry(prefs.getKeyboardShortcut(ViewCameraMoveToNextPoint)));
            newEntries.push_back(Entry(prefs.getKeyboardShortcut(ViewCameraMoveToPreviousPoint)));
            newEntries.push_back(Entry(prefs.getKeyboardShortcut(ViewCameraCenterCameraOnSelection)));

            // mark duplicates
            bool hasDuplicates = false;
            for (size_t i = 0; i < newEntries.size(); i++) {
                Entry& first = newEntries[i];
                if (first.shortcut().key() != WXK_NONE) {
                    for (size_t j = i + 1; j < newEntries.size(); j++) {
                        Entry& second = newEntries[j];
                        if (first.isDuplicateOf(second)) {
                            first.setDuplicate(true);
                            second.setDuplicate(true);
                            hasDuplicates = true;
                        }
                    }
                }
            }
            
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
        wxPanel(parent),
        m_grid(NULL),
        m_table(NULL) {
            wxStaticBox* box = new wxStaticBox(this, wxID_ANY, wxT("Keyboard Shortcuts"));
            wxStaticText* infoText = new wxStaticText(box, wxID_ANY, wxT("Click twice on a key combination to edit the shortcut."));
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

        void KeyboardPreferencePane::OnGridSize(wxSizeEvent& event) {
            int width = m_grid->GetClientSize().x;
            m_grid->SetColSize(0, width / 3);
            m_grid->SetColSize(1, width / 3);
            m_grid->SetColSize(2, width - m_grid->GetColSize(0) - m_grid->GetColSize(1));
            event.Skip();
        }
    }
}
