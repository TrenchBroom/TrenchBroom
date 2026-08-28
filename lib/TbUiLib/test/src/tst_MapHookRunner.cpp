/*
 Copyright (C) 2026

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

#include <QTest>
#include <QtTest/QSignalSpy>

#include "base/PreferenceManager.h"
#include "prefs/Preferences.h"
#include "ui/AppControllerFixture.h"
#include "ui/CatchConfig.h"
#include "ui/MapDocument.h"
#include "ui/MapDocumentFixture.h"
#include "ui/MapHookRunner.h"

#include <catch2/catch_test_macros.hpp>

#include <string>
#include <vector>

namespace tb::ui
{
namespace
{

/**
 * Records every command that would have been run instead of actually spawning a shell
 * process, so the tests are fast, deterministic and don't depend on a shell being
 * available.
 */
class TestMapHookRunner : public MapHookRunner
{
public:
  std::vector<std::string> commands;

  using MapHookRunner::MapHookRunner;

protected:
  void runCommand(const std::string& command) override { commands.push_back(command); }
};

/**
 * Ensures the (process-global) preferences touched by these tests are back to their
 * defaults once a SECTION finishes, whether it passed or failed, so tests don't leak
 * state into each other via the shared PreferenceManager singleton.
 */
struct ResetHookPreferences
{
  ~ResetHookPreferences()
  {
    auto& prefs = PreferenceManager::instance();
    prefs.set(Preferences::MapHookRunOnSave, false);
    prefs.set(Preferences::MapHookRunOnChange, false);
    prefs.set(Preferences::MapHookDebounceMs, 250);
    prefs.set(Preferences::MapHookCommand, std::string{});
  }
};

} // namespace

TEST_CASE("MapHookRunner")
{
  auto appControllerFixture = AppControllerFixture{};

  auto documentFixture = MapDocumentFixture{};
  auto& document = documentFixture.create();

  auto resetPreferences = ResetHookPreferences{};
  auto& prefs = PreferenceManager::instance();

  SECTION("does nothing when both triggers are disabled (the default)")
  {
    auto runner = TestMapHookRunner{document};
    auto spy = QSignalSpy{&runner, &MapHookRunner::commandExecuted};

    document.documentWasSavedNotifier();
    document.documentDidChangeNotifier();

    CHECK_FALSE(spy.wait(100));
    CHECK(runner.commands.empty());
    CHECK_FALSE(runner.debouncePending());
  }

  SECTION("runs immediately (no debounce) when the document is saved")
  {
    prefs.set(Preferences::MapHookRunOnSave, true);
    prefs.set(Preferences::MapHookCommand, std::string{"echo ${MAP_BASE_NAME}"});

    auto runner = TestMapHookRunner{document};
    auto spy = QSignalSpy{&runner, &MapHookRunner::commandExecuted};

    document.documentWasSavedNotifier();

    // No event loop spin needed: notifiers fire synchronously, and "on save" is not
    // debounced.
    REQUIRE(runner.commands.size() == 1);
    CHECK(spy.count() == 1);

    // The command template was interpolated (the placeholder is gone).
    CHECK(runner.commands.front().find("${MAP_BASE_NAME}") == std::string::npos);
  }

  SECTION("does not run on save when MapHookRunOnSave is disabled")
  {
    prefs.set(Preferences::MapHookRunOnSave, false);
    prefs.set(Preferences::MapHookCommand, std::string{"echo hi"});

    auto runner = TestMapHookRunner{document};
    document.documentWasSavedNotifier();

    CHECK(runner.commands.empty());
  }

  SECTION("an empty command never runs, even when enabled")
  {
    prefs.set(Preferences::MapHookRunOnSave, true);
    prefs.set(Preferences::MapHookCommand, std::string{});

    auto runner = TestMapHookRunner{document};
    document.documentWasSavedNotifier();

    CHECK(runner.commands.empty());
  }

  SECTION("rapid-fire changes are coalesced into a single debounced run")
  {
    prefs.set(Preferences::MapHookRunOnChange, true);
    prefs.set(Preferences::MapHookDebounceMs, 60);
    prefs.set(Preferences::MapHookCommand, std::string{"echo hi"});

    auto runner = TestMapHookRunner{document};
    auto spy = QSignalSpy{&runner, &MapHookRunner::commandExecuted};

    // Simulate e.g. every step of a vertex drag: many changes in quick succession,
    // without letting the event loop (and thus the debounce timer) run in between.
    for (auto i = 0; i < 25; ++i)
    {
      document.documentDidChangeNotifier();
    }

    CHECK(runner.debouncePending());
    CHECK(runner.commands.empty());

    REQUIRE(spy.wait(1000));

    CHECK(runner.commands.size() == 1);
    CHECK_FALSE(runner.debouncePending());
  }

  SECTION("restarting the debounce timer keeps pushing the run out, it does not queue "
          "one run per change")
  {
    // This is the regression case for the "vertex drag spawns hundreds of processes" bug:
    // a correct implementation restarts a single timer on every change. A buggy
    // implementation that instead starts a new independent timer per change (e.g. calling
    // the static QTimer::singleShot each time) would run the command once for every
    // change whose individual deadline has passed, instead of once after things settle.
    prefs.set(Preferences::MapHookRunOnChange, true);
    prefs.set(Preferences::MapHookDebounceMs, 100);
    prefs.set(Preferences::MapHookCommand, std::string{"echo hi"});

    auto runner = TestMapHookRunner{document};
    auto spy = QSignalSpy{&runner, &MapHookRunner::commandExecuted};

    document.documentDidChangeNotifier(); // t=0, (buggy: independent) deadline=100
    QTest::qWait(60); // t=60

    document.documentDidChangeNotifier(); // restart: deadline=160 (buggy: also schedules
                                           // an independent deadline=220)
    QTest::qWait(70); // t=130: past the first call's 100ms deadline, but before 160

    // A correct restart-based debounce must not have run yet: the second change pushed
    // the deadline to 160. A buggy "independent timer per call" implementation would
    // already have fired once by now (at t=100).
    CHECK(runner.commands.empty());

    REQUIRE(spy.wait(1000));

    // Exactly one run in total, not two.
    CHECK(runner.commands.size() == 1);
    CHECK(spy.count() == 1);
  }

  SECTION("disabling on-change while a run is pending cancels it")
  {
    prefs.set(Preferences::MapHookRunOnChange, true);
    prefs.set(Preferences::MapHookDebounceMs, 50);
    prefs.set(Preferences::MapHookCommand, std::string{"echo hi"});

    auto runner = TestMapHookRunner{document};

    document.documentDidChangeNotifier();
    CHECK(runner.debouncePending());

    prefs.set(Preferences::MapHookRunOnChange, false);

    QTest::qWait(150);

    CHECK(runner.commands.empty());
  }
}

} // namespace tb::ui
