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

#include "Assets/EntityModel.h"
#include "IO/DiskFileSystem.h"
#include "IO/DiskIO.h"
#include "IO/File.h"
#include "IO/Md3Parser.h"
#include "IO/Quake3ShaderFileSystem.h"
#include "IO/Reader.h"
#include "IO/VirtualFileSystem.h"
#include "Logger.h"

#include "vm/bbox.h"
#include "vm/forward.h"
#include "vm/vec.h"

#include <cstdio>
#include <filesystem>
#include <memory>

#include "Catch2.h"

namespace TrenchBroom
{
namespace IO
{
TEST_CASE("Md3ParserTest.loadFailure_2659")
{
  // see https://github.com/TrenchBroom/TrenchBroom/issues/2659

  NullLogger logger;
  const auto shaderSearchPath = "scripts";
  const auto textureSearchPaths = std::vector<std::filesystem::path>{"models"};
  auto fs = VirtualFileSystem{};
  fs.mount(
    "",
    std::make_unique<DiskFileSystem>(
      std::filesystem::current_path() / "fixture/test/IO/Md3/armor"));
  fs.mount(
    "",
    createImageFileSystem<Quake3ShaderFileSystem>(
      fs, shaderSearchPath, textureSearchPaths, logger)
      .value());

  const auto md3Path = "models/armor_red.md3";
  const auto md3File = fs.openFile(md3Path).value();

  auto reader = md3File->reader().buffer();
  auto parser = Md3Parser{"armor_red", reader, fs};
  auto model = std::unique_ptr<Assets::EntityModel>(parser.initializeModel(logger));

  CHECK(model != nullptr);

  CHECK(model->frameCount() == 30u);
  CHECK(model->surfaceCount() == 2u);

  for (size_t i = 0; i < model->frameCount(); ++i)
  {
    CHECK_NOTHROW(parser.loadFrame(i, *model, logger));
  }
}
} // namespace IO
} // namespace TrenchBroom
