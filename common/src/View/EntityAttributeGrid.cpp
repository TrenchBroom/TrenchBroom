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
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#include "EntityAttributeGrid.h"

#include "Model/EntityAttributes.h"
#include "Model/Object.h"
#include "View/EntityAttributeGridTable.h"
#include "View/EntityAttributeSelectedCommand.h"
#include "View/ViewConstants.h"
#include "View/MapDocument.h"
#include "View/wxUtils.h"

#include <wx/bmpbuttn.h>
#include <wx/checkbox.h>
#include <wx/settings.h>
#include <wx/sizer.h>
#include <wx/textctrl.h>

namespace TrenchBroom {
    namespace View {
        EntityAttributeGrid::EntityAttributeGrid(wxWindow* parent, MapDocumentWPtr document) :
        wxPanel(parent),
        m_document(document),
        m_lastHoveredCell(wxGridCellCoords(-1, -1)),
        m_ignoreSelection(false),
        m_lastSelectedCol(0) {
            createGui(document);
            bindObservers();
        }
        
        EntityAttributeGrid::~EntityAttributeGrid() {
            unbindObservers();
        }
        
        void EntityAttributeGrid::OnAttributeGridSize(wxSizeEvent& event) {
            if (IsBeingDeleted()) return;

            m_grid->SetColSize(0, 100);
            const int colSize = std::max(1, m_grid->GetClientSize().x - m_grid->GetColSize(0));
            m_grid->SetColSize(1, colSize);
            event.Skip();
        }
        
        void EntityAttributeGrid::OnAttributeGridSelectCell(wxGridEvent& event) {
            if (IsBeingDeleted()) return;
            fireSelectionEvent(event.GetRow(), event.GetCol());
        }
        
        void EntityAttributeGrid::tabNavigate(int row, int col, bool forward) {
            if (IsBeingDeleted()) return;

            if (!forward) {
                if (col > 0)
                    moveCursorTo(row, col - 1);
                else if (row > 0)
                    moveCursorTo(row - 1, m_grid->GetNumberCols() - 1);
            } else {
                if (col < m_grid->GetNumberCols() - 1)
                    moveCursorTo(row, col + 1);
                else if (row < m_grid->GetNumberRows() - 1)
                    moveCursorTo(row + 1, 0);
            }
        }

        void EntityAttributeGrid::OnAttributeGridTab(wxGridEvent& event) {
            tabNavigate(event.GetRow(), event.GetCol(), !event.ShiftDown());
        }
        
        void EntityAttributeGrid::moveCursorTo(const int row, const int col) {
            {
                const SetBool ignoreSelection(m_ignoreSelection);
                m_grid->GoToCell(row, col);
                m_grid->SelectRow(row);
            }
            fireSelectionEvent(row, col);
        }

        void EntityAttributeGrid::fireSelectionEvent(const int row, const int col) {
            if (!m_ignoreSelection) {
                const Model::AttributeName name = m_table->attributeName(row);
                m_lastSelectedName = name;
                m_lastSelectedCol = col;
                
                EntityAttributeSelectedCommand command;
                command.setName(name);
                command.SetEventObject(this);
                command.SetId(GetId());
                ProcessEvent(command);
            }
        }

        void EntityAttributeGrid::OnAttributeGridKeyDown(wxKeyEvent& event) {
            if (IsBeingDeleted()) return;

            if (isInsertRowShortcut(event)) {
                addAttribute();
            } else if (isRemoveRowShortcut(event)) {
                if (canRemoveSelectedAttributes())
                    removeSelectedAttributes();
            } else {
                event.Skip();
            }
        }
        
        void EntityAttributeGrid::OnAttributeGridKeyUp(wxKeyEvent& event) {
            if (IsBeingDeleted()) return;

            if (!isInsertRowShortcut(event) && !isRemoveRowShortcut(event))
                event.Skip();
        }

        bool EntityAttributeGrid::isInsertRowShortcut(const wxKeyEvent& event) const {
            return event.GetKeyCode() == WXK_RETURN && event.ControlDown();
        }
        
        bool EntityAttributeGrid::isRemoveRowShortcut(const wxKeyEvent& event) const {
            return (event.GetKeyCode() == WXK_DELETE || event.GetKeyCode() == WXK_BACK) && !m_grid->IsCellEditControlShown();
        }

        void EntityAttributeGrid::OnAttributeGridMouseMove(wxMouseEvent& event) {
            if (IsBeingDeleted()) return;

            int logicalX, logicalY;
            m_grid->CalcUnscrolledPosition(event.GetX(), event.GetY(), &logicalX, &logicalY);
            
            const wxGridCellCoords currentCell = m_grid->XYToCell(logicalX, logicalY);
            if (m_lastHoveredCell != currentCell) {
                const String tooltip = m_table->tooltip(currentCell);
                m_grid->SetToolTip(tooltip);
                m_lastHoveredCell = currentCell;
            }
            event.Skip();
        }
        
        void EntityAttributeGrid::OnUpdateAttributeView(wxUpdateUIEvent& event) {
            if (IsBeingDeleted()) return;

            MapDocumentSPtr document = lock(m_document);
            event.Enable(!document->allSelectedAttributableNodes().empty());
        }

        void EntityAttributeGrid::OnAddAttributeButton(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            addAttribute();
        }

        void EntityAttributeGrid::OnRemovePropertiesButton(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            removeSelectedAttributes();
        }
        
        void EntityAttributeGrid::addAttribute() {
            m_grid->InsertRows(m_table->GetNumberAttributeRows());
            m_grid->SetFocus();
            const int row = m_table->GetNumberAttributeRows() - 1;
            m_grid->SelectRow(row);
            m_grid->GoToCell(row, 0);
            m_grid->ShowCellEditControl();
        }
        
        void EntityAttributeGrid::removeSelectedAttributes() {
            assert(canRemoveSelectedAttributes());
            
            int firstRowIndex = m_grid->GetNumberRows();
            wxArrayInt selectedRows = m_grid->GetSelectedRows();
            for (auto it = selectedRows.rbegin(), end = selectedRows.rend(); it != end; ++it) {
                m_grid->DeleteRows(*it, 1);
                firstRowIndex = std::min(*it, firstRowIndex);
            }
            
            if (firstRowIndex < m_grid->GetNumberRows())
                m_grid->SelectRow(firstRowIndex);
        }

        void EntityAttributeGrid::OnShowDefaultPropertiesCheckBox(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            m_table->setShowDefaultRows(event.IsChecked());
        }
        
        void EntityAttributeGrid::OnUpdateAddAttributeButton(wxUpdateUIEvent& event) {
            if (IsBeingDeleted()) return;

            MapDocumentSPtr document = lock(m_document);
            event.Enable(!document->allSelectedAttributableNodes().empty());
        }
        
        void EntityAttributeGrid::OnUpdateRemovePropertiesButton(wxUpdateUIEvent& event) {
            if (IsBeingDeleted()) return;

            event.Enable(!m_grid->GetSelectedRows().IsEmpty() && canRemoveSelectedAttributes());
        }

        void EntityAttributeGrid::OnUpdateShowDefaultPropertiesCheckBox(wxUpdateUIEvent& event) {
            if (IsBeingDeleted()) return;

            event.Check(m_table->showDefaultRows());
        }

        bool EntityAttributeGrid::canRemoveSelectedAttributes() const {
            for (const int rowIndex : m_grid->GetSelectedRows()) {
                if (!m_table->canRemove(rowIndex))
                    return false;
            }
            return true;
        }

        /**
         * Subclass of wxGridCellTextEditor for setting up autocompletion
         */
        class EntityAttributeCellEditor : public wxGridCellTextEditor
        {
        private:
            EntityAttributeGrid* m_grid;
            EntityAttributeGridTable* m_table;
            int m_row, m_col;
            
        public:
            EntityAttributeCellEditor(EntityAttributeGrid* grid, EntityAttributeGridTable* table)
            : m_grid(grid),
            m_table(table),
            m_row(-1),
            m_col(-1) {}

        private:
            void OnCharHook(wxKeyEvent& event) {
                if (event.GetKeyCode() == WXK_TAB) {
                    // HACK: Consume tab key and use it for cell navigation.
                    // Otherwise, wxTextCtrl::AutoComplete uses it for cycling between completions (on Windows)
                    m_grid->tabNavigate(m_row, m_col, !event.ShiftDown());
                } else {
                	event.Skip();
                }
            }

        public:
            void BeginEdit(int row, int col, wxGrid* grid) override {
                wxGridCellTextEditor::BeginEdit(row, col, grid);

                m_row = row;
                m_col = col;
                
                wxTextCtrl *textCtrl = Text();
                ensure(textCtrl != nullptr, "wxGridCellTextEditor::Create should have created control");

                textCtrl->Bind(wxEVT_CHAR_HOOK, &EntityAttributeCellEditor::OnCharHook, this);

                const wxArrayString completions = m_table->getCompletions(row, col);
                textCtrl->AutoComplete(completions);
            }
            
            bool EndEdit(int row, int col, const wxGrid* grid, const wxString& oldval, wxString *newval) override {
                wxTextCtrl *textCtrl = Text();
                ensure(textCtrl != nullptr, "wxGridCellTextEditor::Create should have created control");
                
                textCtrl->Unbind(wxEVT_CHAR_HOOK, &EntityAttributeCellEditor::OnCharHook, this);
                
                return wxGridCellTextEditor::EndEdit(row, col, grid, oldval, newval);
            }
        };
        
        void EntityAttributeGrid::createGui(MapDocumentWPtr document) {
            SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOX));
            
            m_table = new EntityAttributeGridTable(document);
            
            m_grid = new wxGrid(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE);
            m_grid->SetTable(m_table, true, wxGrid::wxGridSelectRows);
            // m_grid->SetUseNativeColLabels();
            // m_grid->UseNativeColHeader();
            m_grid->SetColLabelSize(18);
            m_grid->SetDefaultCellBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOX));
            m_grid->HideRowLabels();
            
            wxGridCellTextEditor* editor = new EntityAttributeCellEditor(this, m_table);
            m_grid->SetDefaultEditor(editor);
            
            m_grid->DisableColResize(0);
            m_grid->DisableColResize(1);
            m_grid->DisableDragColMove();
            m_grid->DisableDragCell();
            m_grid->DisableDragColSize();
            m_grid->DisableDragGridSize();
            m_grid->DisableDragRowSize();
            
            m_grid->Bind(wxEVT_SIZE, &EntityAttributeGrid::OnAttributeGridSize, this);
            m_grid->Bind(wxEVT_GRID_SELECT_CELL, &EntityAttributeGrid::OnAttributeGridSelectCell, this);
            m_grid->Bind(wxEVT_GRID_TABBING, &EntityAttributeGrid::OnAttributeGridTab, this);
            m_grid->Bind(wxEVT_KEY_DOWN, &EntityAttributeGrid::OnAttributeGridKeyDown, this);
            m_grid->Bind(wxEVT_KEY_UP, &EntityAttributeGrid::OnAttributeGridKeyUp, this);
            m_grid->GetGridWindow()->Bind(wxEVT_MOTION, &EntityAttributeGrid::OnAttributeGridMouseMove, this);
            m_grid->Bind(wxEVT_UPDATE_UI, &EntityAttributeGrid::OnUpdateAttributeView, this);
            
            wxWindow* addAttributeButton = createBitmapButton(this, "Add.png", "Add a new property");
            wxWindow* removePropertiesButton = createBitmapButton(this, "Remove.png", "Remove the selected properties");

            addAttributeButton->Bind(wxEVT_BUTTON, &EntityAttributeGrid::OnAddAttributeButton, this);
            addAttributeButton->Bind(wxEVT_UPDATE_UI, &EntityAttributeGrid::OnUpdateAddAttributeButton, this);
            removePropertiesButton->Bind(wxEVT_BUTTON, &EntityAttributeGrid::OnRemovePropertiesButton, this);
            removePropertiesButton->Bind(wxEVT_UPDATE_UI, &EntityAttributeGrid::OnUpdateRemovePropertiesButton, this);

            wxCheckBox* showDefaultPropertiesCheckBox = new wxCheckBox(this, wxID_ANY, "Show default properties");
            showDefaultPropertiesCheckBox->Bind(wxEVT_CHECKBOX, &EntityAttributeGrid::OnShowDefaultPropertiesCheckBox, this);
            showDefaultPropertiesCheckBox->Bind(wxEVT_UPDATE_UI, &EntityAttributeGrid::OnUpdateShowDefaultPropertiesCheckBox, this);
            
            wxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
            buttonSizer->Add(addAttributeButton, 0, wxALIGN_CENTER_VERTICAL | wxTOP | wxBOTTOM, LayoutConstants::NarrowVMargin);
            buttonSizer->Add(removePropertiesButton, 0, wxALIGN_CENTER_VERTICAL | wxTOP | wxBOTTOM, LayoutConstants::NarrowVMargin);
            buttonSizer->AddSpacer(LayoutConstants::WideHMargin);
            buttonSizer->Add(showDefaultPropertiesCheckBox, 0, wxALIGN_CENTER_VERTICAL | wxTOP | wxBOTTOM, LayoutConstants::NarrowVMargin);
            buttonSizer->AddStretchSpacer();
            
            wxSizer* sizer = new wxBoxSizer(wxVERTICAL);
            sizer->Add(m_grid, 1, wxEXPAND);
            sizer->Add(buttonSizer, 0, wxEXPAND);
            SetSizer(sizer);
        }
        
        void EntityAttributeGrid::bindObservers() {
            MapDocumentSPtr document = lock(m_document);
            document->documentWasNewedNotifier.addObserver(this, &EntityAttributeGrid::documentWasNewed);
            document->documentWasLoadedNotifier.addObserver(this, &EntityAttributeGrid::documentWasLoaded);
            document->nodesDidChangeNotifier.addObserver(this, &EntityAttributeGrid::nodesDidChange);
            document->selectionWillChangeNotifier.addObserver(this, &EntityAttributeGrid::selectionWillChange);
            document->selectionDidChangeNotifier.addObserver(this, &EntityAttributeGrid::selectionDidChange);
        }
        
        void EntityAttributeGrid::unbindObservers() {
            if (!expired(m_document)) {
                MapDocumentSPtr document = lock(m_document);
                document->documentWasNewedNotifier.removeObserver(this, &EntityAttributeGrid::documentWasNewed);
                document->documentWasLoadedNotifier.removeObserver(this, &EntityAttributeGrid::documentWasLoaded);
                document->nodesDidChangeNotifier.removeObserver(this, &EntityAttributeGrid::nodesDidChange);
                document->selectionWillChangeNotifier.removeObserver(this, &EntityAttributeGrid::selectionWillChange);
                document->selectionDidChangeNotifier.removeObserver(this, &EntityAttributeGrid::selectionDidChange);
            }
        }
        
        void EntityAttributeGrid::documentWasNewed(MapDocument* document) {
            updateControls();
        }
        
        void EntityAttributeGrid::documentWasLoaded(MapDocument* document) {
            updateControls();
        }
        
        void EntityAttributeGrid::nodesDidChange(const Model::NodeList& nodes) {
            updateControls();
        }
        
        void EntityAttributeGrid::selectionWillChange() {
            m_grid->SaveEditControlValue();
            m_grid->HideCellEditControl();
        }
        
        void EntityAttributeGrid::selectionDidChange(const Selection& selection) {
            const SetBool ignoreSelection(m_ignoreSelection);
            updateControls();
        }

        void EntityAttributeGrid::updateControls() {
            wxGridUpdateLocker lockGrid(m_grid);
            m_table->update();
            
            int row = m_table->rowForName(m_lastSelectedName);
            if (row >= m_table->GetNumberRows())
                row = m_table->GetNumberRows() - 1;
            if (row == -1 && m_table->GetNumberRows() > 0)
                row = 0;
            
            if (row != -1) {
                // 1981: Ensure that we make a cell visible only if it is completely invisible.
                // The goal is to block the grid from redrawing itself every time this function
                // is called.
                if (!m_grid->IsVisible(row, m_lastSelectedCol, false)) {
                    m_grid->MakeCellVisible(row, m_lastSelectedCol);
                }
                if (m_grid->GetGridCursorRow() != row || m_grid->GetGridCursorCol() != m_lastSelectedCol) {
                    m_grid->SetGridCursor(row, m_lastSelectedCol);
                }
                if (!m_grid->IsInSelection(row, m_lastSelectedCol)) {
                    m_grid->SelectRow(row);
                }
            } else {
                fireSelectionEvent(row, m_lastSelectedCol);
            }
        }
        
        Model::AttributeName EntityAttributeGrid::selectedRowName() const {
            wxArrayInt selectedRows = m_grid->GetSelectedRows();
            if (selectedRows.empty())
                return "";
            const int row = selectedRows.front();
            return m_table->attributeName(row);
        }
    }
}
