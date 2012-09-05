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

#include <wx/config.h>
#include <wx/docview.h>

#include "IO/Pak.h"
#include "Model/Alias.h"
#include "Model/Bsp.h"
#include "Model/MapDocument.h"
#include "Utility/DocManager.h"
#include "View/EditorView.h"

#include <clocale>

BEGIN_EVENT_TABLE(AbstractApp, wxApp)
END_EVENT_TABLE()

bool AbstractApp::OnInit() {
    // set the locale to US so that we can parse floats property
    std::setlocale(LC_ALL, "en_US");
    
    // initialize globals
    TrenchBroom::IO::PakManager::sharedManager = new TrenchBroom::IO::PakManager();
    TrenchBroom::Model::AliasManager::sharedManager = new TrenchBroom::Model::AliasManager();
    TrenchBroom::Model::BspManager::sharedManager = new TrenchBroom::Model::BspManager();
    
	m_docManager = new DocManager();
    new wxDocTemplate(m_docManager, wxT("Quake map document"), wxT("*.map"), wxEmptyString, wxT("map"), wxT("Quake map document"), wxT("TrenchBroom editor view"), CLASSINFO(TrenchBroom::Model::MapDocument), CLASSINFO(TrenchBroom::View::EditorView));
    return true;
}

int AbstractApp::OnExit() {
    m_docManager->FileHistorySave(*wxConfig::Get());
    wxDELETE(m_docManager);
    
    delete TrenchBroom::IO::PakManager::sharedManager;
    TrenchBroom::IO::PakManager::sharedManager = NULL;
    delete TrenchBroom::Model::AliasManager::sharedManager;
    TrenchBroom::Model::AliasManager::sharedManager = NULL;
    delete TrenchBroom::Model::BspManager::sharedManager;
    TrenchBroom::Model::BspManager::sharedManager = NULL;
    
    return wxApp::OnExit();
}

void AbstractApp::OnUnhandledException() {
    try {
        throw;
    } catch (std::exception& e) {
        wxLogError(e.what());
    }
}
