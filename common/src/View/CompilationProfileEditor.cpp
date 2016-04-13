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

#include <wx/gbsizer.h>
#include <wx/menu.h>
#include <wx/settings.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>

namespace TrenchBroom {
    namespace View {

        CompilationProfileEditor::CompilationProfileEditor(wxWindow* parent) :
        wxPanel(parent),
        m_profile(NULL),
        m_nameTxt(NULL),
        m_workDirTxt(NULL),
        m_taskList(NULL) {
            SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOX));

            wxPanel* upperPanel = new wxPanel(this);
            upperPanel->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_FRAMEBK));
            
            wxStaticText* nameLabel = new wxStaticText(upperPanel, wxID_ANY, "Name");
            wxStaticText* workDirLabel = new wxStaticText(upperPanel, wxID_ANY, "Working Directory");
            
            m_nameTxt = new wxTextCtrl(upperPanel, wxID_ANY);
            m_workDirTxt = new wxTextCtrl(upperPanel, wxID_ANY);
            
            m_nameTxt->Bind(wxEVT_TEXT, &CompilationProfileEditor::OnNameChanged, this);
            m_workDirTxt->Bind(wxEVT_TEXT, &CompilationProfileEditor::OnWorkDirChanged, this);
            m_nameTxt->Bind(wxEVT_UPDATE_UI, &CompilationProfileEditor::OnUpdateTxtUI, this);
            m_workDirTxt->Bind(wxEVT_UPDATE_UI, &CompilationProfileEditor::OnUpdateTxtUI, this);
            
            const int LabelFlags   = wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxRIGHT;
            const int EditorFlags  = wxALIGN_CENTER_VERTICAL | wxEXPAND;
            const int LabelMargin  = LayoutConstants::NarrowHMargin;
            
            wxGridBagSizer* upperInnerSizer = new wxGridBagSizer(LayoutConstants::NarrowVMargin);
            upperInnerSizer->Add(nameLabel,      wxGBPosition(0, 0), wxDefaultSpan, LabelFlags, LabelMargin);
            upperInnerSizer->Add(m_nameTxt,      wxGBPosition(0, 1), wxDefaultSpan, EditorFlags);
            upperInnerSizer->Add(workDirLabel,   wxGBPosition(1, 0), wxDefaultSpan, LabelFlags, LabelMargin);
            upperInnerSizer->Add(m_workDirTxt,   wxGBPosition(1, 1), wxDefaultSpan, EditorFlags);
            upperInnerSizer->AddGrowableCol(1);
            
            wxSizer* upperOuterSizer = new wxBoxSizer(wxVERTICAL);
            upperOuterSizer->AddSpacer(LayoutConstants::WideVMargin);
            upperOuterSizer->Add(upperInnerSizer, 0, wxEXPAND | wxLEFT | wxRIGHT, LayoutConstants::MediumHMargin);
            upperOuterSizer->AddSpacer(LayoutConstants::WideVMargin);
            
            upperPanel->SetSizer(upperOuterSizer);
            
            m_taskList = new CompilationTaskList(this);
            
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
            sizer->Add(upperPanel, 0, wxEXPAND);
            sizer->Add(new BorderLine(this, BorderLine::Direction_Horizontal), 0, wxEXPAND);
            sizer->Add(m_taskList, 1, wxEXPAND);
            sizer->Add(new BorderLine(this, BorderLine::Direction_Horizontal), 0, wxEXPAND);
            sizer->Add(buttonSizer, 0, wxEXPAND);
            SetSizer(sizer);
        }
        
        CompilationProfileEditor::~CompilationProfileEditor() {
            if (m_profile != NULL)
                m_profile->profileDidChange.addObserver(this, &CompilationProfileEditor::profileDidChange);
        }

        void CompilationProfileEditor::OnNameChanged(wxCommandEvent& event) {
            assert(m_profile != NULL);
            m_profile->setName(m_nameTxt->GetValue().ToStdString());
        }
        
        void CompilationProfileEditor::OnWorkDirChanged(wxCommandEvent& event) {
            assert(m_profile != NULL);
            m_profile->setWorkDirSpec(m_workDirTxt->GetValue().ToStdString());
        }

        void CompilationProfileEditor::OnUpdateTxtUI(wxUpdateUIEvent& event) {
            event.Enable(m_profile != NULL);
        }

        void CompilationProfileEditor::OnAddTask(wxCommandEvent& event) {
            wxMenu menu;
            menu.Append(1, "Copy Files");
            menu.Append(2, "Run Tool");
            const int result = GetPopupMenuSelectionFromUser(menu);
            
            Model::CompilationTask* task = NULL;
            switch (result) {
                case 1:
                    task = new Model::CompilationCopyFiles("", "");
                    break;
                case 2:
                    task = new Model::CompilationRunTool("", "");
                    break;
                default:
                    return;
            }
            
            assert(task != NULL);
            const int index = m_taskList->GetSelection();
            if (index == wxNOT_FOUND) {
                m_profile->addTask(task);
                m_taskList->SetSelection(static_cast<int>(m_profile->taskCount()) - 1);
            } else {
                m_profile->insertTask(static_cast<size_t>(index), task);
                m_taskList->SetSelection(index);
            }
        }
        
        void CompilationProfileEditor::OnRemoveTask(wxCommandEvent& event) {
            const int index = m_taskList->GetSelection();
            assert(index != wxNOT_FOUND);
            m_profile->removeTask(static_cast<size_t>(index));
        }
        
        void CompilationProfileEditor::OnMoveTaskUp(wxCommandEvent& event) {
            const int index = m_taskList->GetSelection();
            assert(index != wxNOT_FOUND);
            m_profile->moveTaskUp(static_cast<size_t>(index));
            m_taskList->SetSelection(index - 1);
        }
        
        void CompilationProfileEditor::OnMoveTaskDown(wxCommandEvent& event) {
            const int index = m_taskList->GetSelection();
            assert(index != wxNOT_FOUND);
            m_profile->moveTaskDown(static_cast<size_t>(index));
            m_taskList->SetSelection(index + 1);
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
            event.Enable(m_profile != NULL && m_taskList->GetSelection() != wxNOT_FOUND && static_cast<size_t>(m_taskList->GetSelection()) < m_profile->taskCount() - 1);
        }

        void CompilationProfileEditor::setProfile(Model::CompilationProfile* profile) {
            if (m_profile != NULL) {
                m_profile->profileWillBeDeleted.removeObserver(this, &CompilationProfileEditor::profileWillBeDeleted);
                m_profile->profileDidChange.removeObserver(this, &CompilationProfileEditor::profileDidChange);
            }
            m_profile = profile;
            m_taskList->setProfile(profile);
            if (m_profile != NULL) {
                m_profile->profileWillBeDeleted.addObserver(this, &CompilationProfileEditor::profileWillBeDeleted);
                m_profile->profileDidChange.addObserver(this, &CompilationProfileEditor::profileDidChange);
            }
            refresh();
        }

        void CompilationProfileEditor::profileWillBeDeleted() {
            setProfile(NULL);
        }

        void CompilationProfileEditor::profileDidChange() {
            refresh();
        }
        
        void CompilationProfileEditor::refresh() {
            if (m_profile != NULL) {
                m_nameTxt->ChangeValue(m_profile->name());
                m_workDirTxt->ChangeValue(m_profile->workDirSpec());
            } else {
                m_nameTxt->ChangeValue("");
                m_workDirTxt->ChangeValue("");
            }
        }
    }
}
