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
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#include "FlagsPopupEditor.h"
#include "View/FlagsEditor.h"
#include "View/FlagChangedCommand.h"
#include "View/LayoutConstants.h"
#include "View/PopupButton.h"

#include <wx/sizer.h>
#include <wx/stattext.h>

namespace TrenchBroom {
    namespace View {
        FlagsPopupEditor::FlagsPopupEditor(wxWindow* parent, const wxString& label, const size_t numCols) :
        wxPanel(parent) {
            wxStaticText* labelTxt = new wxStaticText(this, wxID_ANY, label);
            m_flagsTxt = new wxStaticText(this, wxID_ANY, _("none"), wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT | wxST_ELLIPSIZE_END);
            m_flagsTxt->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));
            m_button = new PopupButton(this, _("..."), PopupButton::Right);
            
            wxPanel* editorContainer = new wxPanel(m_button->GetPopupWindow(), wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_SIMPLE);
            m_editor = new FlagsEditor(editorContainer, numCols);
            
            wxSizer* editorContainerSizer = new wxBoxSizer(wxVERTICAL);
            editorContainerSizer->Add(m_editor, 1, wxEXPAND | wxALL, LayoutConstants::NotebookPageInnerMargin);
            editorContainer->SetSizer(editorContainerSizer);
            
            wxSizer* popupSizer = new wxBoxSizer(wxVERTICAL);
            popupSizer->Add(editorContainer, 1, wxEXPAND);
            m_button->GetPopupWindow()->SetSizerAndFit(popupSizer);
            
            wxSizer* flagsTxtSizer = new wxBoxSizer(wxVERTICAL);
            flagsTxtSizer->AddStretchSpacer();
            flagsTxtSizer->Add(m_flagsTxt, 0, wxEXPAND);
            flagsTxtSizer->AddStretchSpacer();
            
            wxFlexGridSizer* sizer = new wxFlexGridSizer(1, 4, 0, 0);
            sizer->Add(labelTxt, 0, wxALIGN_CENTER_VERTICAL);
            sizer->Add(flagsTxtSizer, 0, wxEXPAND);
            sizer->AddSpacer(LayoutConstants::ControlHorizontalMargin);
            sizer->Add(m_button, 0, wxALIGN_CENTER_VERTICAL);
            sizer->AddGrowableCol(1);
            SetSizerAndFit(sizer);
            
            m_editor->Bind(EVT_FLAG_CHANGED_EVENT, EVT_FLAG_CHANGED_HANDLER(FlagsPopupEditor::OnFlagChanged), this);
        }

        void FlagsPopupEditor::setFlags(const wxArrayString& labels, const wxArrayString& tooltips) {
            m_editor->setFlags(labels, tooltips);
            m_button->GetPopupWindow()->Fit();
            updateFlagsText();
        }
        
        void FlagsPopupEditor::setFlagValue(const int set, const int mixed) {
            m_editor->setFlagValue(set, mixed);
            updateFlagsText();
        }

        void FlagsPopupEditor::OnFlagChanged(FlagChangedCommand& event) {
            updateFlagsText();
            ProcessEvent(event);
        }

        void FlagsPopupEditor::updateFlagsText() {
            wxString label;
            bool first = true;
            for (size_t i = 0; i < m_editor->getNumFlags(); ++i) {
                if (m_editor->isFlagSet(i)) {
                    if (!first)
                        label << ", ";
                    label << m_editor->getFlagLabel(i);
                    first = false;
                }
            }
            
            if (first) {
                label = "none";
                m_flagsTxt->SetForegroundColour(wxColour(100, 100, 100));
            } else {
                m_flagsTxt->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNTEXT));
            }
            
            m_flagsTxt->SetLabel(label);
        }
    }
}
