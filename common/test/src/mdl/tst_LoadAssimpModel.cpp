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
#include "fs/DiskFileSystem.h"
#include "mdl/EntityModel.h"
#include "mdl/LoadAssimpModel.h"

#include "vm/approx.h"
#include "vm/bbox_io.h" // IWYU pragma: keep

#include "catch/CatchConfig.h"

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

namespace tb::mdl
{

TEST_CASE("loadAssimpModel")
{
  auto logger = NullLogger{};

  SECTION("cube")
  {
    const auto basePath = std::filesystem::current_path() / "fixture/test/io/assimp/cube";
    auto fs = std::make_shared<fs::DiskFileSystem>(basePath);

    SECTION("dae")
    {
      auto modelData = loadAssimpModel("cube.dae", *fs, logger);
      REQUIRE(modelData);

      CHECK(modelData.value().frameCount() == 1);
      CHECK(modelData.value().surfaceCount() == 1);
      CHECK(modelData.value().surface(0u).skinCount() == 1);
    }

    SECTION("mdl")
    {
      auto modelData = loadAssimpModel("cube.mdl", *fs, logger);
      REQUIRE(modelData);

      CHECK(modelData.value().surfaceCount() == 4);
      CHECK(modelData.value().surface(0).skinCount() == 1);
      CHECK(modelData.value().surface(1).skinCount() == 3);
      CHECK(modelData.value().surface(2).skinCount() == 1);
      CHECK(modelData.value().surface(3).skinCount() == 1);
      CHECK(modelData.value().frameCount() == 3);
    }
  }

  SECTION("alignment")
  {
    const auto modelPath = GENERATE(values<std::filesystem::path>({
      "ase/cuboid.ase", // exported with -X forward and +Z up
      "obj/cuboid.obj",
      "fbx/cuboid.fbx", // exported with scale 0.01
      "gltf/cuboid.gltf",
      "glb/cuboid.glb",
    }));

    CAPTURE(modelPath);

    const auto basePath =
      std::filesystem::current_path() / "fixture/test/io/assimp/alignment";
    auto fs = std::make_shared<fs::DiskFileSystem>(basePath);

    auto modelData = loadAssimpModel(modelPath, *fs, logger);
    REQUIRE(modelData);

    REQUIRE(modelData.value().frameCount() == 1);
    REQUIRE(modelData.value().surfaceCount() == 1);
    REQUIRE(modelData.value().surface(0).skinCount() == 1);

    CHECK(vm::approx(modelData.value().bounds(0)) == vm::bbox3f{{0, 0, 0}, {2, 1, 3}});
  }
}

} // namespace tb::mdl
