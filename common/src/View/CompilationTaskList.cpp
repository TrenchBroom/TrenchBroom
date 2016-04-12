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

#include "CompilationTaskList.h"

#include "Model/CompilationProfile.h"
#include "View/BorderLine.h"
#include "View/CompilationVariables.h"
#include "View/TitledPanel.h"
#include "View/ViewConstants.h"

#include <wx/gbsizer.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>

namespace TrenchBroom {
    namespace View {
        template <typename T>
        class CompilationTaskList::TaskEditor : public Item {
        protected:
            const wxSize m_margins;
            const String m_title;
            T* m_task;
            TitledPanel* m_panel;
        protected:
            TaskEditor(wxWindow* parent, const wxSize& margins, const String& title, T* task) :
            Item(parent),
            m_margins(margins),
            m_title(title),
            m_task(task),
            m_panel(NULL) {}
        public:
            void initialize() {
                m_panel = new TitledPanel(this, m_title, true, false);
                wxWindow* editor = createGui(m_panel->getPanel());
                
                wxSizer* editorSizer = new wxBoxSizer(wxVERTICAL);
                editorSizer->AddSpacer(m_margins.y);
                editorSizer->Add(editor, 0, wxEXPAND | wxLEFT | wxRIGHT, m_margins.x);
                editorSizer->AddSpacer(m_margins.y);
                m_panel->getPanel()->SetSizer(editorSizer);
                
                wxSizer* panelSizer = new wxBoxSizer(wxVERTICAL);
                panelSizer->Add(m_panel, 0, wxEXPAND);
                SetSizer(panelSizer);
                
                refresh();
                m_task->taskWillBeDeleted.addObserver(this, &TaskEditor::taskWillBeDeleted);
                m_task->taskDidChange.addObserver(this, &TaskEditor::taskDidChange);
            }
        public:
            virtual ~TaskEditor() {
                if (m_task != NULL) {
                    m_task->taskWillBeDeleted.removeObserver(this, &TaskEditor::taskWillBeDeleted);
                    m_task->taskDidChange.removeObserver(this, &TaskEditor::taskDidChange);
                }
            }
        private:
            void setSelectionColours(const wxColour& foreground, const wxColour& background) {
                setColours(m_panel->getPanel(), foreground, background);
            }
            
            void setDefaultColours(const wxColour& foreground, const wxColour& background) {
                setColours(m_panel->getPanel(), foreground, background);
            }
        protected:
            void enableAutoComplete(wxTextEntry* textEntry) {
                const VariableTable& variables = compilationVariables();
                const StringSet& nameSet = variables.declaredVariables();
                wxArrayString nameArray;
                nameArray.Alloc(nameSet.size());
                
                StringSet::const_iterator it, end;
                for (it = nameSet.begin(), end = nameSet.end(); it != end; ++it) {
                    const String& variableName = *it;
                    const String variableString = variables.buildVariableString(variableName);
                    nameArray.Add(variableString);
                }
                
                textEntry->AutoComplete(nameArray);
            }
        private:
            void taskWillBeDeleted() {
                m_task = NULL;
            }
            
            void taskDidChange() {
                if (m_task != NULL)
                    refresh();
            }
            
            virtual wxWindow* createGui(wxWindow* parent) = 0;
            virtual void refresh() = 0;
        };
        
        class CompilationTaskList::CopyFilesTaskEditor : public TaskEditor<Model::CompilationCopyFiles> {
        private:
            wxTextCtrl* m_sourceEditor;
            wxTextCtrl* m_targetEditor;
        public:
            CopyFilesTaskEditor(wxWindow* parent, const wxSize& margins, Model::CompilationCopyFiles* task) :
            TaskEditor(parent, margins, "Copy Files", task),
            m_sourceEditor(NULL),
            m_targetEditor(NULL) {}
        private:
            wxWindow* createGui(wxWindow* parent) {
                wxPanel* container = new wxPanel(parent);
                
                wxStaticText* sourceLabel = new wxStaticText(container, wxID_ANY, "Source");
                m_sourceEditor = new wxTextCtrl(container, wxID_ANY);
                m_sourceEditor->Bind(wxEVT_TEXT, &CopyFilesTaskEditor::OnSourceSpecChanged, this);
                enableAutoComplete(m_sourceEditor);
                
                wxStaticText* targetLabel = new wxStaticText(container, wxID_ANY, "Target");
                m_targetEditor = new wxTextCtrl(container, wxID_ANY);
                m_targetEditor->Bind(wxEVT_TEXT, &CopyFilesTaskEditor::OnTargetSpecChanged, this);
                enableAutoComplete(m_targetEditor);
                
                const int LabelFlags   = wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxRIGHT;
                const int EditorFlags  = wxALIGN_CENTER_VERTICAL | wxEXPAND;
                const int LabelMargin  = LayoutConstants::NarrowHMargin;
                
                wxGridBagSizer* sizer = new wxGridBagSizer(LayoutConstants::NarrowVMargin);
                sizer->Add(sourceLabel,     wxGBPosition(0, 0), wxDefaultSpan, LabelFlags, LabelMargin);
                sizer->Add(m_sourceEditor,  wxGBPosition(0, 1), wxDefaultSpan, EditorFlags);
                sizer->Add(targetLabel,     wxGBPosition(1, 0), wxDefaultSpan, LabelFlags, LabelMargin);
                sizer->Add(m_targetEditor,  wxGBPosition(1, 1), wxDefaultSpan, EditorFlags);
                
                sizer->AddGrowableCol(1);
                container->SetSizer(sizer);
                return container;
            }
            
            void OnSourceSpecChanged(wxCommandEvent& event) {
                m_task->setSourceSpec(m_sourceEditor->GetValue().ToStdString());
            }
            
            void OnTargetSpecChanged(wxCommandEvent& event) {
                m_task->setTargetSpec(m_targetEditor->GetValue().ToStdString());
            }
            
            void refresh() {
                // call ChangeValue to avoid sending a change event
                m_sourceEditor->ChangeValue(m_task->sourceSpec());
                m_targetEditor->ChangeValue(m_task->targetSpec());
            }
        };
        
        class CompilationTaskList::RunToolTaskEditor : public TaskEditor<Model::CompilationRunTool> {
        private:
            wxTextCtrl* m_toolEditor;
            wxTextCtrl* m_parametersEditor;
        public:
            RunToolTaskEditor(wxWindow* parent, const wxSize& margins, Model::CompilationRunTool* task) :
            TaskEditor(parent, margins, "Run Tool", task),
            m_toolEditor(NULL),
            m_parametersEditor(NULL) {}
        private:
            wxWindow* createGui(wxWindow* parent) {
                wxPanel* container = new wxPanel(parent);
                
                wxStaticText* toolLabel = new wxStaticText(container, wxID_ANY, "Tool");
                m_toolEditor = new wxTextCtrl(container, wxID_ANY);
                m_toolEditor->Bind(wxEVT_TEXT, &RunToolTaskEditor::OnToolSpecChanged, this);
                enableAutoComplete(m_toolEditor);
                
                wxStaticText* parameterLabel = new wxStaticText(container, wxID_ANY, "Parameters");
                m_parametersEditor = new wxTextCtrl(container, wxID_ANY);
                m_parametersEditor->Bind(wxEVT_TEXT, &RunToolTaskEditor::OnParameterSpecChanged, this);
                enableAutoComplete(m_parametersEditor);
                
                const int LabelFlags   = wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxRIGHT;
                const int EditorFlags  = wxALIGN_CENTER_VERTICAL | wxEXPAND;
                const int LabelMargin  = LayoutConstants::NarrowHMargin;
                
                wxGridBagSizer* sizer = new wxGridBagSizer(LayoutConstants::NarrowVMargin);
                sizer->Add(toolLabel,           wxGBPosition(0, 0), wxDefaultSpan, LabelFlags, LabelMargin);
                sizer->Add(m_toolEditor,        wxGBPosition(0, 1), wxDefaultSpan, EditorFlags);
                sizer->Add(parameterLabel,      wxGBPosition(1, 0), wxDefaultSpan, LabelFlags, LabelMargin);
                sizer->Add(m_parametersEditor,  wxGBPosition(1, 1), wxDefaultSpan, EditorFlags);
                
                sizer->AddGrowableCol(1);
                
                container->SetSizer(sizer);
                return container;
            }
            
            void OnToolSpecChanged(wxCommandEvent& event) {
                m_task->setToolSpec(m_toolEditor->GetValue().ToStdString());
            }
            
            void OnParameterSpecChanged(wxCommandEvent& event) {
                m_task->setParameterSpec(m_parametersEditor->GetValue().ToStdString());
            }
            
            void refresh() {
                m_toolEditor->ChangeValue(m_task->toolSpec());
                m_parametersEditor->ChangeValue(m_task->parameterSpec());
            }
        };
        
        CompilationTaskList::CompilationTaskList(wxWindow* parent) :
        ControlListBox(parent, "No Tasks Found"),
        m_profile(NULL) {}

        CompilationTaskList::~CompilationTaskList() {
            if (m_profile != NULL)
                m_profile->profileDidChange.removeObserver(this, &CompilationTaskList::profileDidChange);
        }

        void CompilationTaskList::setProfile(Model::CompilationProfile* profile) {
            if (m_profile != NULL)
                m_profile->profileDidChange.removeObserver(this, &CompilationTaskList::profileDidChange);
            m_profile = profile;
            if (m_profile != NULL)
                m_profile->profileDidChange.addObserver(this, &CompilationTaskList::profileDidChange);
            refresh();
        }
        
        void CompilationTaskList::profileDidChange() {
            refresh();
        }
        
        void CompilationTaskList::refresh() {
            if (m_profile == NULL)
                SetItemCount(0);
            else
                SetItemCount(m_profile->taskCount());
        }

        class CompilationTaskList::CompilationTaskEditorFactory : public Model::CompilationTaskVisitor {
        private:
            wxWindow* m_parent;
            const wxSize m_margins;
            Item* m_result;
        public:
            CompilationTaskEditorFactory(wxWindow* parent, const wxSize& margins) :
            m_parent(parent),
            m_margins(margins),
            m_result(NULL) {}
            
            Item* result() const {
                return m_result;
            }
            
            void visit(Model::CompilationCopyFiles* task) {
                TaskEditor<Model::CompilationCopyFiles>* editor = new CopyFilesTaskEditor(m_parent, m_margins, task);
                editor->initialize();
                m_result = editor;
            }
            
            void visit(Model::CompilationRunTool* task) {
                TaskEditor<Model::CompilationRunTool>* editor = new RunToolTaskEditor(m_parent, m_margins, task);
                editor->initialize();
                m_result = editor;
            }
        };

        ControlListBox::Item* CompilationTaskList::createItem(wxWindow* parent, const wxSize& margins, const size_t index) {
            assert(m_profile != NULL);
            
            CompilationTaskEditorFactory factory(parent, margins);
            Model::CompilationTask* task = m_profile->task(index);
            task->accept(factory);
            return factory.result();
        }
    }
}
