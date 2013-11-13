/*
 Copyright (C) 2010-2012 Kristian Duske
 
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

#include "NewDocumentDialog.h"

#include "View/GameListBox.h"
#include "View/GameSelectedCommand.h"

#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/statline.h>
#include <wx/stattext.h>

namespace TrenchBroom {
    namespace View {
        NewDocumentDialog::NewDocumentDialog(wxWindow* parent) :
        wxDialog(parent, wxID_ANY, _("Create New Map")) {
            SetSize(550, 350);
            createGui();
            bindEvents();
            CentreOnParent();
        }
        
        const String NewDocumentDialog::selectedGameName() const {
            return m_gameListBox->selectedGameName();
        }

        void NewDocumentDialog::OnGameSelected(GameSelectedCommand& event) {
            EndModal(wxID_OK);
        }

        void NewDocumentDialog::OnUpdateOkButton(wxUpdateUIEvent& event) {
            event.Enable(m_gameListBox->GetSelectedCount() > 0);
        }

        void NewDocumentDialog::createGui() {
            wxPanel* infoPanel = createInfoPanel(this);
            wxPanel* gameList = createGameList(this);
            
            wxBoxSizer* innerSizer = new wxBoxSizer(wxHORIZONTAL);
            innerSizer->Add(infoPanel, 0, wxEXPAND);
            innerSizer->Add(new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_VERTICAL), 0, wxEXPAND);
            innerSizer->Add(gameList, 1, wxEXPAND);
            
            wxSizer* buttonSizer = CreateStdDialogButtonSizer(wxOK | wxCANCEL);
            
            wxBoxSizer* outerSizer = new wxBoxSizer(wxVERTICAL);
            outerSizer->Add(innerSizer, 1, wxEXPAND);
            outerSizer->Add(new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL), 0, wxEXPAND);
            outerSizer->Add(buttonSizer, 0, wxEXPAND);
            
            SetSizer(outerSizer);
        }
        
        wxPanel* NewDocumentDialog::createInfoPanel(wxWindow* parent) {
            wxPanel* infoPanel = new wxPanel(parent);
            infoPanel->wxWindowBase::SetBackgroundColour(*wxWHITE);
            
            wxStaticText* header = new wxStaticText(infoPanel, wxID_ANY, _("Create New Map"));
            header->SetFont(header->GetFont().Larger().Larger().Bold());
            
            wxStaticText* info1 = new wxStaticText(infoPanel, wxID_ANY, _("Select a game from the list on the right, then click OK."));
            wxStaticText* info2 = new wxStaticText(infoPanel, wxID_ANY, _("Once the new document is created, you can set up mod directories, entity definitions and textures by going to the map inspector, the entity inspector and the face inspector, respectively."));
            info1->Wrap(200);
            info2->Wrap(200);
            
            wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
            sizer->AddSpacer(20);
            sizer->Add(header, 0, wxLEFT | wxRIGHT, 20);
            sizer->AddSpacer(20);
            sizer->Add(info1, 0, wxLEFT | wxRIGHT, 20);
            sizer->AddSpacer(10);
            sizer->Add(info2, 0, wxLEFT | wxRIGHT, 20);
            infoPanel->SetSizer(sizer);
            
            return infoPanel;
        }
        
        wxPanel* NewDocumentDialog::createGameList(wxWindow* parent) {
            m_gameListBox = new GameListBox(parent);
            return m_gameListBox;
        }
        
        void NewDocumentDialog::bindEvents() {
            m_gameListBox->Bind(EVT_GAME_SELECTED_EVENT, EVT_GAME_SELECTED_HANDLER(NewDocumentDialog::OnGameSelected), this);
            FindWindow(wxID_OK)->Bind(wxEVT_UPDATE_UI, &NewDocumentDialog::OnUpdateOkButton, this);
        }
    }
}
