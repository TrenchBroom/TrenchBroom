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

#include "Logger.h"
#include "io/DiskFileSystem.h"
#include "io/MaterialUtils.h"
#include "io/ReadFreeImageTexture.h"
#include "io/TestEnvironment.h"

#include "kdl/result.h"
#include "kdl/result_io.h" // IWYU pragma: keep

#include <filesystem>

#include "catch/Matchers.h"

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

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

TEST_CASE("findMaterialFile")
{
  auto env = TestEnvironment{};
  env.createDirectory("textures");
  env.createFile("textures/test.png", "");
  env.createFile("textures/test.jpg", "");
  env.createFile("textures/other.txt", "");

  const auto extensions = std::vector<std::filesystem::path>{".png", ".jpg"};

  auto diskFS = DiskFileSystem{env.dir()};
  CHECK(
    findMaterialFile(diskFS, "asdf/test.png", extensions)
    == Result<std::filesystem::path>{std::filesystem::path{"asdf/test.png"}});
  CHECK(
    findMaterialFile(diskFS, "textures/test.png", extensions)
    == Result<std::filesystem::path>{std::filesystem::path{"textures/test.png"}});
  CHECK_THAT(
    findMaterialFile(diskFS, "textures/test.tga", extensions),
    MatchesAnyOf(std::vector{
      Result<std::filesystem::path>{std::filesystem::path{"textures/test.png"}},
      Result<std::filesystem::path>{std::filesystem::path{"textures/test.jpg"}},
    }));
  CHECK(
    findMaterialFile(diskFS, "textures/other.png", extensions)
    == Result<std::filesystem::path>{std::filesystem::path{"textures/other.png"}});
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
