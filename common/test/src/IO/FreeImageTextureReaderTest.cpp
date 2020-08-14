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

#include "GTestCompat.h"

#include "TestLogger.h"

#include "Ensure.h"
#include "Assets/Texture.h"
#include "IO/DiskIO.h"
#include "IO/DiskFileSystem.h"
#include "IO/FreeImageTextureReader.h"
#include "IO/Path.h"
#include "IO/TextureReader.h"

#include <string>

#include "TestUtils.h"

namespace TrenchBroom {
    namespace IO {
        static Assets::Texture loadTexture(const std::string& name) {
            const auto imagePath = Disk::getCurrentWorkingDir() + Path("fixture/test/IO/Image/");
            DiskFileSystem diskFS(imagePath);

            TextureReader::TextureNameStrategy nameStrategy;
            NullLogger logger;
            FreeImageTextureReader textureLoader(nameStrategy, diskFS, logger);

            return textureLoader.readTexture(diskFS.openFile(Path(name)));
        }

        static void assertTexture(const std::string& name, const size_t width, const size_t height) {
            const auto texture = loadTexture(name);

            ASSERT_EQ(name, texture.name());
            ASSERT_EQ(width, texture.width());
            ASSERT_EQ(height, texture.height());
            ASSERT_TRUE((GL_BGRA == texture.format() || GL_RGBA == texture.format()));
            ASSERT_EQ(Assets::TextureType::Opaque, texture.type());
        }

        TEST_CASE("FreeImageTextureReaderTest.testLoadPngs", "[FreeImageTextureReaderTest]") {
            assertTexture("5x5.png",          5,   5);
            assertTexture("707x710.png",      707, 710);
        }

        TEST_CASE("FreeImageTextureReaderTest.testLoadCorruptPng", "[FreeImageTextureReaderTest]") {
            const auto texture = loadTexture("corruptPngTest.png");

            // TextureReader::readTexture is supposed to return a placeholder for corrupt textures
            ASSERT_EQ("corruptPngTest.png", texture.name());
            ASSERT_NE(0u, texture.width());
            ASSERT_NE(0u, texture.height());
        }

        // https://github.com/kduske/TrenchBroom/issues/2474
        static void testImageContents(const Assets::Texture& texture, const ColorMatch match) {
            const std::size_t w = 64u;
            const std::size_t h = 64u;

            ASSERT_EQ(w, texture.width());
            ASSERT_EQ(h, texture.height());
            ASSERT_EQ(1u, texture.buffersIfUnprepared().size());
            ASSERT_TRUE((GL_BGRA == texture.format() || GL_RGBA == texture.format()));
            ASSERT_EQ(Assets::TextureType::Opaque, texture.type());

            for (std::size_t y = 0; y < h; ++y) {
                for (std::size_t x = 0; x < w; ++x) {
                    if (x == 0 && y == 0) {
                        // top left pixel is red
                        checkColor(texture, x, y, 255, 0, 0, 255, match);
                    } else if (x == (w - 1) && y == (h - 1)) {
                        // bottom right pixel is green
                        checkColor(texture, x, y, 0, 255, 0, 255, match);
                    } else {
                        // others are 161, 161, 161
                        checkColor(texture, x, y, 161, 161, 161, 255, match);
                    }
                }
            }
        }

        TEST_CASE("FreeImageTextureReaderTest.testPNGContents", "[FreeImageTextureReaderTest]") {
            testImageContents(loadTexture("pngContentsTest.png"), ColorMatch::Exact);
        }

        TEST_CASE("FreeImageTextureReaderTest.testJPGContents", "[FreeImageTextureReaderTest]") {
            testImageContents(loadTexture("jpgContentsTest.jpg"), ColorMatch::Approximate);
        }

        TEST_CASE("FreeImageTextureReaderTest.alphaMaskTest", "[FreeImageTextureReaderTest]") {
            const auto texture = loadTexture("alphaMaskTest.png");
            const std::size_t w = 25u;
            const std::size_t h = 10u;

            ASSERT_EQ(w, texture.width());
            ASSERT_EQ(h, texture.height());
            ASSERT_EQ(1u, texture.buffersIfUnprepared().size());
            ASSERT_TRUE((GL_BGRA == texture.format() || GL_RGBA == texture.format()));
            ASSERT_EQ(Assets::TextureType::Masked, texture.type());

            auto& mip0Data = texture.buffersIfUnprepared().at(0);
            ASSERT_EQ(w * h * 4, mip0Data.size());

            for (std::size_t y = 0; y < h; ++y) {
                for (std::size_t x = 0; x < w; ++x) {
                    if (x == 0 && y == 0) {
                        // top left pixel is green opaque
                        ASSERT_EQ(0   /* R */, getComponentOfPixel(texture, x, y, Component::R));
                        ASSERT_EQ(255 /* G */, getComponentOfPixel(texture, x, y, Component::G));
                        ASSERT_EQ(0   /* B */, getComponentOfPixel(texture, x, y, Component::B));
                        ASSERT_EQ(255 /* A */, getComponentOfPixel(texture, x, y, Component::A));
                    } else {
                        // others are fully transparent (RGB values are unknown)
                        ASSERT_EQ(0   /* A */, getComponentOfPixel(texture, x, y, Component::A));
                    }
                }
            }
        }
    }
}
