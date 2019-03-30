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
#include <QLabel>


namespace TrenchBroom {
    namespace View {
        wxIMPLEMENT_DYNAMIC_CLASS(CrashDialog, wxDialog)

        CrashDialog::CrashDialog() {}

        void CrashDialog::Create(const IO::Path& reportPath, const IO::Path& mapPath, const IO::Path& logPath) {
            wxDialog::Create(nullptr, wxID_ANY, "Crash");

            QWidget* containerPanel = new QWidget(this);
            QWidget* headerPanel = new QWidget(containerPanel);
            QWidget* reportPanel = new QWidget(containerPanel);
            reportPanel->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOX));
            reportPanel->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOXTEXT));

            QLabel* header = new QLabel(headerPanel, wxID_ANY, "Crash Report");
            header->SetFont(header->GetFont().Scale(1.5).Bold());

            auto* headerPanelSizer = new QVBoxLayout();
            headerPanelSizer->addSpacing(LayoutConstants::DialogOuterMargin);
            headerPanelSizer->addWidget(header, wxSizerFlags().Border(wxLEFT | wxRIGHT, LayoutConstants::DialogOuterMargin));
            headerPanelSizer->addSpacing(LayoutConstants::DialogOuterMargin);
            headerPanel->setLayout(headerPanelSizer);

            QLabel* text1 = new QLabel(reportPanel, wxID_ANY,
                                                   "TrenchBroom has crashed, but was able to save a crash report,\n"
                                                   "a log file and the current state of the map to the following locations.\n\n"
                                                   "Please create an issue report and upload all three files.");

            QLabel* reportLabel = new QLabel(reportPanel, wxID_ANY, "Report");
            reportLabel->SetFont(reportLabel->GetFont().Bold());
            QLabel* reportPathText = new QLabel(reportPanel, wxID_ANY, reportPath.asString());

            QLabel* mapLabel = new QLabel(reportPanel, wxID_ANY, "Map");
            mapLabel->SetFont(mapLabel->GetFont().Bold());
            QLabel* mapPathText = new QLabel(reportPanel, wxID_ANY, mapPath.asString());

            QLabel* logLabel = new QLabel(reportPanel, wxID_ANY, "Log");
            logLabel->SetFont(logLabel->GetFont().Bold());
            QLabel* logPathText = new QLabel(reportPanel, wxID_ANY, logPath.asString());

            QLabel* versionLabel = new QLabel(reportPanel, wxID_ANY, "Version");
            versionLabel->SetFont(versionLabel->GetFont().Bold());
            QLabel* versionText = new QLabel(reportPanel, wxID_ANY, "");// FIXME: getBuildVersion());

            QLabel* buildLabel = new QLabel(reportPanel, wxID_ANY, "Build");
            buildLabel->SetFont(buildLabel->GetFont().Bold());
            QLabel* buildText = new QLabel(reportPanel, wxID_ANY,  "");// FIXME: getBuildIdStr());

            wxGridBagSizer* reportPanelSizer = new wxGridBagSizer(LayoutConstants::NarrowVMargin, LayoutConstants::WideHMargin);
            reportPanelSizer->addWidget(text1,           wxGBPosition(0, 0), wxGBSpan(1, 2));
            reportPanelSizer->addWidget(1, 20,           wxGBPosition(1, 0), wxGBSpan(1, 2));
            reportPanelSizer->addWidget(reportLabel,     wxGBPosition(2, 0), wxGBSpan(1, 1), Qt::AlignVCenter);
            reportPanelSizer->addWidget(reportPathText,  wxGBPosition(2, 1), wxGBSpan(1, 1), Qt::AlignVCenter);
            reportPanelSizer->addWidget(mapLabel,        wxGBPosition(3, 0), wxGBSpan(1, 1), Qt::AlignVCenter);
            reportPanelSizer->addWidget(mapPathText,     wxGBPosition(3, 1), wxGBSpan(1, 1), Qt::AlignVCenter);
            reportPanelSizer->addWidget(logLabel,        wxGBPosition(4, 0), wxGBSpan(1, 1), Qt::AlignVCenter);
            reportPanelSizer->addWidget(logPathText,     wxGBPosition(4, 1), wxGBSpan(1, 1), Qt::AlignVCenter);
            reportPanelSizer->addWidget(1, 20,           wxGBPosition(5, 0), wxGBSpan(1, 2));
            reportPanelSizer->addWidget(versionLabel,    wxGBPosition(6, 0), wxGBSpan(1, 1), Qt::AlignVCenter);
            reportPanelSizer->addWidget(versionText,     wxGBPosition(6, 1), wxGBSpan(1, 1), Qt::AlignVCenter);
            reportPanelSizer->addWidget(buildLabel,      wxGBPosition(7, 0), wxGBSpan(1, 1), Qt::AlignVCenter);
            reportPanelSizer->addWidget(buildText,       wxGBPosition(7, 1), wxGBSpan(1, 1), Qt::AlignVCenter);

            auto* reportPanelOuterSizer = new QVBoxLayout();
            reportPanelOuterSizer->addWidget(new BorderLine(reportPanel), wxSizerFlags().Expand());
            reportPanelOuterSizer->addWidget(reportPanelSizer, wxSizerFlags().Border(wxLEFT | wxRIGHT, LayoutConstants::DialogOuterMargin));
            reportPanelOuterSizer->addSpacing(LayoutConstants::DialogOuterMargin);
            reportPanel->setLayout(reportPanelOuterSizer);

            auto* containerPanelSizer = new QVBoxLayout();
            containerPanelSizer->addWidget(headerPanel, wxSizerFlags().Expand());
            containerPanelSizer->addWidget(reportPanel, wxSizerFlags().Expand());
            containerPanel->setLayout(containerPanelSizer);

            wxButton* reportButton = new wxButton(this, wxID_APPLY, "Report");
            reportButton->Bind(wxEVT_BUTTON, &CrashDialog::OnReport, this);

            auto* buttonSizer = new QHBoxLayout();
            buttonSizer->addStretch(1);
            buttonSizer->addWidget(reportButton, wxSizerFlags().CenterVertical().Border(wxLEFT | wxRIGHT, LayoutConstants::DialogOuterMargin));
            buttonSizer->addWidget(CreateButtonSizer(wxCLOSE));

            auto* outerSizer = new QVBoxLayout();
            outerSizer->addWidget(containerPanel, wxSizerFlags().Expand());
            outerSizer->addWidget(wrapDialogButtonSizer(buttonSizer, this), 0, wxEXPAND);

            setLayout(outerSizer);
        }

        void CrashDialog::OnReport() {
            wxLaunchDefaultBrowser("https://github.com/kduske/TrenchBroom/issues/new");
        }
    }
}
