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

#include "TrenchBroomApp.h"

#include <wx/docview.h>
#include <wx/menu.h>

#include "Utility/DocManager.h"

#include <clocale>

IMPLEMENT_APP(TrenchBroomApp)

BEGIN_EVENT_TABLE(TrenchBroomApp, AbstractApp)
END_EVENT_TABLE()

wxMenu* TrenchBroomApp::CreateEditMenu(const TrenchBroom::Preferences::MultiMenuSelector& selector, wxEvtHandler* eventHandler, bool mapViewFocused) {
	wxMenu* editMenu = AbstractApp::CreateEditMenu(selector, eventHandler, mapViewFocused);
	editMenu->AppendSeparator();
    editMenu->Append(wxID_PREFERENCES, wxT("Preferences"));
	return editMenu;
}

wxMenu* TrenchBroomApp::CreateHelpMenu(const TrenchBroom::Preferences::MultiMenuSelector& selector, wxEvtHandler* eventHandler, bool mapViewFocused) {
    wxMenu* helpMenu = AbstractApp::CreateHelpMenu(selector, eventHandler, mapViewFocused);
    helpMenu->AppendSeparator();
    helpMenu->Append(wxID_ABOUT, wxT("About TrenchBroom..."));
    return helpMenu;
}

bool TrenchBroomApp::OnInit() {
    // set the locale to US so that we can parse floats property
    std::setlocale(LC_ALL, "us");

	if (AbstractApp::OnInit()) {
		SetExitOnFrameDelete(true);
		m_docManager->SetUseSDI(false);
        if (wxApp::argc > 1) {
            wxString filename = wxApp::argv[1];
            if (m_docManager->CreateDocument(filename) == NULL) {
                return false;
            }
        } else {
		    m_docManager->CreateNewDocument();
        }
        return true;
	}

	return false;
}
