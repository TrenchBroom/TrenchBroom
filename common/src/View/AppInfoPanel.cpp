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

#include "AppInfoPanel.h"

#include "IO/Path.h"
#include "IO/ResourceUtils.h"
#include "View/GetVersion.h"

#include <wx/bitmap.h>
#include <wx/sizer.h>
#include <wx/statbmp.h>
#include <wx/statline.h>
#include <wx/stattext.h>

namespace TrenchBroom {
    namespace View {
        AppInfoPanel::AppInfoPanel(wxWindow* parent) :
        wxPanel(parent) {
            createGui();
        }

        void AppInfoPanel::createGui() {
            const wxBitmap appIconImage = IO::loadImageResource("AppIcon.png");
            wxStaticBitmap* appIcon = new wxStaticBitmap(this, wxID_ANY, appIconImage);
            wxStaticText* appName = new wxStaticText(this, wxID_ANY, "TrenchBroom");
            appName->SetFont(appName->GetFont().Larger().Larger().Larger().Larger().Bold());
            wxStaticLine* appLine = new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL);
            wxStaticText* appClaim = new wxStaticText(this, wxID_ANY, "Level Editor");
            
            wxString versionStr("Version ");
            versionStr << getBuildVersion() << " " << getBuildChannel();
            
            wxString buildStr("Build ");
            buildStr << getBuildId() << " " << getBuildType();
            
            wxStaticText* version = new wxStaticText(this, wxID_ANY, versionStr);
            wxStaticText* build = new wxStaticText(this, wxID_ANY, buildStr);
#if !defined(_WIN32)
            version->SetFont(version->GetFont().Smaller());
            build->SetFont(build->GetFont().Smaller());
#endif
            version->SetForegroundColour(wxColor(128, 128, 128));
            build->SetForegroundColour(wxColor(128, 128, 128));
            
            SetBackgroundColour(*wxWHITE);
            
            wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
            sizer->Add(appIcon, 0, wxALIGN_CENTER_HORIZONTAL);
            sizer->Add(appName, 0, wxALIGN_CENTER_HORIZONTAL);
            sizer->Add(appLine, 0, wxEXPAND);
            sizer->Add(appClaim, 0, wxALIGN_CENTER_HORIZONTAL);
            sizer->Add(version, 0, wxALIGN_CENTER_HORIZONTAL);
            sizer->Add(build, 0, wxALIGN_CENTER_HORIZONTAL);
            sizer->AddStretchSpacer();
            SetSizerAndFit(sizer);
        }
    }
}
