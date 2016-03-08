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
#include "View/KeyboardShortcutGridTable.h"
#include "View/ViewConstants.h"

#include <wx/msgdlg.h>
#include <wx/sizer.h>
#include <wx/statbox.h>
#include <wx/stattext.h>

#include <cassert>

namespace TrenchBroom {
    namespace View {
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
            if (IsBeingDeleted()) return;

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

            m_table = new KeyboardShortcutGridTable();
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
        
        bool KeyboardPreferencePane::doCanResetToDefaults() {
            return true;
        }
        
        void KeyboardPreferencePane::doResetToDefaults() {
            ActionManager& actionManager = ActionManager::instance();
            actionManager.resetShortcutsToDefaults();
        }

        void KeyboardPreferencePane::doUpdateControls() {
            m_table->update();
        }
        
        bool KeyboardPreferencePane::doValidate() {
            m_grid->SaveEditControlValue();
            if (m_table->hasDuplicates()) {
                wxMessageBox("Please fix all conflicting shortcuts (highlighted in red).", "Error", wxOK | wxCENTRE, this);
                return false;
            }
            return true;
        }
    }
}
