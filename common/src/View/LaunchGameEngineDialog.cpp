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
#include <wx/msgdlg.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <QLabel>
#include <wx/utils.h>

namespace TrenchBroom {
    namespace View {
        LaunchGameEngineDialog::LaunchGameEngineDialog(QWidget* parent, MapDocumentWPtr document) :
        wxDialog(parent, wxID_ANY, "Launch Engine"),
        m_document(document),
        m_gameEngineList(nullptr),
        m_parameterText(nullptr),
        m_lastProfile(nullptr) {
            createGui();
        }

        void LaunchGameEngineDialog::createGui() {
            setWindowIconTB(this);

            MapDocumentSPtr document = lock(m_document);
            const String& gameName = document->game()->gameName();
            CurrentGameIndicator* gameIndicator = new CurrentGameIndicator(this, gameName);

            QWidget* midPanel = new QWidget(this);

            Model::GameFactory& gameFactory = Model::GameFactory::instance();
            const Model::GameConfig& gameConfig = gameFactory.gameConfig(gameName);
            const Model::GameEngineConfig& gameEngineConfig = gameConfig.gameEngineConfig();
            m_gameEngineList = new GameEngineProfileListBox(midPanel, gameEngineConfig);
            m_gameEngineList->SetEmptyText("Click the 'Configure engines...' button to create a game engine profile.");

            QLabel* header = new QLabel(midPanel, wxID_ANY, "Launch Engine");
            header->SetFont(header->GetFont().Larger().Larger().Bold());

            QLabel* message = new QLabel(midPanel, wxID_ANY, "Select a game engine from the list on the right and edit the commandline parameters in the text box below. You can use variables to refer to the map name and other values.");
            message->Wrap(350);

            wxButton* openPreferencesButton = new wxButton(midPanel, wxID_ANY, "Configure engines...");
            openPreferencesButton->Bind(&QAbstractButton::clicked, &LaunchGameEngineDialog::OnEditGameEnginesButton, this);

            QLabel* parameterLabel = new QLabel(midPanel, wxID_ANY, "Parameters");
            parameterLabel->SetFont(parameterLabel->GetFont().Bold());

            m_parameterText = new AutoCompleteTextControl(midPanel, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
            m_parameterText->Bind(wxEVT_TEXT_ENTER, &LaunchGameEngineDialog::OnLaunch, this);
            m_parameterText->Bind(wxEVT_TEXT, &LaunchGameEngineDialog::OnParameterTextChanged, this);
            m_parameterText->Bind(wxEVT_UPDATE_UI, &LaunchGameEngineDialog::OnUpdateParameterTextUI, this);

            m_parameterText->SetHelper(new ELAutoCompleteHelper(variables()));

            auto* midLeftLayout = new QVBoxLayout();
            midLeftLayout->addSpacing(20);
            midLeftLayout->addWidget(header, wxLayoutFlags().Expand());
            midLeftLayout->addSpacing(20);
            midLeftLayout->addWidget(message, wxLayoutFlags().Expand());
            midLeftLayout->addSpacing(10);
            midLeftLayout->addWidget(openPreferencesButton, wxLayoutFlags().CenterHorizontal());
            midLeftLayout->addStretch(1);
            midLeftLayout->addWidget(parameterLabel);
            midLeftLayout->addSpacing(LayoutConstants::NarrowVMargin);
            midLeftLayout->addWidget(m_parameterText, wxLayoutFlags().Expand());
            midLeftLayout->addSpacing(20);

            auto* midLayout = new QHBoxLayout();
            midLayout->addSpacing(20);
            midLayout->addWidget(midLeftLayout, wxLayoutFlags().Expand().Proportion(1));
            midLayout->addSpacing(20);
            midLayout->addWidget(new BorderLine(midPanel, BorderLine::Direction_Vertical), wxLayoutFlags().Expand());
            midLayout->addWidget(m_gameEngineList, wxLayoutFlags().Expand());
            midLayout->SetItemMinSize(m_gameEngineList, wxSize(250, 280));
            midPanel->setLayout(midLayout);

            wxButton* closeButton = new wxButton(this, wxID_CANCEL, "Cancel");
            closeButton->Bind(&QAbstractButton::clicked, &LaunchGameEngineDialog::OnCloseButton, this);
            closeButton->Bind(wxEVT_UPDATE_UI, &LaunchGameEngineDialog::OnUpdateCloseButtonUI, this);

            wxButton* launchButton = new wxButton(this, wxID_OK, "Launch");
            launchButton->Bind(&QAbstractButton::clicked, &LaunchGameEngineDialog::OnLaunch, this);
            launchButton->Bind(wxEVT_UPDATE_UI, &LaunchGameEngineDialog::OnUpdateLaunchButtonUI, this);

            wxStdDialogButtonLayout* buttonLayout = new wxStdDialogButtonLayout();
            buttonLayout->SetCancelButton(closeButton);
            buttonLayout->SetAffirmativeButton(launchButton);
            buttonLayout->Realize();

            auto* outerLayout = new QVBoxLayout();
            outerLayout->addWidget(gameIndicator, wxLayoutFlags().Expand());
            outerLayout->addWidget(new BorderLine(nullptr, BorderLine::Direction_Horizontal), wxLayoutFlags().Expand());
            outerLayout->addWidget(midPanel, wxLayoutFlags().Expand().Proportion(1));
            outerLayout->addWidget(wrapDialogButtonLayout(buttonLayout, this), wxLayoutFlags().Expand());
            setLayout(outerLayout);

            m_gameEngineList->Bind(wxEVT_LISTBOX, &LaunchGameEngineDialog::OnSelectGameEngineProfile, this);
            m_gameEngineList->Bind(wxEVT_LISTBOX_DCLICK, &LaunchGameEngineDialog::OnLaunch, this);
            Bind(wxEVT_CLOSE_WINDOW, &LaunchGameEngineDialog::OnClose, this);

            if (m_gameEngineList->GetItemCount() > 0)
                m_gameEngineList->SetSelection(0);
        }

        LaunchGameEngineVariables LaunchGameEngineDialog::variables() const {
            return LaunchGameEngineVariables(lock(m_document));
        }

        void LaunchGameEngineDialog::OnSelectGameEngineProfile() {
            m_lastProfile = m_gameEngineList->selectedProfile();
            if (m_lastProfile != nullptr) {
                m_parameterText->ChangeValue(m_lastProfile->parameterSpec());
            } else {
                m_parameterText->ChangeValue("");
            }
        }

        void LaunchGameEngineDialog::OnUpdateParameterTextUI() {
            event.Enable(m_gameEngineList->GetSelection() != wxNOT_FOUND);
        }

        void LaunchGameEngineDialog::OnParameterTextChanged() {
            Model::GameEngineProfile* profile = m_gameEngineList->selectedProfile();
            if (profile != nullptr)
                profile->setParameterSpec(m_parameterText->GetValue().ToStdString());
        }

        void LaunchGameEngineDialog::OnEditGameEnginesButton() {


            const bool wasEmpty = m_gameEngineList->GetItemCount() == 0;

            GameEngineDialog dialog(this, lock(m_document)->game()->gameName());
            dialog.ShowModal();

            if (wasEmpty && m_gameEngineList->GetItemCount() > 0)
                m_gameEngineList->SetSelection(0);
        }

        void LaunchGameEngineDialog::OnCloseButton() {
            EndModal(wxCANCEL);
        }

        void LaunchGameEngineDialog::OnUpdateCloseButtonUI() {
            event.Enable(true);
        }

        void LaunchGameEngineDialog::OnLaunch() {
            try {
                const Model::GameEngineProfile* profile = m_gameEngineList->selectedProfile();
                ensure(profile != nullptr, "profile is null");

                const IO::Path& executablePath = profile->path();
                const String escapedExecutablePath = "\"" + executablePath.asString() + "\"";

                const String& parameterSpec = profile->parameterSpec();
                const String parameters = EL::interpolate(parameterSpec, variables());

                QString launchStr;
#ifdef __APPLE__
                // We have to launch apps via the 'open' command so that we can properly pass parameters.
                launchStr << "/usr/bin/open" << " " << escapedExecutablePath << " --args " << parameters;
#else
                launchStr << escapedExecutablePath << " " << parameters;
#endif

                wxExecuteEnv env;
                env.cwd = executablePath.deleteLastComponent().asString();

                wxExecute(launchStr, wxEXEC_ASYNC, nullptr, &env);
                EndModal(wxOK);
            } catch (const Exception& e) {
                StringStream message;
                message << "Could not launch game engine: " << e.what();
                ::wxMessageBox(message.str(), "TrenchBroom", wxOK | wxICON_ERROR, this);
            }
        }

        void LaunchGameEngineDialog::OnUpdateLaunchButtonUI() {
            event.Enable(m_gameEngineList->GetSelection() != wxNOT_FOUND);
        }

        void LaunchGameEngineDialog::OnClose(wxCloseEvent& event) {
            if (GetParent() != nullptr)
                GetParent()->Raise();
            event.Skip();
        }
    }
}
