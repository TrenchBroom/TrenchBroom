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
#include "IO/Quake3ShaderFileSystem.h"
#include "IO/TraversalMode.h"
#include "IO/VirtualFileSystem.h"
#include "Logger.h"
#include "Matchers.h"

#include <filesystem>
#include <memory>

#include "Catch2.h"

namespace TrenchBroom
{
namespace IO
{
TEST_CASE("Quake3ShaderFileSystemTest.testShaderLinking")
{
  auto logger = NullLogger{};

  const auto workDir = std::filesystem::current_path();
  const auto testDir = workDir / "fixture/test/IO/Shader/fs/linking";
  const auto fallbackDir = testDir / "fallback";
  const auto texturePrefix = std::filesystem::path{"textures"};
  const auto shaderSearchPath = std::filesystem::path{"scripts"};
  const auto textureSearchPaths = std::vector<std::filesystem::path>{texturePrefix};

  // We need to mount the fallback dir so that we can find "__TB_empty.png" which is
  // automatically linked when no editor image is available.
  auto fs = VirtualFileSystem{};
  fs.mount("", std::make_unique<DiskFileSystem>(fallbackDir));
  fs.mount("", std::make_unique<DiskFileSystem>(testDir));
  fs.mount(
    "",
    createImageFileSystem<Quake3ShaderFileSystem>(
      fs, shaderSearchPath, textureSearchPaths, logger)
      .value());

  CHECK_THAT(
    fs.find(texturePrefix / "test", TraversalMode::Flat, makeExtensionPathMatcher({""})),
    MatchesPathsResult({
      texturePrefix / "test/editor_image",
      texturePrefix / "test/not_existing",
      texturePrefix / "test/not_existing2",
      texturePrefix / "test/test",
      texturePrefix / "test/test2",
    }));

  CHECK_THAT(
    fs.find(texturePrefix, TraversalMode::Recursive, makeExtensionPathMatcher({""})),
    MatchesPathsResult({
      texturePrefix / "__TB_empty",
      texturePrefix / "test",
      texturePrefix / "test/editor_image",
      texturePrefix / "test/not_existing",
      texturePrefix / "test/not_existing2",
      texturePrefix / "test/test",
      texturePrefix / "test/test2",
    }));
}

TEST_CASE("Quake3ShaderFileSystemTest.testSkipMalformedFiles")
{
  auto logger = NullLogger{};

  // There is one malformed shader script, this should be skipped.

  const auto workDir = std::filesystem::current_path();
  const auto testDir = workDir / "fixture/test/IO/Shader/fs/failing";
  const auto fallbackDir = testDir / "fallback";
  const auto texturePrefix = std::filesystem::path{"textures"};
  const auto shaderSearchPath = std::filesystem::path{"scripts"};
  const auto textureSearchPaths = std::vector<std::filesystem::path>{texturePrefix};

  // We need to mount the fallback dir so that we can find "__TB_empty.png" which is
  // automatically linked when no editor image is available.
  auto fs = VirtualFileSystem{};
  fs.mount("", std::make_unique<DiskFileSystem>(fallbackDir));
  fs.mount("", std::make_unique<DiskFileSystem>(testDir));
  fs.mount(
    "",
    createImageFileSystem<Quake3ShaderFileSystem>(
      fs, shaderSearchPath, textureSearchPaths, logger)
      .value());

  CHECK_THAT(
    fs.find(texturePrefix / "test", TraversalMode::Flat, makeExtensionPathMatcher({""})),
    MatchesPathsResult({
      texturePrefix / "test/editor_image",
      texturePrefix / "test/not_existing",
      texturePrefix / "test/not_existing2",
      texturePrefix / "test/test",
      texturePrefix / "test/test2",
    }));
}
} // namespace IO
} // namespace TrenchBroom
