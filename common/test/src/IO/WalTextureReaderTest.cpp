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


#include "Assets/Texture.h"
#include "Assets/Palette.h"
#include "IO/DiskIO.h"
#include "IO/DiskFileSystem.h"
#include "IO/Path.h"
#include "IO/WalTextureReader.h"

#include "Catch2.h"
#include "TestLogger.h"

namespace TrenchBroom {
    namespace IO {
        static const auto fixturePath = Path("fixture/test/IO/Wal");

        TEST_CASE("WalTextureReaderTest.testLoadQ2WalDir", "[WalTextureReaderTest]") {
            DiskFileSystem fs(IO::Disk::getCurrentWorkingDir());
            const Assets::Palette palette = Assets::Palette::loadFile(fs, Path("fixture/test/colormap.pcx"));

            TextureReader::PathSuffixNameStrategy nameStrategy(fixturePath.length());
            NullLogger logger;
            WalTextureReader textureReader(nameStrategy, fs, logger, palette);

            using TexInfo = std::tuple<Path, size_t, size_t>;
            const auto [path, width, height] = GENERATE(values<TexInfo>({
                { Path("rtz/b_pv_v1a1.wal"),  128, 256 },
                { Path("rtz/b_pv_v1a2.wal"),  128, 256 },
                { Path("rtz/b_pv_v1a3.wal"),  128, 128 },
                { Path("rtz/b_rc_v16.wal"),   128, 128 },
                { Path("rtz/b_rc_v16w.wal"),  128, 128 },
                { Path("rtz/b_rc_v28.wal"),   128,  64 },
                { Path("rtz/b_rc_v4.wal"),    128, 128 },
            }));

            INFO(path);
            INFO(width);
            INFO(height);

            const auto file = fs.openFile(fixturePath + path);
            REQUIRE(file != nullptr);

            const auto texture = textureReader.readTexture(file);
            const auto& name = path.suffix(2).deleteExtension().asString("/");
            CHECK(texture.name() == name);
            CHECK(texture.width() == width);
            CHECK(texture.height() == height);
        }
    }
}
