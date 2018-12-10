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
#include "StringUtils.h"
#include "IO/DiskFileSystem.h"
#include "IO/FileSystemHierarchy.h"
#include "IO/Path.h"
#include "IO/Quake3ShaderFileSystem.h"

#include <memory>

namespace TrenchBroom {
    namespace IO {
        void assertShader(const Path::List& paths, const String& path);

        TEST(Quake3ShaderFileSystemTest, testShaderLinking) {
            NullLogger logger;
            FileSystemHierarchy fs;

            const auto workDir = IO::Disk::getCurrentWorkingDir();
            const auto testDir = workDir + Path("data/IO/Shader");
            fs.pushFileSystem(std::make_unique<DiskFileSystem>(testDir));
            fs.pushFileSystem(std::make_unique<Quake3ShaderFileSystem>(Path("scripts"), fs, &logger));


            const auto items = fs.findItems(Path("textures/test"));
            ASSERT_EQ(4u, items.size());

            assertShader(items, "textures/test/editor_image.jpg");
            assertShader(items, "textures/test/test.tga");
            assertShader(items, "textures/test/test2.tga");
            assertShader(items, "textures/test/not_existing");
        }

        void assertShader(const Path::List& paths, const String& path) {
            ASSERT_EQ(1u, std::count_if(std::begin(paths), std::end(paths), [&path](const auto& item) { return item == Path(path); }));
        }
    }
}
