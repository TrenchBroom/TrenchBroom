/*
 Copyright (C) 2010 Kristian Duske

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

#include "TestFactory.h"
#include "mdl/BrushNode.h" // IWYU pragma: keep
#include "mdl/Entity.h"
#include "mdl/EntityNode.h"
#include "mdl/Grid.h"
#include "mdl/Map.h"
#include "mdl/MapFixture.h"
#include "mdl/Map_Nodes.h"
#include "mdl/Map_Selection.h"
#include "ui/RotateTool.h"

#include "catch/CatchConfig.h"

#include <catch2/catch_test_macros.hpp>

namespace tb::ui
{

TEST_CASE("RotateTool")
{
  auto fixture = mdl::MapFixture{};
  auto& map = fixture.map();
  fixture.create();

  auto tool = RotateTool{map};
  tool.activate();

  SECTION("resetRotationCenter")
  {
    auto entity1 = mdl::Entity{};
    entity1.setOrigin(vm::vec3d{8, 16, 32});

    auto entity2 = mdl::Entity{};
    entity2.setOrigin(vm::vec3d{16, 24, 32});

    auto* entityNode1 = new mdl::EntityNode{std::move(entity1)};
    auto* entityNode2 = new mdl::EntityNode{std::move(entity2)};
    auto* brushNode = createBrushNode(map);

    addNodes(map, {{parentForNodes(map), {entityNode1, entityNode2, brushNode}}});

    SECTION("If nothing is selected")
    {
      tool.resetRotationCenter();
      CHECK(tool.rotationCenter() == vm::bbox3d{}.center());
    }

    SECTION("If a single entity is selected")
    {
      selectNodes(map, {entityNode1});

      tool.resetRotationCenter();
      CHECK(tool.rotationCenter() == vm::vec3d{8, 16, 32});
    }

    SECTION("If multiple entities are selected")
    {
      selectNodes(map, {entityNode1, entityNode2});

      tool.resetRotationCenter();
      CHECK(tool.rotationCenter() == map.grid().snap(map.selectionBounds()->center()));
    }

    SECTION("If a mix of nodes is selected")
    {
      selectNodes(map, {entityNode1, brushNode});

      tool.resetRotationCenter();
      CHECK(tool.rotationCenter() == map.grid().snap(map.selectionBounds()->center()));
    }
  }
}


} // namespace tb::ui
