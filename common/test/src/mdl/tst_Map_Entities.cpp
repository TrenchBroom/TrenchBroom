/*
 Copyright (C) 2025 Kristian Duske

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

#include "Logger.h"
#include "TestFactory.h"
#include "TestUtils.h"
#include "mdl/BrushNode.h"
#include "mdl/Entity.h"
#include "mdl/EntityDefinition.h"
#include "mdl/EntityDefinitionManager.h"
#include "mdl/EntityNode.h"
#include "mdl/GroupNode.h"
#include "mdl/Map.h"

#include "Catch2.h"

namespace tb::mdl
{

TEST_CASE("Map_Entities")
{
  auto taskManager = createTestTaskManager();
  auto logger = NullLogger{};

  auto map = Map{*taskManager, logger};

  SECTION("createPointEntity")
  {
    SECTION("Linked group update failure")
    {
      auto* entityNode = new EntityNode{Entity{}};
      map.addNodes({{map.parentForNodes(), {entityNode}}});
      map.selectNodes({entityNode});

      // move the entity down
      REQUIRE(map.translateSelection({0, 0, -256}));
      REQUIRE(
        entityNode->physicalBounds() == vm::bbox3d{{-8, -8, -256 - 8}, {8, 8, -256 + 8}});

      auto* groupNode = map.groupSelectedNodes("test");
      auto* linkedGroupNode = map.createLinkedDuplicate();

      // move the linked group up by half the world bounds
      const auto zOffset = map.worldBounds().max.z();
      map.deselectAll();
      map.selectNodes({linkedGroupNode});
      map.translateSelection({0, 0, map.worldBounds().max.z()});
      REQUIRE(
        linkedGroupNode->physicalBounds()
        == vm::bbox3d{{-8, -8, -256 - 8 + zOffset}, {8, 8, -256 + 8 + zOffset}});

      // create a brush entity inside the original group
      map.openGroup(groupNode);
      map.deselectAll();

      map.setEntityDefinitions({
        {"point_entity",
         Color{},
         "this is a point entity",
         {},
         PointEntityDefinition{vm::bbox3d{16.0}, {}, {}}},
      });

      const auto& pointEntityDefinition =
        map.entityDefinitionManager().definitions().front();

      // create a new point entity below the origin -- this entity is temporarily
      // created at the origin and then moved to its eventual position, but the entity
      // at the origin is propagated into the linked group, where it ends up out of
      // world bounds
      CHECK(map.createPointEntity(pointEntityDefinition, {0, 0, -32}) != nullptr);
    }
  }

  SECTION("createBrushEntity")
  {
    SECTION("Linked group update failure")
    {
      auto* entityNode = new EntityNode{Entity{}};
      map.addNodes({{map.parentForNodes(), {entityNode}}});
      map.selectNodes({entityNode});

      // move the entity down
      REQUIRE(map.translateSelection({0, 0, -256}));
      REQUIRE(
        entityNode->physicalBounds() == vm::bbox3d{{-8, -8, -256 - 8}, {8, 8, -256 + 8}});

      auto* groupNode = map.groupSelectedNodes("test");
      auto* linkedGroupNode = map.createLinkedDuplicate();

      // move the linked group up by half the world bounds
      const auto zOffset = map.worldBounds().max.z();
      map.deselectAll();
      map.selectNodes({linkedGroupNode});
      map.translateSelection({0, 0, map.worldBounds().max.z()});
      REQUIRE(
        linkedGroupNode->physicalBounds()
        == vm::bbox3d{{-8, -8, -256 - 8 + zOffset}, {8, 8, -256 + 8 + zOffset}});

      // create a brush entity inside the original group
      map.openGroup(groupNode);
      map.deselectAll();

      auto* brushNode = createBrushNode(map);
      mdl::transformNode(
        *brushNode, vm::translation_matrix(vm::vec3d{0, 0, -32}), map.worldBounds());
      REQUIRE(brushNode->physicalBounds() == vm::bbox3d{{-16, -16, -48}, {16, 16, -16}});

      map.addNodes({{map.parentForNodes(), {brushNode}}});
      map.deselectAll();
      map.selectNodes({brushNode});

      map.setEntityDefinitions({
        {"brush_entity", Color{}, "this is a brush entity", {}},
      });

      const auto& brushEntityDefinition =
        map.entityDefinitionManager().definitions().front();

      // create a brush entity - a temporarily empty entity will be created at the origin
      // and propagated into the linked group, where it ends up out of world bounds and
      // thus failing
      CHECK(map.createBrushEntity(brushEntityDefinition) != nullptr);
    }
  }

  SECTION("setProperty")
  {
    SECTION("Attempt to set a property with 2 out of 3 groups selected")
    {
      // https://github.com/TrenchBroom/TrenchBroom/issues/3768

      auto* entityNode = new EntityNode{Entity{}};
      map.addNodes({{map.parentForNodes(), {entityNode}}});
      map.selectNodes({entityNode});

      auto* groupNode = map.groupSelectedNodes("test");
      auto* linkedGroupNode1 = map.createLinkedDuplicate();
      auto* linkedGroupNode2 = map.createLinkedDuplicate();

      REQUIRE(groupNode != nullptr);
      REQUIRE(linkedGroupNode1 != nullptr);
      REQUIRE(linkedGroupNode2 != nullptr);

      map.deselectAll();

      map.selectNodes({groupNode, linkedGroupNode1});

      // Current design is to reject this because it's modifying entities from multiple
      // groups in a link set. While in this case the change isn't conflicting, some
      // entity changes are, e.g. unprotecting a property with 2 linked groups selected,
      // where entities have different values for that protected property.
      //
      // Additionally, the use case for editing entity properties with the entire map
      // selected seems unlikely.
      CHECK(!map.setProperty("key", "value"));

      auto* groupNodeEntity = dynamic_cast<EntityNode*>(groupNode->children().at(0));
      auto* linkedEntityNode1 =
        dynamic_cast<EntityNode*>(linkedGroupNode1->children().at(0));
      auto* linkedEntityNode2 =
        dynamic_cast<EntityNode*>(linkedGroupNode2->children().at(0));
      REQUIRE(groupNodeEntity != nullptr);
      REQUIRE(linkedEntityNode1 != nullptr);
      REQUIRE(linkedEntityNode2 != nullptr);

      CHECK(!groupNodeEntity->entity().hasProperty("key"));
      CHECK(!linkedEntityNode1->entity().hasProperty("key"));
      CHECK(!linkedEntityNode2->entity().hasProperty("key"));
    }
  }
}

} // namespace tb::mdl
