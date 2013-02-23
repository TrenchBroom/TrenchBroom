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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "AboutDialog.h"

#include "IO/FileManager.h"
#include "Utility/String.h"

#include "Version.h"

#include <wx/bitmap.h>
#include <wx/gbsizer.h>
#include <wx/statbmp.h>
#include <wx/statline.h>
#include <wx/stattext.h>

namespace TrenchBroom {
    namespace View {
        AboutDialog::AboutDialog(wxWindow* parent) :
        wxDialog(parent, wxID_ANY, wxT("About")) {
            IO::FileManager fileManager;
            
            wxBitmap icon(fileManager.appendPath(fileManager.resourceDirectory(), "Icon.png"), wxBITMAP_TYPE_PNG);
            
            wxStaticBitmap* appIcon = new wxStaticBitmap(this, wxID_ANY, icon);
            wxStaticLine* appLine = new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL);
            wxStaticText* appName = new wxStaticText(this, wxID_ANY, wxT("TrenchBroom"));
            appName->SetFont(appName->GetFont().Larger().Larger().Larger().Larger().Bold());
            
            wxStaticText* appClaim = new wxStaticText(this, wxID_ANY, wxT("A Modern Level Editor for Quake"));
            
            wxString versionStr(wxT("Version "));
            versionStr << VERSIONSTR;
            
            wxStaticText* version = new wxStaticText(this, wxID_ANY, versionStr);
            
            wxStaticText* devHeader = new wxStaticText(this, wxID_ANY, wxT("Development"));
            devHeader->SetFont(devHeader->GetFont().Bold());
            wxStaticText* devText = new wxStaticText(this, wxID_ANY, wxT("Kristian Duske"));
            
            wxStaticText* contrHeader = new wxStaticText(this, wxID_ANY, wxT("Contributions"));
            contrHeader->SetFont(contrHeader->GetFont().Bold());
            wxSizer* contrText = CreateTextSizer(wxT("Corey Jones (feedback, testing, documentation)\nAndré König (feedback, testing)\nWouter van Oortmerssen (feedback)\nHannes Kröger (testing)\nMorgan Allen (testing)\nForest Hale (fov code)"));
            
            wxSizer* copyright = CreateTextSizer(wxT("Copyright 2010-2013 Kristian Duske\nQuake is a registered trademark of id Software"));

            wxGridBagSizer* sizer = new wxGridBagSizer();

            int row = 0;
            sizer->Add(0, 10, wxGBPosition(row++, 1));
            sizer->AddGrowableRow(static_cast<size_t>(row - 1));
            sizer->Add(appName, wxGBPosition(row++, 1));
            sizer->Add(appLine, wxGBPosition(row++, 1), wxDefaultSpan, wxEXPAND);
            sizer->Add(appClaim, wxGBPosition(row++, 1));
            sizer->Add(0, 20, wxGBPosition(row++, 1));
            sizer->Add(version, wxGBPosition(row++, 1));
            sizer->Add(0, 20, wxGBPosition(row++, 1));
            sizer->Add(devHeader, wxGBPosition(row++, 1));
            sizer->Add(devText, wxGBPosition(row++, 1));
            sizer->Add(0, 20, wxGBPosition(row++, 1));
            sizer->Add(contrHeader, wxGBPosition(row++, 1));
            sizer->Add(contrText, wxGBPosition(row++, 1));
            sizer->Add(0, 20, wxGBPosition(row++, 1));
            sizer->Add(copyright, wxGBPosition(row++, 1));
            sizer->Add(0, 10, wxGBPosition(row++, 1));
            sizer->AddGrowableRow(static_cast<size_t>(row - 1));
            sizer->Add(appIcon, wxGBPosition(0, 0), wxGBSpan(row, 1), wxALIGN_CENTER);
            
            SetSizer(sizer);
            SetSize(650, 410);
            CenterOnParent();
            
            SetBackgroundColour(*wxWHITE);
        }
    }
}
