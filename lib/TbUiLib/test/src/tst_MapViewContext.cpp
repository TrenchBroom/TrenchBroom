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

#include "gl/PerspectiveCamera.h"
#include "mdl/BrushNode.h"
#include "mdl/Grid.h"
#include "mdl/Map.h"
#include "mdl/Map_Nodes.h"
#include "mdl/TestFactory.h"
#include "ui/CatchConfig.h"
#include "ui/MapDocument.h"
#include "ui/MapDocumentFixture.h"
#include "ui/MapViewContext.h"

#include <catch2/catch_test_macros.hpp>

namespace tb::ui
{

TEST_CASE("MapViewContext")
{
  auto fixture = MapDocumentFixture{};
  auto& map = fixture.create().map();
  map.grid().setSize(2);
  map.grid().toggleVisible();
  map.setCurrentMaterialName("unrest/eq_water");

  auto camera = gl::PerspectiveCamera{};
  camera.setViewport({4, 8, 640, 480});
  camera.moveTo({1.0f, 2.0f, 3.0f});
  camera.lookAt({1.0f, 3.0f, 3.0f}, {0.0f, 0.0f, 1.0f});

  const auto context = captureMapViewContext(map, camera, MapViewType::ThreeD);

  CHECK(context.document.revision == map.modificationCount());
  CHECK(context.document.modified == map.modified());
  CHECK(context.viewType == MapViewType::ThreeD);
  CHECK_FALSE(context.camera.orthographicProjection);
  CHECK(context.camera.viewport.x == 4);
  CHECK(context.camera.viewport.y == 8);
  CHECK(context.camera.viewport.width == 640);
  CHECK(context.camera.viewport.height == 480);
  CHECK(context.camera.position == vm::vec3f{1.0f, 2.0f, 3.0f});
  CHECK(context.grid.size == 2);
  CHECK(context.grid.actualSize == 4.0);
  CHECK(context.grid.snap);
  CHECK_FALSE(context.grid.visible);
  CHECK(context.currentMaterialName == "unrest/eq_water");
  REQUIRE(context.layers.size() == 1u);
  CHECK(context.layers.front().visible);
  CHECK(context.layers.front().current);

  SECTION("picking is semantic and does not use the interactive pick state")
  {
    auto* brushNode = mdl::createBrushNode(map);
    mdl::addNodes(map, {{&mdl::parentForNodes(map), {brushNode}}});

    camera.moveTo({-128.0f, 0.0f, 0.0f});
    camera.lookAt({0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f});

    const auto pickResult = pickMapView(map, camera, 320.0f, 240.0f);
    REQUIRE_FALSE(pickResult.hits.empty());
    REQUIRE(pickResult.hits.front().node.has_value());
    REQUIRE(pickResult.hits.front().faceIndex.has_value());
    CHECK(pickResult.hits.front().node->name == brushNode->name());
    CHECK(pickResult.hits.front().node->path.indices.size() == 2u);
  }
}

} // namespace tb::ui
