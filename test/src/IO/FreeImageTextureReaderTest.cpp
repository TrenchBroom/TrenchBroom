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
            FreeImageTextureReader textureLoader(nameStrategy);

            const auto imagePath = Disk::getCurrentWorkingDir() + Path("fixture/test/IO/Image/");
            DiskFileSystem diskFS(imagePath);

            return std::unique_ptr<const Assets::Texture>{ textureLoader.readTexture(diskFS.openFile(Path(name))) };
        }

        static void assertTexture(const String& name, const size_t width, const size_t height) {
            const auto texture = loadTexture(name);

            ASSERT_TRUE(texture != nullptr);
            ASSERT_EQ(name, texture->name());
            ASSERT_EQ(width, texture->width());
            ASSERT_EQ(height, texture->height());
            ASSERT_TRUE(GL_BGRA == texture->format() || GL_RGBA == texture->format());
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

        enum class Component {
            R, G, B, A
        };

        static int getComponentOfPixel(const Assets::Texture* texture, const int x, const int y, const Component component) {
            const auto format = texture->format();

            ensure(GL_BGRA == format || GL_RGBA == format, "expected GL_BGRA or GL_RGBA");

            int componentIndex;
            if (format == GL_RGBA) {
                switch (component) {
                    case Component::R: componentIndex = 0; break;
                    case Component::G: componentIndex = 1; break;
                    case Component::B: componentIndex = 2; break;
                    case Component::A: componentIndex = 3; break;
                }
            } else {
                switch (component) {
                    case Component::R: componentIndex = 2; break;
                    case Component::G: componentIndex = 1; break;
                    case Component::B: componentIndex = 0; break;
                    case Component::A: componentIndex = 3; break;
                }
            }

            const auto& mip0DataBuffer = texture->buffersIfUnprepared().at(0);
            ensure(texture->width() * texture->height() * 4 == mip0DataBuffer.size(), "unexpected texture data size");
            ensure(x >= 0 && x < static_cast<int>(texture->width()), "x out of range");
            ensure(y >= 0 && y < static_cast<int>(texture->height()), "y out of range");

            const uint8_t* mip0Data = mip0DataBuffer.ptr();

            return static_cast<int>(mip0Data[(static_cast<int>(texture->width()) * 4 * y) + (x * 4) + componentIndex]);
        }

        static void checkColor(const Assets::Texture* texturePtr, const int x, const int y,
            const int r, const int g, const int b, const int a) {

            const auto actualR = getComponentOfPixel(texturePtr, x, y, Component::R);
            const auto actualG = getComponentOfPixel(texturePtr, x, y, Component::G);
            const auto actualB = getComponentOfPixel(texturePtr, x, y, Component::B);
            const auto actualA = getComponentOfPixel(texturePtr, x, y, Component::A);

            // allow some error for lossy formats, e.g. JPG
            EXPECT_TRUE(std::abs(r - actualR) < 3);
            EXPECT_TRUE(std::abs(g - actualG) < 3);
            EXPECT_TRUE(std::abs(b - actualB) < 3);
            EXPECT_EQ(a, actualA);
        }

        // https://github.com/kduske/TrenchBroom/issues/2474
        static void testImageContents(std::unique_ptr<const Assets::Texture> texture) {
            const auto w = 64;
            const auto h = 64;

            ASSERT_TRUE(texture != nullptr);
            ASSERT_EQ(w, texture->width());
            ASSERT_EQ(h, texture->height());
            ASSERT_EQ(1, texture->buffersIfUnprepared().size());
            ASSERT_TRUE(GL_BGRA == texture->format() || GL_RGBA == texture->format());
            ASSERT_EQ(Assets::TextureType::Opaque, texture->type());

            auto* texturePtr = texture.get();
            for (int y = 0; y < h; ++y) {
                for (int x = 0; x < w; ++x) {
                    if (x == 0 && y == 0) {
                        // top left pixel is red
                        checkColor(texturePtr, x, y, 255, 0, 0, 255);
                    } else if (x == (w - 1) && y == (h - 1)) {
                        // bottom right pixel is green
                        checkColor(texturePtr, x, y, 0, 255, 0, 255);
                    } else {
                        // others are 161, 161, 161
                        checkColor(texturePtr, x, y, 161, 161, 161, 255);
                    }
                }
            }
        }

        TEST(FreeImageTextureReaderTest, testPNGContents) {
            testImageContents(loadTexture("pngContentsTest.png"));
        }

        TEST(FreeImageTextureReaderTest, testJPGContents) {
            testImageContents(loadTexture("jpgContentsTest.jpg"));
        }

        TEST(FreeImageTextureReaderTest, alphaMaskTest) {
            const auto texture = loadTexture("alphaMaskTest.png");
            const auto w = 25;
            const auto h = 10;

            ASSERT_TRUE(texture != nullptr);
            ASSERT_EQ(w, texture->width());
            ASSERT_EQ(h, texture->height());
            ASSERT_EQ(1, texture->buffersIfUnprepared().size());
            ASSERT_TRUE(GL_BGRA == texture->format() || GL_RGBA == texture->format());
            ASSERT_EQ(Assets::TextureType::Masked, texture->type());

            auto& mip0Data = texture->buffersIfUnprepared().at(0);
            ASSERT_EQ(w * h * 4, mip0Data.size());

            auto* texturePtr = texture.get();
            for (int y = 0; y < h; ++y) {
                for (int x = 0; x < w; ++x) {
                    if (x == 0 && y == 0) {
                        // top left pixel is green opaque
                        ASSERT_EQ(0   /* R */, getComponentOfPixel(texturePtr, x, y, Component::R));
                        ASSERT_EQ(255 /* G */, getComponentOfPixel(texturePtr, x, y, Component::G));
                        ASSERT_EQ(0   /* B */, getComponentOfPixel(texturePtr, x, y, Component::B));
                        ASSERT_EQ(255 /* A */, getComponentOfPixel(texturePtr, x, y, Component::A));
                    } else {
                        // others are fully transparent (RGB values are unknown)
                        ASSERT_EQ(0   /* A */, getComponentOfPixel(texturePtr, x, y, Component::A));
                    }
                }
            }
        }
    }
}
