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
#include <QLineEdit>
#include <QtTest/QTest>

#include "mdl/Entity.h"
#include "mdl/EntityNode.h"
#include "mdl/Group.h"
#include "mdl/GroupNode.h"
#include "mdl/Map.h"
#include "mdl/Map_Nodes.h"
#include "mdl/Map_Selection.h"
#include "mdl/Node.h"
#include "mdl/Selection.h"
#include "ui/MapDocument.h"
#include "ui/MapDocumentFixture.h"
#include "ui/SearchPanel.h"

#include <vector>

#include <catch2/catch_test_macros.hpp>

namespace tb::ui
{

namespace
{

// a single, non-incremental query evaluation -- bypasses per-keystroke intermediate
// states, which legitimately change the selection along the way for fuzzy (Tier-1)
// text, so tests that only care about one query's own effect use this instead of
// QTest::keyClicks
void setQueryText(QLineEdit& searchBox, const QString& text)
{
  searchBox.setText(text);
  emit searchBox.textEdited(text);
}

// SearchPanel debounces evaluation by 500ms after the last text change -- pump the
// event loop past that so the search has actually run
void waitForSearchDelay()
{
  QTest::qWait(600);
}

} // namespace

TEST_CASE("SearchPanel")
{
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

  auto panel = SearchPanel{document};
  panel.resize(400, 40);
  panel.show();
  QApplication::processEvents();

  auto* searchBox = panel.findChild<QLineEdit*>();
  REQUIRE(searchBox != nullptr);
  searchBox->setFocus();

  SECTION("a full query typed keystroke by keystroke selects the matching nodes")
  {
    QTest::keyClicks(searchBox, R"(classname == "info_player_start")");
    waitForSearchDelay();

    CHECK(map.selection().nodes == std::vector<mdl::Node*>{playerStart});
  }

  SECTION("fuzzy text selects a layer/group by name substring")
  {
    setQueryText(*searchBox, "Combat");
    waitForSearchDelay();

    CHECK(map.selection().nodes == std::vector<mdl::Node*>{combatGroup});
  }

  SECTION("fuzzy text also selects an entity by classname")
  {
    setQueryText(*searchBox, "func_detail");
    waitForSearchDelay();

    CHECK(map.selection().nodes == std::vector<mdl::Node*>{detailEntity});
  }

  SECTION("emptying the search box deselects everything")
  {
    setQueryText(*searchBox, "Combat");
    waitForSearchDelay();
    REQUIRE(map.selection().nodes == std::vector<mdl::Node*>{combatGroup});

    setQueryText(*searchBox, "");
    waitForSearchDelay();

    CHECK(map.selection().nodes == std::vector<mdl::Node*>{});
  }

  SECTION("a malformed query does not change the selection and sets an error tooltip")
  {
    mdl::selectNodes(map, {playerStart});

    setQueryText(*searchBox, R"(classname ==)");
    waitForSearchDelay();

    CHECK(map.selection().nodes == std::vector<mdl::Node*>{playerStart});
    CHECK(!searchBox->toolTip().isEmpty());
  }

  SECTION("clearing the search box clears the error tooltip and deselects everything")
  {
    mdl::selectNodes(map, {playerStart});

    setQueryText(*searchBox, R"(classname ==)");
    waitForSearchDelay();
    REQUIRE(!searchBox->toolTip().isEmpty());

    setQueryText(*searchBox, "");
    waitForSearchDelay();

    CHECK(searchBox->toolTip().isEmpty());
    CHECK(map.selection().nodes == std::vector<mdl::Node*>{});
  }
}

} // namespace tb::ui
