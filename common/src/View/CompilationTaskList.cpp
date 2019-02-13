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

#include "CompilationTaskList.h"

#include "EL/Interpolator.h"
#include "Model/CompilationProfile.h"
#include "View/AutoCompleteTextControl.h"
#include "View/ELAutoCompleteHelper.h"
#include "View/BorderLine.h"
#include "View/CompilationVariables.h"
#include "View/TitledPanel.h"
#include "View/ViewConstants.h"

#include <wx/button.h>
#include <wx/filedlg.h>
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
            MapDocumentWPtr m_document;
            Model::CompilationProfile* m_profile;
            T* m_task;
            TitledPanel* m_panel;
            
            using AutoCompleteTextControlList = std::list<AutoCompleteTextControl*>;
            AutoCompleteTextControlList m_autoCompleteTextControls;
        protected:
            TaskEditor(wxWindow* parent, const wxSize& margins, const String& title, MapDocumentWPtr document, Model::CompilationProfile* profile, T* task) :
            Item(parent),
            m_margins(margins),
            m_title(title),
            m_document(document),
            m_profile(profile),
            m_task(task),
            m_panel(nullptr) {
                ensure(m_profile != nullptr, "profile is null");
                ensure(m_task != nullptr, "task is null");
            }
        public:
            void initialize() {
                m_panel = new TitledPanel(this, m_title);
                wxWindow* editor = createGui(m_panel->getPanel());

                wxSizer* editorSizer = new wxBoxSizer(wxVERTICAL);
                editorSizer->AddSpacer(m_margins.y);
                editorSizer->Add(editor, wxSizerFlags().Expand().Border(wxLEFT | wxRIGHT, m_margins.x));
                editorSizer->AddSpacer(m_margins.y);
                m_panel->getPanel()->SetSizer(editorSizer);

                wxSizer* panelSizer = new wxBoxSizer(wxVERTICAL);
                panelSizer->Add(m_panel, wxSizerFlags().Expand());
                SetSizer(panelSizer);

                refresh();
                addProfileObservers();
                addTaskObservers();
            }
        public:
            ~TaskEditor() override {
                removeProfileObservers();
                removeTaskObservers();
            }
        private:
            void setSelectionColours(const wxColour& foreground, const wxColour& background) override {
                setColours(m_panel->getPanel(), foreground, background);
            }

            void setDefaultColours(const wxColour& foreground, const wxColour& background) override {
                setColours(m_panel->getPanel(), foreground, background);
            }
        protected:
            void enableAutoComplete(AutoCompleteTextControl* control) {
                updateAutoComplete(control);
                m_autoCompleteTextControls.push_back(control);
            }
        private:
            void updateAutoComplete(AutoCompleteTextControl* control) {
                const String workDir = EL::interpolate(m_profile->workDirSpec(), CompilationWorkDirVariables(lock(m_document)));
                const CompilationVariables variables(lock(m_document), workDir);
                
                control->SetHelper(new ELAutoCompleteHelper(variables));
            }
        private:
            void addProfileObservers() {
                m_profile->profileWillBeRemoved.addObserver(this, &TaskEditor::profileWillBeRemoved);
                m_profile->profileDidChange.addObserver(this, &TaskEditor::profileDidChange);
            }
            
            void removeProfileObservers() {
                if (m_profile != nullptr) {
                    m_profile->profileWillBeRemoved.removeObserver(this, &TaskEditor::profileWillBeRemoved);
                    m_profile->profileDidChange.removeObserver(this, &TaskEditor::profileDidChange);
                }
            }
            
            void addTaskObservers() {
                m_task->taskWillBeRemoved.addObserver(this, &TaskEditor::taskWillBeRemoved);
                m_task->taskDidChange.addObserver(this, &TaskEditor::taskDidChange);
            }
            
            void removeTaskObservers() {
                if (m_task != nullptr) {
                    m_task->taskWillBeRemoved.removeObserver(this, &TaskEditor::taskWillBeRemoved);
                    m_task->taskDidChange.removeObserver(this, &TaskEditor::taskDidChange);
                }
            }
            
            void taskWillBeRemoved() {
                removeTaskObservers();
                m_task = nullptr;
            }

            void taskDidChange() {
                if (m_task != nullptr)
                    refresh();
            }

            void profileWillBeRemoved() {
                removeProfileObservers();
                removeTaskObservers();
                m_task = nullptr;
                m_profile = nullptr;
            }
            
            void profileDidChange() {
                for (AutoCompleteTextControl* control : m_autoCompleteTextControls)
                    updateAutoComplete(control);
            }
            
            virtual wxWindow* createGui(wxWindow* parent) = 0;
            virtual void refresh() = 0;
        };

        class CompilationTaskList::ExportMapTaskEditor : public TaskEditor<Model::CompilationExportMap> {
        private:
            AutoCompleteTextControl* m_targetEditor;
        public:
            ExportMapTaskEditor(wxWindow* parent, const wxSize& margins, MapDocumentWPtr document, Model::CompilationProfile* profile, Model::CompilationExportMap* task) :
            TaskEditor(parent, margins, "Export Map", document, profile, task),
            m_targetEditor(nullptr) {}
        private:
            wxWindow* createGui(wxWindow* parent) override {
                wxPanel* container = new wxPanel(parent);

                wxStaticText* targetLabel = new wxStaticText(container, wxID_ANY, "Target");
                m_targetEditor = new AutoCompleteTextControl(container, wxID_ANY);
                m_targetEditor->Bind(wxEVT_TEXT, &ExportMapTaskEditor::OnTargetSpecChanged, this);
                enableAutoComplete(m_targetEditor);

                const int LabelFlags   = wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxRIGHT;
                const int EditorFlags  = wxALIGN_CENTER_VERTICAL | wxEXPAND;
                const int LabelMargin  = LayoutConstants::NarrowHMargin;

                wxGridBagSizer* sizer = new wxGridBagSizer(LayoutConstants::NarrowVMargin);
                sizer->Add(targetLabel,     wxGBPosition(0, 0), wxDefaultSpan, LabelFlags, LabelMargin);
                sizer->Add(m_targetEditor,  wxGBPosition(0, 1), wxDefaultSpan, EditorFlags);

                sizer->AddGrowableCol(1);
                container->SetSizer(sizer);
                return container;
            }

            void OnTargetSpecChanged(wxCommandEvent& event) {
                m_task->setTargetSpec(m_targetEditor->GetValue().ToStdString());
            }

            void refresh() override {
                if (m_targetEditor->GetValue().ToStdString() != m_task->targetSpec()) {
                    m_targetEditor->ChangeValue(m_task->targetSpec());
                }
            }
        };

        class CompilationTaskList::CopyFilesTaskEditor : public TaskEditor<Model::CompilationCopyFiles> {
        private:
            AutoCompleteTextControl* m_sourceEditor;
            AutoCompleteTextControl* m_targetEditor;
        public:
            CopyFilesTaskEditor(wxWindow* parent, const wxSize& margins, MapDocumentWPtr document, Model::CompilationProfile* profile, Model::CompilationCopyFiles* task) :
            TaskEditor(parent, margins, "Copy Files", document, profile, task),
            m_sourceEditor(nullptr),
            m_targetEditor(nullptr) {}
        private:
            wxWindow* createGui(wxWindow* parent) override {
                wxPanel* container = new wxPanel(parent);

                wxStaticText* sourceLabel = new wxStaticText(container, wxID_ANY, "Source");
                m_sourceEditor = new AutoCompleteTextControl(container, wxID_ANY);
                m_sourceEditor->Bind(wxEVT_TEXT, &CopyFilesTaskEditor::OnSourceSpecChanged, this);
                enableAutoComplete(m_sourceEditor);

                wxStaticText* targetLabel = new wxStaticText(container, wxID_ANY, "Target");
                m_targetEditor = new AutoCompleteTextControl(container, wxID_ANY);
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

            void refresh() override {
                // call ChangeValue to avoid sending a change event
                if (m_sourceEditor->GetValue().ToStdString() != m_task->sourceSpec()) {
                    m_sourceEditor->ChangeValue(m_task->sourceSpec());
                }
                if (m_targetEditor->GetValue().ToStdString() != m_task->targetSpec()) {
                    m_targetEditor->ChangeValue(m_task->targetSpec());
                }
            }
        };

        class CompilationTaskList::RunToolTaskEditor : public TaskEditor<Model::CompilationRunTool> {
        private:
            AutoCompleteTextControl* m_toolEditor;
            AutoCompleteTextControl* m_parametersEditor;
        public:
            RunToolTaskEditor(wxWindow* parent, const wxSize& margins, MapDocumentWPtr document, Model::CompilationProfile* profile, Model::CompilationRunTool* task) :
            TaskEditor(parent, margins, "Run Tool", document, profile, task),
            m_toolEditor(nullptr),
            m_parametersEditor(nullptr) {}
        private:
            wxWindow* createGui(wxWindow* parent) override {
                wxPanel* container = new wxPanel(parent);

                wxStaticText* toolLabel = new wxStaticText(container, wxID_ANY, "Tool");
                m_toolEditor = new AutoCompleteTextControl(container, wxID_ANY);
                m_toolEditor->Bind(wxEVT_TEXT, &RunToolTaskEditor::OnToolSpecChanged, this);
                enableAutoComplete(m_toolEditor);

                wxButton* browseToolButton = new wxButton(container, wxID_ANY, "...", wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
                browseToolButton->Bind(wxEVT_BUTTON, &RunToolTaskEditor::OnBrowseTool, this);

                wxStaticText* parameterLabel = new wxStaticText(container, wxID_ANY, "Parameters");
                m_parametersEditor = new AutoCompleteTextControl(container, wxID_ANY);
                m_parametersEditor->Bind(wxEVT_TEXT, &RunToolTaskEditor::OnParameterSpecChanged, this);
                enableAutoComplete(m_parametersEditor);

                const int LabelFlags   = wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxRIGHT;
                const int EditorFlags  = wxALIGN_CENTER_VERTICAL | wxEXPAND;
                const int LabelMargin  = LayoutConstants::NarrowHMargin;

                wxGridBagSizer* sizer = new wxGridBagSizer(LayoutConstants::NarrowVMargin);
                sizer->Add(toolLabel,           wxGBPosition(0, 0), wxDefaultSpan, LabelFlags, LabelMargin);
                sizer->Add(m_toolEditor,        wxGBPosition(0, 1), wxDefaultSpan, EditorFlags);
                sizer->Add(browseToolButton,    wxGBPosition(0, 2), wxDefaultSpan, wxLEFT, LabelMargin);
                sizer->Add(parameterLabel,      wxGBPosition(1, 0), wxDefaultSpan, LabelFlags, LabelMargin);
                sizer->Add(m_parametersEditor,  wxGBPosition(1, 1), wxGBSpan(1, 2), EditorFlags);

                sizer->AddGrowableCol(1);

                container->SetSizer(sizer);
                return container;
            }

            void OnBrowseTool(wxCommandEvent& event) {
                wxFileDialog browseDialog(this, "Select Tool", wxEmptyString, wxEmptyString, wxFileSelectorDefaultWildcardStr, wxFD_OPEN | wxFD_FILE_MUST_EXIST);
                if (browseDialog.ShowModal() == wxID_OK)
                    m_task->setToolSpec(browseDialog.GetPath().ToStdString());
            }

            void OnToolSpecChanged(wxCommandEvent& event) {
                m_task->setToolSpec(m_toolEditor->GetValue().ToStdString());
            }

            void OnParameterSpecChanged(wxCommandEvent& event) {
                m_task->setParameterSpec(m_parametersEditor->GetValue().ToStdString());
            }

            void refresh() override {
                if (m_toolEditor->GetValue().ToStdString() != m_task->toolSpec()) {
                    m_toolEditor->ChangeValue(m_task->toolSpec());
                }
                if (m_parametersEditor->GetValue().ToStdString() != m_task->parameterSpec()) {
                    m_parametersEditor->ChangeValue(m_task->parameterSpec());
                }
            }
        };

        CompilationTaskList::CompilationTaskList(wxWindow* parent, MapDocumentWPtr document) :
        ControlListBox(parent, true, "Click the '+' button to create a task."),
        m_document(document),
        m_profile(nullptr) {}

        CompilationTaskList::~CompilationTaskList() {
            if (m_profile != nullptr)
                m_profile->profileDidChange.removeObserver(this, &CompilationTaskList::profileDidChange);
        }

        void CompilationTaskList::setProfile(Model::CompilationProfile* profile) {
            if (m_profile != nullptr)
                m_profile->profileDidChange.removeObserver(this, &CompilationTaskList::profileDidChange);
            m_profile = profile;
            if (m_profile != nullptr)
                m_profile->profileDidChange.addObserver(this, &CompilationTaskList::profileDidChange);
            refresh();
        }

        void CompilationTaskList::profileDidChange() {
            refresh();
        }

        void CompilationTaskList::refresh() {
            if (m_profile == nullptr)
                SetItemCount(0);
            else
                SetItemCount(m_profile->taskCount());
        }

        class CompilationTaskList::CompilationTaskEditorFactory : public Model::CompilationTaskVisitor {
        private:
            wxWindow* m_parent;
            const wxSize m_margins;
            MapDocumentWPtr m_document;
            Model::CompilationProfile* m_profile;
            Item* m_result;
        public:
            CompilationTaskEditorFactory(wxWindow* parent, const wxSize& margins, MapDocumentWPtr document, Model::CompilationProfile* profile) :
            m_parent(parent),
            m_margins(margins),
            m_document(document),
            m_profile(profile),
            m_result(nullptr) {}

            Item* result() const {
                return m_result;
            }

            void visit(Model::CompilationExportMap* task) override {
                TaskEditor<Model::CompilationExportMap>* editor = new ExportMapTaskEditor(m_parent, m_margins, m_document, m_profile, task);
                editor->initialize();
                m_result = editor;
            }

            void visit(Model::CompilationCopyFiles* task) override {
                TaskEditor<Model::CompilationCopyFiles>* editor = new CopyFilesTaskEditor(m_parent, m_margins, m_document, m_profile, task);
                editor->initialize();
                m_result = editor;
            }

            void visit(Model::CompilationRunTool* task) override {
                TaskEditor<Model::CompilationRunTool>* editor = new RunToolTaskEditor(m_parent, m_margins, m_document, m_profile, task);
                editor->initialize();
                m_result = editor;
            }
        };

        ControlListBox::Item* CompilationTaskList::createItem(wxWindow* parent, const wxSize& margins, const size_t index) {
            ensure(m_profile != nullptr, "profile is null");

            CompilationTaskEditorFactory factory(parent, margins, m_document, m_profile);
            Model::CompilationTask* task = m_profile->task(index);
            task->accept(factory);
            return factory.result();
        }
    }
}
