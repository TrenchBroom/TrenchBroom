/*
 Copyright (C) 2026 MaxED

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

#include "TestEnvironment.h"
#include "fs/DiskFileSystem.h"
#include "gl/Texture.h"
#include "mdl/LoadM32Texture.h"
#include "mdl/TestUtils.h"

#include <catch2/catch_test_macros.hpp>

namespace tb::mdl
{

TEST_CASE("loadM32Texture")
{
  const auto fs = fs::DiskFileSystem{getFixtureRoot()};
  const auto file = fs.openFile("test/mdl/LoadM32Texture/test.m32") | kdl::value();

  auto reader = file->reader().buffer();
  const auto texture = loadM32Texture(reader) | kdl::value();

  CHECK(texture.width() == 2);
  CHECK(texture.height() == 2);
  CHECK(texture.mask() == gl::TextureMask::On);

  checkColor(texture, 0, 0, 255, 0, 0, 255);
  checkColor(texture, 1, 0, 0, 255, 0, 180);
  checkColor(texture, 0, 1, 0, 0, 255, 90);
  checkColor(texture, 1, 1, 255, 255, 255, 0);
}

} // namespace tb::mdl
