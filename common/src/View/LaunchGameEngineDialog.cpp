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

#include "LaunchGameEngineDialog.h"

#include "EL/Interpolator.h"
#include "Model/Game.h"
#include "Model/GameFactory.h"
#include "View/AutoCompleteTextControl.h"
#include "View/ELAutoCompleteHelper.h"
#include "View/BorderLine.h"
#include "View/CompilationVariables.h"
#include "View/CurrentGameIndicator.h"
#include "View/GameEngineDialog.h"
#include "View/GameEngineProfileListBox.h"
#include "View/MapDocument.h"
#include "View/ViewConstants.h"
#include "View/wxUtils.h"

#include <wx/button.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/utils.h>

namespace TrenchBroom {
    namespace View {
        LaunchGameEngineDialog::LaunchGameEngineDialog(wxWindow* parent, MapDocumentWPtr document) :
        wxDialog(parent, wxID_ANY, "Launch Engine"),
        m_document(document),
        m_gameEngineList(NULL),
        m_parameterText(NULL),
        m_lastProfile(NULL) {
            createGui();
        }
        
        void LaunchGameEngineDialog::createGui() {
            setWindowIcon(this);

            MapDocumentSPtr document = lock(m_document);
            const String& gameName = document->game()->gameName();
            CurrentGameIndicator* gameIndicator = new CurrentGameIndicator(this, gameName);
            
            wxPanel* midPanel = new wxPanel(this);
            
            Model::GameFactory& gameFactory = Model::GameFactory::instance();
            const Model::GameConfig& gameConfig = gameFactory.gameConfig(gameName);
            const Model::GameEngineConfig& gameEngineConfig = gameConfig.gameEngineConfig();
            m_gameEngineList = new GameEngineProfileListBox(midPanel, gameEngineConfig);
            m_gameEngineList->SetEmptyText("Click the 'Configure engines...' button to create a game engine profile.");
            
            wxStaticText* header = new wxStaticText(midPanel, wxID_ANY, "Launch Engine");
            header->SetFont(header->GetFont().Larger().Larger().Bold());
            
            wxStaticText* message = new wxStaticText(midPanel, wxID_ANY, "Select a game engine from the list on the right and edit the commandline parameters in the text box below. You can use variables to refer to the map name and other values.");
            message->Wrap(350);
            
            wxButton* openPreferencesButton = new wxButton(midPanel, wxID_ANY, "Configure engines...");
            openPreferencesButton->Bind(wxEVT_BUTTON, &LaunchGameEngineDialog::OnEditGameEnginesButton, this);
            
            wxStaticText* parameterLabel = new wxStaticText(midPanel, wxID_ANY, "Parameters");
            parameterLabel->SetFont(parameterLabel->GetFont().Bold());
            
            m_parameterText = new AutoCompleteTextControl(midPanel, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
            m_parameterText->Bind(wxEVT_TEXT_ENTER, &LaunchGameEngineDialog::OnLaunch, this);
            m_parameterText->Bind(wxEVT_TEXT, &LaunchGameEngineDialog::OnParameterTextChanged, this);
            m_parameterText->Bind(wxEVT_UPDATE_UI, &LaunchGameEngineDialog::OnUpdateParameterTextUI, this);
            
            m_parameterText->SetHelper(new ELAutoCompleteHelper(variables()));
            
            wxSizer* midLeftSizer = new wxBoxSizer(wxVERTICAL);
            midLeftSizer->AddSpacer(20);
            midLeftSizer->Add(header, wxSizerFlags().Expand());
            midLeftSizer->AddSpacer(20);
            midLeftSizer->Add(message, wxSizerFlags().Expand());
            midLeftSizer->AddSpacer(10);
            midLeftSizer->Add(openPreferencesButton, wxSizerFlags().CenterHorizontal());
            midLeftSizer->AddStretchSpacer();
            midLeftSizer->Add(parameterLabel);
            midLeftSizer->AddSpacer(LayoutConstants::NarrowVMargin);
            midLeftSizer->Add(m_parameterText, wxSizerFlags().Expand());
            midLeftSizer->AddSpacer(20);
            
            wxSizer* midSizer = new wxBoxSizer(wxHORIZONTAL);
            midSizer->AddSpacer(20);
            midSizer->Add(midLeftSizer, wxSizerFlags().Expand().Proportion(1));
            midSizer->AddSpacer(20);
            midSizer->Add(new BorderLine(midPanel, BorderLine::Direction_Vertical), wxSizerFlags().Expand());
            midSizer->Add(m_gameEngineList, wxSizerFlags().Expand());
            midSizer->SetItemMinSize(m_gameEngineList, wxSize(250, 280));
            midPanel->SetSizer(midSizer);
            
            wxButton* closeButton = new wxButton(this, wxID_CANCEL, "Cancel");
            closeButton->Bind(wxEVT_BUTTON, &LaunchGameEngineDialog::OnCloseButton, this);
            closeButton->Bind(wxEVT_UPDATE_UI, &LaunchGameEngineDialog::OnUpdateCloseButtonUI, this);

            wxButton* launchButton = new wxButton(this, wxID_OK, "Launch");
            launchButton->Bind(wxEVT_BUTTON, &LaunchGameEngineDialog::OnLaunch, this);
            launchButton->Bind(wxEVT_UPDATE_UI, &LaunchGameEngineDialog::OnUpdateLaunchButtonUI, this);
            
            wxStdDialogButtonSizer* buttonSizer = new wxStdDialogButtonSizer();
            buttonSizer->SetCancelButton(closeButton);
            buttonSizer->SetAffirmativeButton(launchButton);
            buttonSizer->Realize();

            wxSizer* outerSizer = new wxBoxSizer(wxVERTICAL);
            outerSizer->Add(gameIndicator, wxSizerFlags().Expand());
            outerSizer->Add(new BorderLine(this, BorderLine::Direction_Horizontal), wxSizerFlags().Expand());
            outerSizer->Add(midPanel, wxSizerFlags().Expand().Proportion(1));
            outerSizer->Add(wrapDialogButtonSizer(buttonSizer, this), wxSizerFlags().Expand());
            
            SetSizerAndFit(outerSizer);

            m_gameEngineList->Bind(wxEVT_LISTBOX, &LaunchGameEngineDialog::OnSelectGameEngineProfile, this);
            m_gameEngineList->Bind(wxEVT_LISTBOX_DCLICK, &LaunchGameEngineDialog::OnLaunch, this);
            Bind(wxEVT_CLOSE_WINDOW, &LaunchGameEngineDialog::OnClose, this);
            
            if (m_gameEngineList->GetItemCount() > 0)
                m_gameEngineList->SetSelection(0);
        }

        LaunchGameEngineVariables LaunchGameEngineDialog::variables() const {
            return LaunchGameEngineVariables(lock(m_document));
        }

        void LaunchGameEngineDialog::OnSelectGameEngineProfile(wxCommandEvent& event) {
            m_lastProfile = m_gameEngineList->selectedProfile();
            if (m_lastProfile != NULL) {
                m_parameterText->ChangeValue(m_lastProfile->parameterSpec());
            } else {
                m_parameterText->ChangeValue("");
            }
        }

        void LaunchGameEngineDialog::OnUpdateParameterTextUI(wxUpdateUIEvent& event) {
            event.Enable(m_gameEngineList->GetSelection() != wxNOT_FOUND);
        }

        void LaunchGameEngineDialog::OnParameterTextChanged(wxCommandEvent& event) {
            Model::GameEngineProfile* profile = m_gameEngineList->selectedProfile();
            if (profile != NULL)
                profile->setParameterSpec(m_parameterText->GetValue().ToStdString());
        }

        void LaunchGameEngineDialog::OnEditGameEnginesButton(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            const bool wasEmpty = m_gameEngineList->GetItemCount() == 0;
            
            GameEngineDialog dialog(this, lock(m_document)->game()->gameName());
            dialog.ShowModal();

            if (wasEmpty && m_gameEngineList->GetItemCount() > 0)
                m_gameEngineList->SetSelection(0);
        }
        
        void LaunchGameEngineDialog::OnCloseButton(wxCommandEvent& event) {
            EndModal(wxCANCEL);
        }
        
        void LaunchGameEngineDialog::OnUpdateCloseButtonUI(wxUpdateUIEvent& event) {
            event.Enable(true);
        }
        
        void LaunchGameEngineDialog::OnLaunch(wxCommandEvent& event) {
            const Model::GameEngineProfile* profile = m_gameEngineList->selectedProfile();
            ensure(profile != NULL, "profile is null");
            
            const IO::Path& path = profile->path();
            const String& parameterSpec = profile->parameterSpec();
            const String parameters = EL::interpolate(parameterSpec, variables());

            wxString launchStr;
#ifdef __APPLE__
            // We have to launch apps via the 'open' command so that we can properly pass parameters.
            launchStr << "/usr/bin/open " << path.asString() << " --args " << parameters;
#else
            launchStr << path.asString() << " " << parameters;
#endif
            
            wxExecuteEnv env;
            env.cwd = path.deleteLastComponent().asString();
            
            wxExecute(launchStr, wxEXEC_ASYNC, NULL, &env);
            EndModal(wxOK);
        }
        
        void LaunchGameEngineDialog::OnUpdateLaunchButtonUI(wxUpdateUIEvent& event) {
            event.Enable(m_gameEngineList->GetSelection() != wxNOT_FOUND);
        }

        void LaunchGameEngineDialog::OnClose(wxCloseEvent& event) {
            if (GetParent() != NULL)
                GetParent()->Raise();
            event.Skip();
        }
    }
}
