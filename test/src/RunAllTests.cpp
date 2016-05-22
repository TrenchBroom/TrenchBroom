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

#include <wx/wx.h>
#include <wx/config.h>
#include <wx/fileconf.h>
#include <clocale>

int main(int argc, char **argv) {

    wxApp* pApp = new TrenchBroom::View::TrenchBroomApp();
    wxApp::SetInstance(pApp);
    wxEntryStart(argc, argv);

    // use an empty file config so that we always use the default preferences
    wxConfig::Set(new wxFileConfig("TrenchBroom-Test"));

    const char* gtestarg = "--gtest_break_on_failure";
    
    int gargc = argc + 1;
    char** gargv = new char*[gargc];
    for (size_t i = 0; i < static_cast<size_t>(argc); ++i)
        gargv[i] = argv[i];
    gargv[static_cast<size_t>(argc)] = new char[std::strlen(gtestarg)];
    std::strcpy(gargv[static_cast<size_t>(argc)], gtestarg);
    
    ::testing::InitGoogleTest(&gargc, gargv);

    
    // set the locale to US so that we can parse floats attribute
    std::setlocale(LC_NUMERIC, "C");
    const int result = RUN_ALL_TESTS();
    
    wxEntryCleanup();
    delete wxConfig::Set(NULL);
    
    return result;
}
