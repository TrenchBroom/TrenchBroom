/*
 Copyright (C) 2024 Kristian Duske

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

#include "io/MapHeader.h"
#include "io/TestEnvironment.h"

#include <sstream>

#include "catch/CatchConfig.h"

#include <catch2/catch_test_macros.hpp>

namespace tb::io
{
TEST_CASE("readMapHeader")
{
  auto env = io::TestEnvironment{};

  using namespace std::string_literals;

  const auto detectGame = [&](const auto& mapFile) {
    auto stream = std::istringstream{mapFile};
    return readMapHeader(stream);
  };

  CHECK(detectGame(R"(// Game: Quake
// Format: Quake2
)") == std::pair{"Quake"s, mdl::MapFormat::Quake2});


  CHECK(detectGame(R"(// Game: Quake
// Format: Quake2
{
"classname" "worldspawn"
{
( -712 1280 -448 ) ( -904 1280 -448 ) ( -904 992 -448 ) attribsExplicit 56 -32 0 1 1 8 9 700
( -904 992 -416 ) ( -904 1280 -416 ) ( -712 1280 -416 ) attribsOmitted 32 32 0 1 1
( -832 968 -416 ) ( -832 1256 -416 ) ( -832 1256 -448 ) attribsExplicitlyZero 16 96 0 1 1 0 0 0
( -920 1088 -448 ) ( -920 1088 -416 ) ( -680 1088 -416 ) rtz/c_mf_v3c 56 96 0 1 1 0 0 0
( -968 1152 -448 ) ( -920 1152 -448 ) ( -944 1152 -416 ) rtz/c_mf_v3c 56 96 0 1 1 0 0 0
( -896 1056 -416 ) ( -896 1056 -448 ) ( -896 1344 -448 ) rtz/c_mf_v3c 16 96 0 1 1 0 0 0
}
})") == std::pair{"Quake"s, mdl::MapFormat::Quake2});
}

TEST_CASE("writeMapHeader")
{
  auto stream = std::ostringstream{};
  writeMapHeader(stream, "Quake", mdl::MapFormat::Quake2);

  CHECK(stream.str() == R"(// Game: Quake
// Format: Quake2
)");
}

} // namespace tb::io
