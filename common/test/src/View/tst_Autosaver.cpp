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

#include <QString>

#include "IO/DiskFileSystem.h"
#include "IO/TestEnvironment.h"
#include "Logger.h"
#include "Model/BrushNode.h"
#include "Model/EntityNode.h"
#include "Model/LayerNode.h"
#include "TestUtils.h"
#include "View/Autosaver.h"
#include "View/MapDocumentTest.h"

#include "kdl/vector_utils.h"

#include <fmt/format.h>

#include <chrono>
#include <filesystem>
#include <thread>

#include "Catch2.h"

namespace TrenchBroom::View
{

namespace
{
IO::TestEnvironment makeTestEnvironment()
{
  // have a non-ASCII character in the directory name to help catch
  // filename encoding bugs
  const auto hiraganaLetterSmallA = QString(static_cast<QChar>(0x3041));
  const auto dir = (QString::fromStdString(Catch::getResultCapture().getCurrentTestName())
                    + hiraganaLetterSmallA)
                     .toStdString();

  return IO::TestEnvironment{dir, [](auto& env) {
                               env.createDirectory("dir");

                               env.createFile("test.1.map", "some content");
                               env.createFile("test.2.map", "some content");
                               env.createFile("test.20.map", "some content");
                             }};
}

} // namespace

TEST_CASE("AutosaverTest.makeBackupPathMatcher")
{
  const auto env = makeTestEnvironment();
  auto fs = IO::DiskFileSystem{env.dir()};

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

TEST_CASE_METHOD(MapDocumentTest, "MapDocumentTest.autosaverNoSaveUntilSaveInterval")
{
  using namespace std::chrono_literals;

  auto env = IO::TestEnvironment{};
  auto logger = NullLogger{};

  document->saveDocumentAs(env.dir() / "test.map");
  assert(env.fileExists("test.map"));

  auto autosaver = Autosaver{document, 10s};

  // modify the map
  document->addNodes({{document->currentLayer(), {createBrushNode("some_texture")}}});

  autosaver.triggerAutosave(logger);

  CHECK_FALSE(env.fileExists("autosave/test.1.map"));
  CHECK_FALSE(env.directoryExists("autosave"));
}

TEST_CASE_METHOD(MapDocumentTest, "MapDocumentTest.autosaverNoSaveOfUnchangedMap")
{
  using namespace std::chrono_literals;

  auto env = IO::TestEnvironment{};
  auto logger = NullLogger{};

  document->saveDocumentAs(env.dir() / "test.map");
  assert(env.fileExists("test.map"));

  auto autosaver = Autosaver{document, 0s};
  autosaver.triggerAutosave(logger);

  CHECK_FALSE(env.fileExists("autosave/test.1.map"));
  CHECK_FALSE(env.directoryExists("autosave"));
}

TEST_CASE_METHOD(MapDocumentTest, "MapDocumentTest.autosaverSavesAfterSaveInterval")
{
  using namespace std::chrono_literals;

  auto env = IO::TestEnvironment{};
  auto logger = NullLogger{};

  document->saveDocumentAs(env.dir() / "test.map");
  assert(env.fileExists("test.map"));

  auto autosaver = Autosaver{document, 100ms};

  // modify the map
  document->addNodes({{document->currentLayer(), {createBrushNode("some_texture")}}});

  std::this_thread::sleep_for(100ms);

  autosaver.triggerAutosave(logger);

  CHECK(env.fileExists("autosave/test.1.map"));
  CHECK(env.directoryExists("autosave"));
}

TEST_CASE_METHOD(MapDocumentTest, "MapDocumentTest.autosaverSavesAgainAfterSaveInterval")
{
  using namespace std::chrono_literals;

  auto env = IO::TestEnvironment{};
  auto logger = NullLogger{};

  document->saveDocumentAs(env.dir() / "test.map");
  assert(env.fileExists("test.map"));

  auto autosaver = Autosaver{document, 100ms};

  // modify the map
  document->addNodes({{document->currentLayer(), {createBrushNode("some_texture")}}});

  std::this_thread::sleep_for(100ms);

  autosaver.triggerAutosave(logger);

  CHECK(env.fileExists("autosave/test.1.map"));
  CHECK(env.directoryExists("autosave"));

  // Wait for 2 seconds.
  using namespace std::chrono_literals;
  std::this_thread::sleep_for(100ms);

  autosaver.triggerAutosave(logger);
  CHECK_FALSE(env.fileExists("autosave/test.2.map"));

  // modify the map
  document->addNodes({{document->currentLayer(), {createBrushNode("some_texture")}}});

  autosaver.triggerAutosave(logger);
  CHECK(env.fileExists("autosave/test.2.map"));
}

TEST_CASE_METHOD(MapDocumentTest, "MapDocumentTest.autosaverCleanup")
{
  using namespace std::chrono_literals;

  constexpr auto maxBackups = 3u;
  auto env = IO::TestEnvironment{};
  env.createDirectory("autosave");

  SECTION("Files are rotated")
  {
    const auto initialPaths = std::vector<std::filesystem::path>{
      "autosave/test.1.map",
      "autosave/test.2.map",
    };

    for (const auto& path : initialPaths)
    {
      env.createFile(path, path.u8string());
    }

    REQUIRE(env.directoryContents("autosave") == initialPaths);
    REQUIRE(
      kdl::vec_transform(
        initialPaths, [&](const auto& path) { return env.loadFile(path); })
      == std::vector<std::string>{
        "autosave/test.1.map",
        "autosave/test.2.map",
      });

    auto logger = NullLogger{};

    document->saveDocumentAs(env.dir() / "test.map");
    assert(env.fileExists("test.map"));

    auto autosaver = Autosaver{document, 100ms, maxBackups};

    // modify the map
    document->addNodes({{document->currentLayer(), {new Model::EntityNode{{}}}}});

    std::this_thread::sleep_for(100ms);
    autosaver.triggerAutosave(logger);

    const auto allPaths = kdl::vec_push_back(initialPaths, "autosave/test.3.map");

    CHECK(env.directoryContents("autosave") == allPaths);
    CHECK(
      kdl::vec_transform(allPaths, [&](const auto& path) { return env.loadFile(path); })
      == std::vector<std::string>{
        "autosave/test.1.map",
        "autosave/test.2.map",
        R"(// entity 0
{
"classname" "worldspawn"
}
// entity 1
{
}
)",
      });

    // modify the map again
    document->addNodes({{document->currentLayer(), {new Model::EntityNode{{}}}}});

    std::this_thread::sleep_for(100ms);
    autosaver.triggerAutosave(logger);

    CHECK(env.directoryContents("autosave") == allPaths);
    CHECK(
      kdl::vec_transform(allPaths, [&](const auto& path) { return env.loadFile(path); })
      == std::vector<std::string>{
        "autosave/test.2.map",
        R"(// entity 0
{
"classname" "worldspawn"
}
// entity 1
{
}
)",
        R"(// entity 0
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
      });
  }

  SECTION("Gaps are compacted")
  {
    const auto initialPaths = std::vector<std::filesystem::path>{
      "autosave/test.1.map",
      "autosave/test.3.map",
    };

    for (const auto& path : initialPaths)
    {
      env.createFile(path, path.u8string());
    }

    REQUIRE(env.directoryContents("autosave") == initialPaths);
    REQUIRE(
      kdl::vec_transform(
        initialPaths, [&](const auto& path) { return env.loadFile(path); })
      == std::vector<std::string>{
        "autosave/test.1.map",
        "autosave/test.3.map",
      });

    auto logger = NullLogger{};

    document->saveDocumentAs(env.dir() / "test.map");
    assert(env.fileExists("test.map"));

    auto autosaver = Autosaver{document, 100ms, maxBackups};

    // modify the map
    document->addNodes({{document->currentLayer(), {new Model::EntityNode{{}}}}});

    std::this_thread::sleep_for(100ms);
    autosaver.triggerAutosave(logger);

    const auto allPaths = std::vector<std::filesystem::path>{
      "autosave/test.1.map",
      "autosave/test.2.map",
      "autosave/test.3.map",
    };

    CHECK(env.directoryContents("autosave") == allPaths);
    CHECK(
      kdl::vec_transform(allPaths, [&](const auto& path) { return env.loadFile(path); })
      == std::vector<std::string>{
        "autosave/test.1.map",
        "autosave/test.3.map",
        R"(// entity 0
{
"classname" "worldspawn"
}
// entity 1
{
}
)",
      });
  }
}

TEST_CASE_METHOD(MapDocumentTest, "MapDocumentTest.autosaverSavesWhenCrashFilesPresent")
{
  // https://github.com/TrenchBroom/TrenchBroom/issues/2544

  using namespace std::chrono_literals;

  auto env = IO::TestEnvironment{};
  env.createDirectory("autosave");
  env.createFile("autosave/test.1.map", "some content");
  env.createFile("autosave/test.1-crash.map", "some content again");

  auto logger = NullLogger{};

  document->saveDocumentAs(env.dir() / "test.map");
  assert(env.fileExists("test.map"));

  auto autosaver = Autosaver{document, 0s};

  // modify the map
  document->addNodes({{document->currentLayer(), {createBrushNode("some_texture")}}});

  autosaver.triggerAutosave(logger);

  CHECK(env.fileExists("autosave/test.2.map"));
}

} // namespace TrenchBroom::View
