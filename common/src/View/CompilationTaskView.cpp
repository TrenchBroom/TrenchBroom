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

#include "CompilationTaskView.h"

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
        class CompilationTaskView::TaskEditor : public TitledPanel {
        protected:
            T* m_task;
        protected:
            TaskEditor(wxWindow* parent, const String& title, T* task) :
            TitledPanel(parent, title),
            m_task(task) {}
        public:
            void initialize() {
                createGui();
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
            
            virtual void createGui() = 0;
            virtual void refresh() = 0;
        };
        
        class CompilationTaskView::CopyFilesTaskEditor : public TaskEditor<Model::CompilationCopyFiles> {
        private:
            wxTextCtrl* m_sourceEditor;
            wxTextCtrl* m_targetEditor;
        public:
            CopyFilesTaskEditor(wxWindow* parent, Model::CompilationCopyFiles* task) :
            TaskEditor(parent, "Copy Files", task),
            m_sourceEditor(NULL),
            m_targetEditor(NULL) {}
        private:
            void createGui() {
                wxStaticText* sourceLabel = new wxStaticText(getPanel(), wxID_ANY, "Source");
                sourceLabel->SetFont(sourceLabel->GetFont().Bold());
                m_sourceEditor = new wxTextCtrl(getPanel(), wxID_ANY);
                enableAutoComplete(m_sourceEditor);
                
                wxStaticText* targetLabel = new wxStaticText(getPanel(), wxID_ANY, "Target");
                targetLabel->SetFont(targetLabel->GetFont().Bold());
                m_targetEditor = new wxTextCtrl(getPanel(), wxID_ANY);
                enableAutoComplete(m_targetEditor);
                
                const int LabelFlags   = wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxRIGHT;
                const int EditorFlags  = wxALIGN_CENTER_VERTICAL | wxRIGHT;
                const int LabelMargin  = LayoutConstants::NarrowHMargin;
                const int EditorMargin = LayoutConstants::WideHMargin;
                
                wxGridBagSizer* sizer = new wxGridBagSizer(LayoutConstants::NarrowVMargin);
                sizer->Add(sourceLabel,     wxGBPosition(0, 0), wxDefaultSpan, LabelFlags, LabelMargin);
                sizer->Add(m_sourceEditor,    wxGBPosition(0, 1), wxDefaultSpan, EditorFlags, EditorMargin);
                sizer->Add(targetLabel,     wxGBPosition(1, 0), wxDefaultSpan, LabelFlags, LabelMargin);
                sizer->Add(m_targetEditor,    wxGBPosition(1, 1), wxDefaultSpan, EditorFlags, EditorMargin);
                
                sizer->AddGrowableCol(1);
                
                SetSizer(sizer);
            }
            
            void refresh() {
                m_sourceEditor->SetValue(m_task->sourceSpec());
                m_targetEditor->SetValue(m_task->targetSpec());
            }
        };
        
        class CompilationTaskView::RunToolTaskEditor : public TaskEditor<Model::CompilationRunTool> {
        private:
            wxTextCtrl* m_toolEditor;
            wxTextCtrl* m_parametersEditor;
        public:
            RunToolTaskEditor(wxWindow* parent, Model::CompilationRunTool* task) :
            TaskEditor(parent, "Run Tool", task),
            m_toolEditor(NULL),
            m_parametersEditor(NULL) {}
        private:
            void createGui() {
                wxStaticText* toolLabel = new wxStaticText(getPanel(), wxID_ANY, "Tool");
                toolLabel->SetFont(toolLabel->GetFont().Bold());
                m_toolEditor = new wxTextCtrl(getPanel(), wxID_ANY);
                enableAutoComplete(m_toolEditor);
                
                wxStaticText* parameterLabel = new wxStaticText(getPanel(), wxID_ANY, "Parameters");
                parameterLabel->SetFont(parameterLabel->GetFont().Bold());
                m_parametersEditor = new wxTextCtrl(getPanel(), wxID_ANY);
                enableAutoComplete(m_parametersEditor);
                
                const int LabelFlags   = wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxRIGHT;
                const int EditorFlags  = wxALIGN_CENTER_VERTICAL | wxRIGHT;
                const int LabelMargin  = LayoutConstants::NarrowHMargin;
                const int EditorMargin = LayoutConstants::WideHMargin;
                
                wxGridBagSizer* sizer = new wxGridBagSizer(LayoutConstants::NarrowVMargin);
                sizer->Add(toolLabel,       wxGBPosition(0, 0), wxDefaultSpan, LabelFlags, LabelMargin);
                sizer->Add(m_toolEditor,    wxGBPosition(0, 1), wxDefaultSpan, EditorFlags, EditorMargin);
                sizer->Add(parameterLabel,  wxGBPosition(1, 0), wxDefaultSpan, LabelFlags, LabelMargin);
                sizer->Add(m_parametersEditor,    wxGBPosition(1, 1), wxDefaultSpan, EditorFlags, EditorMargin);
                
                sizer->AddGrowableCol(1);
                
                SetSizer(sizer);
            }
            
            void refresh() {
                m_toolEditor->SetValue(m_task->toolSpec());
                m_parametersEditor->SetValue(m_task->toolSpec());
            }
        };
        
        CompilationTaskView::CompilationTaskView(wxWindow* parent) :
        wxPanel(parent, wxID_ANY),
        m_profile(NULL) {}

        CompilationTaskView::~CompilationTaskView() {
            if (m_profile != NULL)
                m_profile->profileDidChange.removeObserver(this, &CompilationTaskView::profileDidChange);
        }

        void CompilationTaskView::setProfile(Model::CompilationProfile* profile) {
            if (m_profile != NULL)
                m_profile->profileDidChange.removeObserver(this, &CompilationTaskView::profileDidChange);
            m_profile = profile;
            if (m_profile != NULL)
                m_profile->profileDidChange.addObserver(this, &CompilationTaskView::profileDidChange);
            refresh();
        }
        
        void CompilationTaskView::profileDidChange() {
            refresh();
        }
        
        class CompilationTaskView::CompilationTaskEditorFactory : public Model::CompilationTaskVisitor {
        private:
            wxWindow* m_parent;
            wxSizer* m_sizer;
        public:
            CompilationTaskEditorFactory(wxWindow* parent, wxSizer* sizer) :
            m_parent(parent),
            m_sizer(sizer) {}
            
            void visit(Model::CompilationCopyFiles* task) {
                TaskEditor<Model::CompilationCopyFiles>* editor = new CopyFilesTaskEditor(m_parent, task);
                editor->initialize();
                m_sizer->Add(editor, 0, wxEXPAND);
            }
            
            void visit(Model::CompilationRunTool* task) {
                TaskEditor<Model::CompilationRunTool>* editor = new RunToolTaskEditor(m_parent, task);
                editor->initialize();
                m_sizer->Add(editor, 0, wxEXPAND);
            }
        };
        
        void CompilationTaskView::refresh() {
            SetSizer(NULL);
            DestroyChildren();
            
            if (m_profile != NULL && m_profile->taskCount() > 0) {
                wxSizer* sizer = new wxBoxSizer(wxVERTICAL);

                CompilationTaskEditorFactory factory(this, sizer);
                m_profile->accept(factory);
                
                sizer->Add(new BorderLine(this, BorderLine::Direction_Horizontal), 0, wxEXPAND);
                sizer->AddStretchSpacer();
                
                SetSizer(sizer);
            }
        }
    }
}
