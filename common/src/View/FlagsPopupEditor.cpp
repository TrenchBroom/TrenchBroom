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
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#include "FlagsPopupEditor.h"
#include "View/FlagsEditor.h"
#include "View/FlagChangedCommand.h"
#include "View/ViewConstants.h"
#include "View/PopupButton.h"

#include <wx/settings.h>
#include <wx/sizer.h>
#include <wx/stattext.h>

namespace TrenchBroom {
    namespace View {
        FlagsPopupEditor::FlagsPopupEditor(wxWindow* parent, const size_t numCols, const wxString& buttonLabel , const bool showFlagsText) :
        wxPanel(parent),
        m_flagsTxt(NULL),
        m_button(NULL),
        m_editor(NULL) {
            wxPanel* flagsPanel = NULL;
            if (showFlagsText) {
                flagsPanel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_SUNKEN);
                flagsPanel->SetBackgroundColour(*wxWHITE);
                
                m_flagsTxt = new wxStaticText(flagsPanel, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT | wxST_ELLIPSIZE_END);
                
                wxSizer* flagsPanelSizer = new wxBoxSizer(wxVERTICAL);
                flagsPanelSizer->AddStretchSpacer();
                flagsPanelSizer->Add(m_flagsTxt, 0, wxEXPAND | wxLEFT | wxRIGHT, LayoutConstants::TextBoxInnerMargin);
                flagsPanelSizer->AddStretchSpacer();
                flagsPanel->SetSizer(flagsPanelSizer);
            }
            
            m_button = new PopupButton(this, buttonLabel);
            m_button->SetToolTip("Click to edit flags");
            
            wxPanel* editorContainer = new wxPanel(m_button->GetPopupWindow(), wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_SIMPLE);
            m_editor = new FlagsEditor(editorContainer, numCols);
            
            wxSizer* editorContainerSizer = new wxBoxSizer(wxVERTICAL);
            editorContainerSizer->Add(m_editor, 1, wxEXPAND | wxALL, LayoutConstants::DialogOuterMargin);
            editorContainer->SetSizer(editorContainerSizer);
            
            wxSizer* popupSizer = new wxBoxSizer(wxVERTICAL);
            popupSizer->Add(editorContainer, 1, wxEXPAND);
            m_button->GetPopupWindow()->SetSizerAndFit(popupSizer);
            
            wxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
            if (flagsPanel != NULL) {
                sizer->Add(flagsPanel, 1, wxEXPAND);
                sizer->AddSpacer(LayoutConstants::MediumHMargin);
            }
            sizer->Add(m_button, 0, wxALIGN_CENTER_VERTICAL);
            SetSizerAndFit(sizer);
            
            m_editor->Bind(FLAG_CHANGED_EVENT, &FlagsPopupEditor::OnFlagChanged, this);
        }

        void FlagsPopupEditor::setFlags(const wxArrayString& labels, const wxArrayString& tooltips) {
            m_editor->setFlags(labels, tooltips);
            m_button->GetPopupWindow()->Fit();
            updateFlagsText();
        }
        
        void FlagsPopupEditor::setFlags(const wxArrayInt& values, const wxArrayString& labels, const wxArrayString& tooltips) {
            m_editor->setFlags(values, labels, tooltips);
            m_button->GetPopupWindow()->Fit();
            updateFlagsText();
        }

        void FlagsPopupEditor::setFlagValue(const int set, const int mixed) {
            m_editor->setFlagValue(set, mixed);
            updateFlagsText();
        }

        void FlagsPopupEditor::OnFlagChanged(FlagChangedCommand& event) {
            if (IsBeingDeleted()) return;

            updateFlagsText();
            ProcessEvent(event);
        }

        bool FlagsPopupEditor::Enable(bool enable) {
            if (wxPanel::Enable(enable)) {
                m_button->Enable(enable);
                updateFlagsText();
                return true;
            }
            return false;
        }
        
        void FlagsPopupEditor::updateFlagsText() {
            if (m_flagsTxt == NULL)
                return;
            
            if (!IsEnabled()) {
                m_flagsTxt->SetForegroundColour(Colors::disabledText());
                m_flagsTxt->SetLabel("n/a");
                m_flagsTxt->UnsetToolTip();
                return;
            }
            
            wxString label;
            bool first = true;
            bool mixed = false;
            for (size_t i = 0; i < m_editor->getNumFlags() && !mixed; ++i) {
                if (m_editor->isFlagMixed(i)) {
                    label = "multi";
                    mixed = true;
                } else if (m_editor->isFlagSet(i)) {
                    if (!first)
                        label << ", ";
                    label << m_editor->getFlagLabel(i);
                    first = false;
                }
            }
            
            m_flagsTxt->SetLabel(label);
            if (!first)
                m_flagsTxt->SetToolTip(label);
            else
                m_flagsTxt->UnsetToolTip();
            
            if (mixed)
                m_flagsTxt->SetForegroundColour(Colors::disabledText());
            else
                m_flagsTxt->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNTEXT));
        }
    }
}
