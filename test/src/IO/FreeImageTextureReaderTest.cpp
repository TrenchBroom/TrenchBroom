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

#include <gtest/gtest.h>

#include "Assets/Texture.h"
#include "IO/DiskFileSystem.h"
#include "IO/FreeImageTextureReader.h"
#include "IO/Path.h"
#include "IO/TextureReader.h"

namespace TrenchBroom {
    namespace IO {
        static std::unique_ptr<const Assets::Texture> loadTexture(const String& name) {
            TextureReader::TextureNameStrategy nameStrategy;
            const auto mips = 4;
            FreeImageTextureReader textureLoader(nameStrategy, mips);

            const auto imagePath = Disk::getCurrentWorkingDir() + Path("data/IO/Image/");
            DiskFileSystem diskFS(imagePath);

            return std::unique_ptr<const Assets::Texture>{ textureLoader.readTexture(diskFS.openFile(Path(name))) };
        }

        static void assertTexture(const String& name, const size_t width, const size_t height) {
            const auto texture = loadTexture(name);

            ASSERT_TRUE(texture != nullptr);
            ASSERT_EQ(name, texture->name());
            ASSERT_EQ(width, texture->width());
            ASSERT_EQ(height, texture->height());
            ASSERT_EQ(GL_RGBA, texture->format());
            ASSERT_EQ(Assets::TextureType::Opaque, texture->type());
        }

        TEST(FreeImageTextureReaderTest, testLoadPngs) {
            assertTexture("5x5.png",          5,   5);
            assertTexture("707x710.png",      707, 710);
        }

        TEST(FreeImageTextureReaderTest, testLoadCorruptPng) {
            const auto texture = loadTexture("corruptPngTest.png");

            // TextureReader::readTexture is supposed to return a placeholder for corrupt textures
            ASSERT_TRUE(texture != nullptr);
            ASSERT_EQ("corruptPngTest.png", texture->name());
            ASSERT_NE(0, texture->width());
            ASSERT_NE(0, texture->height());
        }

        // https://github.com/kduske/TrenchBroom/issues/2474
        TEST(FreeImageTextureReaderTest, testPNGContents) {
            const auto texture = loadTexture("pngContentsTest.png");
            const auto w = 64;
            const auto h = 64;

            ASSERT_TRUE(texture != nullptr);
            ASSERT_EQ(w, texture->width());
            ASSERT_EQ(h, texture->height());
            ASSERT_EQ(4, texture->buffersIfUnprepared().size());
            ASSERT_EQ(GL_RGBA, texture->format());
            ASSERT_EQ(Assets::TextureType::Opaque, texture->type());

            auto& mip0Data = texture->buffersIfUnprepared().at(0);
            ASSERT_EQ(w * h * 4, mip0Data.size());

            auto* mip0DataPtr = mip0Data.ptr();
            for (int y = 0; y < h; ++y) {
                for (int x = 0; x < w; ++x) {
                    if (x == 0 && y == 0) {
                        // top left pixel is red
                        ASSERT_EQ(255 /* R */, mip0DataPtr[w * y + x * 4]);
                        ASSERT_EQ(0   /* G */, mip0DataPtr[w * y + x * 4 + 1]);
                        ASSERT_EQ(0   /* B */, mip0DataPtr[w * y + x * 4 + 2]);
                        ASSERT_EQ(255 /* A */, mip0DataPtr[w * y + x * 4 + 3]);
                    } else if (x == w && y == h) {
                        // bottom right pixel is green
                        ASSERT_EQ(0   /* R */, mip0DataPtr[w * y + x * 4]);
                        ASSERT_EQ(255 /* G */, mip0DataPtr[w * y + x * 4 + 1]);
                        ASSERT_EQ(0   /* B */, mip0DataPtr[w * y + x * 4 + 2]);
                        ASSERT_EQ(255 /* A */, mip0DataPtr[w * y + x * 4 + 3]);
                    } else {
                        // others are 161, 161, 161
                        ASSERT_EQ(161 /* R */, mip0DataPtr[w * y + x * 4]);
                        ASSERT_EQ(161 /* G */, mip0DataPtr[w * y + x * 4 + 1]);
                        ASSERT_EQ(161 /* B */, mip0DataPtr[w * y + x * 4 + 2]);
                        ASSERT_EQ(255 /* A */, mip0DataPtr[w * y + x * 4 + 3]);
                    }
                }
            }
        }

        TEST(FreeImageTextureReaderTest, alphaMaskTest) {
            const auto texture = loadTexture("alphaMaskTest.png");
            const auto w = 25;
            const auto h = 10;

            ASSERT_TRUE(texture != nullptr);
            ASSERT_EQ(w, texture->width());
            ASSERT_EQ(h, texture->height());
            ASSERT_EQ(4, texture->buffersIfUnprepared().size());
            ASSERT_EQ(GL_RGBA, texture->format());
            ASSERT_EQ(Assets::TextureType::Masked, texture->type());

            auto& mip0Data = texture->buffersIfUnprepared().at(0);
            ASSERT_EQ(w * h * 4, mip0Data.size());

            auto* mip0DataPtr = mip0Data.ptr();
            for (int y = 0; y < h; ++y) {
                for (int x = 0; x < w; ++x) {
                    if (x == 0 && y == 0) {
                        // top left pixel is green opaque
                        ASSERT_EQ(0   /* R */, mip0DataPtr[w * y + x * 4]);
                        ASSERT_EQ(255 /* G */, mip0DataPtr[w * y + x * 4 + 1]);
                        ASSERT_EQ(0   /* B */, mip0DataPtr[w * y + x * 4 + 2]);
                        ASSERT_EQ(255 /* A */, mip0DataPtr[w * y + x * 4 + 3]);
                    } else {
                        // others are fully transparent (RGB values are unknown)
                        ASSERT_EQ(0   /* A */, mip0DataPtr[w * y + x * 4 + 3]);
                    }
                }
            }
        }
    }
}
