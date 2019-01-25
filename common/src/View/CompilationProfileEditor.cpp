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

#include "CompilationProfileEditor.h"

#include "Model/CompilationProfile.h"
#include "View/AutoCompleteTextControl.h"
#include "View/ELAutoCompleteHelper.h"
#include "View/BorderLine.h"
#include "View/CompilationTaskList.h"
#include "View/CompilationVariables.h"
#include "View/ViewConstants.h"
#include "View/wxUtils.h"

#include <wx/gbsizer.h>
#include <wx/menu.h>
#include <wx/settings.h>
#include <wx/sizer.h>
#include <wx/simplebook.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>

namespace TrenchBroom {
    namespace View {
        CompilationProfileEditor::CompilationProfileEditor(QWidget* parent, MapDocumentWPtr document) :
        QWidget(parent),
        m_document(document),
        m_profile(nullptr),
        m_book(nullptr),
        m_nameTxt(nullptr),
        m_workDirTxt(nullptr),
        m_taskList(nullptr) {
            SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOX));

            m_book = new wxSimplebook(this);
            m_book->AddPage(createDefaultPage(m_book, "Select a compilation profile."), "Default");
            m_book->AddPage(createEditorPage(m_book), "Editor");
            m_book->SetSelection(0);
            
            wxSizer* bookSizer = new wxBoxSizer(wxVERTICAL);
            bookSizer->Add(m_book, wxSizerFlags().Expand().Proportion(1));
            SetSizer(bookSizer);
        }
        
        CompilationProfileEditor::~CompilationProfileEditor() {
            if (m_profile != nullptr) {
                m_profile->profileWillBeRemoved.removeObserver(this, &CompilationProfileEditor::profileWillBeRemoved);
                m_profile->profileDidChange.removeObserver(this, &CompilationProfileEditor::profileDidChange);
            }
        }
        
        QWidget* CompilationProfileEditor::createEditorPage(QWidget* parent) {
            QWidget* containerPanel = new QWidget(parent);
            containerPanel->SetBackgroundColour(GetBackgroundColour());
            
            QWidget* upperPanel = new QWidget(containerPanel);
            upperPanel->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_FRAMEBK));
            
            QLabel* nameLabel = new QLabel(upperPanel, wxID_ANY, "Name");
            QLabel* workDirLabel = new QLabel(upperPanel, wxID_ANY, "Working Directory");
            
            m_nameTxt = new wxTextCtrl(upperPanel, wxID_ANY);
            m_workDirTxt = new AutoCompleteTextControl(upperPanel, wxID_ANY);
            
            CompilationWorkDirVariables workDirVariables(lock(m_document));
            m_workDirTxt->SetHelper(new ELAutoCompleteHelper(workDirVariables));
            
            m_nameTxt->Bind(wxEVT_TEXT, &CompilationProfileEditor::OnNameChanged, this);
            m_workDirTxt->Bind(wxEVT_TEXT, &CompilationProfileEditor::OnWorkDirChanged, this);
            
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
            
            m_taskList = new CompilationTaskList(containerPanel, m_document);
            
            QWidget* addTaskButton = createBitmapButton(containerPanel, "Add.png", "Add task");
            QWidget* removeTaskButton = createBitmapButton(containerPanel, "Remove.png", "Remove the selected task");
            QWidget* moveTaskUpButton = createBitmapButton(containerPanel, "Up.png", "Move the selected task up");
            QWidget* moveTaskDownButton = createBitmapButton(containerPanel, "Down.png", "Move the selected task down");
            
            addTaskButton->Bind(wxEVT_BUTTON, &CompilationProfileEditor::OnAddTask, this);
            removeTaskButton->Bind(wxEVT_BUTTON, &CompilationProfileEditor::OnRemoveTask, this);
            moveTaskUpButton->Bind(wxEVT_BUTTON, &CompilationProfileEditor::OnMoveTaskUp, this);
            moveTaskDownButton->Bind(wxEVT_BUTTON, &CompilationProfileEditor::OnMoveTaskDown, this);
            addTaskButton->Bind(wxEVT_UPDATE_UI, &CompilationProfileEditor::OnUpdateAddTaskButtonUI, this);
            removeTaskButton->Bind(wxEVT_UPDATE_UI, &CompilationProfileEditor::OnUpdateRemoveTaskButtonUI, this);
            moveTaskUpButton->Bind(wxEVT_UPDATE_UI, &CompilationProfileEditor::OnUpdateMoveTaskUpButtonUI, this);
            moveTaskDownButton->Bind(wxEVT_UPDATE_UI, &CompilationProfileEditor::OnUpdateMoveTaskDownButtonUI, this);
            
            wxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
            const wxSizerFlags buttonFlags = wxSizerFlags().CenterVertical().Border(wxTOP | wxBOTTOM, LayoutConstants::NarrowVMargin);
            buttonSizer->Add(addTaskButton, buttonFlags);
            buttonSizer->Add(removeTaskButton, buttonFlags);
            buttonSizer->AddSpacer(LayoutConstants::WideHMargin);
            buttonSizer->Add(moveTaskUpButton, buttonFlags);
            buttonSizer->Add(moveTaskDownButton, buttonFlags);
            buttonSizer->AddStretchSpacer();
            
            wxSizer* sizer = new wxBoxSizer(wxVERTICAL);
            sizer->Add(upperPanel, wxSizerFlags().Expand());
            sizer->Add(new BorderLine(containerPanel, BorderLine::Direction_Horizontal), wxSizerFlags().Expand());
            sizer->Add(m_taskList, wxSizerFlags().Expand().Proportion(1));
            sizer->Add(new BorderLine(containerPanel, BorderLine::Direction_Horizontal), wxSizerFlags().Expand());
            sizer->Add(buttonSizer, wxSizerFlags().Expand());
            
            containerPanel->SetSizer(sizer);
            return containerPanel;
        }

        void CompilationProfileEditor::OnNameChanged(wxCommandEvent& event) {
            ensure(m_profile != nullptr, "profile is null");
            m_profile->setName(m_nameTxt->GetValue().ToStdString());
        }
        
        void CompilationProfileEditor::OnWorkDirChanged(wxCommandEvent& event) {
            ensure(m_profile != nullptr, "profile is null");
            m_profile->setWorkDirSpec(m_workDirTxt->GetValue().ToStdString());
        }

        void CompilationProfileEditor::OnAddTask(wxCommandEvent& event) {
            wxMenu menu;
            menu.Append(1, "Export Map");
            menu.Append(2, "Copy Files");
            menu.Append(3, "Run Tool");
            const int result = GetPopupMenuSelectionFromUser(menu);
            
            Model::CompilationTask* task = nullptr;
            switch (result) {
                case 1:
                    task = new Model::CompilationExportMap("${WORK_DIR_PATH}/${MAP_BASE_NAME}-compile.map");
                    break;
                case 2:
                    task = new Model::CompilationCopyFiles("", "");
                    break;
                case 3:
                    task = new Model::CompilationRunTool("", "");
                    break;
                default:
                    return;
            }
            
            ensure(task != nullptr, "task is null");
            const int index = m_taskList->GetSelection();
            if (index == wxNOT_FOUND) {
                m_profile->addTask(task);
                m_taskList->SetSelection(static_cast<int>(m_profile->taskCount()) - 1);
            } else {
                m_profile->insertTask(static_cast<size_t>(index + 1), task);
                m_taskList->SetSelection(index + 1);
            }
        }
        
        void CompilationProfileEditor::OnRemoveTask(wxCommandEvent& event) {
            const int index = m_taskList->GetSelection();
            assert(index != wxNOT_FOUND);
            
            if (m_profile->taskCount() == 1) {
                m_taskList->SetSelection(wxNOT_FOUND);
                m_profile->removeTask(static_cast<size_t>(index));
            } else if (index > 0) {
                m_taskList->SetSelection(index - 1);
                m_profile->removeTask(static_cast<size_t>(index));
            } else {
                m_taskList->SetSelection(1);
                m_profile->removeTask(static_cast<size_t>(index));
                m_taskList->SetSelection(0);
            }
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
            event.Enable(m_profile != nullptr);
        }
        
        void CompilationProfileEditor::OnUpdateRemoveTaskButtonUI(wxUpdateUIEvent& event) {
            event.Enable(m_profile != nullptr && m_taskList->GetSelection() != wxNOT_FOUND);
        }
        
        void CompilationProfileEditor::OnUpdateMoveTaskUpButtonUI(wxUpdateUIEvent& event) {
            event.Enable(m_profile != nullptr && m_taskList->GetSelection() != wxNOT_FOUND && m_taskList->GetSelection() > 0);
        }
        
        void CompilationProfileEditor::OnUpdateMoveTaskDownButtonUI(wxUpdateUIEvent& event) {
            event.Enable(m_profile != nullptr && m_taskList->GetSelection() != wxNOT_FOUND && static_cast<size_t>(m_taskList->GetSelection()) < m_profile->taskCount() - 1);
        }

        void CompilationProfileEditor::setProfile(Model::CompilationProfile* profile) {
            if (m_profile != nullptr) {
                m_profile->profileWillBeRemoved.removeObserver(this, &CompilationProfileEditor::profileWillBeRemoved);
                m_profile->profileDidChange.removeObserver(this, &CompilationProfileEditor::profileDidChange);
            }
            m_profile = profile;
            m_taskList->setProfile(profile);
            if (m_profile != nullptr) {
                m_profile->profileWillBeRemoved.addObserver(this, &CompilationProfileEditor::profileWillBeRemoved);
                m_profile->profileDidChange.addObserver(this, &CompilationProfileEditor::profileDidChange);
                m_book->SetSelection(1);
            } else {
                m_book->SetSelection(0);
            }
            refresh();
        }

        void CompilationProfileEditor::profileWillBeRemoved() {
            setProfile(nullptr);
        }

        void CompilationProfileEditor::profileDidChange() {
            refresh();
        }
        
        void CompilationProfileEditor::refresh() {
            if (m_profile != nullptr) {
                if (m_nameTxt->GetValue().ToStdString() != m_profile->name()) {
                    m_nameTxt->ChangeValue(m_profile->name());
                }
                if (m_workDirTxt->GetValue().ToStdString() != m_profile->workDirSpec()) {
                    m_workDirTxt->ChangeValue(m_profile->workDirSpec());
                }
            }
        }
    }
}
