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

#include "CompilationDialog.h"

#include "Model/Game.h"
#include "View/BorderLine.h"
#include "View/CompilationContext.h"
#include "View/CompilationProfileManager.h"
#include "View/CompilationRunner.h"
#include "View/LaunchGameEngineDialog.h"
#include "View/MapDocument.h"
#include "View/MapFrame.h"
#include "View/SplitterWindow2.h"
#include "View/TitledPanel.h"
#include "View/ViewConstants.h"
#include "View/wxUtils.h"

#include <wx/button.h>
#include <wx/msgdlg.h>
#include <wx/settings.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>

namespace TrenchBroom {
    namespace View {
        CompilationDialog::CompilationDialog(MapFrame* mapFrame) :
        wxDialog(mapFrame, wxID_ANY, "Compile", wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER),
        m_mapFrame(mapFrame),
        m_currentRunLabel(NULL),
        m_output(NULL) {
            createGui();
            SetMinSize(wxSize(600, 300));
            SetSize(wxSize(800, 600));
            CentreOnParent();
        }
        
        void CompilationDialog::createGui() {
            setWindowIcon(this);

            MapDocumentSPtr document = m_mapFrame->document();
            Model::GamePtr game = document->game();
            Model::CompilationConfig& compilationConfig = game->compilationConfig();
            
            wxPanel* outerPanel = new wxPanel(this);
            SplitterWindow2* splitter = new SplitterWindow2(outerPanel);
            
            m_profileManager = new CompilationProfileManager(splitter, document , compilationConfig);

            TitledPanel* outputPanel = new TitledPanel(splitter, "Output");
            m_output = new wxTextCtrl(outputPanel->getPanel(), wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxBORDER_NONE | wxTE_MULTILINE | wxTE_READONLY | wxTE_DONTWRAP | wxTE_RICH2);
            m_output->SetFont(Fonts::fixedWidthFont());

            splitter->splitHorizontally(m_profileManager, outputPanel, wxSize(100, 100), wxSize(100, 100));

            wxSizer* outputSizer = new wxBoxSizer(wxVERTICAL);
            outputSizer->Add(m_output, 1, wxEXPAND);
            outputPanel->getPanel()->SetSizer(outputSizer);

            wxSizer* outerPanelSizer = new wxBoxSizer(wxVERTICAL);
            outerPanelSizer->Add(splitter, 1, wxEXPAND);
            outerPanel->SetSizer(outerPanelSizer);
            
            wxButton* launchButton = new wxButton(this, wxID_ANY, "Launch...");
            wxButton* compileButton = new wxButton(this, wxID_OK, "Compile");
            wxButton* closeButton = new wxButton(this, wxID_CANCEL, "Close");
            
            launchButton->Bind(wxEVT_BUTTON, &CompilationDialog::OnLaunchClicked, this);
            launchButton->Bind(wxEVT_UPDATE_UI, &CompilationDialog::OnUpdateLaunchButtonUI, this);
            compileButton->Bind(wxEVT_BUTTON, &CompilationDialog::OnToggleCompileClicked, this);
            compileButton->Bind(wxEVT_UPDATE_UI, &CompilationDialog::OnUpdateCompileButtonUI, this);
			closeButton->Bind(wxEVT_BUTTON, &CompilationDialog::OnCloseButtonClicked, this);
            
            wxStdDialogButtonSizer* stdButtonSizer = new wxStdDialogButtonSizer();
            stdButtonSizer->SetAffirmativeButton(compileButton);
			stdButtonSizer->SetCancelButton(closeButton);
            stdButtonSizer->Realize();
            
            m_currentRunLabel = new wxStaticText(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
            
            wxSizer* currentRunLabelSizer = new wxBoxSizer(wxVERTICAL);
            currentRunLabelSizer->AddStretchSpacer();
            currentRunLabelSizer->Add(m_currentRunLabel, wxSizerFlags().Expand());
            currentRunLabelSizer->AddStretchSpacer();
            
            wxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
            buttonSizer->Add(launchButton, wxSizerFlags().CenterVertical());
            buttonSizer->Add(currentRunLabelSizer, wxSizerFlags().Expand().Proportion(1).Border(wxLEFT | wxRIGHT, LayoutConstants::WideHMargin));
            buttonSizer->Add(stdButtonSizer);
            
            wxSizer* dialogSizer = new wxBoxSizer(wxVERTICAL);
            dialogSizer->Add(outerPanel, wxSizerFlags().Expand().Proportion(1));
            dialogSizer->Add(wrapDialogButtonSizer(buttonSizer, this), wxSizerFlags().Expand());
            SetSizer(dialogSizer);
            
            m_run.Bind(wxEVT_COMPILATION_END, &CompilationDialog::OnCompilationEnd, this);
            Bind(wxEVT_CLOSE_WINDOW, &CompilationDialog::OnClose, this);
        }

        void CompilationDialog::OnLaunchClicked(wxCommandEvent& event) {
            LaunchGameEngineDialog dialog(this, m_mapFrame->document());
            dialog.ShowModal();
        }
        
        void CompilationDialog::OnUpdateLaunchButtonUI(wxUpdateUIEvent& event) {
            event.Enable(!m_run.running());
        }
        

        void CompilationDialog::OnToggleCompileClicked(wxCommandEvent& event) {
            if (m_run.running()) {
                m_run.terminate();
            } else {
                const Model::CompilationProfile* profile = m_profileManager->selectedProfile();
                ensure(profile != NULL, "profile is null");
                m_output->Clear();
                
                if (testRun())
                    m_run.test(profile, m_mapFrame->document(), m_output);
                else
                    m_run.run(profile, m_mapFrame->document(), m_output);
                
                m_currentRunLabel->SetLabel("Running " + profile->name());
                Layout();
            }
        }

        void CompilationDialog::OnUpdateCompileButtonUI(wxUpdateUIEvent& event) {
            if (m_run.running()) {
                event.SetText("Stop");
                event.Enable(true);
            } else {
                if (testRun())
                    event.SetText("Test");
                else
                    event.SetText("Run");
                event.Enable(m_profileManager->selectedProfile() != NULL);
            }
        }

		void CompilationDialog::OnCloseButtonClicked(wxCommandEvent& event) {
			Close();
		}

		void CompilationDialog::OnClose(wxCloseEvent& event) {
            if (event.CanVeto() && m_run.running()) {
                const int result = ::wxMessageBox("Closing this dialog will stop the running compilation. Are you sure?", "TrenchBroom", wxYES_NO, this);
                if (result == wxNO)
                    event.Veto();
                else
                    m_run.terminate();
            }
            if (!event.GetVeto()) {
                m_mapFrame->compilationDialogWillClose();
                if (GetParent() != NULL)
                    GetParent()->Raise();
                Destroy();
            }
        }

        void CompilationDialog::OnCompilationEnd(wxEvent& event) {
            m_currentRunLabel->SetLabel("");
        }
        
        bool CompilationDialog::testRun() const {
            return wxGetKeyState(WXK_ALT);
        }
    }
}
