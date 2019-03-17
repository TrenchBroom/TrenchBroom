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

#include "CrashDialog.h"

#include "StringUtils.h"
#include "View/BorderLine.h"
#include "View/GetVersion.h"
#include "View/ViewConstants.h"
#include "View/wxUtils.h"

#include <wx/button.h>
#include <wx/gbsizer.h>
#include <wx/panel.h>
#include <wx/settings.h>
#include <wx/sizer.h>
#include <wx/stattext.h>


namespace TrenchBroom {
    namespace View {
        wxIMPLEMENT_DYNAMIC_CLASS(CrashDialog, wxDialog)

        CrashDialog::CrashDialog() {}

        void CrashDialog::Create(const IO::Path& reportPath, const IO::Path& mapPath, const IO::Path& logPath) {
            wxDialog::Create(nullptr, wxID_ANY, "Crash");

            wxPanel* containerPanel = new wxPanel(this);
            wxPanel* headerPanel = new wxPanel(containerPanel);
            wxPanel* reportPanel = new wxPanel(containerPanel);
            reportPanel->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOX));
            reportPanel->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOXTEXT));

            wxStaticText* header = new wxStaticText(headerPanel, wxID_ANY, "Crash Report");
            header->SetFont(header->GetFont().Scale(1.5).Bold());

            wxSizer* headerPanelSizer = new wxBoxSizer(wxVERTICAL);
            headerPanelSizer->AddSpacer(LayoutConstants::DialogOuterMargin);
            headerPanelSizer->Add(header, wxSizerFlags().Border(wxLEFT | wxRIGHT, LayoutConstants::DialogOuterMargin));
            headerPanelSizer->AddSpacer(LayoutConstants::DialogOuterMargin);
            headerPanel->SetSizer(headerPanelSizer);

            wxStaticText* text1 = new wxStaticText(reportPanel, wxID_ANY,
                                                   "TrenchBroom has crashed, but was able to save a crash report,\n"
                                                   "a log file and the current state of the map to the following locations.\n\n"
                                                   "Please create an issue report and upload all three files.");

            wxStaticText* reportLabel = new wxStaticText(reportPanel, wxID_ANY, "Report");
            reportLabel->SetFont(reportLabel->GetFont().Bold());
            wxStaticText* reportPathText = new wxStaticText(reportPanel, wxID_ANY, reportPath.asString());

            wxStaticText* mapLabel = new wxStaticText(reportPanel, wxID_ANY, "Map");
            mapLabel->SetFont(mapLabel->GetFont().Bold());
            wxStaticText* mapPathText = new wxStaticText(reportPanel, wxID_ANY, mapPath.asString());

            wxStaticText* logLabel = new wxStaticText(reportPanel, wxID_ANY, "Log");
            logLabel->SetFont(logLabel->GetFont().Bold());
            wxStaticText* logPathText = new wxStaticText(reportPanel, wxID_ANY, logPath.asString());

            wxStaticText* versionLabel = new wxStaticText(reportPanel, wxID_ANY, "Version");
            versionLabel->SetFont(versionLabel->GetFont().Bold());
            wxStaticText* versionText = new wxStaticText(reportPanel, wxID_ANY, getBuildVersion());

            wxStaticText* buildLabel = new wxStaticText(reportPanel, wxID_ANY, "Build");
            buildLabel->SetFont(buildLabel->GetFont().Bold());
            wxStaticText* buildText = new wxStaticText(reportPanel, wxID_ANY, getBuildIdStr());

            wxGridBagSizer* reportPanelSizer = new wxGridBagSizer(LayoutConstants::NarrowVMargin, LayoutConstants::WideHMargin);
            reportPanelSizer->Add(text1,           wxGBPosition(0, 0), wxGBSpan(1, 2));
            reportPanelSizer->Add(1, 20,           wxGBPosition(1, 0), wxGBSpan(1, 2));
            reportPanelSizer->Add(reportLabel,     wxGBPosition(2, 0), wxGBSpan(1, 1), wxALIGN_CENTER_VERTICAL);
            reportPanelSizer->Add(reportPathText,  wxGBPosition(2, 1), wxGBSpan(1, 1), wxALIGN_CENTER_VERTICAL);
            reportPanelSizer->Add(mapLabel,        wxGBPosition(3, 0), wxGBSpan(1, 1), wxALIGN_CENTER_VERTICAL);
            reportPanelSizer->Add(mapPathText,     wxGBPosition(3, 1), wxGBSpan(1, 1), wxALIGN_CENTER_VERTICAL);
            reportPanelSizer->Add(logLabel,        wxGBPosition(4, 0), wxGBSpan(1, 1), wxALIGN_CENTER_VERTICAL);
            reportPanelSizer->Add(logPathText,     wxGBPosition(4, 1), wxGBSpan(1, 1), wxALIGN_CENTER_VERTICAL);
            reportPanelSizer->Add(1, 20,           wxGBPosition(5, 0), wxGBSpan(1, 2));
            reportPanelSizer->Add(versionLabel,    wxGBPosition(6, 0), wxGBSpan(1, 1), wxALIGN_CENTER_VERTICAL);
            reportPanelSizer->Add(versionText,     wxGBPosition(6, 1), wxGBSpan(1, 1), wxALIGN_CENTER_VERTICAL);
            reportPanelSizer->Add(buildLabel,      wxGBPosition(7, 0), wxGBSpan(1, 1), wxALIGN_CENTER_VERTICAL);
            reportPanelSizer->Add(buildText,       wxGBPosition(7, 1), wxGBSpan(1, 1), wxALIGN_CENTER_VERTICAL);

            wxSizer* reportPanelOuterSizer = new wxBoxSizer(wxVERTICAL);
            reportPanelOuterSizer->Add(new BorderLine(reportPanel), wxSizerFlags().Expand());
            reportPanelOuterSizer->Add(reportPanelSizer, wxSizerFlags().Border(wxLEFT | wxRIGHT, LayoutConstants::DialogOuterMargin));
            reportPanelOuterSizer->AddSpacer(LayoutConstants::DialogOuterMargin);
            reportPanel->SetSizer(reportPanelOuterSizer);

            wxSizer* containerPanelSizer = new wxBoxSizer(wxVERTICAL);
            containerPanelSizer->Add(headerPanel, wxSizerFlags().Expand());
            containerPanelSizer->Add(reportPanel, wxSizerFlags().Expand());
            containerPanel->SetSizer(containerPanelSizer);

            wxButton* reportButton = new wxButton(this, wxID_APPLY, "Report");
            reportButton->Bind(wxEVT_BUTTON, &CrashDialog::OnReport, this);

            wxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
            buttonSizer->AddStretchSpacer();
            buttonSizer->Add(reportButton, wxSizerFlags().CenterVertical().Border(wxLEFT | wxRIGHT, LayoutConstants::DialogOuterMargin));
            buttonSizer->Add(CreateButtonSizer(wxCLOSE));

            wxSizer* outerSizer = new wxBoxSizer(wxVERTICAL);
            outerSizer->Add(containerPanel, wxSizerFlags().Expand());
            outerSizer->Add(wrapDialogButtonSizer(buttonSizer, this), 0, wxEXPAND);

            SetSizerAndFit(outerSizer);
        }

        void CrashDialog::OnReport(wxCommandEvent& event) {
            wxLaunchDefaultBrowser("https://github.com/kduske/TrenchBroom/issues/new");
        }
    }
}
