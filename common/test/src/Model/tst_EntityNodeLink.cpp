/*
 Copyright (C) 2010-2017 Kristian Duske

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

#include "Assets/DecalDefinition.h"
#include "Assets/EntityDefinition.h"
#include "Assets/ModelDefinition.h"
#include "Assets/PropertyDefinition.h"
#include "Model/Entity.h"
#include "Model/EntityNode.h"
#include "Model/EntityNodeBase.h"
#include "Model/LayerNode.h"
#include "Model/MapFormat.h"
#include "Model/WorldNode.h"

#include "kdl/vector_utils.h"

#include <vector>

#include "Catch2.h"

namespace TrenchBroom::Model
{
namespace
{
std::shared_ptr<Assets::EntityDefinition> createTestEntityDefinition()
{
  auto propertyDefinitions = std::vector<std::shared_ptr<Assets::PropertyDefinition>>{};
  propertyDefinitions.emplace_back(std::make_shared<Assets::PropertyDefinition>(
    EntityPropertyKeys::Targetname,
    Assets::PropertyDefinitionType::TargetSourceProperty,
    "",
    "",
    false));
  propertyDefinitions.emplace_back(std::make_shared<Assets::PropertyDefinition>(
    EntityPropertyKeys::Target,
    Assets::PropertyDefinitionType::TargetDestinationProperty,
    "",
    "",
    false));

  return std::make_shared<Assets::PointEntityDefinition>(
    "",
    Color{},
    vm::bbox3(-64.0f, +64.0f),
    "",
    propertyDefinitions,
    Assets::ModelDefinition{},
    Assets::DecalDefinition{});
}
} // namespace

TEST_CASE("EntityNodeLinkTest.testCreateLink")
{
  auto worldNode = WorldNode{{}, {}, MapFormat::Standard};
  auto* sourceNode = new EntityNode{Entity{}};
  auto* targetNode = new EntityNode{Entity{}};
  worldNode.defaultLayer()->addChild(sourceNode);
  worldNode.defaultLayer()->addChild(targetNode);

  sourceNode->setEntity(Entity{{{EntityPropertyKeys::Target, "a"}}});

  targetNode->setEntity(Entity{{{EntityPropertyKeys::Targetname, "a"}}});

  auto definition = createTestEntityDefinition();
  sourceNode->setDefinition(definition.get());
  targetNode->setDefinition(definition.get());

  CHECK(sourceNode->linkSources().empty());
  CHECK_THAT(
    sourceNode->linkTargets(),
    Catch::UnorderedEquals(std::vector<EntityNodeBase*>{targetNode}));

  CHECK_THAT(
    targetNode->linkSources(),
    Catch::UnorderedEquals(std::vector<EntityNodeBase*>{sourceNode}));
  CHECK(targetNode->linkTargets().empty());

  sourceNode->setDefinition(nullptr);
  targetNode->setDefinition(nullptr);
}

TEST_CASE("EntityNodeLinkTest.testCreateMultiSourceLink")
{
  auto worldNode = WorldNode{{}, {}, MapFormat::Standard};
  auto* sourceNode1 = new EntityNode{Entity{}};
  auto* sourceNode2 = new EntityNode{Entity{}};
  auto* targetNode = new EntityNode{Entity{}};
  worldNode.defaultLayer()->addChild(sourceNode1);
  worldNode.defaultLayer()->addChild(sourceNode2);
  worldNode.defaultLayer()->addChild(targetNode);

  sourceNode1->setEntity(Entity{{{EntityPropertyKeys::Target, "a"}}});

  sourceNode2->setEntity(Entity{{{EntityPropertyKeys::Target, "a"}}});

  targetNode->setEntity(Entity{{{EntityPropertyKeys::Targetname, "a"}}});

  auto definition = createTestEntityDefinition();
  sourceNode1->setDefinition(definition.get());
  sourceNode2->setDefinition(definition.get());
  targetNode->setDefinition(definition.get());

  CHECK(sourceNode1->linkSources().empty());
  CHECK_THAT(
    sourceNode1->linkTargets(),
    Catch::UnorderedEquals(std::vector<EntityNodeBase*>{targetNode}));

  CHECK(sourceNode2->linkSources().empty());
  CHECK_THAT(
    sourceNode2->linkTargets(),
    Catch::UnorderedEquals(std::vector<EntityNodeBase*>{targetNode}));

  CHECK_THAT(
    targetNode->linkSources(),
    Catch::UnorderedEquals(std::vector<EntityNodeBase*>{sourceNode1, sourceNode2}));
  CHECK(targetNode->linkTargets().empty());

  sourceNode1->setDefinition(nullptr);
  sourceNode2->setDefinition(nullptr);
  targetNode->setDefinition(nullptr);
}

TEST_CASE("EntityNodeLinkTest.testCreateMultiTargetLink")
{
  auto worldNode = WorldNode{{}, {}, MapFormat::Standard};
  auto* sourceNode = new EntityNode{Entity{}};
  auto* targetNode1 = new EntityNode{Entity{}};
  auto* targetNode2 = new EntityNode{Entity{}};
  worldNode.defaultLayer()->addChild(sourceNode);
  worldNode.defaultLayer()->addChild(targetNode1);
  worldNode.defaultLayer()->addChild(targetNode2);

  sourceNode->setEntity(Entity(
    {{EntityPropertyKeys::Target + "1", "a1"},
     {EntityPropertyKeys::Target + "2", "a2"}}));

  // here we need to query for all entities having a numbered "target" property,
  // not just those having a "target" property
  targetNode1->setEntity(Entity{{{EntityPropertyKeys::Targetname, "a1"}}});

  targetNode2->setEntity(Entity{{{EntityPropertyKeys::Targetname, "a2"}}});

  auto definition = createTestEntityDefinition();
  sourceNode->setDefinition(definition.get());
  targetNode1->setDefinition(definition.get());
  targetNode2->setDefinition(definition.get());

  CHECK(sourceNode->linkSources().empty());
  CHECK_THAT(
    sourceNode->linkTargets(),
    Catch::UnorderedEquals(std::vector<EntityNodeBase*>{targetNode1, targetNode2}));

  CHECK_THAT(
    targetNode1->linkSources(),
    Catch::UnorderedEquals(std::vector<EntityNodeBase*>{sourceNode}));
  CHECK(targetNode1->linkTargets().empty());

  CHECK_THAT(
    targetNode2->linkSources(),
    Catch::UnorderedEquals(std::vector<EntityNodeBase*>{sourceNode}));
  CHECK(targetNode2->linkTargets().empty());

  sourceNode->setDefinition(nullptr);
  targetNode1->setDefinition(nullptr);
  targetNode2->setDefinition(nullptr);
}

TEST_CASE("EntityNodeLinkTest.testLoadLink")
{
  auto worldNode = WorldNode{{}, {}, MapFormat::Standard};
  auto* sourceNode = new EntityNode(Entity{{{EntityPropertyKeys::Target, "a"}}});
  auto* targetNode = new EntityNode(Entity{{{EntityPropertyKeys::Targetname, "a"}}});

  auto definition = createTestEntityDefinition();
  sourceNode->setDefinition(definition.get());
  targetNode->setDefinition(definition.get());

  worldNode.defaultLayer()->addChild(sourceNode);
  worldNode.defaultLayer()->addChild(targetNode);

  CHECK(sourceNode->linkSources().empty());
  CHECK_THAT(
    sourceNode->linkTargets(),
    Catch::UnorderedEquals(std::vector<EntityNodeBase*>{targetNode}));

  CHECK_THAT(
    targetNode->linkSources(),
    Catch::UnorderedEquals(std::vector<EntityNodeBase*>{sourceNode}));
  CHECK(targetNode->linkTargets().empty());

  sourceNode->setDefinition(nullptr);
  targetNode->setDefinition(nullptr);
}

TEST_CASE("EntityNodeLinkTest.testCreateLinkByChangingSource")
{
  auto worldNode = WorldNode{{}, {}, MapFormat::Standard};
  auto* sourceNode = new EntityNode(Entity{{{EntityPropertyKeys::Target, "a"}}});
  auto* targetNode = new EntityNode(Entity{{{EntityPropertyKeys::Targetname, "b"}}});

  auto definition = createTestEntityDefinition();
  sourceNode->setDefinition(definition.get());
  targetNode->setDefinition(definition.get());

  worldNode.defaultLayer()->addChild(sourceNode);
  worldNode.defaultLayer()->addChild(targetNode);

  REQUIRE(sourceNode->linkSources().empty());
  REQUIRE(sourceNode->linkTargets().empty());
  REQUIRE(targetNode->linkSources().empty());
  REQUIRE(targetNode->linkTargets().empty());

  sourceNode->setEntity(Entity{{{EntityPropertyKeys::Target, "b"}}});

  CHECK(sourceNode->linkSources().empty());
  CHECK_THAT(
    sourceNode->linkTargets(),
    Catch::UnorderedEquals(std::vector<EntityNodeBase*>{targetNode}));

  CHECK_THAT(
    targetNode->linkSources(),
    Catch::UnorderedEquals(std::vector<EntityNodeBase*>{sourceNode}));
  CHECK(targetNode->linkTargets().empty());

  sourceNode->setDefinition(nullptr);
  targetNode->setDefinition(nullptr);
}

TEST_CASE("EntityNodeLinkTest.testCreateLinkByChangingTarget")
{
  auto worldNode = WorldNode{{}, {}, MapFormat::Standard};
  auto* sourceNode = new EntityNode(Entity{{{EntityPropertyKeys::Target, "a"}}});
  auto* targetNode = new EntityNode(Entity{{{EntityPropertyKeys::Targetname, "b"}}});

  auto definition = createTestEntityDefinition();
  sourceNode->setDefinition(definition.get());
  targetNode->setDefinition(definition.get());

  worldNode.defaultLayer()->addChild(sourceNode);
  worldNode.defaultLayer()->addChild(targetNode);

  REQUIRE(sourceNode->linkSources().empty());
  REQUIRE(sourceNode->linkTargets().empty());
  REQUIRE(targetNode->linkSources().empty());
  REQUIRE(targetNode->linkTargets().empty());

  targetNode->setEntity(Entity{{{EntityPropertyKeys::Targetname, "a"}}});

  CHECK(sourceNode->linkSources().empty());
  CHECK_THAT(
    sourceNode->linkTargets(),
    Catch::UnorderedEquals(std::vector<EntityNodeBase*>{targetNode}));

  CHECK_THAT(
    targetNode->linkSources(),
    Catch::UnorderedEquals(std::vector<EntityNodeBase*>{sourceNode}));
  CHECK(targetNode->linkTargets().empty());

  sourceNode->setDefinition(nullptr);
  targetNode->setDefinition(nullptr);
}

TEST_CASE("EntityNodeLinkTest.testRemoveLinkByChangingSource")
{
  auto worldNode = WorldNode{{}, {}, MapFormat::Standard};
  auto* sourceNode = new EntityNode(Entity{{{EntityPropertyKeys::Target, "a"}}});
  auto* targetNode = new EntityNode(Entity{{{EntityPropertyKeys::Targetname, "a"}}});

  auto definition = createTestEntityDefinition();
  sourceNode->setDefinition(definition.get());
  targetNode->setDefinition(definition.get());

  worldNode.defaultLayer()->addChild(sourceNode);
  worldNode.defaultLayer()->addChild(targetNode);

  sourceNode->setEntity(Entity{{{EntityPropertyKeys::Target, "b"}}});

  CHECK(sourceNode->linkTargets().empty());
  CHECK(targetNode->linkSources().empty());

  sourceNode->setDefinition(nullptr);
  targetNode->setDefinition(nullptr);
}

TEST_CASE("EntityNodeLinkTest.testRemoveLinkByChangingTarget")
{
  auto worldNode = WorldNode{{}, {}, MapFormat::Standard};
  auto* sourceNode = new EntityNode(Entity{{{EntityPropertyKeys::Target, "a"}}});
  auto* targetNode = new EntityNode(Entity{{{EntityPropertyKeys::Targetname, "a"}}});

  auto definition = createTestEntityDefinition();
  sourceNode->setDefinition(definition.get());
  targetNode->setDefinition(definition.get());

  worldNode.defaultLayer()->addChild(sourceNode);
  worldNode.defaultLayer()->addChild(targetNode);

  targetNode->setEntity(Entity{{{EntityPropertyKeys::Targetname, "b"}}});

  CHECK(sourceNode->linkTargets().empty());
  CHECK(targetNode->linkSources().empty());

  sourceNode->setDefinition(nullptr);
  targetNode->setDefinition(nullptr);
}

TEST_CASE("EntityNodeLinkTest.testRemoveLinkByRemovingSource")
{
  auto worldNode = WorldNode{{}, {}, MapFormat::Standard};
  auto* sourceNode = new EntityNode(Entity{{{EntityPropertyKeys::Target, "a"}}});
  auto* targetNode = new EntityNode(Entity{{{EntityPropertyKeys::Targetname, "a"}}});

  auto definition = createTestEntityDefinition();
  sourceNode->setDefinition(definition.get());
  targetNode->setDefinition(definition.get());

  worldNode.defaultLayer()->addChild(sourceNode);
  worldNode.defaultLayer()->addChild(targetNode);

  worldNode.defaultLayer()->removeChild(sourceNode);

  CHECK(sourceNode->linkTargets().empty());
  CHECK(targetNode->linkSources().empty());

  sourceNode->setDefinition(nullptr);
  targetNode->setDefinition(nullptr);

  delete sourceNode;
}

TEST_CASE("EntityNodeLinkTest.testRemoveLinkByRemovingTarget")
{
  auto worldNode = WorldNode{{}, {}, MapFormat::Standard};
  auto* sourceNode = new EntityNode(Entity{{{EntityPropertyKeys::Target, "a"}}});
  auto* targetNode = new EntityNode(Entity{{{EntityPropertyKeys::Targetname, "a"}}});

  auto definition = createTestEntityDefinition();
  sourceNode->setDefinition(definition.get());
  targetNode->setDefinition(definition.get());

  worldNode.defaultLayer()->addChild(sourceNode);
  worldNode.defaultLayer()->addChild(targetNode);

  worldNode.defaultLayer()->removeChild(targetNode);

  CHECK(sourceNode->linkTargets().empty());
  CHECK(targetNode->linkSources().empty());

  sourceNode->setDefinition(nullptr);
  targetNode->setDefinition(nullptr);

  delete targetNode;
}
} // namespace TrenchBroom::Model
