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

#include "AbstractApp.h"

#include <wx/docview.h>

#include "Model/MapDocument.h"
#include "View/EditorView.h"

BEGIN_EVENT_TABLE(AbstractApp, wxApp)
END_EVENT_TABLE()

bool AbstractApp::OnInit() {
	m_docManager = new wxDocManager();
    new wxDocTemplate(m_docManager, wxT("Quake map document"), wxT("*.map"), wxEmptyString, wxT("map"), wxT("Quake map document"), wxT("TrenchBroom editor view"), CLASSINFO(TrenchBroom::Model::MapDocument), CLASSINFO(TrenchBroom::View::EditorView));
    return true;
}

int AbstractApp::OnExit() {
    wxDELETE(m_docManager);
    return wxApp::OnExit();
}

void AbstractApp::OnUnhandledException() {
    try {
        throw;
    } catch (std::exception& e) {
        wxLogWarning(e.what());
    }
}
