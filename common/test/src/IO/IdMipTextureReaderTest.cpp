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

#include "TestLogger.h"

#include "Logger.h"
#include "Assets/Palette.h"
#include "Assets/Texture.h"
#include "Assets/TextureCollection.h"
#include "IO/DiskFileSystem.h"
#include "IO/DiskIO.h"
#include "IO/IdMipTextureReader.h"
#include "IO/Path.h"
#include "IO/TextureReader.h"
#include "IO/WadFileSystem.h"

#include <string>

#include "Catch2.h"
#include "GTestCompat.h"

namespace TrenchBroom {
    namespace IO {
        static void assertTexture(const std::string& name, const size_t width, const size_t height, const FileSystem& fs, const TextureReader& loader) {

            const Assets::Texture texture = loader.readTexture(fs.openFile(Path(name + ".D")));
            ASSERT_EQ(name, texture.name());
            ASSERT_EQ(width, texture.width());
            ASSERT_EQ(height, texture.height());
        }

        TEST_CASE("IdMipTextureReaderTest.testLoadWad", "[IdMipTextureReaderTest]") {
            DiskFileSystem fs(IO::Disk::getCurrentWorkingDir());
            const Assets::Palette palette = Assets::Palette::loadFile(fs, Path("fixture/test/palette.lmp"));

            TextureReader::TextureNameStrategy nameStrategy;
            NullLogger logger;
            IdMipTextureReader textureLoader(nameStrategy, fs, logger, palette);

            const Path wadPath = Disk::getCurrentWorkingDir() + Path("fixture/test/IO/Wad/cr8_czg.wad");
            WadFileSystem wadFS(wadPath, logger);

            assertTexture("cr8_czg_1",          64,  64, wadFS, textureLoader);
            assertTexture("cr8_czg_2",          64,  64, wadFS, textureLoader);
            assertTexture("cr8_czg_3",          64, 128, wadFS, textureLoader);
            assertTexture("cr8_czg_4",          64, 128, wadFS, textureLoader);
            assertTexture("cr8_czg_5",          64, 128, wadFS, textureLoader);
            assertTexture("speedM_1",          128, 128, wadFS, textureLoader);
            assertTexture("cap4can-o-jam",      64,  64, wadFS, textureLoader);
            assertTexture("can-o-jam",          64,  64, wadFS, textureLoader);
            assertTexture("eat_me",             64,  64, wadFS, textureLoader);
            assertTexture("coffin1",           128, 128, wadFS, textureLoader);
            assertTexture("coffin2",           128, 128, wadFS, textureLoader);
            assertTexture("czg_fronthole",     128, 128, wadFS, textureLoader);
            assertTexture("czg_backhole",      128, 128, wadFS, textureLoader);
            assertTexture("u_get_this",         64,  64, wadFS, textureLoader);
            assertTexture("for_sux-m-ass",      64,  64, wadFS, textureLoader);
            assertTexture("dex_5",             128, 128, wadFS, textureLoader);
            assertTexture("polished_turd",      64,  64, wadFS, textureLoader);
            assertTexture("crackpipes",        128, 128, wadFS, textureLoader);
            assertTexture("bongs2",            128, 128, wadFS, textureLoader);
            assertTexture("blowjob_machine",   128, 128, wadFS, textureLoader);
            assertTexture("lasthopeofhuman",   128, 128, wadFS, textureLoader);
        }
    }
}
