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

#include <cstdio>
#include <memory>

#include "Catch2.h"

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
            REQUIRE(md3File != nullptr);

            auto reader = md3File->reader().buffer();
            auto parser = Md3Parser("bfg", std::begin(reader), std::end(reader), *fs);
            auto model = std::unique_ptr<Assets::EntityModel>(parser.initializeModel(logger));
            parser.loadFrame(0, *model, logger);

            CHECK(model != nullptr);

            CHECK(model->frameCount() == 1u);
            CHECK(model->surfaceCount() == 2u);

            const auto* frame = model->frame("MilkShape 3D");
            CHECK(frame != nullptr);
            CHECK(vm::is_equal(vm::bbox3f(vm::vec3f(-10.234375, -10.765625, -9.4375), vm::vec3f(30.34375, 10.765625, 11.609375)), frame->bounds(), 0.01f));

            const auto* surface1 = model->surface("x_bfg");
            CHECK(surface1 != nullptr);
            CHECK(surface1->frameCount() == 1u);
            CHECK(surface1->skinCount() == 1u);

            const auto* skin1 = surface1->skin("models/weapons2/bfg/LDAbfg");
            CHECK(skin1 != nullptr);

            const auto* surface2 = model->surface("x_fx");
            CHECK(surface2 != nullptr);
            CHECK(surface2->frameCount() == 1u);
            CHECK(surface2->skinCount() == 1u);

            const auto* skin2 = surface2->skin("models/weapons2/bfg/LDAbfg_z");
            CHECK(skin2 != nullptr);
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
            REQUIRE(md3File != nullptr);

            auto reader = md3File->reader().buffer();
            auto parser = Md3Parser("armor_red", std::begin(reader), std::end(reader), *fs);
            auto model = std::unique_ptr<Assets::EntityModel>(parser.initializeModel(logger));

            CHECK(model != nullptr);

            CHECK(model->frameCount() == 30u);
            CHECK(model->surfaceCount() == 2u);

            for (size_t i = 0; i < model->frameCount(); ++i) {
                CHECK_NOTHROW(parser.loadFrame(i, *model, logger));
            }
        }
    }
}
