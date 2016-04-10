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
#include "View/ViewConstants.h"

#include <wx/gbsizer.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>

namespace TrenchBroom {
    namespace View {
        template <typename T>
        class CompilationTaskList::TaskEditor : public wxPanel {
        protected:
            const String m_title;
            T* m_task;
        protected:
            TaskEditor(wxWindow* parent, const String& title, T* task) :
            wxPanel(parent),
            m_title(title),
            m_task(task) {}
        public:
            void initialize() {
                wxStaticText* titleText = new wxStaticText(this, wxID_ANY, m_title);
                titleText->SetFont(titleText->GetFont().Bold());

                wxWindow* editor = createGui();
                
                wxSizer* sizer = new wxBoxSizer(wxVERTICAL);
                sizer->Add(titleText, 0);
                sizer->Add(new BorderLine(this, BorderLine::Direction_Horizontal), 0, wxEXPAND);
                sizer->AddSpacer(LayoutConstants::WideVMargin);
                sizer->Add(editor, 0, wxEXPAND);
                SetSizer(sizer);
                
                refresh();
                m_task->taskDidChange.addObserver(this, &TaskEditor::taskDidChange);
            }
        public:
            virtual ~TaskEditor() {
                m_task->taskDidChange.removeObserver(this, &TaskEditor::taskDidChange);
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
            void taskDidChange() {
                refresh();
            }
            
            virtual wxWindow* createGui() = 0;
            virtual void refresh() = 0;
        };
        
        class CompilationTaskList::CopyFilesTaskEditor : public TaskEditor<Model::CompilationCopyFiles> {
        private:
            wxTextCtrl* m_sourceEditor;
            wxTextCtrl* m_targetEditor;
        public:
            CopyFilesTaskEditor(wxWindow* parent, Model::CompilationCopyFiles* task) :
            TaskEditor(parent, "Copy Files", task),
            m_sourceEditor(NULL),
            m_targetEditor(NULL) {}
        private:
            wxWindow* createGui() {
                wxPanel* parent = new wxPanel(this);
                
                wxStaticText* sourceLabel = new wxStaticText(parent, wxID_ANY, "Source");
                m_sourceEditor = new wxTextCtrl(parent, wxID_ANY);
                enableAutoComplete(m_sourceEditor);
                
                wxStaticText* targetLabel = new wxStaticText(parent, wxID_ANY, "Target");
                m_targetEditor = new wxTextCtrl(parent, wxID_ANY);
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
                
                parent->SetSizer(sizer);
                return parent;
            }
            
            void refresh() {
                m_sourceEditor->SetValue(m_task->sourceSpec());
                m_targetEditor->SetValue(m_task->targetSpec());
            }
        };
        
        class CompilationTaskList::RunToolTaskEditor : public TaskEditor<Model::CompilationRunTool> {
        private:
            wxTextCtrl* m_toolEditor;
            wxTextCtrl* m_parametersEditor;
        public:
            RunToolTaskEditor(wxWindow* parent, Model::CompilationRunTool* task) :
            TaskEditor(parent, "Run Tool", task),
            m_toolEditor(NULL),
            m_parametersEditor(NULL) {}
        private:
            wxWindow* createGui() {
                wxPanel* parent = new wxPanel(this);
                
                wxStaticText* toolLabel = new wxStaticText(parent, wxID_ANY, "Tool");
                m_toolEditor = new wxTextCtrl(parent, wxID_ANY);
                enableAutoComplete(m_toolEditor);
                
                wxStaticText* parameterLabel = new wxStaticText(parent, wxID_ANY, "Parameters");
                m_parametersEditor = new wxTextCtrl(parent, wxID_ANY);
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
                
                parent->SetSizer(sizer);
                return parent;
            }
            
            void refresh() {
                m_toolEditor->SetValue(m_task->toolSpec());
                m_parametersEditor->SetValue(m_task->toolSpec());
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
            wxWindow* m_result;
        public:
            CompilationTaskEditorFactory(wxWindow* parent) :
            m_parent(parent),
            m_result(NULL) {}
            
            wxWindow* result() const {
                return m_result;
            }
            
            void visit(Model::CompilationCopyFiles* task) {
                TaskEditor<Model::CompilationCopyFiles>* editor = new CopyFilesTaskEditor(m_parent, task);
                editor->initialize();
                m_result = editor;
            }
            
            void visit(Model::CompilationRunTool* task) {
                TaskEditor<Model::CompilationRunTool>* editor = new RunToolTaskEditor(m_parent, task);
                editor->initialize();
                m_result = editor;
            }
        };

        wxWindow* CompilationTaskList::createItem(wxWindow* parent, const size_t index) {
            assert(m_profile != NULL);
            
            CompilationTaskEditorFactory factory(parent);
            Model::CompilationTask* task = m_profile->task(index);
            task->accept(factory);
            return factory.result();
        }
    }
}
