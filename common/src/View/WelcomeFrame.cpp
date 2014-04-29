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
#include "IO/Path.h"
#include "IO/ResourceUtils.h"
#include "View/GetVersion.h"
#include "View/ViewConstants.h"
#include "View/RecentDocumentListBox.h"
#include "View/RecentDocumentSelectedCommand.h"

#include <wx/bitmap.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/statbmp.h>
#include <wx/statline.h>
#include <wx/stattext.h>

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
            wxPanel* appPanel = createAppPanel(container);
            m_recentDocumentListBox = new RecentDocumentListBox(container);
            m_recentDocumentListBox->SetToolTip("Double click on a map to open it");
            
            wxBoxSizer* innerSizer = new wxBoxSizer(wxHORIZONTAL);
            innerSizer->Add(appPanel, 0, wxEXPAND);
            innerSizer->Add(new wxStaticLine(container, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_VERTICAL), 0, wxEXPAND);
            innerSizer->Add(m_recentDocumentListBox, 1, wxEXPAND);
            innerSizer->SetItemMinSize(m_recentDocumentListBox, wxSize(300, wxDefaultSize.y));
            container->SetSizer(innerSizer);
            
            wxBoxSizer* outerSizer = new wxBoxSizer(wxHORIZONTAL);
            outerSizer->Add(container, 1, wxEXPAND);
            
            SetSizerAndFit(outerSizer);
        }

        void WelcomeFrame::OnCreateNewDocumentClicked(wxCommandEvent& event) {
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
            
            const wxBitmap appIconImage = IO::loadImageResource(IO::Path("images/AppIcon.png"));
            wxStaticBitmap* appIcon = new wxStaticBitmap(appPanel, wxID_ANY, appIconImage);
            wxStaticText* appName = new wxStaticText(appPanel, wxID_ANY, "TrenchBroom");
            appName->SetFont(appName->GetFont().Larger().Larger().Larger().Larger().Bold());
            wxStaticLine* appLine = new wxStaticLine(appPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL);
            wxStaticText* appClaim = new wxStaticText(appPanel, wxID_ANY, "A Modern Level Editor");
            wxString versionStr("Version ");
            versionStr << getVersion();
            wxStaticText* version = new wxStaticText(appPanel, wxID_ANY, versionStr);
#if not defined _WIN32
            version->SetFont(version->GetFont().Smaller());
#endif
            version->SetForegroundColour(wxColor(128, 128, 128));
            
            wxBoxSizer* innerSizer = new wxBoxSizer(wxVERTICAL);
            innerSizer->Add(appIcon, 0, wxALIGN_CENTER_HORIZONTAL);
            innerSizer->Add(appName, 0, wxALIGN_CENTER_HORIZONTAL);
            innerSizer->Add(appLine, 0, wxEXPAND);
            innerSizer->Add(appClaim, 0, wxALIGN_CENTER_HORIZONTAL);
            innerSizer->Add(version, 0, wxALIGN_CENTER_HORIZONTAL);
            innerSizer->AddStretchSpacer();
            
            m_createNewDocumentButton = new wxButton(appPanel, wxID_ANY, "Create document...");
            m_createNewDocumentButton->SetToolTip("Create a new map document");
            m_openOtherDocumentButton = new wxButton(appPanel, wxID_ANY, "Open document...");
            m_openOtherDocumentButton->SetToolTip("Open an existing map document");
            
            wxBoxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
            buttonSizer->Add(m_createNewDocumentButton, 1, wxEXPAND);
            buttonSizer->AddSpacer(LayoutConstants::ControlHorizontalMargin);
            buttonSizer->Add(m_openOtherDocumentButton, 1, wxEXPAND);

            wxBoxSizer* outerSizer = new wxBoxSizer(wxVERTICAL);
            outerSizer->Add(innerSizer, 0, wxALIGN_CENTER_HORIZONTAL | wxLEFT | wxRIGHT, 50);
            outerSizer->AddSpacer(20);
            outerSizer->Add(buttonSizer, 0, wxALIGN_CENTER_HORIZONTAL | wxLEFT | wxRIGHT, 50);
            outerSizer->AddSpacer(20);
            
            appPanel->SetSizer(outerSizer);

            return appPanel;
        }

        void WelcomeFrame::bindEvents() {
            m_createNewDocumentButton->Bind(wxEVT_BUTTON, &WelcomeFrame::OnCreateNewDocumentClicked, this);
            m_openOtherDocumentButton->Bind(wxEVT_BUTTON, &WelcomeFrame::OnOpenOtherDocumentClicked, this);
            m_recentDocumentListBox->Bind(EVT_RECENT_DOCUMENT_SELECTED_EVENT, EVT_RECENT_DOCUMENT_SELECTED_HANDLER(WelcomeFrame::OnRecentDocumentSelected), this);
        }
    }
}
