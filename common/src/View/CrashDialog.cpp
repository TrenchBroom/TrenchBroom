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

#include "CrashDialog.h"

#include "StringUtils.h"
#include "View/GetVersion.h"
#include "View/ViewConstants.h"
#include "View/wxUtils.h"

#include <wx/button.h>
#include <wx/gbsizer.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/stattext.h>


namespace TrenchBroom {
    namespace View {
        wxIMPLEMENT_DYNAMIC_CLASS(CrashDialog, wxDialog)

        CrashDialog::CrashDialog() {}
        
        void CrashDialog::Create(const IO::Path& logPath, const IO::Path& mapPath) {
            wxDialog::Create(NULL, wxID_ANY, "Crash");
            
            wxPanel* panel = new wxPanel(this);
            
            wxStaticText* header = new wxStaticText(panel, wxID_ANY, "Crash Report");
            header->SetFont(header->GetFont().Scale(1.5).Bold());

            wxStaticText* text1 = new wxStaticText(panel, wxID_ANY,
                                                   "TrenchBroom has crashed, but was able to save a crash report\n"
                                                   "and the current state of the map to the following locations.\n\n"
                                                   "Please create an issue report and upload both files.");
            
            wxStaticText* logLabel = new wxStaticText(panel, wxID_ANY, "Report");
            logLabel->SetFont(logLabel->GetFont().Bold());
            wxStaticText* logPathText = new wxStaticText(panel, wxID_ANY, logPath.asString());

            wxStaticText* mapLabel = new wxStaticText(panel, wxID_ANY, "Map");
            mapLabel->SetFont(mapLabel->GetFont().Bold());
            wxStaticText* mapPathText = new wxStaticText(panel, wxID_ANY, mapPath.asString());

            wxStaticText* versionLabel = new wxStaticText(panel, wxID_ANY, "Version");
            versionLabel->SetFont(versionLabel->GetFont().Bold());
            wxStaticText* versionText = new wxStaticText(panel, wxID_ANY, getBuildVersion());
            
            wxStaticText* buildLabel = new wxStaticText(panel, wxID_ANY, "Build");
            buildLabel->SetFont(buildLabel->GetFont().Bold());
            wxStaticText* buildText = new wxStaticText(panel, wxID_ANY, getBuildIdStr());
            
            wxGridBagSizer* textSizer = new wxGridBagSizer(LayoutConstants::NarrowVMargin, LayoutConstants::WideHMargin);
            textSizer->Add(header,          wxGBPosition(0, 0), wxGBSpan(1, 2), wxBOTTOM, 20);
            textSizer->Add(text1,           wxGBPosition(1, 0), wxGBSpan(1, 2), wxBOTTOM, 20);
            textSizer->Add(logLabel,        wxGBPosition(2, 0), wxGBSpan(1, 1));
            textSizer->Add(logPathText,     wxGBPosition(2, 1), wxGBSpan(1, 1));
            textSizer->Add(mapLabel,        wxGBPosition(3, 0), wxGBSpan(1, 1));
            textSizer->Add(mapPathText,     wxGBPosition(3, 1), wxGBSpan(1, 1), wxBOTTOM, 20);
            textSizer->Add(versionLabel,    wxGBPosition(4, 0), wxGBSpan(1, 1));
            textSizer->Add(versionText,     wxGBPosition(4, 1), wxGBSpan(1, 1));
            textSizer->Add(buildLabel,      wxGBPosition(5, 0), wxGBSpan(1, 1));
            textSizer->Add(buildText,       wxGBPosition(5, 1), wxGBSpan(1, 1));
            
            wxSizer* panelSizer = new wxBoxSizer(wxHORIZONTAL);
            panelSizer->AddSpacer(LayoutConstants::DialogOuterMargin);
            panelSizer->Add(textSizer, 1, wxEXPAND | wxTOP | wxBOTTOM, LayoutConstants::DialogOuterMargin);
            panelSizer->AddSpacer(LayoutConstants::DialogOuterMargin);

            panel->SetSizer(panelSizer);
            
            wxButton* reportButton = new wxButton(this, wxID_APPLY, "Report");
            reportButton->Bind(wxEVT_BUTTON, &CrashDialog::OnReport, this);
            
            wxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
            buttonSizer->AddStretchSpacer();
            buttonSizer->Add(reportButton, 0, wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT, LayoutConstants::DialogOuterMargin);
            buttonSizer->Add(CreateButtonSizer(wxCLOSE));
            
            wxSizer* outerSizer = new wxBoxSizer(wxVERTICAL);
            outerSizer->Add(panel, 1, wxEXPAND);
            outerSizer->Add(wrapDialogButtonSizer(buttonSizer, this), 0, wxEXPAND);
            
            SetSizerAndFit(outerSizer);
        }

        void CrashDialog::OnReport(wxCommandEvent& event) {
            wxLaunchDefaultBrowser("https://github.com/kduske/TrenchBroom/issues/new");
        }
    }
}
