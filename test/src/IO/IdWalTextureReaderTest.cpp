/*
 Copyright (C) 2010-2016 Kristian Duske
 
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

#include "Assets/AssetTypes.h"
#include "Assets/Texture.h"
#include "Assets/Palette.h"
#include "IO/DiskFileSystem.h"
#include "IO/Path.h"
#include "IO/IdWalTextureReader.h"

namespace TrenchBroom {
    namespace IO {
        inline void assertTexture(const Path& path, const size_t width, const size_t height, const FileSystem& fs, const TextureReader& reader) {
            const Path filePath = Path("data/IO/Wal") + path;
            Assets::Texture* texture = reader.readTexture(fs.openFile(filePath));
            ASSERT_TRUE(texture != NULL);
            
            const String& name = path.suffix(2).deleteExtension().asString('/');
            ASSERT_EQ(name, texture->name());
            ASSERT_EQ(width, texture->width());
            ASSERT_EQ(height, texture->height());
        }
        
        TEST(IdWalTextureReaderTest, testLoadWalDir) {
            DiskFileSystem fs(IO::Disk::getCurrentWorkingDir());
            const Assets::Palette palette = Assets::Palette::loadFile(fs, Path("data/colormap.pcx"));
            
            TextureReader::PathSuffixNameStrategy nameStrategy(2, true);
            IdWalTextureReader textureReader(nameStrategy, palette);
            
            assertTexture(Path("rtz/b_pv_v1a1.wal"),  128, 256, fs, textureReader);
            assertTexture(Path("rtz/b_pv_v1a2.wal"),  128, 256, fs, textureReader);
            assertTexture(Path("rtz/b_pv_v1a3.wal"),  128, 128, fs, textureReader);
            assertTexture(Path("rtz/b_rc_v16.wal"),   128, 128, fs, textureReader);
            assertTexture(Path("rtz/b_rc_v16w.wal"),  128, 128, fs, textureReader);
            assertTexture(Path("rtz/b_rc_v28.wal"),   128,  64, fs, textureReader);
            assertTexture(Path("rtz/b_rc_v4.wal"),    128, 128, fs, textureReader);
        }
    }
}
