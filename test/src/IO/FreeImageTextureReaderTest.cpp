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
        static void assertTexture(const String& name, const size_t width, const size_t height, const FileSystem& fs, const TextureReader& loader) {
            const Assets::Texture* texture = loader.readTexture(fs.openFile(Path(name)));
            ASSERT_TRUE(texture != nullptr);
            ASSERT_EQ(name, texture->name());
            ASSERT_EQ(width, texture->width());
            ASSERT_EQ(height, texture->height());
            delete texture;
        }
        
        TEST(FreeImageTextureReaderTest, testLoadPngs) {
            DiskFileSystem fs(IO::Disk::getCurrentWorkingDir());

            TextureReader::TextureNameStrategy nameStrategy;
            const auto mips = 4;
            FreeImageTextureReader textureLoader(nameStrategy, mips);

            const Path imagePath = Disk::getCurrentWorkingDir() + Path("data/IO/Image/");
            DiskFileSystem diskFS(imagePath );

            assertTexture("5x5.png",          5,   5,   diskFS, textureLoader);
            assertTexture("707x710.png",      707, 710, diskFS, textureLoader);
        }

        // https://github.com/kduske/TrenchBroom/issues/2474
        TEST(FreeImageTextureReaderTest, testPNGContents) {
            DiskFileSystem fs(IO::Disk::getCurrentWorkingDir());

            TextureReader::TextureNameStrategy nameStrategy;
            const auto mips = 4;
            FreeImageTextureReader textureLoader(nameStrategy, mips);

            const Path imagePath = Disk::getCurrentWorkingDir() + Path("data/IO/Image/");
            DiskFileSystem diskFS(imagePath);

            const Assets::Texture* texture = textureLoader.readTexture(diskFS.openFile(Path("pngContentsTest.png")));
            ASSERT_TRUE(texture != nullptr);
            ASSERT_EQ(64, texture->width());
            ASSERT_EQ(64, texture->height());
            ASSERT_EQ(4, texture->buffersIfUnprepared().size());
            ASSERT_EQ(GL_BGR, texture->format());

            auto& mip0Data = texture->buffersIfUnprepared().at(0);
            ASSERT_EQ(64 * 64 * 3, mip0Data.size());

            auto* mip0DataPtr = mip0Data.ptr();
            const auto w = 64;
            const auto h = 64;

            for (int y = 0; y < h; ++y) {
                for (int x = 0; x < w; ++x) {
                    if (x == 0 && y == 0) {
                        // top left pixel is red
                        ASSERT_EQ(0,   mip0DataPtr[w * y + x * 3]);
                        ASSERT_EQ(0,   mip0DataPtr[w * y + x * 3 + 1]);
                        ASSERT_EQ(255, mip0DataPtr[w * y + x * 3 + 2]);
                    } else if (x == w && y == h) {
                        // bottom right pixel is green
                        ASSERT_EQ(0,   mip0DataPtr[w * y + x * 3]);
                        ASSERT_EQ(255, mip0DataPtr[w * y + x * 3 + 1]);
                        ASSERT_EQ(0,   mip0DataPtr[w * y + x * 3 + 2]);
                    } else {
                        // others are 161, 161, 161
                        ASSERT_EQ(161, mip0DataPtr[w * y + x * 3]);
                        ASSERT_EQ(161, mip0DataPtr[w * y + x * 3 + 1]);
                        ASSERT_EQ(161, mip0DataPtr[w * y + x * 3 + 2]);
                    }
                }
            }

            delete texture;
        }
    }
}
