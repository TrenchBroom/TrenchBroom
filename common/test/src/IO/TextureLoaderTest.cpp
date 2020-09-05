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

#include "Logger.h"
#include "Assets/Texture.h"
#include "Assets/TextureManager.h"
#include "IO/DiskFileSystem.h"
#include "IO/DiskIO.h"
#include "IO/Path.h"
#include "IO/TextureLoader.h"
#include "Model/GameConfig.h"

#include <string>

namespace TrenchBroom {
    namespace IO {
        static void assertTexture(const std::string& name, const size_t width, const size_t height, const Assets::TextureManager& manager) {
            const Assets::Texture* texture = manager.texture(name);
            ASSERT_TRUE(texture != nullptr);
            ASSERT_EQ(name, texture->name());
            ASSERT_EQ(width, texture->width());
            ASSERT_EQ(height, texture->height());
        }

        static void assertTextureMissing(const std::string& name, const Assets::TextureManager& manager) {
            const Assets::Texture* texture = manager.texture(name);
            ASSERT_TRUE(texture == nullptr);
        }

        TEST_CASE("TextureLoaderTest.testLoad", "[TextureLoaderTest]") {
            const std::vector<IO::Path> paths({ Path("fixture/test/IO/Wad/cr8_czg.wad") });

            const IO::Path root = IO::Disk::getCurrentWorkingDir();
            const std::vector<IO::Path> fileSearchPaths{ root };
            const IO::DiskFileSystem fileSystem(root, true);

            const Model::TextureConfig textureConfig(
                Model::TexturePackageConfig(
                    Model::PackageFormatConfig("wad", "idmip")),
                    Model::PackageFormatConfig("D", "idmip"),
                    IO::Path("fixture/test/palette.lmp"),
                    "wad",
                    IO::Path(),
                    {});

            auto logger = NullLogger();
            auto textureManager = Assets::TextureManager(0, 0, logger);

            IO::TextureLoader textureLoader(fileSystem, fileSearchPaths, textureConfig, logger);
            textureLoader.loadTextures(paths, textureManager);

            assertTexture("cr8_czg_1", 64, 64, textureManager);
            assertTexture("cr8_czg_2", 64, 64, textureManager);
            assertTexture("cr8_czg_3", 64, 128, textureManager);
            assertTexture("cr8_czg_4", 64, 128, textureManager);
            assertTexture("cr8_czg_5", 64, 128, textureManager);
            assertTexture("speedM_1", 128, 128, textureManager);
            assertTexture("cap4can-o-jam", 64, 64, textureManager);
            assertTexture("can-o-jam", 64, 64, textureManager);
            assertTexture("eat_me", 64, 64, textureManager);
            assertTexture("coffin1", 128, 128, textureManager);
            assertTexture("coffin2", 128, 128, textureManager);
            assertTexture("czg_fronthole", 128, 128, textureManager);
            assertTexture("czg_backhole", 128, 128, textureManager);
            assertTexture("u_get_this", 64, 64, textureManager);
            assertTexture("for_sux-m-ass", 64, 64, textureManager);
            assertTexture("dex_5", 128, 128, textureManager);
            assertTexture("polished_turd", 64, 64, textureManager);
            assertTexture("crackpipes", 128, 128, textureManager);
            assertTexture("bongs2", 128, 128, textureManager);
            assertTexture("blowjob_machine", 128, 128, textureManager);
            assertTexture("lasthopeofhuman", 128, 128, textureManager);
        }

        TEST_CASE("TextureLoaderTest.testLoadExclusions", "[TextureLoaderTest]") {
            const std::vector<IO::Path> paths({ Path("fixture/test/IO/Wad/cr8_czg.wad") });

            const IO::Path root = IO::Disk::getCurrentWorkingDir();
            const std::vector<IO::Path> fileSearchPaths{ root };
            const IO::DiskFileSystem fileSystem(root, true);

            const Model::TextureConfig textureConfig(
                Model::TexturePackageConfig(
                    Model::PackageFormatConfig("wad", "idmip")),
                    Model::PackageFormatConfig("D", "idmip"),
                    IO::Path("fixture/test/palette.lmp"),
                    "wad",
                    IO::Path(),
                    {
                        "*-jam",
                        "coffin2",
                        "czg_*"
                    });

            auto logger = NullLogger();
            auto textureManager = Assets::TextureManager(0, 0, logger);

            IO::TextureLoader textureLoader(fileSystem, fileSearchPaths, textureConfig, logger);
            textureLoader.loadTextures(paths, textureManager);

            assertTexture("cr8_czg_1", 64, 64, textureManager);
            assertTexture("cr8_czg_2", 64, 64, textureManager);
            assertTexture("cr8_czg_3", 64, 128, textureManager);
            assertTexture("cr8_czg_4", 64, 128, textureManager);
            assertTexture("cr8_czg_5", 64, 128, textureManager);
            assertTexture("speedM_1", 128, 128, textureManager);
            assertTextureMissing("cap4can-o-jam", textureManager);
            assertTextureMissing("can-o-jam", textureManager);
            assertTexture("eat_me", 64, 64, textureManager);
            assertTexture("coffin1", 128, 128, textureManager);
            assertTextureMissing("coffin2", textureManager);
            assertTextureMissing("czg_fronthole", textureManager);
            assertTextureMissing("czg_backhole", textureManager);
            assertTexture("u_get_this", 64, 64, textureManager);
            assertTexture("for_sux-m-ass", 64, 64, textureManager);
            assertTexture("dex_5", 128, 128, textureManager);
            assertTexture("polished_turd", 64, 64, textureManager);
            assertTexture("crackpipes", 128, 128, textureManager);
            assertTexture("bongs2", 128, 128, textureManager);
            assertTexture("blowjob_machine", 128, 128, textureManager);
            assertTexture("lasthopeofhuman", 128, 128, textureManager);
        }
    }
}
