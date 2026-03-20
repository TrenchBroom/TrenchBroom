/*
Copyright (C) 2026 Kristian Duske

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

#include <QApplication>
#include <QPushButton>
#include <QtTest/QSignalSpy>

#include "mdl/CompilationProfile.h"
#include "mdl/CompilationTask.h"
#include "mdl/Map.h"
#include "ui/AppControllerFixture.h"
#include "ui/CatchConfig.h"
#include "ui/CompilationDialog.h"
#include "ui/CompilationProfileManager.h"
#include "ui/MapDocument.h"
#include "ui/MapDocumentFixture.h"

#include <catch2/catch_test_macros.hpp>

namespace tb::ui
{

namespace
{

mdl::MapFixtureConfig fixtureConfigWithCompilationProfiles(
  std::vector<mdl::CompilationProfile> profiles)
{
  auto fixtureConfig = mdl::MapFixtureConfig{};
  fixtureConfig.gameInfo.compilationConfig.profiles = std::move(profiles);
  return fixtureConfig;
}

} // namespace

TEST_CASE("CompilationDialog")
{
  auto appControllerFixture = AppControllerFixture{};
  auto& appController = appControllerFixture.appController();


  SECTION("selectProfile")
  {
    auto profiles = std::vector<mdl::CompilationProfile>{
      mdl::CompilationProfile{"profile 1", "${MAP_DIR_PATH}", {}},
      mdl::CompilationProfile{"profile 2", "${MAP_DIR_PATH}", {}},
    };

    auto documentFixture = MapDocumentFixture{};
    auto& document =
      documentFixture.create(fixtureConfigWithCompilationProfiles(profiles));

    auto dialog = CompilationDialog{appController, document};

    auto* profileManager = dialog.findChild<CompilationProfileManager*>();
    REQUIRE(profileManager != nullptr);

    SECTION("returns true and selects an existing profile")
    {
      const auto result = dialog.selectProfile(profiles[1]);

      CHECK(result);
      REQUIRE(profileManager->selectedProfile() != nullptr);
      CHECK(profileManager->selectedProfile()->name == "profile 2");
    }

    SECTION("returns false for an unknown profile")
    {
      const auto unknown = mdl::CompilationProfile{"unknown", "${MAP_DIR_PATH}", {}};

      const auto result = dialog.selectProfile(unknown);

      CHECK_FALSE(result);
      REQUIRE(profileManager->selectedProfile() != nullptr);
      CHECK(profileManager->selectedProfile()->name == "profile 1");
    }
  }

  SECTION("selectFirstProfile")
  {
    auto documentFixture = MapDocumentFixture{};
    auto& document = documentFixture.create(fixtureConfigWithCompilationProfiles({
      {"profile 1", "${MAP_DIR_PATH}", {}},
      {"profile 2", "${MAP_DIR_PATH}", {}},
      {"profile 3", "${MAP_DIR_PATH}", {}},
    }));

    auto dialog = CompilationDialog{appController, document};

    auto* profileManager = dialog.findChild<CompilationProfileManager*>();
    REQUIRE(profileManager != nullptr);

    const auto& profiles = document.map().gameInfo().compilationConfig.profiles;
    REQUIRE(dialog.selectProfile(profiles[2]));
    REQUIRE(profileManager->selectedProfile()->name == profiles[2].name);

    dialog.selectFirstProfile();

    REQUIRE(profileManager->selectedProfile()->name == profiles[0].name);
  }

  SECTION("runSelectedProfile")
  {
    SECTION("does nothing when no profile is selected")
    {
      auto documentFixture = MapDocumentFixture{};
      auto& document = documentFixture.create();

      auto dialog = CompilationDialog{appController, document};
      auto spy = QSignalSpy{&dialog, &CompilationDialog::compilationProfileStarted};

      dialog.runSelectedProfile();
      QApplication::processEvents();

      CHECK(spy.count() == 0);
    }

    SECTION("starts the selected profile")
    {
      auto documentFixture = MapDocumentFixture{};
      auto& document = documentFixture.create(fixtureConfigWithCompilationProfiles({
        mdl::CompilationProfile{
          "runnable profile",
          "${MAP_DIR_PATH}",
          {mdl::CompilationRunTool{true, "/does/not/exist", "", true}},
        },
      }));

      auto dialog = CompilationDialog{appController, document};
      auto spy = QSignalSpy{&dialog, &CompilationDialog::compilationProfileStarted};

      dialog.runSelectedProfile();
      QApplication::processEvents();

      CHECK(spy.count() == 1);
    }
  }
}

} // namespace tb::ui
