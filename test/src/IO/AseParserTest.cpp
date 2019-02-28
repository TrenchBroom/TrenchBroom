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

#include <gtest/gtest.h>

#include "Logger.h"
#include "StringUtils.h"
#include "IO/AseParser.h"
#include "IO/DiskFileSystem.h"
#include "IO/Quake3ShaderFileSystem.h"
#include "IO/TextureReader.h"

namespace TrenchBroom {
    namespace IO {
        TEST(AseParserTest, loadWithoutException) {
            NullLogger logger;
            auto searchPaths = Path::List { Path("models") };
            std::shared_ptr<FileSystem> fs = std::make_shared<DiskFileSystem>(IO::Disk::getCurrentWorkingDir() + Path("data/IO/Ase/wedge_with_shader"));
            fs = std::make_shared<Quake3ShaderFileSystem>(fs, searchPaths, logger);

            const auto aseFile = fs->openFile(Path("models/mapobjects/wedges/wedge_45.ase"));
            const auto basePath = Path("maps");
            AseParser parser(aseFile->begin(), aseFile->end(), *fs, basePath, Path());

            const auto* model = parser.parseModel(logger);
            ASSERT_NE(nullptr, model);
        }
    }
}

