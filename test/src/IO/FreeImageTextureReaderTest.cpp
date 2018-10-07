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
        inline void assertTexture(const String& name, const size_t width, const size_t height, const FileSystem& fs, const TextureReader& loader) {
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
            FreeImageTextureReader textureLoader(nameStrategy);
            
            const Path imagePath = Disk::getCurrentWorkingDir() + Path("data/IO/Image/");
            DiskFileSystem diskFS(imagePath );

            assertTexture("5x5.png",          5,   5,   diskFS, textureLoader);
            assertTexture("707x710.png",      707, 710, diskFS, textureLoader);
        }
    }
}
