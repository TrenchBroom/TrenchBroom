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

#include "IO/Path.h"
#include "IO/TextureReader.h"

#include "Catch2.h"

namespace TrenchBroom
{
namespace IO
{
TEST_CASE("getTextureNameFromTexture")
{
  CHECK(getTextureNameFromTexture("name", Path{}) == "name");
  CHECK(getTextureNameFromTexture("name", Path{"this"}) == "name");
  CHECK(getTextureNameFromTexture("name", Path{"this/that"}) == "name");
}

TEST_CASE("makeGetTextureNameFromPathSuffix")
{
  using T = std::tuple<size_t, Path, std::string>;

  const auto [prefixLength, path, expectedResult] = GENERATE(values<T>({
    {1, Path{}, ""},
    {1, Path{"/textures"}, ""},
    {1, Path{"/textures/e1m1"}, "e1m1"},
    {1, Path{"/textures/e1m1/haha"}, "e1m1/haha"},
    {1, Path{"/textures/e1m1/haha.jpg"}, "e1m1/haha"},
    {1, Path{"/textures/nesting/e1m1/haha.jpg"}, "nesting/e1m1/haha"},
    {2, Path{"/textures/nesting/e1m1/haha.jpg"}, "e1m1/haha"},
  }));

  CAPTURE(prefixLength, path);

  CHECK(makeGetTextureNameFromPathSuffix(prefixLength)("", path) == expectedResult);
}

TEST_CASE("makeGetTextureNameFromString")
{
  CHECK(makeGetTextureNameFromString("string")("name", Path{"this/that"}) == "string");
}
} // namespace IO
} // namespace TrenchBroom
