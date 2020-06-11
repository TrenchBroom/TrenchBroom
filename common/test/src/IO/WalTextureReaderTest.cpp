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

#include "Assets/Texture.h"
#include "Assets/Palette.h"
#include "IO/DiskIO.h"
#include "IO/DiskFileSystem.h"
#include "IO/Path.h"
#include "IO/WalTextureReader.h"

namespace TrenchBroom {
    namespace IO {
        static const auto fixturePath = Path("fixture/test/IO/Wal");
    
        static void assertTexture(const Path& path, const size_t width, const size_t height, const FileSystem& fs, const TextureReader& reader) {
            const Path filePath = fixturePath + path;
            auto texture = std::unique_ptr<Assets::Texture>{ reader.readTexture(fs.openFile(filePath)) };
            ASSERT_TRUE(texture != nullptr);

            const auto& name = path.suffix(2).deleteExtension().asString("/");
            ASSERT_EQ(name, texture->name());
            ASSERT_EQ(width, texture->width());
            ASSERT_EQ(height, texture->height());
        }

        TEST_CASE("WalTextureReaderTest.testLoadQ2WalDir", "[WalTextureReaderTest]") {
            DiskFileSystem fs(IO::Disk::getCurrentWorkingDir());
            const Assets::Palette palette = Assets::Palette::loadFile(fs, Path("fixture/test/colormap.pcx"));

            const Path texturePath = fs.root() + fixturePath;
            TextureReader::PathSuffixNameStrategy nameStrategy(texturePath.length());
            NullLogger logger;
            WalTextureReader textureReader(nameStrategy, fs, logger, palette);

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
