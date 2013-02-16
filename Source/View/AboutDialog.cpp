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

#include <wx/html/htmlwin.h>
#include <wx/sizer.h>

namespace TrenchBroom {
    namespace View {
        AboutDialog::AboutDialog(wxWindow* parent) :
        wxDialog(parent, wxID_ANY, wxT("About")) {
            IO::FileManager fileManager;
            
            const String helpPath = fileManager.appendPath(fileManager.resourceDirectory(), "About");
            const String aboutPath = fileManager.appendPath(helpPath, "about.html");
            
            wxHtmlWindow* htmlWindow = new wxHtmlWindow(this);
            htmlWindow->LoadFile(wxFileName(aboutPath));
            
            wxSizer* sizer = new wxBoxSizer(wxVERTICAL);
            sizer->Add(htmlWindow, 1, wxEXPAND);
            
            SetSizer(sizer);
            SetSize(650, 360);
            CenterOnParent();
        }
    }
}
