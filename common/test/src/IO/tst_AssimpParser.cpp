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
#include "IO/AssimpParser.h"
#include "IO/DiskFileSystem.h"
#include "Logger.h"

#include "Catch2.h" // IWYU pragma: keep

namespace TrenchBroom::IO
{

TEST_CASE("AssimpParserTest.loadBlenderModel")
{
  auto logger = NullLogger{};

  const auto basePath = std::filesystem::current_path() / "fixture/test/IO/assimp";
  auto fs = std::make_shared<DiskFileSystem>(basePath);

  auto assimpParser = AssimpParser{"cube.dae", *fs};

  auto model = assimpParser.initializeModel(logger);
  CHECK(model != nullptr);

  CHECK(model->frameCount() == 1);
  CHECK(model->surfaceCount() == 1);
  CHECK(model->surface(0).skinCount() == 1);
}

TEST_CASE("AssimpParserTest.loadHLModelWithSkins")
{
  auto logger = NullLogger{};

  const auto basePath = std::filesystem::current_path() / "fixture/test/IO/assimp";
  auto fs = std::make_shared<DiskFileSystem>(basePath);

  auto assimpParser = AssimpParser{"cube.mdl", *fs};

  auto model = assimpParser.initializeModel(logger);
  CHECK(model != nullptr);

  CHECK(model->frameCount() == 1);
  CHECK(model->surfaceCount() == 2);
  CHECK(model->surface(0).skinCount() == 1);
  CHECK(model->surface(1).skinCount() == 3);
}

} // namespace TrenchBroom::IO
