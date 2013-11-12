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

#include "WelcomeDialog.h"

#include "IO/Path.h"
#include "IO/ResourceUtils.h"
#include "View/LayoutConstants.h"
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
        WelcomeDialog::WelcomeDialog() :
        wxDialog(NULL, wxID_ANY, _("Welcome to TrenchBroom"), wxDefaultPosition, wxDefaultSize, (wxDEFAULT_DIALOG_STYLE | wxDIALOG_NO_PARENT)
#if not defined __APPLE__
                 & ~wxCLOSE_BOX
#endif
                 ),
        m_documentPath("") {
            SetSize(700, 420);
            createGui();
            bindEvents();
            Centre();
        }

        void WelcomeDialog::createGui() {
            wxPanel* container = new wxPanel(this);
            wxPanel* appPanel = createAppPanel(container);
            m_recentDocumentListBox = new RecentDocumentListBox(container);
            
            wxBoxSizer* innerSizer = new wxBoxSizer(wxHORIZONTAL);
            innerSizer->Add(appPanel, 0, wxEXPAND);
            innerSizer->Add(new wxStaticLine(container, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_VERTICAL), 0, wxEXPAND);
            innerSizer->Add(m_recentDocumentListBox, 1, wxEXPAND);
            container->SetSizerAndFit(innerSizer);
            
            wxBoxSizer* outerSizer = new wxBoxSizer(wxHORIZONTAL);
            outerSizer->Add(container, 1, wxEXPAND);
            
            SetSizer(outerSizer);
        }
        
        const IO::Path& WelcomeDialog::documentPath() const {
            return m_documentPath;
        }

        void WelcomeDialog::OnCreateNewDocumentClicked(wxCommandEvent& event) {
            EndModal(CreateNewDocument);
        }
        
        void WelcomeDialog::OnOpenOtherDocumentClicked(wxCommandEvent& event) {
            const wxString pathStr = ::wxLoadFileSelector("", "map", "", NULL);
            if (!pathStr.empty()) {
                m_documentPath = IO::Path(pathStr.ToStdString());
                EndModal(OpenDocument);
            } else {
                EndModal(wxID_CANCEL);
            }
        }

        void WelcomeDialog::OnRecentDocumentSelected(RecentDocumentSelectedCommand& event) {
            m_documentPath = event.documentPath();
            EndModal(OpenDocument);
        }

        wxPanel* WelcomeDialog::createAppPanel(wxWindow* parent) {
            wxPanel* appPanel = new wxPanel(parent);
            appPanel->SetBackgroundColour(*wxWHITE);
            
            const wxBitmap appIconImage = IO::loadImageResource(IO::Path("images/AppIcon.png"));
            wxStaticBitmap* appIcon = new wxStaticBitmap(appPanel, wxID_ANY, appIconImage);
            wxStaticText* appName = new wxStaticText(appPanel, wxID_ANY, _("TrenchBroom"));
            appName->SetFont(appName->GetFont().Larger().Larger().Larger().Larger().Bold());
            wxStaticLine* appLine = new wxStaticLine(appPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL);
            wxStaticText* appClaim = new wxStaticText(appPanel, wxID_ANY, _("A Modern Level Editor for Quake"));
            wxString versionStr(_("Version 2.0"));
            // versionStr << VERSIONSTR;
            wxStaticText* version = new wxStaticText(appPanel, wxID_ANY, versionStr);
            version->SetFont(version->GetFont().Smaller());
            version->SetForegroundColour(wxColor(96, 96, 96));
            
            wxBoxSizer* innerSizer = new wxBoxSizer(wxVERTICAL);
            innerSizer->Add(appIcon, 0, wxALIGN_CENTER_HORIZONTAL);
            innerSizer->Add(appName, 0, wxALIGN_CENTER_HORIZONTAL);
            innerSizer->Add(appLine, 0, wxEXPAND);
            innerSizer->Add(appClaim, 0, wxALIGN_CENTER_HORIZONTAL);
            innerSizer->Add(version, 0, wxALIGN_CENTER_HORIZONTAL);
            innerSizer->AddStretchSpacer();
            
            m_createNewDocumentButton = new wxButton(appPanel, wxID_ANY, _("Create new map..."));
            m_openOtherDocumentButton = new wxButton(appPanel, wxID_ANY, _("Open existing map..."));
            
            wxBoxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
            buttonSizer->Add(m_createNewDocumentButton, 1, wxEXPAND);
            buttonSizer->AddSpacer(LayoutConstants::ControlHorizontalMargin);
            buttonSizer->Add(m_openOtherDocumentButton, 1, wxEXPAND);

            wxBoxSizer* outerSizer = new wxBoxSizer(wxVERTICAL);
            outerSizer->Add(innerSizer, 0, wxALIGN_CENTER_HORIZONTAL | wxLEFT | wxRIGHT, 50);
            outerSizer->AddSpacer(20);
            outerSizer->Add(buttonSizer, 0, wxALIGN_CENTER_HORIZONTAL | wxLEFT | wxRIGHT, 50);
            
            appPanel->SetSizer(outerSizer);

            return appPanel;
        }

        void WelcomeDialog::bindEvents() {
            m_createNewDocumentButton->Bind(wxEVT_BUTTON, &WelcomeDialog::OnCreateNewDocumentClicked, this);
            m_openOtherDocumentButton->Bind(wxEVT_BUTTON, &WelcomeDialog::OnOpenOtherDocumentClicked, this);
            m_recentDocumentListBox->Bind(EVT_RECENT_DOCUMENT_SELECTED_EVENT, EVT_RECENT_DOCUMENT_SELECTED_HANDLER(WelcomeDialog::OnRecentDocumentSelected), this);
        }
    }
}
