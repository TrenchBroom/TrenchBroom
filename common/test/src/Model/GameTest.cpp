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

#include "Logger.h"
#include "Assets/Texture.h"
#include "Assets/TextureCollection.h"
#include "Assets/TextureManager.h"
#include "IO/DiskIO.h"
#include "IO/IOUtils.h"
#include "IO/GameConfigParser.h"
#include "IO/Path.h"
#include "Model/EntityNode.h"
#include "Model/GameConfig.h"
#include "Model/GameImpl.h"

#include <kdl/vector_utils.h>

#include "Catch2.h"

namespace TrenchBroom {
    namespace Model {
        TEST_CASE("GameTest.findTextureCollections", "[GameTest]") {
            auto config = GameConfig(
                "Quake2",
                IO::Path(),
                IO::Path(),
                false,
                std::vector<MapFormatConfig>(),
                FileSystemConfig(
                    IO::Path("baseq2"),
                    PackageFormatConfig()
                ),
                TextureConfig(
                    TexturePackageConfig(IO::Path("textures")),
                    PackageFormatConfig("wal", "wal"),
                    IO::Path("pics/colormap.pcx"),
                    "_tb_textures",
                    IO::Path(),
                    std::vector<std::string>()
                ),
                EntityConfig(),
                FaceAttribsConfig(),
                std::vector<SmartTag>(),
                std::nullopt, // soft map bounds
                {} // compilation tools
            );
            const auto gamePath = IO::Disk::getCurrentWorkingDir() + IO::Path("fixture/test/Model/Game/Quake2");
            auto logger = NullLogger();
            GameImpl game(config, gamePath, logger);
            
            CHECK_THAT(game.findTextureCollections(), Catch::UnorderedEquals(std::vector<IO::Path>{
                IO::Path("textures"),
                IO::Path("textures/e1m1"),
                IO::Path("textures/e1m1/f1")
            }));
        }
        
        TEST_CASE("GameTest.loadCorruptPackages", "[GameTest]") {
            // https://github.com/TrenchBroom/TrenchBroom/issues/2496

            const auto games = std::vector<IO::Path> {
                IO:: Path("Quake"),
                IO::Path("Daikatana"),
                IO::Path("Quake3")
            };

            for (const auto& game : games) {
                const auto configPath = IO::Disk::getCurrentWorkingDir() + IO::Path("fixture/games/") + game + IO::Path("GameConfig.cfg");
                const auto configStr = IO::Disk::readTextFile(configPath);
                auto configParser = IO::GameConfigParser(configStr, configPath);
                auto config = configParser.parse();

                const auto gamePath = IO::Disk::getCurrentWorkingDir() + IO::Path("fixture/test/Model/Game/CorruptPak");
                auto logger = NullLogger();
                UNSCOPED_INFO("Should not throw when loading corrupted package file for game " << game);
                CHECK_NOTHROW(GameImpl(config, gamePath, logger));
            }
        }

        TEST_CASE("GameTest.loadQuake3Shaders", "[GameTest]") {
            const auto configPath = IO::Disk::getCurrentWorkingDir() + IO::Path("fixture/games//Quake3/GameConfig.cfg");
            const auto configStr = IO::Disk::readTextFile(configPath);
            auto configParser = IO::GameConfigParser(configStr, configPath);
            auto config = configParser.parse();

            const auto gamePath = IO::Disk::getCurrentWorkingDir() + IO::Path("fixture/test/Model/Game/Quake3");
            auto logger = NullLogger();
            auto game = GameImpl(config, gamePath, logger);

            CHECK_THAT(game.findTextureCollections(), Catch::UnorderedEquals(std::vector<IO::Path>{
                IO::Path("textures"),
                IO::Path("textures/skies"),
                IO::Path("textures/skies/hub1"),
                IO::Path("textures/test"),
            }));

            auto worldspawn = Entity({
                {"_tb_textures", "textures/test;textures/skies/hub1"}
            });

            auto textureManager = Assets::TextureManager(0, 0, logger);
            game.loadTextureCollections(worldspawn, IO::Path(), textureManager, logger);

            CHECK(textureManager.collections().size() == 2u);

            /*
             * The shader script contains five entries:
             * textures/test/test overrides an existing texture and points it to an editor image
             * textures/test/not_existing does not override an existing texture and points to an editor image
             * textures/test/test2 overrides an existing texture, but the editor image is missing
             * textures/test/not_existing2 does not override an existing texture, and no editor image
             * textures/skies/hub1/dusk has a deeper directory structure, and has an editor image
             *
             * Due to the directory structure, the shader script induces four texture collections:
             * - textures
             * - textures/test
             * - textures/skies
             * - textures/skies/hub1
             *
             * Of these, we only load textures/test and textures/skies/hub1.
             *
             * The file system contains three textures:
             * textures/test/test.tga is overridden by the shader script
             * textures/test/test2.tga is overridden by the shader script
             * textures/test/editor_image.jpg is not overridden by a shader
             *
             * In total, we expect the following entries in texture collection textures/test:
             * test/test -> test/editor_image.jpg
             * test/not_existing -> test/editor_image.jpg
             * test/editor_image
             * test/not_existing2 -> __TB_empty.png
             * test/test2 -> __TB_empty.png
             *
             * and one entry in texture collection textures/skies/hub1:
             * skies/hub1/dusk -> test/editor_image.jpg
             */

            const auto& testCollection = textureManager.collections().front();
            const auto testTextureNames = kdl::vec_transform(testCollection.textures(), [](const auto& texture) { return texture.name(); });

            CHECK_THAT(testTextureNames, Catch::UnorderedEquals(std::vector<std::string>{
                "test/test",
                "test/not_existing",
                "test/editor_image",
                "test/not_existing2",
                "test/test2",
            }));

            const auto& skiesCollection = textureManager.collections().back();
            const auto skiesTextureNames = kdl::vec_transform(skiesCollection.textures(), [](const auto& texture) { return texture.name(); });

            CHECK_THAT(skiesTextureNames, Catch::UnorderedEquals(std::vector<std::string>{
                "skies/hub1/dusk",
            }));
        }
    }
}
