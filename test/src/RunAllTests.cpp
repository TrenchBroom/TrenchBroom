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

#include <gtest/gtest.h>

#include "TrenchBroomApp.h"

#include <wx/config.h>
#include <wx/fileconf.h>
#include <clocale>

int main(int argc, char **argv) {

    wxApp* pApp = new TrenchBroom::View::TrenchBroomApp();
    wxApp::SetInstance(pApp);
    ensure(wxEntryStart(argc, argv), "wxWidgets initialization failed");

    ensure(wxApp::GetInstance() == pApp, "invalid app instance");

    // use an empty file config so that we always use the default preferences
    wxConfig::Set(new wxFileConfig("TrenchBroom-Test"));

    ::testing::InitGoogleTest(&argc, argv);
    
    // set the locale to US so that we can parse floats attribute
    std::setlocale(LC_NUMERIC, "C");
    const int result = RUN_ALL_TESTS();
    
    wxEntryCleanup();
    delete wxConfig::Set(NULL);
    
    return result;
}
