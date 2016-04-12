/*
 Copyright (C) 2010-2016 Kristian Duske
 
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

#include "CompilationProfileEditor.h"

#include "Model/CompilationProfile.h"
#include "View/BorderLine.h"
#include "View/CompilationTaskList.h"
#include "View/ViewConstants.h"
#include "View/wxUtils.h"

#include <wx/menu.h>
#include <wx/settings.h>
#include <wx/sizer.h>

namespace TrenchBroom {
    namespace View {

        CompilationProfileEditor::CompilationProfileEditor(wxWindow* parent) :
        wxPanel(parent),
        m_profile(NULL),
        m_taskList(new CompilationTaskList(this)) {
            SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOX));
            
            wxWindow* addTaskButton = createBitmapButton(this, "Add.png", "Add task");
            wxWindow* removeTaskButton = createBitmapButton(this, "Remove.png", "Remove the selected task");
            wxWindow* moveTaskUpButton = createBitmapButton(this, "Up.png", "Move the selected task up");
            wxWindow* moveTaskDownButton = createBitmapButton(this, "Down.png", "Move the selected task down");
            
            addTaskButton->Bind(wxEVT_BUTTON, &CompilationProfileEditor::OnAddTask, this);
            removeTaskButton->Bind(wxEVT_BUTTON, &CompilationProfileEditor::OnRemoveTask, this);
            moveTaskUpButton->Bind(wxEVT_BUTTON, &CompilationProfileEditor::OnMoveTaskUp, this);
            moveTaskDownButton->Bind(wxEVT_BUTTON, &CompilationProfileEditor::OnMoveTaskDown, this);
            addTaskButton->Bind(wxEVT_UPDATE_UI, &CompilationProfileEditor::OnUpdateAddTaskButtonUI, this);
            removeTaskButton->Bind(wxEVT_UPDATE_UI, &CompilationProfileEditor::OnUpdateRemoveTaskButtonUI, this);
            moveTaskUpButton->Bind(wxEVT_UPDATE_UI, &CompilationProfileEditor::OnUpdateMoveTaskUpButtonUI, this);
            moveTaskDownButton->Bind(wxEVT_UPDATE_UI, &CompilationProfileEditor::OnUpdateMoveTaskDownButtonUI, this);
            
            wxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
            buttonSizer->Add(addTaskButton, 0, wxALIGN_CENTER_VERTICAL | wxTOP | wxBOTTOM, LayoutConstants::NarrowVMargin);
            buttonSizer->Add(removeTaskButton, 0, wxALIGN_CENTER_VERTICAL | wxTOP | wxBOTTOM, LayoutConstants::NarrowVMargin);
            buttonSizer->AddSpacer(LayoutConstants::WideHMargin);
            buttonSizer->Add(moveTaskUpButton, 0, wxALIGN_CENTER_VERTICAL | wxTOP | wxBOTTOM, LayoutConstants::NarrowVMargin);
            buttonSizer->Add(moveTaskDownButton, 0, wxALIGN_CENTER_VERTICAL | wxTOP | wxBOTTOM, LayoutConstants::NarrowVMargin);
            buttonSizer->AddStretchSpacer();
            
            wxSizer* sizer = new wxBoxSizer(wxVERTICAL);
            sizer->Add(m_taskList, 1, wxEXPAND);
            sizer->Add(new BorderLine(this, BorderLine::Direction_Horizontal), 0, wxEXPAND);
            sizer->Add(buttonSizer, 0, wxEXPAND);
            SetSizer(sizer);
        }
        
        CompilationProfileEditor::~CompilationProfileEditor() {
            if (m_profile != NULL)
                m_profile->profileDidChange.addObserver(this, &CompilationProfileEditor::profileDidChange);
        }

        void CompilationProfileEditor::OnAddTask(wxCommandEvent& event) {
            wxMenu menu;
            menu.Append(1, "Copy Files");
            menu.Append(2, "Run Tool");
            const int result = GetPopupMenuSelectionFromUser(menu);
            switch (result) {
                case 1:
                    break;
                case 2:
                    break;
                default:
                    break;
            }
        }
        
        void CompilationProfileEditor::OnRemoveTask(wxCommandEvent& event) {
        }
        
        void CompilationProfileEditor::OnMoveTaskUp(wxCommandEvent& event) {
        }
        
        void CompilationProfileEditor::OnMoveTaskDown(wxCommandEvent& event) {
        }
        
        void CompilationProfileEditor::OnUpdateAddTaskButtonUI(wxUpdateUIEvent& event) {
            event.Enable(m_profile != NULL);
        }
        
        void CompilationProfileEditor::OnUpdateRemoveTaskButtonUI(wxUpdateUIEvent& event) {
            event.Enable(m_profile != NULL && m_taskList->GetSelection() != wxNOT_FOUND);
        }
        
        void CompilationProfileEditor::OnUpdateMoveTaskUpButtonUI(wxUpdateUIEvent& event) {
            event.Enable(m_profile != NULL && m_taskList->GetSelection() != wxNOT_FOUND && m_taskList->GetSelection() > 0);
        }
        
        void CompilationProfileEditor::OnUpdateMoveTaskDownButtonUI(wxUpdateUIEvent& event) {
            event.Enable(m_profile != NULL && m_taskList->GetSelection() != wxNOT_FOUND && static_cast<size_t>(m_taskList->GetSelection()) > m_profile->taskCount());
        }

        void CompilationProfileEditor::setProfile(Model::CompilationProfile* profile) {
            if (m_profile != NULL)
                m_profile->profileDidChange.removeObserver(this, &CompilationProfileEditor::profileDidChange);
            m_profile = profile;
            m_taskList->setProfile(profile);
            if (m_profile != NULL)
                m_profile->profileDidChange.addObserver(this, &CompilationProfileEditor::profileDidChange);
        }

        void CompilationProfileEditor::profileDidChange() {
            refresh();
        }
        
        void CompilationProfileEditor::refresh() {
        }
    }
}
