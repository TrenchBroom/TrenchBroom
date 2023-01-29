/*
 Copyright (C) 2020 Kristian Duske

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

#include "Assets/Texture.h"
#include "IO/DiskFileSystem.h"
#include "IO/DiskIO.h"
#include "IO/ResourceUtils.h"

#include <memory>

#include "Catch2.h"
#include "TestLogger.h"

namespace TrenchBroom
{
namespace IO
{
TEST_CASE("ResourceUtilsTest.loadDefaultTexture")
{
  auto fs = std::make_shared<DiskFileSystem>(
    IO::Disk::getCurrentWorkingDir() + Path("fixture/test/IO/ResourceUtils/assets"));
  NullLogger logger;

  auto texture = loadDefaultTexture(*fs, logger, "some_name");
  CHECK(texture.name() == "some_name");
}
} // namespace IO
} // namespace TrenchBroom
