/*
 Copyright (C) 2023 Kristian Duske

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

#include <QMenu>
#include <QSettings>
#include <QtTest/QSignalSpy>

#include "View/RecentDocuments.h"

#include "kdl/vector_utils.h"

#include "Catch2.h"

namespace TrenchBroom::View
{

namespace
{
std::vector<QString> getTexts(const QList<QAction*>& actions)
{
  auto result = std::vector<QString>{};
  for (const auto* action : actions)
  {
    result.push_back(action->text());
  }
  return result;
}
} // namespace

TEST_CASE("RecentDocuments")
{
  SECTION("load and save")
  {
    saveRecentDocuments({});
    CHECK(loadRecentDocuments(5).empty());

    saveRecentDocuments({"this/that.map", "that/this.map"});
    CHECK(
      loadRecentDocuments(5)
      == std::vector<std::filesystem::path>{"this/that.map", "that/this.map"});

    saveRecentDocuments({"some/other.map"});
    CHECK(loadRecentDocuments(5) == std::vector<std::filesystem::path>{"some/other.map"});

    saveRecentDocuments({
      "1.map",
      "2.map",
    });
    CHECK(loadRecentDocuments(1) == std::vector<std::filesystem::path>{"1.map"});
  }

  SECTION("constructor")
  {
    saveRecentDocuments({
      "1.map",
      "2.map",
    });

    auto recentDocuments = RecentDocuments{5};
    CHECK(
      recentDocuments.recentDocuments()
      == std::vector<std::filesystem::path>{
        "1.map",
        "2.map",
      });
  }

  SECTION("updatePath")
  {
    saveRecentDocuments({
      "1.map",
      "2.map",
    });
    auto recentDocuments = RecentDocuments{5};

    auto spy = QSignalSpy{&recentDocuments, SIGNAL(didChange())};

    recentDocuments.updatePath("2.map");
    CHECK(
      recentDocuments.recentDocuments()
      == std::vector<std::filesystem::path>{
        "2.map",
        "1.map",
      });
    CHECK(
      loadRecentDocuments(5)
      == std::vector<std::filesystem::path>{
        "2.map",
        "1.map",
      });
    CHECK(spy.count() == 1);

    recentDocuments.updatePath("3.map");
    CHECK(
      recentDocuments.recentDocuments()
      == std::vector<std::filesystem::path>{
        "3.map",
        "2.map",
        "1.map",
      });
    CHECK(spy.count() == 2);

    recentDocuments.updatePath("3.map");
    CHECK(
      recentDocuments.recentDocuments()
      == std::vector<std::filesystem::path>{
        "3.map",
        "2.map",
        "1.map",
      });
    CHECK(spy.count() == 3);

    recentDocuments.updatePath("4.map");
    recentDocuments.updatePath("5.map");
    recentDocuments.updatePath("6.map");
    CHECK(
      recentDocuments.recentDocuments()
      == std::vector<std::filesystem::path>{
        "6.map",
        "5.map",
        "4.map",
        "3.map",
        "2.map",
      });
    CHECK(
      loadRecentDocuments(5)
      == std::vector<std::filesystem::path>{
        "6.map",
        "5.map",
        "4.map",
        "3.map",
        "2.map",
      });
    CHECK(spy.count() == 6);
  }

  SECTION("removePath")
  {
    saveRecentDocuments({
      "1.map",
      "2.map",
      "3.map",
    });
    auto recentDocuments = RecentDocuments{5};

    auto spy = QSignalSpy{&recentDocuments, SIGNAL(didChange())};

    recentDocuments.removePath("2.map");
    CHECK(
      recentDocuments.recentDocuments()
      == std::vector<std::filesystem::path>{
        "1.map",
        "3.map",
      });
    CHECK(
      loadRecentDocuments(5)
      == std::vector<std::filesystem::path>{
        "1.map",
        "3.map",
      });
    CHECK(spy.count() == 1);

    recentDocuments.removePath("1.map");
    CHECK(
      recentDocuments.recentDocuments()
      == std::vector<std::filesystem::path>{
        "3.map",
      });
    CHECK(spy.count() == 2);

    recentDocuments.removePath("1.map");
    CHECK(
      recentDocuments.recentDocuments()
      == std::vector<std::filesystem::path>{
        "3.map",
      });
    CHECK(spy.count() == 2);

    recentDocuments.removePath("3.map");
    CHECK(recentDocuments.recentDocuments().empty());
    CHECK(loadRecentDocuments(5).empty());
    CHECK(spy.count() == 3);
  }

  SECTION("menus")
  {
    auto menu1 = QMenu{};
    auto menu2 = QMenu{};

    saveRecentDocuments({
      "1.map",
      "2.map",
      "3.map",
    });
    auto recentDocuments = RecentDocuments{5};

    recentDocuments.addMenu(menu1);
    CHECK(
      getTexts(menu1.actions())
      == std::vector<QString>{
        "1.map",
        "2.map",
        "3.map",
      });

    recentDocuments.addMenu(menu2);
    CHECK(
      getTexts(menu2.actions())
      == std::vector<QString>{
        "1.map",
        "2.map",
        "3.map",
      });

    recentDocuments.updatePath("4.map");
    CHECK(
      getTexts(menu1.actions())
      == std::vector<QString>{
        "4.map",
        "1.map",
        "2.map",
        "3.map",
      });
    CHECK(
      getTexts(menu2.actions())
      == std::vector<QString>{
        "4.map",
        "1.map",
        "2.map",
        "3.map",
      });

    recentDocuments.removePath("1.map");
    CHECK(
      getTexts(menu1.actions())
      == std::vector<QString>{
        "4.map",
        "2.map",
        "3.map",
      });
    CHECK(
      getTexts(menu2.actions())
      == std::vector<QString>{
        "4.map",
        "2.map",
        "3.map",
      });

    recentDocuments.removeMenu(menu2);
    CHECK(
      getTexts(menu1.actions())
      == std::vector<QString>{
        "4.map",
        "2.map",
        "3.map",
      });
    CHECK(menu2.actions().empty());
  }
}
} // namespace TrenchBroom::View