/*
 Copyright (C) 2018 Eric Wasylishen

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
#include "Assets/EntityModel.h"
#include "Assets/Texture.h"
#include "IO/AseParser.h"
#include "IO/DiskFileSystem.h"
#include "IO/DiskIO.h"
#include "IO/File.h"
#include "IO/Quake3ShaderFileSystem.h"
#include "IO/Reader.h"
#include "IO/TextureReader.h"

namespace TrenchBroom {
    namespace IO {
        TEST_CASE("AseParserTest.loadWithoutException", "[AseParserTest]") {
            NullLogger logger;
            
            const auto defaultAssetsPath = Disk::getCurrentWorkingDir() + Path("fixture/test/IO/ResourceUtils/assets");
            std::shared_ptr<FileSystem> fs = std::make_shared<DiskFileSystem>(defaultAssetsPath);
            
            const auto basePath = Disk::getCurrentWorkingDir() + Path("fixture/test/IO/Ase/wedge_with_shader");
            fs = std::make_shared<DiskFileSystem>(fs, basePath);

            const auto shaderSearchPath = Path("scripts");
            const auto textureSearchPaths = std::vector<Path> { Path("models") };
            fs = std::make_shared<Quake3ShaderFileSystem>(fs, shaderSearchPath, textureSearchPaths, logger);

            const auto aseFile = fs->openFile(Path("models/mapobjects/wedges/wedge_45.ase"));
            auto reader = aseFile->reader().buffer();
            AseParser parser("wedge", std::begin(reader), std::end(reader), *fs);

            auto model = parser.initializeModel(logger);
            ASSERT_NE(nullptr, model);

            ASSERT_NO_THROW(parser.loadFrame(0, *model, logger));
            ASSERT_TRUE(model->frame(0)->loaded());
        }
        
        TEST_CASE("AseParserTest.fallbackToMaterialName", "[AseParserTest]") {
            NullLogger logger;

            const auto defaultAssetsPath = Disk::getCurrentWorkingDir() + Path("fixture/test/IO/ResourceUtils/assets");
            std::shared_ptr<FileSystem> fs = std::make_shared<DiskFileSystem>(defaultAssetsPath);
            
            const auto basePath = Disk::getCurrentWorkingDir() + Path("fixture/test/IO/Ase/fallback_to_materialname");
            fs = std::make_shared<DiskFileSystem>(fs, basePath);

            const auto shaderSearchPath = Path("scripts");
            const auto textureSearchPaths = std::vector<Path> { Path("textures") };
            fs = std::make_shared<Quake3ShaderFileSystem>(fs, shaderSearchPath, textureSearchPaths, logger);

            const auto aseFile = fs->openFile(Path("models/wedge_45.ase"));
            auto reader = aseFile->reader().buffer();
            AseParser parser("wedge", std::begin(reader), std::end(reader), *fs);

            auto model = parser.initializeModel(logger);
            ASSERT_NE(nullptr, model);

            ASSERT_NO_THROW(parser.loadFrame(0, *model, logger));
            ASSERT_TRUE(model->frame(0)->loaded());
            
            // account for the default texture
            ASSERT_EQ(2u, model->surface(0).skinCount());
            ASSERT_EQ("textures/bigtile", model->surface(0).skin(0)->name());
        }
        
        TEST_CASE("AseParserTest.loadDefaultMaterial", "[AseParserTest]") {
            NullLogger logger;

            const auto defaultAssetsPath = Disk::getCurrentWorkingDir() + Path("fixture/test/IO/ResourceUtils/assets");
            std::shared_ptr<FileSystem> fs = std::make_shared<DiskFileSystem>(defaultAssetsPath);
            
            const auto basePath = Disk::getCurrentWorkingDir() + Path("fixture/test/IO/Ase/load_default_material");
            fs = std::make_shared<DiskFileSystem>(fs, basePath);

            const auto shaderSearchPath = Path("scripts");
            const auto textureSearchPaths = std::vector<Path> { Path("textures") };
            fs = std::make_shared<Quake3ShaderFileSystem>(fs, shaderSearchPath, textureSearchPaths, logger);

            const auto aseFile = fs->openFile(Path("models/wedge_45.ase"));
            auto reader = aseFile->reader().buffer();
            AseParser parser("wedge", std::begin(reader), std::end(reader), *fs);

            auto model = parser.initializeModel(logger);
            ASSERT_NE(nullptr, model);

            ASSERT_NO_THROW(parser.loadFrame(0, *model, logger));
            ASSERT_TRUE(model->frame(0)->loaded());
            
            // account for the default texture
            ASSERT_EQ(2u, model->surface(0).skinCount());
            // shader name is correct, but we loaded the default material
            
            const auto* texture = model->surface(0).skin(0);
            ASSERT_EQ("textures/bigtile", texture->name());
            ASSERT_EQ(32u, texture->width());
            ASSERT_EQ(32u, texture->height());
        }

        TEST_CASE("AseParserTest.parseFailure_2657", "[AseParserTest]") {
            NullLogger logger;

            const auto defaultAssetsPath = Disk::getCurrentWorkingDir() + Path("fixture/test/IO/ResourceUtils/assets");
            std::shared_ptr<FileSystem> fs = std::make_shared<DiskFileSystem>(defaultAssetsPath);
            
            const auto basePath = Disk::getCurrentWorkingDir() + Path("fixture/test/IO/Ase/steelstorm_player");
            fs = std::make_shared<DiskFileSystem>(fs, basePath);

            const auto shaderSearchPath = Path("scripts");
            const auto textureSearchPaths = std::vector<Path> { Path("models") };
            fs = std::make_shared<Quake3ShaderFileSystem>(fs, shaderSearchPath, textureSearchPaths, logger);

            const auto aseFile = fs->openFile(Path("player.ase"));
            auto reader = aseFile->reader().buffer();
            AseParser parser("player", std::begin(reader), std::end(reader), *fs);

            auto model = parser.initializeModel(logger);
            ASSERT_NE(nullptr, model);

            ASSERT_NO_THROW(parser.loadFrame(0, *model, logger));
            ASSERT_TRUE(model->frame(0)->loaded());
        }

        TEST_CASE("AseParserTest.parseFailure_2679", "[AseParserTest]") {
            NullLogger logger;

            const auto defaultAssetsPath = Disk::getCurrentWorkingDir() + Path("fixture/test/IO/ResourceUtils/assets");
            std::shared_ptr<FileSystem> fs = std::make_shared<DiskFileSystem>(defaultAssetsPath);
            
            const auto basePath = Disk::getCurrentWorkingDir() + Path("fixture/test/IO/Ase/no_scene_directive");
            fs = std::make_shared<DiskFileSystem>(fs, basePath);

            const auto shaderSearchPath = Path("scripts");
            const auto textureSearchPaths = std::vector<Path> { Path("models") };
            fs = std::make_shared<Quake3ShaderFileSystem>(fs, shaderSearchPath, textureSearchPaths, logger);

            const auto aseFile = fs->openFile(Path("wedge_45.ase"));
            auto reader = aseFile->reader().buffer();
            AseParser parser("wedge", std::begin(reader), std::end(reader), *fs);

            auto model = parser.initializeModel(logger);
            ASSERT_NE(nullptr, model);

            ASSERT_NO_THROW(parser.loadFrame(0, *model, logger));
            ASSERT_TRUE(model->frame(0)->loaded());
        }
    
        TEST_CASE("AseParserTest.parseFailure_2898_vertex_index", "[AseParserTest]") {
            NullLogger logger;

            const auto defaultAssetsPath = Disk::getCurrentWorkingDir() + Path("fixture/test/IO/ResourceUtils/assets");
            std::shared_ptr<FileSystem> fs = std::make_shared<DiskFileSystem>(defaultAssetsPath);
            
            const auto basePath = Disk::getCurrentWorkingDir() + Path("fixture/test/IO/Ase/index_out_of_bounds");
            fs = std::make_shared<DiskFileSystem>(fs, basePath);

            const auto shaderSearchPath = Path("scripts");
            const auto textureSearchPaths = std::vector<Path> { Path("models") };
            fs = std::make_shared<Quake3ShaderFileSystem>(fs, shaderSearchPath, textureSearchPaths, logger);

            const auto aseFile = fs->openFile(Path("wedge_45.ase"));
            auto reader = aseFile->reader().buffer();
            AseParser parser("wedge", std::begin(reader), std::end(reader), *fs);

            auto model = parser.initializeModel(logger);
            ASSERT_NE(nullptr, model);

            ASSERT_NO_THROW(parser.loadFrame(0, *model, logger));
            ASSERT_TRUE(model->frame(0)->loaded());
        }
        
        TEST_CASE("AseParserTest.parseFailure_2898_no_uv", "[AseParserTest]") {
            NullLogger logger;

            const auto defaultAssetsPath = Disk::getCurrentWorkingDir() + Path("fixture/test/IO/ResourceUtils/assets");
            std::shared_ptr<FileSystem> fs = std::make_shared<DiskFileSystem>(defaultAssetsPath);
            
            const auto basePath = Disk::getCurrentWorkingDir() + Path("fixture/test/IO/Ase/index_out_of_bounds");
            fs = std::make_shared<DiskFileSystem>(fs, basePath);

            const auto shaderSearchPath = Path("scripts");
            const auto textureSearchPaths = std::vector<Path> { Path("models") };
            fs = std::make_shared<Quake3ShaderFileSystem>(fs, shaderSearchPath, textureSearchPaths, logger);

            const auto aseFile = fs->openFile(Path("wedge_45_no_uv.ase"));
            auto reader = aseFile->reader().buffer();
            AseParser parser("wedge", std::begin(reader), std::end(reader), *fs);

            auto model = parser.initializeModel(logger);
            ASSERT_NE(nullptr, model);

            ASSERT_NO_THROW(parser.loadFrame(0, *model, logger));
            ASSERT_TRUE(model->frame(0)->loaded());
        }
    }
}

