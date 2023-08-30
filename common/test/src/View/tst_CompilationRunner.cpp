/*
Copyright (C) 2020 Kristian Duske

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

#include <QObject>
#include <QTextEdit>
#include <QtTest/QSignalSpy>

#include "EL/VariableStore.h"
#include "IO/TestEnvironment.h"
#include "MapDocumentTest.h"
#include "Model/CompilationTask.h"
#include "TestUtils.h"
#include "View/CompilationContext.h"
#include "View/CompilationRunner.h"
#include "View/CompilationVariables.h"
#include "View/TextOutputAdapter.h"

#include <chrono>
#include <condition_variable>
#include <filesystem>
#include <mutex>
#include <thread>

#include "Catch2.h"

namespace TrenchBroom::View
{
class ExecuteTask
{
private:
  CompilationTaskRunner& m_runner;
  std::mutex m_mutex;
  std::condition_variable m_condition;

public:
  bool started = false;
  bool errored = false;
  bool ended = false;

  explicit ExecuteTask(CompilationTaskRunner& runner)
    : m_runner{runner}
  {
    QObject::connect(&m_runner, &CompilationTaskRunner::start, [&]() {
      started = true;
      auto lock = std::unique_lock<std::mutex>{m_mutex};
      m_condition.notify_all();
    });
    QObject::connect(&m_runner, &CompilationTaskRunner::error, [&]() {
      errored = true;
      auto lock = std::unique_lock<std::mutex>{m_mutex};
      m_condition.notify_all();
    });
    QObject::connect(&m_runner, &CompilationTaskRunner::end, [&]() {
      ended = true;
      auto lock = std::unique_lock<std::mutex>{m_mutex};
      m_condition.notify_all();
    });
  }

  void executeAndWait(const int timeout)
  {
    m_runner.execute();

    auto lock = std::unique_lock<std::mutex>{m_mutex};
    m_condition.wait_for(
      lock, std::chrono::milliseconds{timeout}, [&]() { return errored || ended; });
  }
};

TEST_CASE_METHOD(MapDocumentTest, "CompilationRunToolTaskRunner.runMissingTool")
{
  auto variables = EL::NullVariableStore{};
  auto output = QTextEdit{};
  auto outputAdapter = TextOutputAdapter{&output};

  auto context = CompilationContext{document, variables, outputAdapter, false};

  auto task = Model::CompilationRunTool{true, "", "", false};
  auto runner = CompilationRunToolTaskRunner{context, task};

  auto exec = ExecuteTask{runner};
  exec.executeAndWait(500);

  CHECK(exec.started);
  CHECK(exec.errored);
  CHECK_FALSE(exec.ended);
}

TEST_CASE_METHOD(
  MapDocumentTest, "CompilationCopyFilesTaskRunner.createTargetDirectories")
{
  auto variables = EL::NullVariableStore{};
  auto output = QTextEdit{};
  auto outputAdapter = TextOutputAdapter{&output};

  auto context = CompilationContext{document, variables, outputAdapter, false};

  auto testEnvironment = IO::TestEnvironment{};

  const auto sourcePath = "my_map.map";
  testEnvironment.createFile(sourcePath, "{}");

  const auto targetPath = std::filesystem::path{"some/other/path"};

  auto task = Model::CompilationCopyFiles{
    true,
    (testEnvironment.dir() / sourcePath).string(),
    (testEnvironment.dir() / targetPath).string()};
  auto runner = CompilationCopyFilesTaskRunner{context, task};

  REQUIRE_NOTHROW(runner.execute());

  CHECK(testEnvironment.directoryExists(targetPath));
  CHECK(testEnvironment.loadFile(targetPath / sourcePath) == "{}");
}

TEST_CASE_METHOD(MapDocumentTest, "CompilationRenameFileTaskRunner.renameFile")
{
  const auto overwrite = GENERATE(true, false);

  auto variables = EL::NullVariableStore{};
  auto output = QTextEdit{};
  auto outputAdapter = TextOutputAdapter{&output};

  auto context = CompilationContext{document, variables, outputAdapter, false};

  auto testEnvironment = IO::TestEnvironment{};

  const auto sourcePath = "my_map.map";
  testEnvironment.createFile(sourcePath, "{}");

  const auto targetPath = std::filesystem::path{"some/other/path/your_map.map"};
  if (overwrite)
  {
    testEnvironment.createDirectory(targetPath.parent_path());
    testEnvironment.createFile(targetPath, "{...}");
    REQUIRE(testEnvironment.loadFile(targetPath) == "{...}");
  }

  auto task = Model::CompilationRenameFile{
    true,
    (testEnvironment.dir() / sourcePath).string(),
    (testEnvironment.dir() / targetPath).string()};
  auto runner = CompilationRenameFileTaskRunner{context, task};

  REQUIRE_NOTHROW(runner.execute());

  CHECK(testEnvironment.loadFile(targetPath) == "{}");
}

TEST_CASE_METHOD(MapDocumentTest, "CompilationDeleteFilesTaskRunner.deleteTargetPattern")
{
  auto variables = EL::NullVariableStore{};
  auto output = QTextEdit{};
  auto outputAdapter = TextOutputAdapter{&output};

  auto context = CompilationContext{document, variables, outputAdapter, false};

  auto testEnvironment = IO::TestEnvironment{};

  const auto file1 = "file1.lit";
  const auto file2 = "file2.lit";
  const auto file3 = "file3.map";
  const auto dir = "somedir.lit";

  testEnvironment.createFile(file1, "");
  testEnvironment.createFile(file2, "");
  testEnvironment.createFile(file3, "");
  testEnvironment.createDirectory(dir);

  auto task =
    Model::CompilationDeleteFiles{true, (testEnvironment.dir() / "*.lit").string()};
  auto runner = CompilationDeleteFilesTaskRunner{context, task};

  REQUIRE_NOTHROW(runner.execute());

  CHECK(!testEnvironment.fileExists(file1));
  CHECK(!testEnvironment.fileExists(file2));
  CHECK(testEnvironment.fileExists(file3));
  CHECK(testEnvironment.directoryExists(dir));
}

TEST_CASE_METHOD(MapDocumentTest, "CompilationRunner.stopAfterFirstError")
{
  auto variables = EL::NullVariableStore{};
  auto output = QTextEdit{};
  auto outputAdapter = TextOutputAdapter{&output};

  const auto does_not_exist = "does_not_exist.map";
  const auto does_exist = "does_exist.map";
  const auto should_not_exist = "should_not_exist.map";

  auto testEnvironment = IO::TestEnvironment{};
  testEnvironment.createFile(does_exist, "");

  auto compilationProfile = Model::CompilationProfile{
    "name",
    testEnvironment.dir(),
    {
      Model::CompilationCopyFiles{true, does_not_exist, "does_not_matter.map"},
      Model::CompilationCopyFiles{true, does_exist, should_not_exist},
    }};

  auto runner = CompilationRunner{
    CompilationContext{document, variables, outputAdapter, false}, compilationProfile};

  auto compilationStartedSpy = QSignalSpy{&runner, SIGNAL(compilationStarted())};
  auto compilationEndedSpy = QSignalSpy{&runner, SIGNAL(compilationEnded())};

  REQUIRE(compilationStartedSpy.isValid());
  REQUIRE(compilationEndedSpy.isValid());

  runner.execute();
  REQUIRE(!runner.running());
  REQUIRE(compilationStartedSpy.count() == 1);
  REQUIRE(compilationEndedSpy.count() == 1);

  CHECK_FALSE(testEnvironment.fileExists(should_not_exist));
}

TEST_CASE("CompilationRunner.interpolateToolsVariables")
{
  auto [document, game, gameConfig] = View::loadMapDocument(
    "fixture/test/View/MapDocumentTest/valveFormatMapWithoutFormatTag.map",
    "Quake",
    Model::MapFormat::Unknown);
  const auto testWorkDir = std::string{"/some/path"};
  auto variables = CompilationVariables{document, testWorkDir};
  auto output = QTextEdit{};
  auto outputAdapter = TextOutputAdapter{&output};

  auto context = CompilationContext{document, variables, outputAdapter, false};

  const auto startSubstr = std::string{"foo "};
  const auto midSubstr = std::string{" bar "};
  const auto toInterpolate = startSubstr + std::string{"${MAP_DIR_PATH}"} + midSubstr
                             + std::string{"${WORK_DIR_PATH}"};
  const auto expected =
    startSubstr + document->path().parent_path().string() + midSubstr + testWorkDir;

  const auto interpolated = context.interpolate(toInterpolate);

  CHECK(interpolated == expected);
}
} // namespace TrenchBroom::View
