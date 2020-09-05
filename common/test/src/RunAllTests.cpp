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

#define CATCH_CONFIG_RUNNER
#include <catch2/catch.hpp>

#include "GTestCompat.h"

#include "TrenchBroomApp.h"
#include "Ensure.h"

#include <clocale>

int main(int argc, char **argv) {
    TrenchBroom::View::TrenchBroomApp app(argc, argv);

/*
    // use an empty file config so that we always use the default preferences
    // this must happen exactly between creating the app instance and initializing it
    // so that the app itself will not try to access the config file before we reset it here
    const auto configFileName = "TrenchBroom-Test.ini";
    const auto configFilePath = wxFileConfig::GetLocalFile(configFileName);
    wxRemove(configFilePath.GetPath());
    wxConfig::Set(new wxFileConfig(wxEmptyString, wxEmptyString, configFileName));
*/
    TrenchBroom::View::setCrashReportGUIEnbled(false);

    ensure(qApp == &app, "invalid app instance");

    // set the locale to US so that we can parse floats attribute
    std::setlocale(LC_NUMERIC, "C");

    const int result = Catch::Session().run(argc, argv);

    return result;
}
