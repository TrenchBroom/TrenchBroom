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

#include "Assets/EntityModel.h"
#include "Error.h"
#include "IO/AssimpLoader.h"
#include "IO/DiskFileSystem.h"
#include "Logger.h"

#include "Catch2.h" // IWYU pragma: keep

namespace TrenchBroom::IO
{

TEST_CASE("AssimpLoaderTest.loadBlenderModel")
{
  auto logger = NullLogger{};

  const auto basePath = std::filesystem::current_path() / "fixture/test/IO/assimp";
  auto fs = std::make_shared<DiskFileSystem>(basePath);

  auto loader = AssimpLoader{"cube.dae", *fs};

  auto model = loader.load(logger);
  CHECK(model.is_success());

  CHECK(model.value().data().frameCount() == 1);
  CHECK(model.value().data().surfaceCount() == 1);
  CHECK(model.value().data().surface(0u).skinCount() == 1);
}

TEST_CASE("AssimpLoaderTest.loadHLModelWithSkins")
{
  auto logger = NullLogger{};

  const auto basePath = std::filesystem::current_path() / "fixture/test/IO/assimp";
  auto fs = std::make_shared<DiskFileSystem>(basePath);

  auto loader = AssimpLoader{"cube.mdl", *fs};

  auto model = loader.load(logger);
  CHECK(model.is_success());

  CHECK(model.value().data().surfaceCount() == 4);
  CHECK(model.value().data().surface(0).skinCount() == 1);
  CHECK(model.value().data().surface(1).skinCount() == 3);
  CHECK(model.value().data().surface(2).skinCount() == 1);
  CHECK(model.value().data().surface(3).skinCount() == 1);
}

TEST_CASE("AssimpLoaderTest.loadHLModelWithAnimations")
{
  auto logger = NullLogger{};

  const auto basePath = std::filesystem::current_path() / "fixture/test/IO/assimp";
  auto fs = std::make_shared<DiskFileSystem>(basePath);

  auto loader = AssimpLoader{"cube.mdl", *fs};

  auto model = loader.load(logger);
  CHECK(model.is_success());
  CHECK(model.value().data().frameCount() == 3);
}

} // namespace TrenchBroom::IO
