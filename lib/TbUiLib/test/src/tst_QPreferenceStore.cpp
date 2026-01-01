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

#include <QCoreApplication>
#include <QJsonObject>
#include <QLockFile>

#include "Observer.h"
#include "fs/TestEnvironment.h"
#include "ui/CatchConfig.h"
#include "ui/QPathUtils.h"
#include "ui/QPreferenceStore.h"

#include <chrono> // IWYU pragma: keep
#include <condition_variable>
#include <filesystem>
#include <mutex>
#include <string>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

namespace tb::ui
{

using namespace std::chrono_literals;
using namespace std::string_literals;

TEST_CASE("QPreferenceStore")
{
  auto checkAndWaitUntil = [mutex = std::mutex{},
                            conditionVariable = std::condition_variable{}](
                             const auto endTime, const auto& condition) mutable {
    while (std::chrono::steady_clock::now() < endTime)
    {
      qApp->processEvents();

      auto lock = std::unique_lock{mutex};
      if (conditionVariable.wait_for(lock, 10ms, condition))
      {
        return true;
      }
    }

    return false;
  };

  auto env = fs::TestEnvironment{};

  const auto preferenceFilename = "prefs.json";
  const auto preferenceFilePath = env.dir() / preferenceFilename;

  SECTION("loading and saving preferences")
  {
    env.createFile(preferenceFilename, R"({
  "some/path": "asdf"
}
)");

    auto preferenceStore = QPreferenceStore{pathAsQString(preferenceFilePath), 50ms};

    SECTION("loading a transient preference returns false")
    {
      auto value = std::string{};
      CHECK(!preferenceStore.load("some/other/path", value));
      CHECK(value == "");
    }

    SECTION("loading a persistent preference returns the persistent value")
    {
      auto value = std::string{};
      CHECK(preferenceStore.load("some/path", value));
      CHECK(value == "asdf");
    }

    SECTION("saving a preference updates its value")
    {
      const auto path = GENERATE("some/path", "some/other/path");

      preferenceStore.save(path, "fdsa"s);

      auto value = std::string{};
      CHECK(preferenceStore.load(path, value));
      CHECK(value == "fdsa");

      SECTION("saving the preference again updates its value")
      {
        preferenceStore.save(path, "qwer"s);
        CHECK(preferenceStore.load(path, value));
        CHECK(value == "qwer");
      }
    }
  }

  SECTION("missing preference file")
  {
    auto preferenceStore = QPreferenceStore{pathAsQString(preferenceFilePath), 50ms};

    auto value = std::string{};
    CHECK(!preferenceStore.load("some/path", value));
    CHECK(value == "");
  }

  SECTION("loads preference file")
  {
    env.createFile(preferenceFilename, R"({
  "some/path": "asdf"
}
)");

    auto preferenceStore = QPreferenceStore{pathAsQString(preferenceFilePath), 50ms};

    auto value = std::string{};
    CHECK(preferenceStore.load("some/path", value));
    CHECK(value == "asdf");
  }

  SECTION("preferences aren't saved immediately")
  {
    auto preferenceStore = QPreferenceStore{pathAsQString(preferenceFilePath), 500ms};

    preferenceStore.save("some/path", "asdf"s);
    CHECK(!env.fileExists(preferenceFilename));
  }

  // The following tests are unreliable on Windows
#if !defined(_WIN32) && !defined(_WIN64)
  SECTION("preferences are saved after a delay")
  {
    auto preferenceStore = QPreferenceStore{pathAsQString(preferenceFilePath), 100ms};

    preferenceStore.save("some/path", "asdf"s);
    const auto startTime = std::chrono::steady_clock::now();

    REQUIRE(!env.fileExists(preferenceFilename));

    REQUIRE(checkAndWaitUntil(
      startTime + 500ms, [&]() { return env.fileExists(preferenceFilename); }));
    CHECK(env.loadFile(preferenceFilename) == R"({
    "some/path": "asdf"
}
)");
  }

  SECTION("preferences save delay extends when new values are set")
  {
    auto preferenceStore = QPreferenceStore{pathAsQString(preferenceFilePath), 500ms};

    preferenceStore.save("some/path", "asdf"s);
    const auto startTime = std::chrono::steady_clock::now();

    REQUIRE(!checkAndWaitUntil(
      startTime + 300ms, [&]() { return env.fileExists(preferenceFilename); }));

    preferenceStore.save("some/path", "fdsa"s);

    REQUIRE(!checkAndWaitUntil(
      startTime + 600ms, [&]() { return env.fileExists(preferenceFilename); }));

    REQUIRE(checkAndWaitUntil(
      startTime + 1000ms, [&]() { return env.fileExists(preferenceFilename); }));

    CHECK(env.loadFile(preferenceFilename) == R"({
    "some/path": "fdsa"
}
)");
  }

  SECTION("preferences reload when the file changes on disk")
  {
    env.createFile(preferenceFilename, R"({
  "some/path": "asdf"
}
)");

    auto preferenceStore = QPreferenceStore{pathAsQString(preferenceFilePath), 500ms};
    auto preferencesWereReloaded = Observer<const std::vector<std::filesystem::path>>{
      preferenceStore.preferencesWereReloadedNotifier};

    auto value = std::string{};
    REQUIRE(preferenceStore.load("some/path", value));
    REQUIRE(value == "asdf");

    env.createFile(preferenceFilename, R"({
  "some/path": "fdsa"
}
)");

    CHECK(checkAndWaitUntil(std::chrono::steady_clock::now() + 1000ms, [&]() {
      return !preferencesWereReloaded.collected.empty();
    }));

    CHECK(
      preferencesWereReloaded.collected
      == std::set{std::vector{std::filesystem::path{"some/path"}}});

    CHECK(preferenceStore.load("some/path", value));
    CHECK(value == "fdsa");
  }
#endif
}

TEST_CASE("Preference lock file")
{
// ensure that a lock file can be created in a directory with non-ASCII characters
#ifdef _WIN32
  const auto lockFilePath =
    std::filesystem::path{LR"(fixture\test\Кристиян\ぁ\preferences-v2.json.lck)"};
#else
  const auto lockFilePath =
    std::filesystem::path{R"(fixture/test/Кристиян/ぁ/preferences-v2.json.lck)"};
#endif
  std::filesystem::create_directories(lockFilePath.parent_path());

  auto lockFile = QLockFile{pathAsQPath(lockFilePath)};
  CHECK(lockFile.lock());
}

} // namespace tb::ui
