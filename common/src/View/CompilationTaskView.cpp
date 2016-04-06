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
#include "View/TitledPanel.h"
#include "View/ViewConstants.h"

#include <wx/gbsizer.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>

namespace TrenchBroom {
    namespace View {
        class CompilationTaskView::CopyFilesTaskEditor : public TitledPanel {
        private:
            Model::CompilationCopyFiles& m_task;
        public:
            CopyFilesTaskEditor(wxWindow* parent, Model::CompilationCopyFiles& task) :
            TitledPanel(parent, "Copy Files"),
            m_task(task) {
                wxStaticText* sourceLabel = new wxStaticText(getPanel(), wxID_ANY, "Source");
                sourceLabel->SetFont(sourceLabel->GetFont().Bold());
                wxTextCtrl* sourceEditor = new wxTextCtrl(getPanel(), wxID_ANY);
                
                wxStaticText* targetLabel = new wxStaticText(getPanel(), wxID_ANY, "Target");
                targetLabel->SetFont(targetLabel->GetFont().Bold());
                wxTextCtrl* targetEditor = new wxTextCtrl(getPanel(), wxID_ANY);
                
                const int LabelFlags   = wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxRIGHT;
                const int EditorFlags  = wxALIGN_CENTER_VERTICAL | wxRIGHT;
                const int LabelMargin  = LayoutConstants::NarrowHMargin;
                const int EditorMargin = LayoutConstants::WideHMargin;

                wxGridBagSizer* sizer = new wxGridBagSizer(LayoutConstants::NarrowVMargin);
                sizer->Add(sourceLabel,     wxGBPosition(0, 0), wxDefaultSpan, LabelFlags, LabelMargin);
                sizer->Add(sourceEditor,    wxGBPosition(0, 1), wxDefaultSpan, EditorFlags, EditorMargin);
                sizer->Add(targetLabel,     wxGBPosition(1, 0), wxDefaultSpan, LabelFlags, LabelMargin);
                sizer->Add(targetEditor,    wxGBPosition(1, 1), wxDefaultSpan, EditorFlags, EditorMargin);
                
                sizer->AddGrowableCol(1);
                
                SetSizer(sizer);
            }
        };
        
        class CompilationTaskView::RunToolTaskEditor : public TitledPanel {
        private:
            Model::CompilationRunTool& m_task;
        public:
            RunToolTaskEditor(wxWindow* parent, Model::CompilationRunTool& task) :
            TitledPanel(parent, "Run Tool"),
            m_task(task) {
                wxStaticText* toolLabel = new wxStaticText(getPanel(), wxID_ANY, "Tool");
                toolLabel->SetFont(toolLabel->GetFont().Bold());
                wxTextCtrl* sourceEditor = new wxTextCtrl(getPanel(), wxID_ANY);
                
                wxStaticText* parameterLabel = new wxStaticText(getPanel(), wxID_ANY, "Parameters");
                parameterLabel->SetFont(parameterLabel->GetFont().Bold());
                wxTextCtrl* targetEditor = new wxTextCtrl(getPanel(), wxID_ANY);
                
                const int LabelFlags   = wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxRIGHT;
                const int EditorFlags  = wxALIGN_CENTER_VERTICAL | wxRIGHT;
                const int LabelMargin  = LayoutConstants::NarrowHMargin;
                const int EditorMargin = LayoutConstants::WideHMargin;
                
                wxGridBagSizer* sizer = new wxGridBagSizer(LayoutConstants::NarrowVMargin);
                sizer->Add(toolLabel,       wxGBPosition(0, 0), wxDefaultSpan, LabelFlags, LabelMargin);
                sizer->Add(sourceEditor,    wxGBPosition(0, 1), wxDefaultSpan, EditorFlags, EditorMargin);
                sizer->Add(parameterLabel,  wxGBPosition(1, 0), wxDefaultSpan, LabelFlags, LabelMargin);
                sizer->Add(targetEditor,    wxGBPosition(1, 1), wxDefaultSpan, EditorFlags, EditorMargin);
                
                sizer->AddGrowableCol(1);
                
                SetSizer(sizer);
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
            
            void visit(Model::CompilationCopyFiles& task) {
                m_sizer->Add(new CopyFilesTaskEditor(m_parent, task), 0, wxEXPAND);
            }
            
            void visit(Model::CompilationRunTool& task) {
                m_sizer->Add(new RunToolTaskEditor(m_parent, task), 0, wxEXPAND);
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
