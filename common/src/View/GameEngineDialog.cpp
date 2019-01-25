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

#include "GameEngineDialog.h"

#include "IO/Path.h"
#include "Model/GameFactory.h"
#include "View/BorderLine.h"
#include "View/CurrentGameIndicator.h"
#include "View/GameEngineProfileManager.h"
#include "View/ViewConstants.h"
#include "View/wxUtils.h"

#include <wx/button.h>
#include <wx/settings.h>
#include <wx/simplebook.h>
#include <wx/sizer.h>
#include <wx/statbmp.h>
#include <QLabel>
#include <wx/textctrl.h>

namespace TrenchBroom {
    namespace View {
        GameEngineDialog::GameEngineDialog(QWidget* parent, const String& gameName) :
        wxDialog(parent, wxID_ANY, "Game Engines"),
        m_gameName(gameName),
        m_profileManager(nullptr) {
            createGui();
            SetSize(600, 400);
            CentreOnParent();
        }

        void GameEngineDialog::createGui() {
            setWindowIcon(this);

            CurrentGameIndicator* gameIndicator = new CurrentGameIndicator(this, m_gameName);
            
            Model::GameFactory& gameFactory = Model::GameFactory::instance();
            Model::GameConfig& gameConfig = gameFactory.gameConfig(m_gameName);
            m_profileManager = new GameEngineProfileManager(this, gameConfig.gameEngineConfig());
            
            wxButton* closeButton = new wxButton(this, wxID_CANCEL, "Close");
            closeButton->Bind(wxEVT_BUTTON, &GameEngineDialog::OnCloseButtonClicked, this);
            closeButton->Bind(wxEVT_UPDATE_UI, &GameEngineDialog::OnUpdateCloseButtonUI, this);
            
            wxStdDialogButtonSizer* buttonSizer = new wxStdDialogButtonSizer();
            buttonSizer->SetCancelButton(closeButton);
            buttonSizer->Realize();
            
            auto* outerSizer = new QVBoxLayout();
            outerSizer->Add(gameIndicator, wxSizerFlags().Expand());
            outerSizer->Add(new BorderLine(this, BorderLine::Direction_Horizontal), wxSizerFlags().Expand());
            outerSizer->Add(m_profileManager, wxSizerFlags().Expand().Proportion(1));
            outerSizer->Add(wrapDialogButtonSizer(buttonSizer, this), wxSizerFlags().Expand());
            SetSizer(outerSizer);

            Bind(wxEVT_CLOSE_WINDOW, &GameEngineDialog::OnClose, this);
        }

        void GameEngineDialog::OnUpdateCloseButtonUI(wxUpdateUIEvent& event) {
            event.Enable(true);
        }

        void GameEngineDialog::OnCloseButtonClicked(wxCommandEvent& event) {
            EndModal(wxID_OK);
        }

        void GameEngineDialog::OnClose(wxCloseEvent& event) {
            if (GetParent() != nullptr)
                GetParent()->Raise();
            event.Skip();
        }
    }
}
