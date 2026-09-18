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
#include <QLabel>
#include <QLineEdit>
#include <QWidget>
#include <QtTest/QTest>

#include "mdl/BrushNode.h"
#include "mdl/Entity.h"
#include "mdl/EntityNode.h"
#include "mdl/Group.h"
#include "mdl/GroupNode.h"
#include "mdl/Map.h"
#include "mdl/Map_Nodes.h"
#include "mdl/Map_Selection.h"
#include "mdl/Node.h"
#include "mdl/Selection.h"
#include "mdl/TestFactory.h"
#include "ui/AppControllerFixture.h"
#include "ui/MapDocument.h"
#include "ui/MapDocumentFixture.h"
#include "ui/SearchHelpPopup.h"
#include "ui/SearchPanel.h"

#include <algorithm>
#include <vector>

#include <catch2/catch_test_macros.hpp>

namespace tb::ui
{

namespace
{

// a single, non-incremental query evaluation -- bypasses per-keystroke intermediate
// states, which legitimately change the selection along the way for fuzzy (Tier-1) text,
// so tests that only care about one query's own effect use this instead of
// QTest::keyClicks
void setQueryText(QLineEdit& searchBox, const QString& text)
{
  searchBox.setText(text);
  emit searchBox.textEdited(text);
}

} // namespace

TEST_CASE("SearchPanel")
{
  auto appControllerFixture = AppControllerFixture{};
  auto& appController = appControllerFixture.appController();

  auto fixture = MapDocumentFixture{};
  auto& document = fixture.create();
  auto& map = document.map();

  auto* playerStart =
    new mdl::EntityNode{mdl::Entity{{{"classname", "info_player_start"}}}};
  mdl::addNodes(map, {{&mdl::parentForNodes(map), {playerStart}}});

  auto* detailEntity = new mdl::EntityNode{mdl::Entity{{{"classname", "func_detail"}}}};
  mdl::addNodes(map, {{&mdl::parentForNodes(map), {detailEntity}}});

  auto* combatGroup = new mdl::GroupNode{mdl::Group{"Combat"}};
  mdl::addNodes(map, {{&mdl::parentForNodes(map), {combatGroup}}});

  mdl::addNodes(map, {{combatGroup, {mdl::createBrushNode(map)}}});

  auto panel = SearchPanel{appController, document};
  panel.resize(400, 40);
  panel.show();
  QApplication::processEvents();

  auto* searchBox = panel.findChild<QLineEdit*>();
  REQUIRE(searchBox != nullptr);
  searchBox->setFocus();

  auto* popup = panel.findChild<SearchHelpPopup*>();
  REQUIRE(popup != nullptr);

  SECTION("a query typed keystroke by keystroke runs once typing pauses")
  {
    QTest::keyClicks(searchBox, R"(classname == "info_player_start")");
    CHECK(map.selection().nodes == std::vector<mdl::Node*>{});

    // the search runs 500ms after the last keystroke, but the timer can fire much later
    // on a busy machine
    REQUIRE(QTest::qWaitFor([&] { return map.selection().hasNodes(); }, 5000));

    CHECK(map.selection().nodes == std::vector<mdl::Node*>{playerStart});
    CHECK(popup->status() == "1 object selected");
  }

  SECTION("return runs the pending search immediately")
  {
    setQueryText(*searchBox, "Combat");
    QTest::keyClick(searchBox, Qt::Key_Return);

    CHECK(map.selection().nodes == std::vector<mdl::Node*>{combatGroup});
  }

  SECTION("fuzzy text selects a layer/group by name substring")
  {
    setQueryText(*searchBox, "Combat");
    QTest::keyClick(searchBox, Qt::Key_Return);

    CHECK(map.selection().nodes == std::vector<mdl::Node*>{combatGroup});
  }

  SECTION("fuzzy text also selects an entity by classname")
  {
    setQueryText(*searchBox, "func_detail");
    QTest::keyClick(searchBox, Qt::Key_Return);

    CHECK(map.selection().nodes == std::vector<mdl::Node*>{detailEntity});
  }

  SECTION("emptying the search box deselects everything")
  {
    setQueryText(*searchBox, "Combat");
    QTest::keyClick(searchBox, Qt::Key_Return);
    REQUIRE(map.selection().nodes == std::vector<mdl::Node*>{combatGroup});

    setQueryText(*searchBox, "");
    QTest::keyClick(searchBox, Qt::Key_Return);

    CHECK(map.selection().nodes == std::vector<mdl::Node*>{});
  }

  SECTION("a query that matches nothing deselects everything and says so")
  {
    mdl::selectNodes(map, {playerStart});

    setQueryText(*searchBox, R"(classname ==)");
    QTest::keyClick(searchBox, Qt::Key_Return);

    CHECK(map.selection().nodes == std::vector<mdl::Node*>{});
    CHECK(popup->status() == "'classname ==' did not match anything");
  }

  SECTION("a query that matches several objects reports how many")
  {
    setQueryText(*searchBox, R"(classname like "*_*")");
    QTest::keyClick(searchBox, Qt::Key_Return);

    CHECK(popup->status() == "2 objects selected");
  }

  SECTION("clearing the search box clears the status and deselects everything")
  {
    mdl::selectNodes(map, {playerStart});

    setQueryText(*searchBox, R"(classname ==)");
    QTest::keyClick(searchBox, Qt::Key_Return);
    REQUIRE(!popup->status().isEmpty());

    setQueryText(*searchBox, "");
    QTest::keyClick(searchBox, Qt::Key_Return);

    CHECK(popup->status().isEmpty());
    CHECK(map.selection().nodes == std::vector<mdl::Node*>{});
  }

  SECTION("the popup links to the query language section of the manual")
  {
    CHECK(std::ranges::any_of(popup->findChildren<QLabel*>(), [](const auto* label) {
      return label->text().contains(R"(<a href="query_language">)");
    }));
  }

  SECTION("the popup takes the focus when its window is activated")
  {
    // Qt moves the focus to the focus widget of an activated window and clears the focus
    // if there is none, which would hide the popup when it is first clicked
    CHECK(popup->focusWidget() == popup);
  }

  SECTION("the popup is shown while the search box or the popup has focus")
  {
    // emitted directly because whether the test window becomes active, and hence
    // whether setFocus changes the application's focus widget, depends on the platform
    emit qApp->focusChanged(nullptr, searchBox);
    CHECK(popup->isVisible());

    emit qApp->focusChanged(searchBox, popup);
    CHECK(popup->isVisible());

    emit qApp->focusChanged(popup, &panel);
    CHECK(!popup->isVisible());

    emit qApp->focusChanged(&panel, searchBox);
    REQUIRE(popup->isVisible());

    emit qApp->focusChanged(searchBox, nullptr);
    CHECK(!popup->isVisible());
  }

  SECTION("the panel is expanded while the search box has focus or is not empty")
  {
    emit qApp->focusChanged(searchBox, nullptr);
    REQUIRE(!panel.expanded());

    emit qApp->focusChanged(nullptr, searchBox);
    CHECK(panel.expanded());

    emit qApp->focusChanged(searchBox, popup);
    CHECK(panel.expanded());

    emit qApp->focusChanged(popup, nullptr);
    CHECK(!panel.expanded());

    searchBox->setText("Combat");
    CHECK(panel.expanded());

    emit qApp->focusChanged(nullptr, searchBox);
    emit qApp->focusChanged(searchBox, nullptr);
    CHECK(panel.expanded());

    searchBox->clear();
    CHECK(!panel.expanded());
  }

  SECTION("escape hides the popup until the search box gets focus again")
  {
    emit qApp->focusChanged(nullptr, searchBox);
    REQUIRE(popup->isVisible());

    QTest::keyClick(searchBox, Qt::Key_Escape);
    CHECK(!popup->isVisible());

    QTest::keyClicks(searchBox, "light");
    CHECK(!popup->isVisible());

    emit qApp->focusChanged(searchBox, nullptr);
    emit qApp->focusChanged(nullptr, searchBox);
    CHECK(popup->isVisible());
  }

  SECTION("the popup follows the search box when a widget containing it moves")
  {
    auto container = QWidget{};
    container.resize(600, 40);

    auto* containedPanel = new SearchPanel{appController, document, &container};
    containedPanel->resize(400, 40);
    container.show();
    QApplication::processEvents();

    auto* containedSearchBox = containedPanel->findChild<QLineEdit*>();
    auto* containedPopup = containedPanel->findChild<SearchHelpPopup*>();

    emit qApp->focusChanged(nullptr, containedSearchBox);
    REQUIRE(containedPopup->isVisible());

    containedPanel->move(100, 0);
    QApplication::processEvents();

    CHECK(
      containedPopup->pos()
      == containedSearchBox->mapToGlobal(QPoint{0, containedSearchBox->height()}));
  }
}

} // namespace tb::ui
