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
#include "IO/Path.h"
#include "IO/Quake3ShaderFileSystem.h"

#include <memory>

namespace TrenchBroom {
    namespace IO {
        void assertShader(const Path::List& paths, const String& path);

        TEST(Quake3ShaderFileSystemTest, testShaderLinking) {
            NullLogger logger;

            const auto workDir = IO::Disk::getCurrentWorkingDir();
            const auto fallbackDir = workDir + Path("games/Quake3/assets");
            const auto testDir = workDir + Path("data/IO/Shader");
            const auto prefix = Path("textures");
            const auto extensions = StringList { "tga", "jpg" };

            // We need to add the fallback dir so that we can find "__TB_empty.tga" which is automatically linked when
            // no editor image is available.
            std::unique_ptr<FileSystem> fs = std::make_unique<DiskFileSystem>(fallbackDir);
            fs = std::make_unique<DiskFileSystem>(std::move(fs), testDir);
            fs = std::make_unique<Quake3ShaderFileSystem>(std::move(fs), prefix, extensions, &logger);


            const auto items = fs->findItems(Path("textures/test"));
            ASSERT_EQ(5u, items.size());

            assertShader(items, "textures/test/editor_image.jpg");
            assertShader(items, "textures/test/test.tga");
            assertShader(items, "textures/test/test2.tga");
            assertShader(items, "textures/test/not_existing");
            assertShader(items, "textures/test/not_existing2");

            auto file = fs->openFile(Path("textures/test/editor_image.jpg"));
            ASSERT_FALSE(file->hasAttribute(MappedFile::Transparency));

            file = fs->openFile(Path("textures/test/test.tga"));
            ASSERT_TRUE(file->hasAttribute(MappedFile::Transparency));
            ASSERT_FLOAT_EQ(0.1f, file->getAttribute(MappedFile::Transparency));

            file = fs->openFile(Path("textures/test/test2.tga"));
            ASSERT_FALSE(file->hasAttribute(MappedFile::Transparency));

            file = fs->openFile(Path("textures/test/not_existing"));
            ASSERT_TRUE(file->hasAttribute(MappedFile::Transparency));
            ASSERT_FLOAT_EQ(0.2f, file->getAttribute(MappedFile::Transparency));

            file = fs->openFile(Path("textures/test/not_existing2"));
            ASSERT_TRUE(file->hasAttribute(MappedFile::Transparency));
            ASSERT_FLOAT_EQ(0.3f, file->getAttribute(MappedFile::Transparency));
        }

        void assertShader(const Path::List& paths, const String& path) {
            ASSERT_EQ(1u, std::count_if(std::begin(paths), std::end(paths), [&path](const auto& item) { return item == Path(path); }));
        }
    }
}
