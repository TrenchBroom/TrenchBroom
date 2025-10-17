/*
 Copyright (C) 2025 Kristian Duske

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

#include "CmdTool.h"
#include "el/VariableStore.h"
#include "mdl/GameEngineProfile.h"
#include "ui/LaunchGameEngine.h"

#include "kdl/filesystem_utils.h"

#include <thread>

#include "catch/CatchConfig.h"

#include <catch2/catch_test_macros.hpp>

namespace tb::ui
{

TEST_CASE("launchGameEngineProfile")
{
  auto variables = el::VariableTable{};

  SECTION("return an error if engine doesn't exist")
  {
    auto profile = mdl::GameEngineProfile{
      "some_name",
      "/does/not/exist",
      "",
    };

    CHECK(launchGameEngineProfile(profile, variables).is_error());
  }

  SECTION("passes arguments correctly to the engine")
  {
    auto profile = mdl::GameEngineProfile{
      "some_name",
      CMD_TOOL_PATH,
      R"(--printArgs 1 2 str "string with spaces")",
    };

    auto logFile = kdl::tmp_file{};

    CHECK(launchGameEngineProfile(profile, variables, logFile) == Result<void>{});

    // wait for cmd-tool to finish
    std::this_thread::sleep_for(std::chrono::milliseconds{500});

    CHECK(kdl::read_file(logFile) == R"(1
2
str
string with spaces
)");
  }
}

} // namespace tb::ui
