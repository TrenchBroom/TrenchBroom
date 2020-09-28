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

namespace TrenchBroom {
    namespace IO {
        TEST_CASE("PathSuffixNameStrategyTest.getTextureName", "[PathSuffixNameStrategyTest]") {
            CHECK(TextureReader::PathSuffixNameStrategy(1u).textureName("", Path()) == "");
            CHECK(TextureReader::PathSuffixNameStrategy(1u).textureName("", Path("/textures")) == "");
            CHECK(TextureReader::PathSuffixNameStrategy(1u).textureName("", Path("/textures/e1m1")) == "e1m1");
            CHECK(TextureReader::PathSuffixNameStrategy(1u).textureName("", Path("/textures/e1m1/haha")) == "e1m1/haha");
            CHECK(TextureReader::PathSuffixNameStrategy(1u).textureName("", Path("/textures/e1m1/haha.jpg")) == "e1m1/haha");
            CHECK(TextureReader::PathSuffixNameStrategy(1u).textureName("", Path("/textures/nesting/e1m1/haha.jpg")) == "nesting/e1m1/haha");
        }
    }
}
