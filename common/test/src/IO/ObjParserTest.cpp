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
#include "Assets/EntityModel.h"
#include "IO/DiskFileSystem.h"
#include "IO/DiskIO.h"
#include "IO/File.h"
#include "IO/ObjParser.h"
#include "IO/Reader.h"

namespace TrenchBroom {
    namespace IO {
        TEST_CASE("ObjParserTest.loadValidObj", "[ObjParserTest]") {
            NullLogger logger;

            const auto basePath = Disk::getCurrentWorkingDir() + Path("fixture/test/IO/Obj");
            DiskFileSystem fs(basePath);

            const auto mdlPath = Path("pointyship.obj");
            const auto mdlFile = fs.openFile(mdlPath);
            ASSERT_NE(nullptr, mdlFile);

            auto reader = mdlFile->reader().buffer();
            auto parser = NvObjParser(mdlPath, std::begin(reader), std::end(reader), fs);
            auto model = parser.initializeModel(logger);
            parser.loadFrame(0, *model, logger);

            EXPECT_NE(nullptr, model);
            EXPECT_EQ(1u, model->surfaceCount());
            EXPECT_EQ(1u, model->frameCount());

            const auto surfaces = model->surfaces();
            const auto& surface = *surfaces.front();
            EXPECT_EQ(1u, surface.skinCount());
            EXPECT_EQ(1u, surface.frameCount());
        }
    }
}
