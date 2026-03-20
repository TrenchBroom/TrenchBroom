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
#include <QListWidget>
#include <QToolButton>
#include <QtTest/QSignalSpy>
#include <QtTest/QTest>

#include "mdl/CompilationConfig.h"
#include "mdl/CompilationProfile.h"
#include "ui/CatchConfig.h"
#include "ui/CompilationProfileListBox.h"
#include "ui/CompilationProfileManager.h"
#include "ui/MapDocumentFixture.h"

#include <memory>

#include <catch2/catch_test_macros.hpp>

namespace tb::ui
{

TEST_CASE("CompilationProfileManager")
{
  auto fixture = MapDocumentFixture{};
  auto& document = fixture.create();

  auto config = mdl::CompilationConfig{};

  SECTION("selectedProfile")
  {
    SECTION("returns null when no profile is selected")
    {
      auto manager = std::make_unique<CompilationProfileManager>(document, config);

      CHECK(manager->selectedProfile() == nullptr);
    }

    SECTION("returns the selected profile otherwise")
    {
      config.profiles.push_back(mdl::CompilationProfile{"test 1", "", {}});
      config.profiles.push_back(mdl::CompilationProfile{"test 2", "", {}});

      auto manager = std::make_unique<CompilationProfileManager>(document, config);

      auto* profileListBox = manager->findChild<CompilationProfileListBox*>();
      REQUIRE(profileListBox != nullptr);

      auto* listWidget =
        profileListBox->findChild<QListWidget*>("controlListBox_listWidget");
      REQUIRE(listWidget != nullptr);

      listWidget->setCurrentRow(1);
      QApplication::processEvents();

      REQUIRE(manager->selectedProfile() != nullptr);
      CHECK(manager->selectedProfile()->name == "test 2");
    }
  }

  SECTION("selectProfile")
  {
    SECTION("selects existing profile")
    {
      config.profiles.push_back(mdl::CompilationProfile{"test 1", "", {}});
      config.profiles.push_back(mdl::CompilationProfile{"test 2", "", {}});

      auto manager = std::make_unique<CompilationProfileManager>(document, config);

      REQUIRE(manager->selectedProfile() != nullptr);
      CHECK(manager->selectedProfile()->name == "test 1");

      const auto result = manager->selectProfile(config.profiles[1]);

      CHECK(result);
      REQUIRE(manager->selectedProfile() != nullptr);
      CHECK(manager->selectedProfile()->name == "test 2");
    }

    SECTION("returns false for unknown profile")
    {
      config.profiles.push_back(mdl::CompilationProfile{"test 1", "", {}});
      config.profiles.push_back(mdl::CompilationProfile{"test 2", "", {}});

      auto manager = std::make_unique<CompilationProfileManager>(document, config);

      REQUIRE(manager->selectedProfile() != nullptr);
      CHECK(manager->selectedProfile()->name == "test 1");

      const auto unknownProfile = mdl::CompilationProfile{"unknown", "", {}};
      const auto result = manager->selectProfile(unknownProfile);

      CHECK_FALSE(result);
      REQUIRE(manager->selectedProfile() != nullptr);
      CHECK(manager->selectedProfile()->name == "test 1");
    }
  }

  SECTION("selectFirstProfile")
  {
    SECTION("selects first profile when another is selected")
    {
      config.profiles.push_back(mdl::CompilationProfile{"test 1", "", {}});
      config.profiles.push_back(mdl::CompilationProfile{"test 2", "", {}});
      config.profiles.push_back(mdl::CompilationProfile{"test 3", "", {}});

      auto manager = std::make_unique<CompilationProfileManager>(document, config);

      manager->selectProfile(config.profiles[2]);
      REQUIRE(manager->selectedProfile() != nullptr);
      CHECK(manager->selectedProfile()->name == "test 3");

      manager->selectFirstProfile();

      REQUIRE(manager->selectedProfile() != nullptr);
      CHECK(manager->selectedProfile()->name == "test 1");
    }

    SECTION("does nothing when there are no profiles")
    {
      auto manager = std::make_unique<CompilationProfileManager>(document, config);

      manager->selectFirstProfile();

      CHECK(manager->selectedProfile() == nullptr);
    }
  }

  SECTION("remove button disabled when no profile is selected")
  {
    config.profiles.push_back(mdl::CompilationProfile{"test 1", "", {}});

    auto manager = std::make_unique<CompilationProfileManager>(document, config);

    manager->resize(700, 400);
    manager->show();
    QApplication::processEvents();

    auto* profileListBox = manager->findChild<CompilationProfileListBox*>();
    REQUIRE(profileListBox != nullptr);

    auto* listWidget =
      profileListBox->findChild<QListWidget*>("controlListBox_listWidget");
    REQUIRE(listWidget != nullptr);

    auto* removeButton =
      manager->findChild<QToolButton*>("CompilationProfileManager_RemoveProfileButton");
    REQUIRE(removeButton != nullptr);
    REQUIRE(removeButton->isEnabled());

    listWidget->setCurrentRow(-1);
    QApplication::processEvents();

    CHECK(manager->selectedProfile() == nullptr);
    CHECK_FALSE(removeButton->isEnabled());
  }

  SECTION("select profile by clicking profile list item")
  {
    config.profiles.push_back(mdl::CompilationProfile{"test 1", "", {}});
    config.profiles.push_back(mdl::CompilationProfile{"test 2", "", {}});

    auto manager = std::make_unique<CompilationProfileManager>(document, config);

    manager->resize(700, 400);
    manager->show();
    QApplication::processEvents();

    auto* profileListBox = manager->findChild<CompilationProfileListBox*>();
    REQUIRE(profileListBox != nullptr);

    auto* listWidget =
      profileListBox->findChild<QListWidget*>("controlListBox_listWidget");
    REQUIRE(listWidget != nullptr);
    REQUIRE(listWidget->count() == 2);

    auto spy = QSignalSpy{manager.get(), SIGNAL(selectedProfileChanged())};

    auto* secondItem = listWidget->item(1);
    REQUIRE(secondItem != nullptr);

    listWidget->scrollToItem(secondItem);
    QApplication::processEvents();

    QTest::mouseClick(
      listWidget->viewport(),
      Qt::LeftButton,
      Qt::NoModifier,
      listWidget->visualItemRect(secondItem).center());

    QApplication::processEvents();

    CHECK(spy.count() == 1);
    REQUIRE(manager->selectedProfile() != nullptr);
    CHECK(manager->selectedProfile()->name == "test 2");
  }

  SECTION("add profile by clicking button")
  {
    auto manager = std::make_unique<CompilationProfileManager>(document, config);

    manager->resize(700, 400);
    manager->show();
    QApplication::processEvents();

    auto* addButton =
      manager->findChild<QToolButton*>("CompilationProfileManager_AddProfileButton");

    REQUIRE(addButton != nullptr);

    QTest::mouseClick(addButton, Qt::LeftButton);
    QApplication::processEvents();

    CHECK(manager->config().profiles.size() == 1u);
    REQUIRE(manager->selectedProfile() != nullptr);
    CHECK(manager->selectedProfile()->name == "unnamed");
  }

  SECTION("remove profile by clicking button")
  {
    config.profiles.push_back(mdl::CompilationProfile{"test 1", "", {}});
    config.profiles.push_back(mdl::CompilationProfile{"test 2", "", {}});

    auto manager = std::make_unique<CompilationProfileManager>(document, config);

    manager->resize(700, 400);
    manager->show();
    QApplication::processEvents();

    auto* removeButton =
      manager->findChild<QToolButton*>("CompilationProfileManager_RemoveProfileButton");
    REQUIRE(removeButton != nullptr);
    REQUIRE(removeButton->isEnabled());

    QTest::mouseClick(removeButton, Qt::LeftButton);
    QApplication::processEvents();

    CHECK(manager->config().profiles.size() == 1u);
    REQUIRE(manager->selectedProfile() != nullptr);
    CHECK(manager->selectedProfile()->name == "test 2");
  }
}

} // namespace tb::ui
