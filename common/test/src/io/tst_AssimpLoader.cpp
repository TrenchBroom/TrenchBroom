/*
 Copyright (C) 2023 Daniel Walder
 Copyright (C) 2022 Kristian Duske

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

#include "Logger.h"
#include "io/AssimpLoader.h"
#include "io/DiskFileSystem.h"
#include "mdl/EntityModel.h"

#include "Catch2.h"

namespace tb::io
{

TEST_CASE("AssimpLoader")
{
  auto logger = NullLogger{};

  SECTION("cube")
  {
    const auto basePath = std::filesystem::current_path() / "fixture/test/io/assimp/cube";
    auto fs = std::make_shared<DiskFileSystem>(basePath);

    SECTION("dae")
    {
      auto loader = AssimpLoader{"cube.dae", *fs};
      auto modelData = loader.load(logger);
      REQUIRE(modelData.is_success());

      CHECK(modelData.value().frameCount() == 1);
      CHECK(modelData.value().surfaceCount() == 1);
      CHECK(modelData.value().surface(0u).skinCount() == 1);
    }

    SECTION("mdl")
    {
      auto loader = AssimpLoader{"cube.mdl", *fs};

      auto modelData = loader.load(logger);
      REQUIRE(modelData.is_success());

      CHECK(modelData.value().surfaceCount() == 4);
      CHECK(modelData.value().surface(0).skinCount() == 1);
      CHECK(modelData.value().surface(1).skinCount() == 3);
      CHECK(modelData.value().surface(2).skinCount() == 1);
      CHECK(modelData.value().surface(3).skinCount() == 1);
      CHECK(modelData.value().frameCount() == 3);
    }
  }
}

} // namespace tb::io
