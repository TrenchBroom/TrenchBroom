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

#include "io/DiskFileSystem.h"
#include "io/MaterialUtils.h"
#include "io/ReadFreeImageTexture.h"
#include "Logger.h"

#include "kdl/result.h"

#include <filesystem>

#include "Catch2.h"

namespace tb::io
{

TEST_CASE("getMaterialNameFromPathSuffix")
{
  using T = std::tuple<size_t, std::filesystem::path, std::string>;

  const auto [prefixLength, path, expectedResult] = GENERATE(values<T>({
    {1, "", ""},
    {1, "textures", ""},
    {1, "textures/e1m1", "e1m1"},
    {1, "textures/e1m1/haha", "e1m1/haha"},
    {1, "textures/e1m1/haha.jpg", "e1m1/haha"},
    {1, "textures/nesting/e1m1/haha.jpg", "nesting/e1m1/haha"},
    {2, "textures/nesting/e1m1/haha.jpg", "e1m1/haha"},
    {3, "/textures/nesting/e1m1/haha.jpg", "e1m1/haha"},
  }));

  CAPTURE(prefixLength, path);

  CHECK(getMaterialNameFromPathSuffix(path, prefixLength) == expectedResult);
}

TEST_CASE("makeReadTextureErrorHandler")
{
  auto logger = NullLogger{};
  auto diskFS = DiskFileSystem{
    std::filesystem::current_path() / "fixture/test/io/ReadTextureErrorHandler"};

  const auto file = diskFS.openFile("textures/corruptPngTest.png") | kdl::value();
  auto reader = file->reader().buffer();
  auto result = readFreeImageTexture(reader);
  REQUIRE(result.is_error());

  const auto defaultTexture = std::move(result)
                              | kdl::or_else(makeReadTextureErrorHandler(diskFS, logger))
                              | kdl::value();
  CHECK(defaultTexture.width() == 32);
  CHECK(defaultTexture.height() == 32);
}

} // namespace tb::io
