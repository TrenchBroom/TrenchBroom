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
#include "Model/LayerNode.h"
#include "TestUtils.h"
#include "View/Autosaver.h"
#include "View/MapDocumentTest.h"

#include <chrono>
#include <filesystem>
#include <thread>

#include "Catch2.h"

namespace TrenchBroom
{
namespace View
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
  using namespace std::literals::chrono_literals;

  IO::TestEnvironment env;
  NullLogger logger;

  document->saveDocumentAs(env.dir() / "test.map");
  assert(env.fileExists("test.map"));

  Autosaver autosaver(document, 10s);

  // modify the map
  document->addNodes({{document->currentLayer(), {createBrushNode("some_texture")}}});

  autosaver.triggerAutosave(logger);

  CHECK_FALSE(env.fileExists("autosave/test.1.map"));
  CHECK_FALSE(env.directoryExists("autosave"));
}

TEST_CASE_METHOD(MapDocumentTest, "MapDocumentTest.autosaverNoSaveOfUnchangedMap")
{
  using namespace std::literals::chrono_literals;

  IO::TestEnvironment env;
  NullLogger logger;

  document->saveDocumentAs(env.dir() / "test.map");
  assert(env.fileExists("test.map"));

  Autosaver autosaver(document, 0s);
  autosaver.triggerAutosave(logger);

  CHECK_FALSE(env.fileExists("autosave/test.1.map"));
  CHECK_FALSE(env.directoryExists("autosave"));
}

TEST_CASE_METHOD(MapDocumentTest, "MapDocumentTest.autosaverSavesAfterSaveInterval")
{
  using namespace std::literals::chrono_literals;

  IO::TestEnvironment env;
  NullLogger logger;

  document->saveDocumentAs(env.dir() / "test.map");
  assert(env.fileExists("test.map"));

  Autosaver autosaver(document, 100ms);

  // modify the map
  document->addNodes({{document->currentLayer(), {createBrushNode("some_texture")}}});

  // Wait for 2 seconds.
  using namespace std::chrono_literals;
  std::this_thread::sleep_for(100ms);

  autosaver.triggerAutosave(logger);

  CHECK(env.fileExists("autosave/test.1.map"));
  CHECK(env.directoryExists("autosave"));
}

TEST_CASE_METHOD(MapDocumentTest, "MapDocumentTest.autosaverSavesAgainAfterSaveInterval")
{
  using namespace std::literals::chrono_literals;

  IO::TestEnvironment env;
  NullLogger logger;

  document->saveDocumentAs(env.dir() / "test.map");
  assert(env.fileExists("test.map"));

  Autosaver autosaver(document, 100ms);

  // modify the map
  document->addNodes({{document->currentLayer(), {createBrushNode("some_texture")}}});

  // Wait for 2 seconds.
  using namespace std::chrono_literals;
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

TEST_CASE_METHOD(MapDocumentTest, "MapDocumentTest.autosaverSavesWhenCrashFilesPresent")
{
  // https://github.com/TrenchBroom/TrenchBroom/issues/2544

  using namespace std::literals::chrono_literals;

  IO::TestEnvironment env;
  env.createDirectory("autosave");
  env.createFile("autosave/test.1.map", "some content");
  env.createFile("autosave/test.1-crash.map", "some content again");

  NullLogger logger;

  document->saveDocumentAs(env.dir() / "test.map");
  assert(env.fileExists("test.map"));

  Autosaver autosaver(document, 0s);

  // modify the map
  document->addNodes({{document->currentLayer(), {createBrushNode("some_texture")}}});

  autosaver.triggerAutosave(logger);

  CHECK(env.fileExists("autosave/test.2.map"));
}
} // namespace View
} // namespace TrenchBroom
