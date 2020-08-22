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

#include <stdio.h>

#include <catch2/catch.hpp>

#include "GTestCompat.h"

#include "Logger.h"
#include "Assets/EntityModel.h"
#include "IO/DiskFileSystem.h"
#include "IO/DiskIO.h"
#include "IO/File.h"
#include "IO/Md3Parser.h"
#include "IO/Path.h"
#include "IO/Quake3ShaderFileSystem.h"
#include "IO/Reader.h"

#include <vecmath/forward.h>
#include <vecmath/bbox.h>
#include <vecmath/vec.h>

#include <memory>

namespace TrenchBroom {
    namespace IO {
        TEST_CASE("Md3ParserTest.loadValidMd3", "[Md3ParserTest]") {
            NullLogger logger;
            const auto shaderSearchPath = Path("scripts");
            const auto textureSearchPaths = std::vector<Path> { Path("models") };
            std::shared_ptr<FileSystem> fs = std::make_shared<DiskFileSystem>(IO::Disk::getCurrentWorkingDir() + Path("fixture/test/IO/Md3/bfg"));
                                        fs = std::make_shared<Quake3ShaderFileSystem>(fs, shaderSearchPath, textureSearchPaths, logger);

            const auto md3Path = IO::Path("models/weapons2/bfg/bfg.md3");
            const auto md3File = fs->openFile(md3Path);
            ASSERT_NE(nullptr, md3File);

            auto reader = md3File->reader().buffer();
            auto parser = Md3Parser("bfg", std::begin(reader), std::end(reader), *fs);
            auto model = std::unique_ptr<Assets::EntityModel>(parser.initializeModel(logger));
            parser.loadFrame(0, *model, logger);

            ASSERT_NE(nullptr, model);

            ASSERT_EQ(1u, model->frameCount());
            ASSERT_EQ(2u, model->surfaceCount());

            const auto* frame = model->frame("MilkShape 3D");
            ASSERT_NE(nullptr, frame);
            ASSERT_TRUE(vm::is_equal(vm::bbox3f(vm::vec3f(-10.234375, -10.765625, -9.4375), vm::vec3f(30.34375, 10.765625, 11.609375)), frame->bounds(), 0.01f));

            const auto* surface1 = model->surface("x_bfg");
            ASSERT_NE(nullptr, surface1);
            ASSERT_EQ(1u, surface1->frameCount());
            ASSERT_EQ(1u, surface1->skinCount());

            const auto* skin1 = surface1->skin("models/weapons2/bfg/LDAbfg");
            ASSERT_NE(nullptr, skin1);

            const auto* surface2 = model->surface("x_fx");
            ASSERT_NE(nullptr, surface2);
            ASSERT_EQ(1u, surface2->frameCount());
            ASSERT_EQ(1u, surface2->skinCount());

            const auto* skin2 = surface2->skin("models/weapons2/bfg/LDAbfg_z");
            ASSERT_NE(nullptr, skin2);
        }

        TEST_CASE("Md3ParserTest.loadFailure_2659", "[Md3ParserTest]") {
            // see https://github.com/TrenchBroom/TrenchBroom/issues/2659

            NullLogger logger;
            const auto shaderSearchPath = Path("scripts");
            const auto textureSearchPaths = std::vector<Path> { Path("models") };
            std::shared_ptr<FileSystem> fs = std::make_shared<DiskFileSystem>(IO::Disk::getCurrentWorkingDir() + Path("fixture/test/IO/Md3/armor"));
            fs = std::make_shared<Quake3ShaderFileSystem>(fs, shaderSearchPath, textureSearchPaths, logger);

            const auto md3Path = IO::Path("models/armor_red.md3");
            const auto md3File = fs->openFile(md3Path);
            ASSERT_NE(nullptr, md3File);

            auto reader = md3File->reader().buffer();
            auto parser = Md3Parser("armor_red", std::begin(reader), std::end(reader), *fs);
            auto model = std::unique_ptr<Assets::EntityModel>(parser.initializeModel(logger));

            ASSERT_NE(nullptr, model);

            ASSERT_EQ(30u, model->frameCount());
            ASSERT_EQ(2u, model->surfaceCount());

            for (size_t i = 0; i < model->frameCount(); ++i) {
                ASSERT_NO_THROW(parser.loadFrame(i, *model, logger));
            }
        }
    }
}
