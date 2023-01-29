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

#include "Assets/Quake3Shader.h"
#include "IO/DiskFileSystem.h"
#include "IO/DiskIO.h"
#include "IO/FileMatcher.h"
#include "IO/Path.h"
#include "IO/Quake3ShaderFileSystem.h"
#include "Logger.h"

#include <memory>

#include "Catch2.h"

namespace TrenchBroom
{
namespace IO
{
TEST_CASE("Quake3ShaderFileSystemTest.testShaderLinking")
{
  NullLogger logger;

  const auto workDir = IO::Disk::getCurrentWorkingDir();
  const auto testDir = workDir + Path("fixture/test/IO/Shader/fs/linking");
  const auto fallbackDir = testDir + Path("fallback");
  const auto texturePrefix = Path("textures");
  const auto shaderSearchPath = Path("scripts");
  const auto textureSearchPaths = std::vector<Path>{texturePrefix};

  // We need to add the fallback dir so that we can find "__TB_empty.png" which is
  // automatically linked when no editor image is available.
  std::shared_ptr<FileSystem> fs = std::make_shared<DiskFileSystem>(fallbackDir);
  fs = std::make_shared<DiskFileSystem>(fs, testDir);
  fs = std::make_shared<Quake3ShaderFileSystem>(
    fs, shaderSearchPath, textureSearchPaths, logger);

  CHECK_THAT(
    fs->findItems(texturePrefix + Path("test"), FileExtensionMatcher("")),
    Catch::Matchers::UnorderedEquals(std::vector<Path>{
      texturePrefix + Path("test/editor_image"),
      texturePrefix + Path("test/test"),
      texturePrefix + Path("test/test2"),
      texturePrefix + Path("test/not_existing"),
      texturePrefix + Path("test/not_existing2"),
    }));
}

TEST_CASE("Quake3ShaderFileSystemTest.testSkipMalformedFiles")
{
  NullLogger logger;

  // There is one malformed shader script, this should be skipped.

  const auto workDir = IO::Disk::getCurrentWorkingDir();
  const auto testDir = workDir + Path("fixture/test/IO/Shader/fs/failing");
  const auto fallbackDir = testDir + Path("fallback");
  const auto texturePrefix = Path("textures");
  const auto shaderSearchPath = Path("scripts");
  const auto textureSearchPaths = std::vector<Path>{texturePrefix};

  // We need to add the fallback dir so that we can find "__TB_empty.png" which is
  // automatically linked when no editor image is available.
  std::shared_ptr<FileSystem> fs = std::make_shared<DiskFileSystem>(fallbackDir);
  fs = std::make_shared<DiskFileSystem>(fs, testDir);
  fs = std::make_shared<Quake3ShaderFileSystem>(
    fs, shaderSearchPath, textureSearchPaths, logger);

  CHECK_THAT(
    fs->findItems(texturePrefix + Path("test"), FileExtensionMatcher("")),
    Catch::Matchers::UnorderedEquals(std::vector<Path>{
      texturePrefix + Path("test/editor_image"),
      texturePrefix + Path("test/test"),
      texturePrefix + Path("test/test2"),
      texturePrefix + Path("test/not_existing"),
      texturePrefix + Path("test/not_existing2"),
    }));
}
} // namespace IO
} // namespace TrenchBroom
