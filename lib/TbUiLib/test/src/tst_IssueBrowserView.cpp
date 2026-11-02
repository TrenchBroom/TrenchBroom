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
#include <QScrollBar>
#include <QTableView>

#include "mdl/Entity.h"
#include "mdl/EntityNode.h"
#include "mdl/Map.h"
#include "mdl/Map_Entities.h"
#include "mdl/Map_Nodes.h"
#include "mdl/Map_Selection.h"
#include "ui/CatchConfig.h"
#include "ui/IssueBrowserView.h"
#include "ui/MapDocument.h"
#include "ui/MapDocumentFixture.h"

#include <vector>

#include <catch2/catch_test_macros.hpp>

namespace tb::ui
{

TEST_CASE("IssueBrowserView")
{
  auto fixture = MapDocumentFixture{};
  auto& document = fixture.create();
  auto& map = document.map();

  auto* entityNode = new mdl::EntityNode{mdl::Entity{{
    {"classname", "some_class"},
    {"some_key", ""},
  }}};
  mdl::addNodes(map, {{&mdl::parentForNodes(map), {entityNode}}});

  auto view = IssueBrowserView{document};
  view.validate();

  const auto* tableView = view.findChild<QTableView*>("IssueBrowserView_TableView");
  REQUIRE(tableView != nullptr);

  const auto* model = tableView->model();
  REQUIRE(model->rowCount() > 0);

  SECTION("reload")
  {
    SECTION("discards the issues until the view is validated again")
    {
      // Changing the entity destroys its issues, so the view must not hold on to them
      // until it is validated again
      mdl::selectNodes(map, {entityNode});
      REQUIRE(mdl::setEntityProperty(map, "some_other_key", "some_value"));

      view.reload();
      CHECK(model->rowCount() == 0);

      view.validate();
      CHECK(model->rowCount() > 0);
    }

    SECTION("preserves the scroll position")
    {
      auto entityNodes = std::vector<mdl::Node*>{};
      for (size_t i = 0; i < 100; ++i)
      {
        entityNodes.push_back(new mdl::EntityNode{mdl::Entity{{
          {"classname", "some_class"},
          {"some_key", ""},
        }}});
      }
      mdl::addNodes(map, {{&mdl::parentForNodes(map), entityNodes}});

      view.reload();
      view.validate();

      view.resize(400, 200);
      view.show();
      QApplication::processEvents();

      auto* scrollBar = tableView->verticalScrollBar();
      REQUIRE(scrollBar->maximum() > 0);

      scrollBar->setValue(scrollBar->maximum() / 2);
      const auto scrollPosition = scrollBar->value();

      // let the table view lay out its empty model while the view is invalid
      view.reload();
      QApplication::processEvents();

      view.validate();
      CHECK(scrollBar->value() == scrollPosition);
    }
  }
}

} // namespace tb::ui
