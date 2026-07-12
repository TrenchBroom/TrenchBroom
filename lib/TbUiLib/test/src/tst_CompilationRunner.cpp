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

#include <QCoreApplication>
#include <QObject>
#include <QTextEdit>
#include <QtSystemDetection>
#include <QtTest/QSignalSpy>

#include "CmdTool.h"
#include "el/VariableStore.h"
#include "fs/TestEnvironment.h"
#include "mdl/CompilationProfile.h"
#include "mdl/CompilationTask.h"
#include "mdl/EntityNode.h"
#include "mdl/GameEngineProfile.h"
#include "mdl/Map.h"
#include "mdl/MapFixture.h"
#include "mdl/Map_Nodes.h"
#include "ui/CatchConfig.h"
#include "ui/CompilationContext.h"
#include "ui/CompilationRunner.h"
#include "ui/CompilationVariables.h"
#include "ui/TextOutputAdapter.h"

#include "kd/k.h"
#include "kd/string_utils.h"

#include <chrono>
#include <condition_variable>
#include <filesystem>
#include <mutex>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

namespace tb::ui
{
using namespace std::chrono_literals;
using namespace std::string_literals;

using namespace Catch::Matchers;

namespace
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
      auto lock = std::unique_lock<std::mutex>{m_mutex};
      started = true;
      m_condition.notify_all();
    });
    QObject::connect(&m_runner, &CompilationTaskRunner::error, [&]() {
      auto lock = std::unique_lock<std::mutex>{m_mutex};
      errored = true;
      m_condition.notify_all();
    });
    QObject::connect(&m_runner, &CompilationTaskRunner::end, [&]() {
      auto lock = std::unique_lock<std::mutex>{m_mutex};
      ended = true;
      m_condition.notify_all();
    });
  }

  bool executeAndWait(const std::chrono::milliseconds timeout)
  {
    m_runner.execute();

    const auto endTime = std::chrono::system_clock::now() + timeout;
    while (std::chrono::system_clock::now() < endTime)
    {
      qApp->processEvents();

      auto lock = std::unique_lock<std::mutex>{m_mutex};
      if (m_condition.wait_for(lock, 50ms, [&]() { return errored || ended; }))
      {
        return true;
      }
    }

    return false;
  }
};

} // namespace

TEST_CASE("CompilationRunToolTaskRunner")
{
  auto fixture = mdl::MapFixture{};
  auto& map = fixture.create();

  SECTION("runMissingTool")
  {
    auto variables = el::NullVariableStore{};
    auto output = QTextEdit{};
    auto outputAdapter = TextOutputAdapter{&output};

    auto context = CompilationContext{map, variables, outputAdapter, false};

    auto task = mdl::CompilationRunTool{K(enabled), "", "", false};
    auto runner = CompilationRunToolTaskRunner{context, task};

    auto exec = ExecuteTask{runner};
    REQUIRE(exec.executeAndWait(5000ms));

    CHECK(exec.started);
    CHECK(exec.errored);
    CHECK_FALSE(exec.ended);
  }

  SECTION("system specific path separators")
  {
    const auto pathSeparator = std::string{std::filesystem::path::preferred_separator};
    const auto incompatiblePathSeparator = pathSeparator == "/" ? "\\"s : "/"s;

    const auto systemPath = std::string{CMD_TOOL_PATH};
    const auto incompatiblePath =
      kdl::str_replace_every(systemPath, pathSeparator, incompatiblePathSeparator);

    const auto toolPath = GENERATE_REF(systemPath, incompatiblePath);

    CAPTURE(toolPath);

    auto variables = el::NullVariableStore{};
    auto output = QTextEdit{};
    auto outputAdapter = TextOutputAdapter{&output};

    auto context = CompilationContext{map, variables, outputAdapter, false};

    const auto treatNonZeroResultCodeAsError = GENERATE(true, false);
    auto task = mdl::CompilationRunTool{
      K(enabled), toolPath, "--exit 0", treatNonZeroResultCodeAsError};
    auto runner = CompilationRunToolTaskRunner{context, task};

    auto exec = ExecuteTask{runner};
    REQUIRE(exec.executeAndWait(5000ms));

    CHECK(exec.started);
    CHECK_FALSE(exec.errored);
    CHECK(exec.ended);
  }

  SECTION("toolReturnsZeroExitCode")
  {
    auto variables = el::NullVariableStore{};
    auto output = QTextEdit{};
    auto outputAdapter = TextOutputAdapter{&output};

    auto context = CompilationContext{map, variables, outputAdapter, false};

    const auto treatNonZeroResultCodeAsError = GENERATE(true, false);
    auto task = mdl::CompilationRunTool{
      true, CMD_TOOL_PATH, "--exit 0", treatNonZeroResultCodeAsError};
    auto runner = CompilationRunToolTaskRunner{context, task};

    auto exec = ExecuteTask{runner};
    REQUIRE(exec.executeAndWait(5000ms));

    CHECK(exec.started);
    CHECK_FALSE(exec.errored);
    CHECK(exec.ended);
  }

  SECTION("toolReturnsNonZeroExitCode")
  {
    auto variables = el::NullVariableStore{};
    auto output = QTextEdit{};
    auto outputAdapter = TextOutputAdapter{&output};

    auto context = CompilationContext{map, variables, outputAdapter, false};

    const auto treatNonZeroResultCodeAsError = GENERATE(true, false);
    auto task = mdl::CompilationRunTool{
      true, CMD_TOOL_PATH, "--exit 1", treatNonZeroResultCodeAsError};
    auto runner = CompilationRunToolTaskRunner{context, task};

    auto exec = ExecuteTask{runner};
    REQUIRE(exec.executeAndWait(5000ms));

    CHECK(exec.started);
    CHECK(exec.errored == treatNonZeroResultCodeAsError);
    CHECK(exec.ended == !treatNonZeroResultCodeAsError);
  }

  SECTION("argumentPassing")
  {
    auto variables = el::NullVariableStore{};
    auto output = QTextEdit{};
    auto outputAdapter = TextOutputAdapter{&output};

    auto context = CompilationContext{map, variables, outputAdapter, false};

    auto task = mdl::CompilationRunTool{
      true, CMD_TOOL_PATH, R"(--printArgs 1 2 str "escaped str")", false};
    auto runner = CompilationRunToolTaskRunner{context, task};

    auto exec = ExecuteTask{runner};
    REQUIRE(exec.executeAndWait(5000ms));

    REQUIRE(exec.started);
    REQUIRE_FALSE(exec.errored);
    REQUIRE(exec.ended);

    CHECK_THAT(output.toPlainText().toStdString(), ContainsSubstring(R"(1
2
str
escaped str)"));
  }

#if !defined(Q_OS_WIN)
  // the test is unreliable on Windows
  SECTION("toolAborts")
  {
    auto variables = el::NullVariableStore{};
    auto output = QTextEdit{};
    auto outputAdapter = TextOutputAdapter{&output};

    auto context = CompilationContext{map, variables, outputAdapter, false};

    const auto treatNonZeroResultCodeAsError = GENERATE(true, false);
    auto task = mdl::CompilationRunTool{
      true, CMD_TOOL_PATH, "--abort", treatNonZeroResultCodeAsError};
    auto runner = CompilationRunToolTaskRunner{context, task};

    auto exec = ExecuteTask{runner};
    REQUIRE(exec.executeAndWait(5000ms));

    CHECK(exec.started);
    CHECK(exec.errored);
    CHECK_FALSE(exec.ended);
  }
#endif

#if !defined(Q_OS_MACOS) || defined(NDEBUG)
  // the test is unreliable on macOS in debug mode
  SECTION("toolCrashes")
  {
    auto variables = el::NullVariableStore{};
    auto output = QTextEdit{};
    auto outputAdapter = TextOutputAdapter{&output};

    auto context = CompilationContext{map, variables, outputAdapter, false};

    const auto treatNonZeroResultCodeAsError = GENERATE(true, false);
    auto task = mdl::CompilationRunTool{
      true, CMD_TOOL_PATH, "--crash", treatNonZeroResultCodeAsError};
    auto runner = CompilationRunToolTaskRunner{context, task};

    auto exec = ExecuteTask{runner};
    REQUIRE(exec.executeAndWait(5000ms));

    CHECK(exec.started);
#if defined(Q_OS_WIN)
    // QProcess does not report a crash on SIGSEGV on Windows
    CHECK(exec.errored == treatNonZeroResultCodeAsError);
    CHECK(exec.ended == !treatNonZeroResultCodeAsError);
#else
    CHECK(exec.errored);
    CHECK_FALSE(exec.ended);
#endif
  }
#endif
}

TEST_CASE("CompilationLaunchEngineTaskRunner")
{
  auto fixtureConfig = mdl::MapFixtureConfig{};
  fixtureConfig.gameInfo.gameEngineConfig.profiles = {
    mdl::GameEngineProfile{
      .id = "quakespasm-id",
      .name = "Quakespasm",
      .path = CMD_TOOL_PATH,
      .parameterSpec = "--exit 0"},
    mdl::GameEngineProfile{
      .id = "missing-engine-id",
      .name = "Missing Engine",
      .path = "/does/not/exist",
      .parameterSpec = ""},
  };

  auto fixture = mdl::MapFixture{};
  auto& map = fixture.create(fixtureConfig);

  SECTION("launchEngine")
  {
    auto variables = el::NullVariableStore{};
    auto output = QTextEdit{};
    auto outputAdapter = TextOutputAdapter{&output};

    auto context = CompilationContext{map, variables, outputAdapter, false};

    auto task = mdl::CompilationLaunchEngine{
      K(enabled), "quakespasm-id", !K(treatLaunchFailureAsError)};
    auto runner = CompilationLaunchEngineTaskRunner{context, task};

    auto exec = ExecuteTask{runner};
    REQUIRE(exec.executeAndWait(5000ms));

    CHECK(exec.started);
    CHECK_FALSE(exec.errored);
    CHECK(exec.ended);
    CHECK_THAT(
      output.toPlainText().toStdString(),
      ContainsSubstring("#### Launching engine profile 'Quakespasm' at '"));
  }

  SECTION("invalidEngineProfile")
  {
    auto variables = el::NullVariableStore{};
    auto output = QTextEdit{};
    auto outputAdapter = TextOutputAdapter{&output};

    auto context = CompilationContext{map, variables, outputAdapter, false};

    const auto engineProfileId = GENERATE(""s, "deleted-profile-id"s);
    const auto treatLaunchFailureAsError = GENERATE(true, false);
    CAPTURE(engineProfileId, treatLaunchFailureAsError);
    auto task = mdl::CompilationLaunchEngine{
      K(enabled), engineProfileId, treatLaunchFailureAsError};
    auto runner = CompilationLaunchEngineTaskRunner{context, task};

    auto exec = ExecuteTask{runner};
    REQUIRE(exec.executeAndWait(5000ms));

    CHECK(exec.started);
    CHECK(exec.errored == treatLaunchFailureAsError);
    CHECK(exec.ended == !treatLaunchFailureAsError);
    CHECK_THAT(
      output.toPlainText().toStdString(), ContainsSubstring("#### Launch failed: "));
  }

  SECTION("launchFailure")
  {
    auto variables = el::NullVariableStore{};
    auto output = QTextEdit{};
    auto outputAdapter = TextOutputAdapter{&output};

    auto context = CompilationContext{map, variables, outputAdapter, false};

    const auto treatLaunchFailureAsError = GENERATE(true, false);
    CAPTURE(treatLaunchFailureAsError);
    auto task = mdl::CompilationLaunchEngine{
      K(enabled), "missing-engine-id", treatLaunchFailureAsError};
    auto runner = CompilationLaunchEngineTaskRunner{context, task};

    auto exec = ExecuteTask{runner};
    REQUIRE(exec.executeAndWait(5000ms));

    CHECK(exec.started);
    CHECK(exec.errored == treatLaunchFailureAsError);
    CHECK(exec.ended == !treatLaunchFailureAsError);
    CHECK_THAT(
      output.toPlainText().toStdString(), ContainsSubstring("#### Launch failed: "));
  }

  SECTION("testModeDoesNotLaunch")
  {
    auto variables = el::NullVariableStore{};
    auto output = QTextEdit{};
    auto outputAdapter = TextOutputAdapter{&output};

    auto context = CompilationContext{map, variables, outputAdapter, true};

    auto task = mdl::CompilationLaunchEngine{
      K(enabled), "missing-engine-id", K(treatLaunchFailureAsError)};
    auto runner = CompilationLaunchEngineTaskRunner{context, task};

    auto exec = ExecuteTask{runner};
    REQUIRE(exec.executeAndWait(5000ms));

    CHECK(exec.started);
    CHECK_FALSE(exec.errored);
    CHECK(exec.ended);
    CHECK_THAT(
      output.toPlainText().toStdString(),
      ContainsSubstring("#### Launching engine profile 'Missing Engine' at '"));
  }
}

TEST_CASE("CompilationExportMapTaskRunner")
{
  auto fixture = mdl::MapFixture{};
  auto& map = fixture.create();

  auto testEnvironment = fs::TestEnvironment{};

  const auto testWorkDir = testEnvironment.dir().string();
  auto variables = CompilationVariables{map, testWorkDir};
  auto output = QTextEdit{};
  auto outputAdapter = TextOutputAdapter{&output};

  auto context = CompilationContext{map, variables, outputAdapter, false};

  SECTION("exportMap")
  {
    const auto [exportSpec, expectedFilePath] =
      GENERATE(table<std::string, std::filesystem::path>({
        {"${WORK_DIR_PATH}/exported.map", "exported.map"},
        {"${WORK_DIR_PATH}\\exported.map", "exported.map"},
        {"exported.map", "exported.map"},
        {"some/nested/exported.map", "some/nested/exported.map"},
      }));

    CAPTURE(exportSpec);

    auto node = new mdl::EntityNode{mdl::Entity{}};
    addNodes(map, {{parentForNodes(map), {node}}});

    auto task = mdl::CompilationExportMap{K(enabled), !K(stripTbProperties), exportSpec};

    auto runner = CompilationExportMapTaskRunner{context, task};
    REQUIRE_NOTHROW(runner.execute());

    CHECK(testEnvironment.fileExists(expectedFilePath));
  }

  SECTION("variable interpolation error")
  {
    auto node = new mdl::EntityNode{mdl::Entity{}};
    addNodes(map, {{parentForNodes(map), {node}}});

    auto task = mdl::CompilationExportMap{
      K(enabled), !K(stripTbProperties), "${WORK_DIR_PATH/exported.map"};

    auto runner = CompilationExportMapTaskRunner{context, task};
    REQUIRE_NOTHROW(runner.execute());

    CHECK(!testEnvironment.fileExists("exported.map"));
  }
}

TEST_CASE("CompilationCopyFilesTaskRunner")
{
  auto fixture = mdl::MapFixture{};
  auto& map = fixture.create();

  auto testEnvironment = fs::TestEnvironment{};

  const auto testWorkDir = testEnvironment.dir().string();
  auto variables = CompilationVariables{map, testWorkDir};
  auto output = QTextEdit{};
  auto outputAdapter = TextOutputAdapter{&output};

  auto context = CompilationContext{map, variables, outputAdapter, false};

  SECTION("createTargetDirectories")
  {
    const auto sourcePath = "my_map.map";
    testEnvironment.createFile(sourcePath, "{}");

    const auto [targetSpec, expectedTargetPath] =
      GENERATE(table<std::string, std::filesystem::path>({
        {"${WORK_DIR_PATH}/some/other/path", "some/other/path"},
        {"${WORK_DIR_PATH}\\some\\other\\path", "some/other/path"},
        {"some/other/path", "some/other/path"},
        {"some\\other\\path", "some/other/path"},
      }));

    CAPTURE(targetSpec);

    auto task = mdl::CompilationCopyFiles{true, sourcePath, targetSpec};
    auto runner = CompilationCopyFilesTaskRunner{context, task};

    REQUIRE_NOTHROW(runner.execute());

    CHECK(testEnvironment.directoryExists(expectedTargetPath));
    CHECK(testEnvironment.loadFile(expectedTargetPath / sourcePath) == "{}");
  }

  SECTION("variable interpolation errors")
  {
    const auto sourcePath =
      GENERATE("${WORK_DIR_PATH}/source.map", "${WORK_DIR_PATH/source.map}");
    const auto targetPath =
      GENERATE("${WORK_DIR_PATH}/target.map", "${WORK_DIR_PATH/target.map}");

    auto task = mdl::CompilationCopyFiles{K(enabled), sourcePath, targetPath};
    auto runner = CompilationCopyFilesTaskRunner{context, task};

    REQUIRE_NOTHROW(runner.execute());
  }
}

TEST_CASE("CompilationRenameFileTaskRunner")
{
  auto fixture = mdl::MapFixture{};
  auto& map = fixture.create();

  auto testEnvironment = fs::TestEnvironment{};

  const auto testWorkDir = testEnvironment.dir().string();
  auto variables = CompilationVariables{map, testWorkDir};
  auto output = QTextEdit{};
  auto outputAdapter = TextOutputAdapter{&output};

  auto context = CompilationContext{map, variables, outputAdapter, false};

  SECTION("renameFile")
  {
    const auto overwrite = GENERATE(true, false);

    const auto sourcePath = "my_map.map";
    testEnvironment.createFile(sourcePath, "{}");

    const auto [targetSpec, expectedTargetPath] =
      GENERATE(table<std::string, std::filesystem::path>({
        {"${WORK_DIR_PATH}/some/other/path/your_map.map", "some/other/path/your_map.map"},
        {"${WORK_DIR_PATH}\\some\\other\\path\\your_map.map",
         "some/other/path/your_map.map"},
        {"some/other/path/your_map.map", "some/other/path/your_map.map"},
        {"some\\other\\path\\your_map.map", "some/other/path/your_map.map"},
      }));

    CAPTURE(targetSpec);

    if (overwrite)
    {
      testEnvironment.createDirectory(expectedTargetPath.parent_path());
      testEnvironment.createFile(expectedTargetPath, "{...}");
      REQUIRE(testEnvironment.loadFile(expectedTargetPath) == "{...}");
    }

    auto task = mdl::CompilationRenameFile{true, sourcePath, targetSpec};
    auto runner = CompilationRenameFileTaskRunner{context, task};

    REQUIRE_NOTHROW(runner.execute());

    CHECK(testEnvironment.loadFile(expectedTargetPath) == "{}");
  }

  SECTION("variable interpolation errors")
  {
    const auto sourcePath =
      GENERATE("${WORK_DIR_PATH}/source.map", "${WORK_DIR_PATH/source.map}");
    const auto targetPath =
      GENERATE("${WORK_DIR_PATH}/target.map", "${WORK_DIR_PATH/target.map}");

    auto task = mdl::CompilationRenameFile{K(enabled), sourcePath, targetPath};
    auto runner = CompilationRenameFileTaskRunner{context, task};

    REQUIRE_NOTHROW(runner.execute());
  }
}

TEST_CASE("CompilationDeleteFilesTaskRunner")
{
  auto fixture = mdl::MapFixture{};
  auto& map = fixture.create();

  auto testEnvironment = fs::TestEnvironment{};

  const auto testWorkDir = testEnvironment.dir().string();
  auto variables = CompilationVariables{map, testWorkDir};
  auto output = QTextEdit{};
  auto outputAdapter = TextOutputAdapter{&output};

  auto context = CompilationContext{map, variables, outputAdapter, false};

  SECTION("deleteTargetPattern")
  {
    const auto file1 = "file1.lit";
    const auto file2 = "file2.lit";
    const auto file3 = "file3.map";
    const auto dir = "somedir.lit";

    testEnvironment.createFile(file1, "");
    testEnvironment.createFile(file2, "");
    testEnvironment.createFile(file3, "");
    testEnvironment.createDirectory(dir);

    const auto targetSpec = GENERATE(
      std::filesystem::path{"${WORK_DIR_PATH}/*.lit"}, std::filesystem::path{"*.lit"});

    CAPTURE(targetSpec);

    auto task = mdl::CompilationDeleteFiles{K(enabled), targetSpec.string()};
    auto runner = CompilationDeleteFilesTaskRunner{context, task};

    REQUIRE_NOTHROW(runner.execute());

    CHECK(!testEnvironment.fileExists(file1));
    CHECK(!testEnvironment.fileExists(file2));
    CHECK(testEnvironment.fileExists(file3));
    CHECK(testEnvironment.directoryExists(dir));
  }

  SECTION("variable interpolation error")
  {
    auto task = mdl::CompilationDeleteFiles{K(enabled), "${WORK_DIR_PATH/exported.map"};
    auto runner = CompilationDeleteFilesTaskRunner{context, task};

    REQUIRE_NOTHROW(runner.execute());
  }
}

TEST_CASE("CompilationRunner")
{
  auto fixtureConfig = mdl::MapFixtureConfig{};
  fixtureConfig.gameInfo.gameConfig.forceEmptyNewMap = false;
  fixtureConfig.gameInfo.gameConfig.fileFormats = std::vector<mdl::MapFormatConfig>{
    {"Valve", {}},
  };
  fixtureConfig.gameInfo.gameEngineConfig.profiles = {
    mdl::GameEngineProfile{
      .id = "quakespasm-id",
      .name = "Quakespasm",
      .path = CMD_TOOL_PATH,
      .parameterSpec = "--exit 0"},
  };

  auto fixture = mdl::MapFixture{};
  auto& map = fixture.load(
    "test/ui/CompilationRunner/valveFormatMapWithoutFormatTag.map", fixtureConfig);

  const auto testWorkDir = std::string{"/some/path"};
  auto variables = CompilationVariables{map, testWorkDir};
  auto output = QTextEdit{};
  auto outputAdapter = TextOutputAdapter{&output};

  auto context = CompilationContext{map, variables, outputAdapter, false};

  auto testEnvironment = fs::TestEnvironment{};

  SECTION("stopAfterFirstError")
  {
    const auto does_not_exist = "does_not_exist.map";
    const auto does_exist = "does_exist.map";
    const auto should_not_exist = "should_not_exist.map";

    testEnvironment.createFile(does_exist, "");

    auto compilationProfile = mdl::CompilationProfile{
      "name",
      testEnvironment.dir().string(),
      {
        mdl::CompilationCopyFiles{K(enabled), does_not_exist, "does_not_matter.map"},
        mdl::CompilationCopyFiles{K(enabled), does_exist, should_not_exist},
      }};

    auto runner = CompilationRunner{
      CompilationContext{map, variables, outputAdapter, false}, compilationProfile};

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

  SECTION("runLaunchEngineTask")
  {
    auto compilationProfile = mdl::CompilationProfile{
      "name",
      testEnvironment.dir().string(),
      {
        mdl::CompilationLaunchEngine{
          K(enabled), "quakespasm-id", !K(treatLaunchFailureAsError)},
      }};

    auto runner = CompilationRunner{
      CompilationContext{map, variables, outputAdapter, false}, compilationProfile};

    auto compilationStartedSpy = QSignalSpy{&runner, SIGNAL(compilationStarted())};
    auto compilationEndedSpy = QSignalSpy{&runner, SIGNAL(compilationEnded())};

    REQUIRE(compilationStartedSpy.isValid());
    REQUIRE(compilationEndedSpy.isValid());

    runner.execute();
    REQUIRE(!runner.running());
    REQUIRE(compilationStartedSpy.count() == 1);
    REQUIRE(compilationEndedSpy.count() == 1);

    CHECK_THAT(
      output.toPlainText().toStdString(),
      ContainsSubstring("#### Launching engine profile 'Quakespasm' at '"));
  }

  SECTION("disabledLaunchEngineTaskIsIgnored")
  {
    auto compilationProfile = mdl::CompilationProfile{
      "name",
      testEnvironment.dir().string(),
      {
        mdl::CompilationLaunchEngine{!K(enabled), "", K(treatLaunchFailureAsError)},
      }};

    auto runner = CompilationRunner{
      CompilationContext{map, variables, outputAdapter, false}, compilationProfile};

    auto compilationStartedSpy = QSignalSpy{&runner, SIGNAL(compilationStarted())};
    auto compilationEndedSpy = QSignalSpy{&runner, SIGNAL(compilationEnded())};

    REQUIRE(compilationStartedSpy.isValid());
    REQUIRE(compilationEndedSpy.isValid());

    runner.execute();
    REQUIRE(!runner.running());
    REQUIRE(compilationStartedSpy.count() == 0);
    REQUIRE(compilationEndedSpy.count() == 0);
    CHECK(output.toPlainText().isEmpty());
  }

  SECTION("interpolateToolsVariables")
  {
    using namespace std::string_literals;

    const auto startSubstr = "foo "s;
    const auto midSubstr = " bar "s;
    const auto toInterpolate =
      startSubstr + "${MAP_DIR_PATH}"s + midSubstr + "${WORK_DIR_PATH}"s;

    CHECK(
      context.interpolate(toInterpolate)
      == startSubstr + map.path().parent_path().string() + midSubstr + testWorkDir);
  }
}

} // namespace tb::ui
