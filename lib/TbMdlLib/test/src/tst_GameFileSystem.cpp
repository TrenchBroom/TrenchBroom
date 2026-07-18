/*
 Copyright (C) 2025 Kristian Duske

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

#include "TestEnvironment.h"
#include "base/Logger.h"
#include "fs/DiskFileSystem.h"
#include "fs/PathInfo.h"
#include "fs/TestEnvironment.h"
#include "fs/TestUtils.h"
#include "fs/TraversalMode.h"
#include "mdl/CatchConfig.h"
#include "mdl/EnvironmentConfig.h"
#include "mdl/GameConfig.h"
#include "mdl/GameFileSystem.h"
#include "mdl/TestUtils.h"

#include "kd/ranges/concat_view.h"
#include "kd/ranges/repeat_view.h"
#include "kd/ranges/to.h"

#include <filesystem>
#include <functional>
#include <thread>
#include <tuple>
#include <vector>

#include <catch2/catch_test_macros.hpp>

namespace tb::mdl
{

TEST_CASE("GameFileSystem")
{
  /*
  id1
    id1_pak0_loose_file - contents: "id1"
    pak0.pak
      id1_pak0_1.txt - contents: "id1_pak0_1"
      id1_pak0_2.txt - contents: "id1_pak0_2"
      id1_pak0_loose_file - contents: "pak0"
  mod1
    id1_pak0_loose_file - contents: "mod1"
    pak0.pak
      id1_pak0_2.txt - contents: "mod1_pak0_2", overrides id1/pak0.pak
      mod1_pak0_1.txt - contents: "mod1_pak0_1"
      mod1_pak0_2.txt - contents: "mod1_pak0_2"
    pak1.PAK
      mod1_pak0_2.txt - contents: "mod1_pak1_2", overrides mod1/pak0.pak
  */
  const auto fixturePath = getFixtureRoot() / "test/mdl/GameFileSystem";

  auto logger = NullLogger{};

  auto fs = GameFileSystem{};

  auto environmentConfig = EnvironmentConfig{};
  const auto gameConfig = GameConfig{
    "some game",
    {},
    {},
    false,
    {},
    {
      "id1",
      {
        {".pak"},
        "idpak",
      },
    },
    {},
    {},
    {},
    {},
    {},
    {},
  };

  SECTION("Mounts packages in game path")
  {
    fs.initialize(environmentConfig, gameConfig, fixturePath, {}, logger);

    CHECK(fs.pathInfo("id1_pak0_1.txt") == fs::PathInfo::File);
    CHECK(fs.pathInfo("id1_pak0_2.txt") == fs::PathInfo::File);
    CHECK(fs.pathInfo("id1_pak0_loose_file.txt") == fs::PathInfo::File);
    CHECK(fs.pathInfo("mod1_pak0_1.txt") == fs::PathInfo::Unknown);
  }

  SECTION("Packages files override loose files")
  {
    fs.initialize(environmentConfig, gameConfig, fixturePath, {}, logger);

    CHECK(fs::readTextFile(fs, "id1_pak0_loose_file.txt") == "pak0");
  }

  SECTION("Mounts packages in additional search paths")
  {
    fs.initialize(
      environmentConfig, gameConfig, fixturePath, {fixturePath / "mod1"}, logger);

    CHECK(fs.pathInfo("id1_pak0_1.txt") == fs::PathInfo::File);
    CHECK(fs.pathInfo("id1_pak0_2.txt") == fs::PathInfo::File);
    CHECK(fs.pathInfo("id1_pak0_loose_file.txt") == fs::PathInfo::File);
    CHECK(fs.pathInfo("mod1_pak0_1.txt") == fs::PathInfo::File);
    CHECK(fs.pathInfo("mod1_pak0_2.txt") == fs::PathInfo::File);
  }

  SECTION("Additional search paths override game path")
  {
    fs.initialize(
      environmentConfig, gameConfig, fixturePath, {fixturePath / "mod1"}, logger);

    CHECK(fs::readTextFile(fs, "id1_pak0_loose_file.txt") == "mod1");
    CHECK(fs::readTextFile(fs, "id1_pak0_1.txt") == "id1_pak0_1");
    CHECK(fs::readTextFile(fs, "id1_pak0_2.txt") == "mod1_pak0_2");
    CHECK(fs::readTextFile(fs, "mod1_pak0_1.txt") == "mod1_pak0_1");
    CHECK(fs::readTextFile(fs, "mod1_pak0_2.txt") == "mod1_pak1_2");
  }

  SECTION("Game path is case insensitive")
  {

    const auto ucConfig = GameConfig{
      "some game",
      {},
      {},
      false,
      {},
      {
        "ID1",
        {
          {".pak"},
          "idpak",
        },
      },
      {},
      {},
      {},
      {},
      {},
      {},
    };

    fs.initialize(environmentConfig, ucConfig, fixturePath, {}, logger);

    CHECK(fs.pathInfo("id1_pak0_1.txt") == fs::PathInfo::File);
    CHECK(fs.pathInfo("id1_pak0_2.txt") == fs::PathInfo::File);
    CHECK(fs.pathInfo("id1_pak0_loose_file.txt") == fs::PathInfo::File);
    CHECK(fs.pathInfo("mod1_pak0_1.txt") == fs::PathInfo::Unknown);
  }
}

TEST_CASE("GameFileSystem concurrent reload/mount/unmount vs. reads")
{
  using ThreadFunc = std::function<void()>;

  // Confirms VirtualFileSystem's mount-point lock and DiskFileSystem's cache lock
  // compose correctly together: one thread repeatedly reloads and remounts a
  // DiskFileSystem while other threads concurrently read through the same
  // GameFileSystem. Not a deterministic assertion of absence-of-race (that's what
  // running this under -DTB_ENABLE_TSAN=ON is for) - this just exercises the pattern
  // under contention, with a fixed iteration count so it terminates (a lock-ordering
  // bug here would hang or crash rather than fail an assertion).
  auto env = fs::TestEnvironment{[](auto& e) {
    e.createDirectory("dir");
    e.createFile("dir/file.txt", "some content");
  }};

  auto gfs = GameFileSystem{};
  gfs.mount("", std::make_unique<fs::DiskFileSystem>(env.dir()));

  constexpr auto numReaderThreads = 4;
  constexpr auto numIterations = 500;

  auto writer = ThreadFunc{[&]() {
    for (auto i = 0; i < numIterations; ++i)
    {
      std::ignore = gfs.reload();

      auto diskFs = std::make_unique<fs::DiskFileSystem>(env.dir());
      const auto id = gfs.mount("mnt", std::move(diskFs));
      gfs.unmount(id);
    }
  }};

  auto reader = ThreadFunc{[&]() {
    for (auto i = 0; i < numIterations; ++i)
    {
      static_cast<void>(gfs.pathInfo("dir"));
      static_cast<void>(gfs.find("dir", fs::TraversalMode::Flat));
      static_cast<void>(gfs.openFile("dir/file.txt"));
    }
  }};

  auto threads =
    kdl::views::concat(
      std::views::single(writer), kdl::views::repeat(reader, numReaderThreads))
    | std::views::transform([](auto func) { return std::thread{std::move(func)}; })
    | kdl::ranges::to<std::vector>();

  for (auto& thread : threads)
  {
    thread.join();
  }

  CHECK(gfs.pathInfo("dir") == fs::PathInfo::Directory);
}

} // namespace tb::mdl
