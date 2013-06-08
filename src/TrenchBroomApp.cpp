/*
 Copyright (C) 2010-2013 Kristian Duske
 
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

#include <clocale>

#include "IO/Path.h"
#include "View/MapDocument.h"

#ifndef TESTING
IMPLEMENT_APP(TrenchBroomApp)
#endif

TrenchBroomApp::TrenchBroomApp() :
wxApp(),
m_documentManager(useSDI()) {}

bool TrenchBroomApp::OnInit() {
    if (!wxApp::OnInit())
        return false;

    std::setlocale(LC_NUMERIC, "C");

#ifndef __APPLE__
    if (wxApp::argc > 1) {
        const wxString filename = wxApp::argv[1];
        return openDocument(filename.ToStdString());
    } else {
        return newDocument();
    }
#endif
    return true;
}

int TrenchBroomApp::OnExit() {
    return wxApp::OnExit();
}

#ifdef __APPLE__
void TrenchBroomApp::MacNewFile() {
    newDocument();
}

void TrenchBroomApp::MacOpenFiles(const wxArrayString& filenames) {
    wxArrayString::const_iterator it, end;
    for (it = filenames.begin(), end = filenames.end(); it != end; ++it) {
        const wxString& filename = *it;
        openDocument(filename.ToStdString());
    }
}
#endif

bool TrenchBroomApp::useSDI() {
#ifdef _WIN32
    return true;
#else
    return false;
#endif
}

bool TrenchBroomApp::newDocument() {
    TrenchBroom::View::MapDocument::Ptr document = m_documentManager.newDocument();
    if (document == NULL)
        return false;
    document->createOrRaiseFrame();
    return true;
}

bool TrenchBroomApp::openDocument(const String& pathStr) {
    const TrenchBroom::IO::Path path(pathStr);
    TrenchBroom::View::MapDocument::Ptr document = m_documentManager.openDocument(path);
    if (document == NULL)
        return false;
    document->createOrRaiseFrame();
    return true;
}
