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

#include <gtest/gtest.h>

#include "Logger.h"
#include "IO/DiskFileSystem.h"
#include "IO/DiskIO.h"
#include "IO/MappedFile.h"
#include "IO/MdlParser.h"
#include "Model/Entity.h"
#include "Assets/EntityModel.h"
#include "Assets/Palette.h"

namespace TrenchBroom {
    namespace IO {
        TEST(MdlParserTest, loadValidMdl) {
            NullLogger logger;

            DiskFileSystem fs(IO::Disk::getCurrentWorkingDir());
            const Assets::Palette palette = Assets::Palette::loadFile(fs, Path("data/test/palette.lmp"));

            const auto mdlPath = IO::Disk::getCurrentWorkingDir() + IO::Path("data/test/IO/Mdl/armor.mdl");
            const MappedFile::Ptr mdlFile = Disk::openFile(mdlPath);
            ASSERT_NE(nullptr, mdlFile);

            auto parser = MdlParser("armor", mdlFile->begin(), mdlFile->end(), palette);
            auto* model = parser.initializeModel(logger);
            EXPECT_NE(nullptr, model);
            EXPECT_EQ(1u, model->surfaceCount());
            EXPECT_EQ(1u, model->frameCount());

            const auto surfaces = model->surfaces();
            const auto& surface = *surfaces.front();
            EXPECT_EQ(3u, surface.skinCount());
            EXPECT_EQ(1u, surface.frameCount());

            delete model;
        }

        TEST(MdlParserTest, loadInvalidMdl) {
            NullLogger logger;

            DiskFileSystem fs(IO::Disk::getCurrentWorkingDir());
            const Assets::Palette palette = Assets::Palette::loadFile(fs, Path("data/test/palette.lmp"));

            const auto mdlPath = IO::Disk::getCurrentWorkingDir() + IO::Path("data/test/IO/Mdl/invalid.mdl");
            const MappedFile::Ptr mdlFile = Disk::openFile(mdlPath);
            ASSERT_NE(nullptr, mdlFile);

            auto parser = MdlParser("armor", mdlFile->begin(), mdlFile->end(), palette);
            EXPECT_THROW(parser.initializeModel(logger), AssetException);
        }
    }
}
