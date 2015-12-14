/*
 Copyright (C) 2010-2014 Kristian Duske
 
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

#include "WelcomeFrame.h"

#include "TrenchBroomApp.h"
#include "View/AppInfoPanel.h"
#include "View/BorderLine.h"
#include "View/ViewConstants.h"
#include "View/RecentDocumentListBox.h"
#include "View/RecentDocumentSelectedCommand.h"

#include <wx/panel.h>
#include <wx/sizer.h>

namespace TrenchBroom {
    namespace View {
        IMPLEMENT_DYNAMIC_CLASS(WelcomeFrame, wxFrame)

        WelcomeFrame::WelcomeFrame() :
        wxFrame(NULL, wxID_ANY, "Welcome to TrenchBroom", wxDefaultPosition, wxDefaultSize, wxCAPTION | wxCLOSE_BOX | wxCLIP_CHILDREN) {
            createGui();
            bindEvents();
            Centre();
        }

        void WelcomeFrame::createGui() {
            wxPanel* container = new wxPanel(this);
            container->SetBackgroundColour(*wxWHITE);
            
            wxPanel* appPanel = createAppPanel(container);
            m_recentDocumentListBox = new RecentDocumentListBox(container);
            m_recentDocumentListBox->SetToolTip("Double click on a map to open it");
            
            wxBoxSizer* innerSizer = new wxBoxSizer(wxHORIZONTAL);
            innerSizer->Add(appPanel, 0, wxALIGN_CENTRE_VERTICAL);
            innerSizer->Add(new BorderLine(container, BorderLine::Direction_Vertical), 0, wxEXPAND);
            innerSizer->Add(m_recentDocumentListBox, 1, wxEXPAND);
            innerSizer->SetItemMinSize(m_recentDocumentListBox, wxSize(300, 10 * m_recentDocumentListBox->itemHeight()));
            container->SetSizer(innerSizer);
            
            wxBoxSizer* outerSizer = new wxBoxSizer(wxHORIZONTAL);
            outerSizer->Add(container, 1, wxEXPAND);
            
            SetSizerAndFit(outerSizer);
        }

        void WelcomeFrame::OnCreateNewDocumentClicked(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            Hide();
            TrenchBroomApp& app = TrenchBroomApp::instance();
            try {
                if (app.newDocument())
                    Destroy();
                else
                    Show();
            } catch (...) {
                Show();
            }
        }
        
        void WelcomeFrame::OnOpenOtherDocumentClicked(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            const wxString pathStr = ::wxLoadFileSelector("", "map", "", NULL);
            if (!pathStr.empty()) {
                Hide();
                TrenchBroomApp& app = TrenchBroomApp::instance();
                try {
                    if (app.openDocument(pathStr.ToStdString()))
                        Destroy();
                    else
                        Show();
                } catch (...) {
                    Show();
                }
            }
        }

        void WelcomeFrame::OnRecentDocumentSelected(RecentDocumentSelectedCommand& event) {
            if (IsBeingDeleted()) return;

            Hide();
            TrenchBroomApp& app = TrenchBroomApp::instance();
            try {
                if (app.openDocument(event.documentPath().asString()))
                    Destroy();
                else
                    Show();
            } catch (...) {
                Show();
            }
        }

        wxPanel* WelcomeFrame::createAppPanel(wxWindow* parent) {
            wxPanel* appPanel = new wxPanel(parent);
            appPanel->SetBackgroundColour(*wxWHITE);
            AppInfoPanel* infoPanel = new AppInfoPanel(appPanel);
            
            m_createNewDocumentButton = new wxButton(appPanel, wxID_ANY, "New map...");
            m_createNewDocumentButton->SetToolTip("Create a new map document");
            m_openOtherDocumentButton = new wxButton(appPanel, wxID_ANY, "Browse...");
            m_openOtherDocumentButton->SetToolTip("Open an existing map document");
            
            wxBoxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
            buttonSizer->Add(m_createNewDocumentButton, 1, wxEXPAND);
            buttonSizer->AddSpacer(LayoutConstants::WideHMargin);
            buttonSizer->Add(m_openOtherDocumentButton, 1, wxEXPAND);

            wxBoxSizer* outerSizer = new wxBoxSizer(wxVERTICAL);
            outerSizer->Add(infoPanel, 0, wxALIGN_CENTER_HORIZONTAL | wxLEFT | wxRIGHT, 50);
            outerSizer->AddSpacer(20);
            outerSizer->Add(buttonSizer, 0, wxALIGN_CENTER_HORIZONTAL | wxLEFT | wxRIGHT, 50);
            outerSizer->AddSpacer(20);
            
            appPanel->SetSizer(outerSizer);

            return appPanel;
        }

        void WelcomeFrame::bindEvents() {
            m_createNewDocumentButton->Bind(wxEVT_BUTTON, &WelcomeFrame::OnCreateNewDocumentClicked, this);
            m_openOtherDocumentButton->Bind(wxEVT_BUTTON, &WelcomeFrame::OnOpenOtherDocumentClicked, this);
            m_recentDocumentListBox->Bind(RECENT_DOCUMENT_SELECTED_EVENT, &WelcomeFrame::OnRecentDocumentSelected, this);
        }
    }
}
