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
#include "Assets/Quake3Shader.h"
#include "IO/DiskFileSystem.h"
#include "IO/DiskIO.h"
#include "IO/FileMatcher.h"
#include "IO/Path.h"
#include "IO/Quake3ShaderFileSystem.h"

#include <memory>

namespace TrenchBroom {
    namespace IO {
        void assertShader(const std::vector<Path>& paths, const Path& path);

        TEST_CASE("Quake3ShaderFileSystemTest.testShaderLinking", "[Quake3ShaderFileSystemTest]") {
            NullLogger logger;

            const auto workDir = IO::Disk::getCurrentWorkingDir();
            const auto testDir = workDir + Path("fixture/test/IO/Shader/fs/linking");
            const auto fallbackDir = testDir + Path("fallback");
            const auto texturePrefix = Path("textures");
            const auto shaderSearchPath = Path("scripts");
            const auto textureSearchPaths = std::vector<Path> { texturePrefix };

            // We need to add the fallback dir so that we can find "__TB_empty.png" which is automatically linked when
            // no editor image is available.
            std::shared_ptr<FileSystem> fs = std::make_shared<DiskFileSystem>(fallbackDir);
            fs = std::make_shared<DiskFileSystem>(fs, testDir);
            fs = std::make_shared<Quake3ShaderFileSystem>(fs, shaderSearchPath, textureSearchPaths, logger);

            const auto items = fs->findItems(texturePrefix + Path("test"), FileExtensionMatcher(""));
            ASSERT_EQ(5u, items.size());

            assertShader(items, texturePrefix + Path("test/editor_image"));
            assertShader(items, texturePrefix + Path("test/test"));
            assertShader(items, texturePrefix + Path("test/test2"));
            assertShader(items, texturePrefix + Path("test/not_existing"));
            assertShader(items, texturePrefix + Path("test/not_existing2"));
        }

        TEST_CASE("Quake3ShaderFileSystemTest.testSkipMalformedFiles", "[Quake3ShaderFileSystemTest]") {
            NullLogger logger;

            // There is one malformed shader script, this should be skipped.

            const auto workDir = IO::Disk::getCurrentWorkingDir();
            const auto testDir = workDir + Path("fixture/test/IO/Shader/fs/failing");
            const auto fallbackDir = testDir + Path("fallback");
            const auto texturePrefix = Path("textures");
            const auto shaderSearchPath = Path("scripts");
            const auto textureSearchPaths = std::vector<Path> { texturePrefix };

            // We need to add the fallback dir so that we can find "__TB_empty.png" which is automatically linked when
            // no editor image is available.
            std::shared_ptr<FileSystem> fs = std::make_shared<DiskFileSystem>(fallbackDir);
            fs = std::make_shared<DiskFileSystem>(fs, testDir);
            fs = std::make_shared<Quake3ShaderFileSystem>(fs, shaderSearchPath, textureSearchPaths, logger);

            const auto items = fs->findItems(texturePrefix + Path("test"), FileExtensionMatcher(""));
            ASSERT_EQ(5u, items.size());

            assertShader(items, texturePrefix + Path("test/editor_image"));
            assertShader(items, texturePrefix + Path("test/test"));
            assertShader(items, texturePrefix + Path("test/test2"));
            assertShader(items, texturePrefix + Path("test/not_existing"));
            assertShader(items, texturePrefix + Path("test/not_existing2"));
        }

        void assertShader(const std::vector<Path>& paths, const Path& path) {
            ASSERT_EQ(1, std::count_if(std::begin(paths), std::end(paths), [&path](const auto& item) { return item == path; }));
        }
    }
}
