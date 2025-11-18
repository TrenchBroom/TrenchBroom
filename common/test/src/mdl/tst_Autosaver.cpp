/*
 Copyright (C) 2010 Kristian Duske

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

#include <QString>

#include "MapFixture.h"
#include "TestFactory.h"
#include "io/DiskFileSystem.h"
#include "io/TestEnvironment.h"
#include "mdl/Autosaver.h"
#include "mdl/BrushNode.h" // IWYU pragma: keep
#include "mdl/EditorContext.h"
#include "mdl/EntityNode.h"
#include "mdl/LayerNode.h" // IWYU pragma: keep
#include "mdl/Map.h"
#include "mdl/Map_Nodes.h"

#include "kd/vector_utils.h"

#include <fmt/format.h>

#include <chrono>
#include <filesystem>
#include <ranges>
#include <thread>

#include "catch/CatchConfig.h"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_range_equals.hpp>

namespace tb::mdl
{
using namespace Catch::Matchers;

namespace
{

io::TestEnvironment makeTestEnvironment()
{
  // have a non-ASCII character in the directory name to help catch
  // filename encoding bugs
  const auto hiraganaLetterSmallA = QString(static_cast<QChar>(0x3041));
  const auto dir = (QString::fromStdString(Catch::getResultCapture().getCurrentTestName())
                    + hiraganaLetterSmallA)
                     .toStdString();

  return io::TestEnvironment{dir, [](auto& env) {
                               env.createDirectory("dir");

                               env.createFile("test.1.map", "some content");
                               env.createFile("test.2.map", "some content");
                               env.createFile("test.20.map", "some content");
                             }};
}

} // namespace

TEST_CASE("makeBackupPathMatcher")
{
  const auto env = makeTestEnvironment();
  auto fs = io::DiskFileSystem{env.dir()};

  const auto matcher = makeBackupPathMatcher("test");
  const auto getPathInfo = [&](const auto& p) { return fs.pathInfo(p); };

  CHECK(matcher("test.1.map", getPathInfo));
  CHECK(matcher("test.2.map", getPathInfo));
  CHECK(matcher("test.20.map", getPathInfo));
  CHECK_FALSE(matcher("dir", getPathInfo));
  CHECK_FALSE(matcher("test.map", getPathInfo));
  CHECK_FALSE(matcher("test.1-crash.map", getPathInfo));
  CHECK_FALSE(matcher("test.2-crash.map", getPathInfo));
}

TEST_CASE("Autosaver")
{
  using namespace std::chrono_literals;

  auto fixture = MapFixture{};
  auto& map = fixture.map();
  fixture.create();

  auto env = io::TestEnvironment{};

  const auto loadFile = [&](const auto& path) { return env.loadFile(path); };

  SECTION("Don't trigger autosave before the save interval expires")
  {
    REQUIRE(map.saveAs(env.dir() / "test.map"));
    REQUIRE(env.fileExists("test.map"));

    auto autosaver = Autosaver{map, 10s};

    // modify the map
    addNodes(
      map,
      {{map.editorContext().currentLayer(), {createBrushNode(map, "some_material")}}});

    autosaver.triggerAutosave();

    CHECK_FALSE(env.fileExists("autosave/test.1.map"));
    CHECK_FALSE(env.directoryExists("autosave"));
  }

  SECTION("Trigger a save when the interval expires")
  {
    REQUIRE(map.saveAs(env.dir() / "test.map"));
    REQUIRE(env.fileExists("test.map"));

    auto autosaver = Autosaver{map, 100ms};

    // modify the map
    addNodes(
      map,
      {{map.editorContext().currentLayer(), {createBrushNode(map, "some_material")}}});

    std::this_thread::sleep_for(100ms);
    autosaver.triggerAutosave();

    CHECK(env.fileExists("autosave/test.1.map"));
    CHECK(env.directoryExists("autosave"));
  }

  SECTION("Trigger another save when the interval expires again and the map is changed")
  {
    REQUIRE(map.saveAs(env.dir() / "test.map"));
    REQUIRE(env.fileExists("test.map"));

    auto autosaver = Autosaver{map, 100ms};

    // modify the map
    addNodes(
      map,
      {{map.editorContext().currentLayer(), {createBrushNode(map, "some_material")}}});

    std::this_thread::sleep_for(100ms);

    autosaver.triggerAutosave();

    CHECK(env.fileExists("autosave/test.1.map"));
    CHECK(env.directoryExists("autosave"));

    // Wait for 2 seconds.
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(100ms);

    autosaver.triggerAutosave();
    CHECK_FALSE(env.fileExists("autosave/test.2.map"));

    // modify the map
    addNodes(
      map,
      {{map.editorContext().currentLayer(), {createBrushNode(map, "some_material")}}});

    autosaver.triggerAutosave();
    CHECK(env.fileExists("autosave/test.2.map"));
  }

  SECTION("Don't save unchanged maps")
  {
    REQUIRE(map.saveAs(env.dir() / "test.map"));
    REQUIRE(env.fileExists("test.map"));

    auto autosaver = Autosaver{map, 0s};
    autosaver.triggerAutosave();

    CHECK_FALSE(env.fileExists("autosave/test.1.map"));
    CHECK_FALSE(env.directoryExists("autosave"));
  }

  SECTION("Autosave works when crash files are present")
  {
    // https://github.com/TrenchBroom/TrenchBroom/issues/2544

    env.createDirectory("autosave");
    env.createFile("autosave/test.1.map", "some content");
    env.createFile("autosave/test.1-crash.map", "some content again");

    REQUIRE(map.saveAs(env.dir() / "test.map"));
    REQUIRE(env.fileExists("test.map"));

    auto autosaver = Autosaver{map, 0s};

    // modify the map
    addNodes(
      map,
      {{map.editorContext().currentLayer(), {createBrushNode(map, "some_material")}}});

    autosaver.triggerAutosave();

    CHECK(env.fileExists("autosave/test.2.map"));
  }

  SECTION("Cleanup")
  {
    constexpr auto maxBackups = 3u;
    env.createDirectory("autosave");

    SECTION("Files are rotated")
    {
      const auto initialPaths = std::vector<std::filesystem::path>{
        "autosave/test.1.map",
        "autosave/test.2.map",
      };

      for (const auto& path : initialPaths)
      {
        env.createFile(path, path.string());
      }

      REQUIRE(env.directoryContents("autosave") == initialPaths);
      REQUIRE_THAT(
        initialPaths | std::views::transform(loadFile),
        RangeEquals(std::vector{
          "autosave/test.1.map",
          "autosave/test.2.map",
        }));

      REQUIRE(map.saveAs(env.dir() / "test.map"));
      REQUIRE(env.fileExists("test.map"));

      auto autosaver = Autosaver{map, 100ms, maxBackups};

      // modify the map
      auto* entity = new EntityNode{{}};
      addNodes(map, {{map.editorContext().currentLayer(), {entity}}});

      std::this_thread::sleep_for(100ms);
      autosaver.triggerAutosave();

      const auto allPaths = kdl::vec_push_back(initialPaths, "autosave/test.3.map");

      CHECK(env.directoryContents("autosave") == allPaths);
      CHECK_THAT(
        allPaths | std::views::transform(loadFile),
        RangeEquals(std::vector{
          "autosave/test.1.map",
          "autosave/test.2.map",
          R"(// Game: Test
// Format: Standard
// entity 0
{
"classname" "worldspawn"
}
// entity 1
{
}
)",
        }));

      // modify the map again
      addNodes(map, {{map.editorContext().currentLayer(), {new EntityNode{{}}}}});

      std::this_thread::sleep_for(100ms);
      autosaver.triggerAutosave();

      CHECK(env.directoryContents("autosave") == allPaths);
      CHECK_THAT(
        allPaths | std::views::transform(loadFile),
        RangeEquals(std::vector{
          "autosave/test.2.map",
          R"(// Game: Test
// Format: Standard
// entity 0
{
"classname" "worldspawn"
}
// entity 1
{
}
)",
          R"(// Game: Test
// Format: Standard
// entity 0
{
"classname" "worldspawn"
}
// entity 1
{
}
// entity 2
{
}
)",
        }));
    }

    SECTION("Gaps are compacted")
    {
      const auto initialPaths = std::vector<std::filesystem::path>{
        "autosave/test.1.map",
        "autosave/test.3.map",
      };

      for (const auto& path : initialPaths)
      {
        env.createFile(path, path.string());
      }

      REQUIRE(env.directoryContents("autosave") == initialPaths);
      REQUIRE_THAT(
        initialPaths | std::views::transform(loadFile),
        RangeEquals(std::vector{
          "autosave/test.1.map",
          "autosave/test.3.map",
        }));

      REQUIRE(map.saveAs(env.dir() / "test.map"));
      REQUIRE(env.fileExists("test.map"));

      auto autosaver = Autosaver{map, 100ms, maxBackups};

      // modify the map
      addNodes(map, {{map.editorContext().currentLayer(), {new EntityNode{{}}}}});

      std::this_thread::sleep_for(100ms);
      autosaver.triggerAutosave();

      const auto allPaths = std::vector<std::filesystem::path>{
        "autosave/test.1.map",
        "autosave/test.2.map",
        "autosave/test.3.map",
      };

      CHECK(env.directoryContents("autosave") == allPaths);
      CHECK_THAT(
        allPaths | std::views::transform(loadFile),
        RangeEquals(std::vector{
          "autosave/test.1.map",
          "autosave/test.3.map",
          R"(// Game: Test
// Format: Standard
// entity 0
{
"classname" "worldspawn"
}
// entity 1
{
}
)",
        }));
    }
  }
}

} // namespace tb::mdl
