/*
 Copyright (C) 2026 Atirna

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

#include <QToolButton>

#include "mdl/Layer.h"
#include "mdl/LayerNode.h"
#include "mdl/Map.h"
#include "mdl/Map_Layers.h"
#include "mdl/Map_Nodes.h"
#include "mdl/WorldNode.h"
#include "ui/AppControllerFixture.h"
#include "ui/CatchConfig.h"
#include "ui/LayerListBox.h"
#include "ui/MapDocument.h"
#include "ui/MapDocumentFixture.h"

#include <catch2/catch_test_macros.hpp>

namespace tb::ui
{

TEST_CASE("LayerListBox")
{
  auto appControllerFixture = AppControllerFixture{};

  auto documentFixture = MapDocumentFixture{};
  auto& document = documentFixture.create();
  auto& map = document.map();

  auto listBox = LayerListBox{document};

  auto* customLayerNode = new mdl::LayerNode{mdl::Layer{"custom layer"}};
  mdl::addNodes(map, {{&map.worldNode(), {customLayerNode}}});

  SECTION("updates the visibility icon when a layer is isolated")
  {
    const auto widgets = listBox.findChildren<LayerListBoxWidget*>();
    REQUIRE(widgets.size() == 2);

    mdl::isolateLayers(map, {customLayerNode});

    CHECK(map.worldNode().defaultLayer()->hidden());
    CHECK(!customLayerNode->hidden());

    for (const auto* widget : widgets)
    {
      const auto* hiddenButton =
        widget->findChild<QToolButton*>("LayerListBoxWidget_HiddenButton");
      REQUIRE(hiddenButton != nullptr);
      CHECK(hiddenButton->isChecked() == widget->layer()->hidden());
    }
  }
}

} // namespace tb::ui
