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

#include "mdl/BezierPatch.h"
#include "mdl/Brush.h"
#include "mdl/BrushBuilder.h"
#include "mdl/BrushNode.h"
#include "mdl/Entity.h"
#include "mdl/EntityDefinition.h"
#include "mdl/EntityNode.h"
#include "mdl/EntityProperties.h"
#include "mdl/EntityRotation.h"
#include "mdl/Group.h"
#include "mdl/GroupNode.h"
#include "mdl/Layer.h"
#include "mdl/LayerNode.h"
#include "mdl/MapFormat.h"
#include "mdl/PatchNode.h"
#include "mdl/WorldNode.h"

#include "kdl/result.h"

#include "vm/bbox.h"
#include "vm/util.h"
#include "vm/vec.h"

#include <string>

#include "Catch2.h"

namespace tb::mdl
{

TEST_CASE("EntityNodeTest.canAddChild")
{
  constexpr auto worldBounds = vm::bbox3d{8192.0};
  constexpr auto mapFormat = MapFormat::Quake3;

  auto worldNode = WorldNode{{}, {}, mapFormat};
  auto layerNode = LayerNode{Layer{"layer"}};
  auto groupNode = GroupNode{Group{"group"}};
  auto entityNode = EntityNode{Entity{}};
  auto brushNode = BrushNode{
    BrushBuilder{mapFormat, worldBounds}.createCube(64.0, "material") | kdl::value()};

  // clang-format off
  auto patchNode = PatchNode{BezierPatch{3, 3, {
    {0, 0, 0}, {1, 0, 1}, {2, 0, 0},
    {0, 1, 1}, {1, 1, 2}, {2, 1, 1},
    {0, 2, 0}, {1, 2, 1}, {2, 2, 0} }, "material"}};
  // clang-format on

  CHECK_FALSE(entityNode.canAddChild(&worldNode));
  CHECK_FALSE(entityNode.canAddChild(&layerNode));
  CHECK_FALSE(entityNode.canAddChild(&groupNode));
  CHECK_FALSE(entityNode.canAddChild(&entityNode));
  CHECK(entityNode.canAddChild(&brushNode));
  CHECK(entityNode.canAddChild(&patchNode));
}

TEST_CASE("EntityNodeTest.canRemoveChild")
{
  constexpr auto worldBounds = vm::bbox3d{8192.0};
  constexpr auto mapFormat = MapFormat::Quake3;

  const auto worldNode = WorldNode{{}, {}, mapFormat};
  auto layerNode = LayerNode{Layer{"layer"}};
  auto groupNode = GroupNode{Group{"group"}};
  auto entityNode = EntityNode{Entity{}};
  auto brushNode = BrushNode{
    BrushBuilder{mapFormat, worldBounds}.createCube(64.0, "material") | kdl::value()};

  // clang-format off
  auto patchNode = PatchNode{BezierPatch{3, 3, {
    {0, 0, 0}, {1, 0, 1}, {2, 0, 0},
    {0, 1, 1}, {1, 1, 2}, {2, 1, 1},
    {0, 2, 0}, {1, 2, 1}, {2, 2, 0} }, "material"}};
  // clang-format on

  CHECK(entityNode.canRemoveChild(&worldNode));
  CHECK(worldNode.canRemoveChild(&layerNode));
  CHECK(entityNode.canRemoveChild(&groupNode));
  CHECK(entityNode.canRemoveChild(&entityNode));
  CHECK(entityNode.canRemoveChild(&brushNode));
  CHECK(entityNode.canRemoveChild(&patchNode));
}

TEST_CASE("EntityNodeTest.setPointEntity")
{
  constexpr auto worldBounds = vm::bbox3d{8192.0};
  constexpr auto mapFormat = MapFormat::Quake3;

  auto entityNode = EntityNode{Entity{}};
  auto brushNode1 = BrushNode{
    BrushBuilder{mapFormat, worldBounds}.createCube(64.0, "material") | kdl::value()};
  auto brushNode2 = BrushNode{
    BrushBuilder{mapFormat, worldBounds}.createCube(64.0, "material") | kdl::value()};

  REQUIRE(entityNode.entity().pointEntity());
  entityNode.addChild(&brushNode1);
  CHECK_FALSE(entityNode.entity().pointEntity());
  entityNode.addChild(&brushNode2);
  CHECK_FALSE(entityNode.entity().pointEntity());

  entityNode.removeChild(&brushNode1);
  CHECK_FALSE(entityNode.entity().pointEntity());
  entityNode.removeChild(&brushNode2);
  CHECK(entityNode.entity().pointEntity());
}

TEST_CASE("EntityNodeTest.area")
{
  auto definition = PointEntityDefinition(
    "some_name",
    Color(),
    vm::bbox3d(vm::vec3d{0, 0, 0}, vm::vec3d(1.0, 2.0, 3.0)),
    "",
    {},
    {},
    {});
  auto entityNode = EntityNode{Entity{}};
  entityNode.setDefinition(&definition);

  CHECK(entityNode.projectedArea(vm::axis::x) == 6.0);
  CHECK(entityNode.projectedArea(vm::axis::y) == 3.0);
  CHECK(entityNode.projectedArea(vm::axis::z) == 2.0);
}

static const std::string TestClassname = "something";

class EntityNodeTest
{
protected:
  vm::bbox3d m_worldBounds;
  EntityNode* m_entity;
  WorldNode* m_world;

  EntityNodeTest()
  {
    m_worldBounds = vm::bbox3d(8192.0);
    m_entity = new EntityNode(Entity{{{EntityPropertyKeys::Classname, TestClassname}}});
    m_world = new WorldNode({}, {}, MapFormat::Standard);
  }

  virtual ~EntityNodeTest()
  {
    // Only some of the tests add the entity to the world
    if (m_entity->parent() == nullptr)
    {
      delete m_entity;
    }
    delete m_world;
  }
};

TEST_CASE_METHOD(EntityNodeTest, "EntityNodeTest.originUpdateWithSetProperties")
{
  const vm::vec3d newOrigin(10, 20, 30);
  const vm::bbox3d newBounds(
    newOrigin - (EntityNode::DefaultBounds.size() / 2.0),
    newOrigin + (EntityNode::DefaultBounds.size() / 2.0));

  m_entity->setEntity(Entity{{{"origin", "10 20 30"}}});
  CHECK(m_entity->entity().origin() == newOrigin);
  CHECK(m_entity->logicalBounds() == newBounds);
}

TEST_CASE_METHOD(EntityNodeTest, "EntityNodeTest.originUpdateWithAddOrUpdateProperties")
{
  const vm::vec3d newOrigin(10, 20, 30);
  const vm::bbox3d newBounds(
    newOrigin - (EntityNode::DefaultBounds.size() / 2.0),
    newOrigin + (EntityNode::DefaultBounds.size() / 2.0));

  m_entity->setEntity(Entity{{{"origin", "10 20 30"}}});
  CHECK(m_entity->entity().origin() == newOrigin);
  CHECK(m_entity->logicalBounds() == newBounds);
}

// Same as above, but add the entity to a world
TEST_CASE_METHOD(EntityNodeTest, "EntityNodeTest.originUpdateInWorld")
{
  m_world->defaultLayer()->addChild(m_entity);

  const vm::vec3d newOrigin(10, 20, 30);
  const vm::bbox3d newBounds(
    newOrigin - (EntityNode::DefaultBounds.size() / 2.0),
    newOrigin + (EntityNode::DefaultBounds.size() / 2.0));

  m_entity->setEntity(Entity{{{"origin", "10 20 30"}}});
  CHECK(m_entity->entity().origin() == newOrigin);
  CHECK(m_entity->logicalBounds() == newBounds);
}

} // namespace tb::mdl
