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

#include <catch2/catch.hpp>

#include "TestLogger.h"
#include "TestUtils.h"

#include "Assets/Texture.h"
#include "Assets/Palette.h"
#include "IO/DiskIO.h"
#include "IO/DiskFileSystem.h"
#include "IO/Path.h"
#include "IO/M8TextureReader.h"

#include <memory>

namespace TrenchBroom {
    namespace IO {
        TEST_CASE("M8TextureReaderTest.testBasicLoading", "[M8TextureReaderTest]") {
            DiskFileSystem fs(IO::Disk::getCurrentWorkingDir());
            
            TextureReader::PathSuffixNameStrategy nameStrategy(1);
            NullLogger logger;
            M8TextureReader textureReader(nameStrategy, fs, logger);

            const Path filePath = Path("fixture/test/IO/M8/test.m8");

            auto texture = std::unique_ptr<Assets::Texture>{ textureReader.readTexture(fs.openFile(filePath)) };
            REQUIRE(texture != nullptr);

            CHECK("test" == texture->name());
            CHECK(64 == texture->width());
            CHECK(64 == texture->height());

            for (size_t y = 0; y < 64; ++y) {
                for (size_t x = 0; x < 64; ++x) {
                    // One pixel is blue, the others are black
                    if (x == 4 && y == 1) {                        
                        checkColor(texture.get(), x, y, 20, 20, 138, 255);
                    } else {
                        checkColor(texture.get(), x, y, 0,   0,   0, 255);
                    }
                }
            }
        }
    }
}
