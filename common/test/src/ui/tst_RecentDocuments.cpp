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

#include "ui/RecentDocuments.h"

#include "Catch2.h"

namespace tb::ui
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
  auto filterPredicate = [](auto path) { return path != "filter.map"; };

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
      "filter.map",
    });
    CHECK(loadRecentDocuments(1) == std::vector<std::filesystem::path>{"1.map"});
  }

  SECTION("constructor")
  {
    saveRecentDocuments({
      "1.map",
      "2.map",
      "filter.map",
    });

    auto recentDocuments = RecentDocuments{5, filterPredicate};
    CHECK(recentDocuments.recentDocuments().empty());
  }

  SECTION("reload")
  {
    saveRecentDocuments({
      "1.map",
      "2.map",
      "filter.map",
    });

    auto recentDocuments = RecentDocuments{5, filterPredicate};
    REQUIRE(recentDocuments.recentDocuments().empty());

    auto spy = QSignalSpy{&recentDocuments, SIGNAL(didChange())};

    REQUIRE(spy.count() == 0);

    recentDocuments.reload();
    CHECK(
      recentDocuments.recentDocuments()
      == std::vector<std::filesystem::path>{
        "1.map",
        "2.map",
      });
    CHECK(spy.count() == 1);

    recentDocuments.reload();
    CHECK(
      recentDocuments.recentDocuments()
      == std::vector<std::filesystem::path>{
        "1.map",
        "2.map",
      });
    CHECK(spy.count() == 1);

    saveRecentDocuments({
      "1.map",
      "2.map",
      "3.map",
    });

    recentDocuments.reload();
    CHECK(
      recentDocuments.recentDocuments()
      == std::vector<std::filesystem::path>{
        "1.map",
        "2.map",
        "3.map",
      });
    CHECK(spy.count() == 2);
  }

  SECTION("updatePath")
  {
    saveRecentDocuments({
      "1.map",
      "2.map",
      "filter.map",
    });
    auto recentDocuments = RecentDocuments{5, filterPredicate};
    recentDocuments.reload();

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
        "filter.map",
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
    CHECK(spy.count() == 2);

    recentDocuments.updatePath("filter.map");
    CHECK(
      recentDocuments.recentDocuments()
      == std::vector<std::filesystem::path>{
        "3.map",
        "2.map",
        "1.map",
      });
    CHECK(
      loadRecentDocuments(5)
      == std::vector<std::filesystem::path>{
        "filter.map",
        "3.map",
        "2.map",
        "1.map",
      });
    CHECK(spy.count() == 2);

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
      });
    CHECK(
      loadRecentDocuments(5)
      == std::vector<std::filesystem::path>{
        "6.map",
        "5.map",
        "4.map",
        "filter.map",
        "3.map",
      });
    CHECK(spy.count() == 5);
  }

  SECTION("removePath")
  {
    saveRecentDocuments({
      "1.map",
      "2.map",
      "3.map",
      "filter.map",
    });
    auto recentDocuments = RecentDocuments{5, filterPredicate};
    recentDocuments.reload();

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
        "filter.map",
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
    CHECK(
      loadRecentDocuments(5)
      == std::vector<std::filesystem::path>{
        "filter.map",
      });
    CHECK(spy.count() == 3);

    recentDocuments.removePath("filter.map");
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
      "filter.map",
    });
    auto recentDocuments = RecentDocuments{5, filterPredicate};
    recentDocuments.reload();

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

} // namespace tb::ui