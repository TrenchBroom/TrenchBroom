/*
 Copyright (C) 2010 Kristian Duske

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

#include "Ensure.h"
#include "TestPreferenceManager.h"
#include "TrenchBroomApp.h"

#include "Catch2.h"

int main(int argc, char** argv)
{
  tb::PreferenceManager::createInstance<tb::TestPreferenceManager>();
  tb::ui::TrenchBroomApp app(argc, argv);

  tb::ui::setCrashReportGUIEnbled(false);

  ensure(qApp == &app, "invalid app instance");

  return Catch::Session().run(argc, argv);
}
