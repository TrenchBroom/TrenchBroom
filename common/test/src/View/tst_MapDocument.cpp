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

#include "MapDocumentTest.h"
#include "TestUtils.h"

#include "Assets/EntityDefinition.h"
#include "Assets/PropertyDefinition.h"
#include "Exceptions.h"
#include "IO/WorldReader.h"
#include "Model/BrushBuilder.h"
#include "Model/BrushNode.h"
#include "Model/Entity.h"
#include "Model/EntityNode.h"
#include "Model/Group.h"
#include "Model/GroupNode.h"
#include "Model/LayerNode.h"
#include "Model/PatchNode.h"
#include "Model/TestGame.h"
#include "Model/WorldNode.h"
#include "View/MapDocumentCommandFacade.h"

#include "kdl/map_utils.h"
#include <kdl/result.h>
#include <kdl/vector_utils.h>

#include <filesystem>

#include "Catch2.h"

namespace TrenchBroom
{
namespace View
{
MapDocumentTest::MapDocumentTest()
  : MapDocumentTest(Model::MapFormat::Standard)
{
}

MapDocumentTest::MapDocumentTest(const Model::MapFormat mapFormat)
  : m_mapFormat(mapFormat)
  , m_pointEntityDef(nullptr)
  , m_brushEntityDef(nullptr)
{
  SetUp();
}

void MapDocumentTest::SetUp()
{
  game = std::make_shared<Model::TestGame>();
  document = MapDocumentCommandFacade::newMapDocument();
  document->newDocument(m_mapFormat, vm::bbox3(8192.0), game);

  // create two entity definitions
  m_pointEntityDef = new Assets::PointEntityDefinition(
    "point_entity", Color(), vm::bbox3(16.0), "this is a point entity", {}, {});
  m_brushEntityDef = new Assets::BrushEntityDefinition(
    "brush_entity", Color(), "this is a brush entity", {});

  document->setEntityDefinitions(
    std::vector<Assets::EntityDefinition*>{m_pointEntityDef, m_brushEntityDef});
}

MapDocumentTest::~MapDocumentTest()
{
  m_pointEntityDef = nullptr;
  m_brushEntityDef = nullptr;
}

Model::BrushNode* MapDocumentTest::createBrushNode(
  const std::string& textureName,
  const std::function<void(Model::Brush&)>& brushFunc) const
{
  const Model::WorldNode* world = document->world();
  Model::BrushBuilder builder(
    world->mapFormat(), document->worldBounds(), document->game()->defaultFaceAttribs());
  Model::Brush brush = builder.createCube(32.0, textureName).value();
  brushFunc(brush);
  return new Model::BrushNode(std::move(brush));
}

Model::PatchNode* MapDocumentTest::createPatchNode(const std::string& textureName) const
{
  // clang-format off
  return new Model::PatchNode{Model::BezierPatch{3, 3, {
    {0, 0, 0}, {1, 0, 1}, {2, 0, 0},
    {0, 1, 1}, {1, 1, 2}, {2, 1, 1},
    {0, 2, 0}, {1, 2, 1}, {2, 2, 0} }, textureName}};
  // clang-format on
}

ValveMapDocumentTest::ValveMapDocumentTest()
  : MapDocumentTest(Model::MapFormat::Valve)
{
}

Quake3MapDocumentTest::Quake3MapDocumentTest()
  : MapDocumentTest{Model::MapFormat::Quake3}
{
}

TEST_CASE_METHOD(MapDocumentTest, "MapDocumentTest.throwExceptionDuringCommand")
{
  CHECK_THROWS_AS(document->throwExceptionDuringCommand(), CommandProcessorException);
}

TEST_CASE("MapDocumentTest.detectValveFormatMap")
{
  auto [document, game, gameConfig] = View::loadMapDocument(
    "fixture/test/View/MapDocumentTest/valveFormatMapWithoutFormatTag.map",
    "Quake",
    Model::MapFormat::Unknown);
  CHECK(document->world()->mapFormat() == Model::MapFormat::Valve);
  CHECK(document->world()->defaultLayer()->childCount() == 1);
}

TEST_CASE("MapDocumentTest.detectStandardFormatMap")
{
  auto [document, game, gameConfig] = View::loadMapDocument(
    "fixture/test/View/MapDocumentTest/standardFormatMapWithoutFormatTag.map",
    "Quake",
    Model::MapFormat::Unknown);
  CHECK(document->world()->mapFormat() == Model::MapFormat::Standard);
  CHECK(document->world()->defaultLayer()->childCount() == 1);
}

TEST_CASE("MapDocumentTest.detectEmptyMap")
{
  auto [document, game, gameConfig] = View::loadMapDocument(
    "fixture/test/View/MapDocumentTest/emptyMapWithoutFormatTag.map",
    "Quake",
    Model::MapFormat::Unknown);
  // an empty map detects as Valve because Valve is listed first in the Quake game config
  CHECK(document->world()->mapFormat() == Model::MapFormat::Valve);
  CHECK(document->world()->defaultLayer()->childCount() == 0);
}

TEST_CASE("MapDocumentTest.mixedFormats")
{
  // map has both Standard and Valve brushes
  CHECK_THROWS_AS(
    View::loadMapDocument(
      "fixture/test/View/MapDocumentTest/mixedFormats.map",
      "Quake",
      Model::MapFormat::Unknown),
    IO::WorldReaderException);
}

TEST_CASE_METHOD(MapDocumentTest, "Brush Node Selection")
{
  auto* brushNodeInDefaultLayer = createBrushNode("brushNodeInDefaultLayer");
  auto* brushNodeInCustomLayer = createBrushNode("brushNodeInCustomLayer");
  auto* brushNodeInEntity = createBrushNode("brushNodeInEntity");
  auto* brushNodeInGroup = createBrushNode("brushNodeInGroup");
  auto* brushNodeInNestedGroup = createBrushNode("brushNodeInNestedGroup");

  auto* customLayerNode = new Model::LayerNode{Model::Layer{"customLayerNode"}};
  auto* brushEntityNode = new Model::EntityNode{Model::Entity{}};
  auto* pointEntityNode = new Model::EntityNode{Model::Entity{}};
  auto* outerGroupNode = new Model::GroupNode{Model::Group{"outerGroupNode"}};
  auto* innerGroupNode = new Model::GroupNode{Model::Group{"outerGroupNode"}};

  document->addNodes(
    {{document->world()->defaultLayer(),
      {brushNodeInDefaultLayer, brushEntityNode, pointEntityNode, outerGroupNode}},
     {document->world(), {customLayerNode}}});

  document->addNodes({
    {customLayerNode, {brushNodeInCustomLayer}},
    {outerGroupNode, {innerGroupNode, brushNodeInGroup}},
    {brushEntityNode, {brushNodeInEntity}},
  });

  document->addNodes({{innerGroupNode, {brushNodeInNestedGroup}}});

  const auto getPath = [&](const Model::Node* node) {
    return node->pathFrom(*document->world());
  };
  const auto resolvePaths = [&](const std::vector<Model::NodePath>& paths) {
    auto result = std::vector<Model::Node*>{};
    for (const auto& path : paths)
    {
      result.push_back(document->world()->resolvePath(path));
    }
    return result;
  };

  SECTION("allSelectedBrushNodes")
  {
    using T = std::vector<Model::NodePath>;

    // clang-format off
    const auto 
    paths = GENERATE_COPY(values<T>({
    {},
    {getPath(brushNodeInDefaultLayer)},
    {getPath(brushNodeInDefaultLayer), getPath(brushNodeInCustomLayer)},
    {getPath(brushNodeInDefaultLayer), getPath(brushNodeInCustomLayer), getPath(brushNodeInEntity)},
    {getPath(brushNodeInGroup)},
    {getPath(brushNodeInGroup), getPath(brushNodeInNestedGroup)},
    }));
    // clang-format on

    const auto nodes = resolvePaths(paths);
    const auto brushNodes = kdl::vec_element_cast<Model::BrushNode*>(nodes);

    document->selectNodes(nodes);

    CHECK_THAT(
      document->allSelectedBrushNodes(), Catch::Matchers::UnorderedEquals(brushNodes));
  }

  SECTION("hasAnySelectedBrushNodes")
  {
    using T = std::tuple<std::vector<Model::NodePath>, bool>;

    // clang-format off
    const auto 
    [pathsToSelect,                      expectedResult] = GENERATE_COPY(values<T>({
    {std::vector<Model::NodePath>{},     false},
    {{getPath(pointEntityNode)},         false},
    {{getPath(brushEntityNode)},         true},
    {{getPath(outerGroupNode)},          true},
    {{getPath(brushNodeInDefaultLayer)}, true},
    {{getPath(brushNodeInCustomLayer)},  true},
    {{getPath(brushNodeInEntity)},       true},
    {{getPath(brushNodeInGroup)},        true},
    {{getPath(brushNodeInNestedGroup)},  true},
    }));
    // clang-format on

    CAPTURE(pathsToSelect);

    const auto nodes = resolvePaths(pathsToSelect);
    document->selectNodes(nodes);

    CHECK(document->hasAnySelectedBrushNodes() == expectedResult);
  }
}

TEST_CASE_METHOD(MapDocumentTest, "selectByLineNumber")
{
  /*
  - defaultLayer
    - brush                    4,  5
    - pointEntity             10, 15
    - patch                   16, 20
    - brushEntity             20, 30
      - brushInEntity1        23, 25
      - brushInEntity2        26, 29
    - outerGroup              31, 50
      - brushInOuterGroup     32, 38
      - innerGroup            39, 49
        - brushInInnerGroup   43, 48
  */

  auto* brush = createBrushNode("brush");
  auto* pointEntity = new Model::EntityNode{Model::Entity{}};
  auto* patch = createPatchNode("patch");

  auto* brushEntity = new Model::EntityNode{Model::Entity{}};
  auto* brushInEntity1 = createBrushNode("brushInEntity1");
  auto* brushInEntity2 = createBrushNode("brushInEntity2");

  auto* outerGroup = new Model::GroupNode{Model::Group{"outerGroup"}};
  auto* brushInOuterGroup = createBrushNode("brushInOuterGroup");
  auto* innerGroup = new Model::GroupNode{Model::Group{"innerGroup"}};
  auto* brushInInnerGroup = createBrushNode("brushInInnerGroup");

  brush->setFilePosition(4, 2);
  pointEntity->setFilePosition(10, 5);
  patch->setFilePosition(16, 4);
  brushEntity->setFilePosition(20, 10);
  brushInEntity1->setFilePosition(23, 2);
  brushInEntity2->setFilePosition(26, 3);
  outerGroup->setFilePosition(31, 19);
  brushInOuterGroup->setFilePosition(32, 6);
  innerGroup->setFilePosition(39, 10);
  brushInInnerGroup->setFilePosition(43, 5);

  const auto map = std::map<const Model::Node*, std::string>{
    {brush, "brush"},
    {pointEntity, "pointEntity"},
    {patch, "patch"},
    {brushEntity, "brushEntity"},
    {brushInEntity1, "brushInEntity1"},
    {brushInEntity2, "brushInEntity2"},
    {outerGroup, "outerGroup"},
    {brushInOuterGroup, "brushInOuterGroup"},
    {innerGroup, "innerGroup"},
    {brushInInnerGroup, "brushInInnerGroup"},
  };

  const auto mapNodeNames = [&](const auto& nodes) {
    return kdl::vec_transform(nodes, [&](const Model::Node* node) {
      return kdl::map_find_or_default(map, node, std::string{"<unknown>"});
    });
  };

  document->addNodes({
    {document->world()->defaultLayer(),
     {brush, pointEntity, patch, brushEntity, outerGroup}},
  });

  document->addNodes({
    {brushEntity, {brushInEntity1, brushInEntity2}},
    {outerGroup, {brushInOuterGroup, innerGroup}},
  });

  document->addNodes({{innerGroup, {brushInInnerGroup}}});

  document->deselectAll();

  using T = std::tuple<std::vector<size_t>, std::vector<std::string>>;

  SECTION("outer group is closed")
  {
    const auto [lineNumbers, expectedNodeNames] = GENERATE(values<T>({
      {{0}, {}},
      {{4}, {"brush"}},
      {{5}, {"brush"}},
      {{4, 5}, {"brush"}},
      {{6}, {}},
      {{7}, {}},
      {{12}, {"pointEntity"}},
      {{16}, {"patch"}},
      {{20}, {"brushInEntity1", "brushInEntity2"}},
      {{24}, {"brushInEntity1"}},
      {{26}, {"brushInEntity2"}},
      {{31}, {"outerGroup"}},
      {{32}, {"outerGroup"}},
      {{39}, {"outerGroup"}},
      {{43}, {"outerGroup"}},
      {{0, 4, 12, 24, 32}, {"brush", "pointEntity", "brushInEntity1", "outerGroup"}},
    }));

    CAPTURE(lineNumbers);

    document->selectNodesWithFilePosition(lineNumbers);
    CHECK_THAT(
      mapNodeNames(document->selectedNodes().nodes()),
      Catch::Matchers::UnorderedEquals(expectedNodeNames));
  }

  SECTION("outer group is open")
  {
    document->openGroup(outerGroup);

    const auto [lineNumbers, expectedNodeNames] = GENERATE(values<T>({
      {{31}, {}},
      {{32}, {"brushInOuterGroup"}},
      {{39}, {"innerGroup"}},
      {{43}, {"innerGroup"}},
    }));

    CAPTURE(lineNumbers);

    document->selectNodesWithFilePosition(lineNumbers);
    CHECK_THAT(
      mapNodeNames(document->selectedNodes().nodes()),
      Catch::Matchers::UnorderedEquals(expectedNodeNames));
  }

  SECTION("inner group is open")
  {
    document->openGroup(outerGroup);
    document->openGroup(innerGroup);

    const auto [lineNumbers, expectedNodeNames] = GENERATE(values<T>({
      {{31}, {}},
      {{32}, {}},
      {{39}, {}},
      {{43}, {"brushInInnerGroup"}},
    }));

    CAPTURE(lineNumbers);

    document->selectNodesWithFilePosition(lineNumbers);
    CHECK_THAT(
      mapNodeNames(document->selectedNodes().nodes()),
      Catch::Matchers::UnorderedEquals(expectedNodeNames));
  }
}

TEST_CASE_METHOD(MapDocumentTest, "canUpdateLinkedGroups")
{
  auto* innerGroupNode = new Model::GroupNode{Model::Group{"inner"}};
  setLinkedGroupId(*innerGroupNode, "asdf");

  auto* entityNode = new Model::EntityNode{Model::Entity{}};
  innerGroupNode->addChild(entityNode);

  auto* linkedInnerGroupNode = static_cast<Model::GroupNode*>(
    innerGroupNode->cloneRecursively(document->worldBounds()));

  auto* linkedEntityNode =
    dynamic_cast<Model::EntityNode*>(linkedInnerGroupNode->children().front());
  REQUIRE(linkedEntityNode != nullptr);

  auto* outerGroupNode = new Model::GroupNode{Model::Group{"outer"}};
  outerGroupNode->addChildren({innerGroupNode, linkedInnerGroupNode});

  document->addNodes({{document->parentForNodes(), {outerGroupNode}}});
  document->selectNodes({outerGroupNode});

  const auto entityNodes = document->allSelectedEntityNodes();
  REQUIRE_THAT(
    entityNodes,
    Catch::UnorderedEquals(
      std::vector<Model::EntityNodeBase*>{entityNode, linkedEntityNode}));

  CHECK(document->canUpdateLinkedGroups({entityNode}));
  CHECK(document->canUpdateLinkedGroups({linkedEntityNode}));
  CHECK_FALSE(
    document->canUpdateLinkedGroups(kdl::vec_element_cast<Model::Node*>(entityNodes)));
}

TEST_CASE_METHOD(MapDocumentTest, "createPointEntity")
{
  document->selectAllNodes();
  document->deleteObjects();

  SECTION("Point entity is created and selected")
  {
    auto* entityNode =
      document->createPointEntity(m_pointEntityDef, vm::vec3{16.0, 32.0, 48.0});
    CHECK(entityNode != nullptr);
    CHECK(entityNode->entity().definition() == m_pointEntityDef);
    CHECK(entityNode->entity().origin() == vm::vec3{16.0, 32.0, 48.0});
    CHECK(document->selectedNodes().nodes() == std::vector<Model::Node*>{entityNode});
  }

  SECTION("Selected objects are deselect and not translated")
  {
    auto* existingNode = document->createPointEntity(m_pointEntityDef, vm::vec3::zero());
    document->selectNodes({existingNode});

    const auto origin = existingNode->entity().origin();
    document->createPointEntity(m_pointEntityDef, {16, 16, 16});

    CHECK(existingNode->entity().origin() == origin);
  }

  SECTION("Default entity properties")
  {
    // set up a document with an entity config having setDefaultProperties set to true
    game->setWorldNodeToLoad(std::make_unique<Model::WorldNode>(
      Model::EntityPropertyConfig{{}, true(setDefaultProperties)},
      Model::Entity{},
      Model::MapFormat::Standard));
    document->loadDocument(Model::MapFormat::Standard, document->worldBounds(), game, "");

    auto* definitionWithDefaults = new Assets::PointEntityDefinition{
      "some_name",
      Color{},
      vm::bbox3{32.0},
      "",
      {
        std::make_shared<Assets::StringPropertyDefinition>(
          "some_default_prop", "", "", !true(readOnly), "value"),
      },
      {}};
    document->setEntityDefinitions({definitionWithDefaults});

    auto* entityNode = document->createPointEntity(definitionWithDefaults, {0, 0, 0});
    REQUIRE(entityNode != nullptr);
    CHECK_THAT(
      entityNode->entity().properties(),
      Catch::Matchers::UnorderedEquals(std::vector<Model::EntityProperty>{
        {Model::EntityPropertyKeys::Classname, "some_name"},
        {"some_default_prop", "value"},
      }));
  }
}

TEST_CASE_METHOD(MapDocumentTest, "createBrushEntity")
{
  document->selectAllNodes();
  document->deleteObjects();

  SECTION("Brush entity is created and selected")
  {
    auto* brushNode = createBrushNode("some_texture");
    document->addNodes({{document->parentForNodes(), {brushNode}}});

    document->selectNodes({brushNode});
    auto* entityNode = document->createBrushEntity(m_brushEntityDef);
    CHECK(entityNode != nullptr);
    CHECK(entityNode->entity().definition() == m_brushEntityDef);
    CHECK(document->selectedNodes().nodes() == std::vector<Model::Node*>{brushNode});
  }

  SECTION("Copies properties from existing brush entity")
  {
    auto* brushNode1 = createBrushNode("some_texture");
    auto* brushNode2 = createBrushNode("some_texture");
    auto* brushNode3 = createBrushNode("some_texture");
    document->addNodes(
      {{document->parentForNodes(), {brushNode1, brushNode2, brushNode3}}});

    document->selectNodes({brushNode1, brushNode2, brushNode3});
    auto* previousEntityNode = document->createBrushEntity(m_brushEntityDef);

    document->setProperty("prop", "value");
    REQUIRE(previousEntityNode->entity().hasProperty("prop", "value"));

    document->deselectAll();
    document->selectNodes({brushNode1, brushNode2});

    auto* newEntityNode = document->createBrushEntity(m_brushEntityDef);
    CHECK(newEntityNode != nullptr);
    CHECK(newEntityNode->entity().hasProperty("prop", "value"));
  }

  SECTION("Default entity properties")
  {
    // set up a document with an entity config having setDefaultProperties set to true
    game->setWorldNodeToLoad(std::make_unique<Model::WorldNode>(
      Model::EntityPropertyConfig{{}, true(setDefaultProperties)},
      Model::Entity{},
      Model::MapFormat::Standard));
    document->loadDocument(Model::MapFormat::Standard, document->worldBounds(), game, "");

    auto* definitionWithDefaults = new Assets::BrushEntityDefinition{
      "some_name",
      Color{},
      "",
      {
        std::make_shared<Assets::StringPropertyDefinition>(
          "some_default_prop", "", "", !true(readOnly), "value"),
      }};
    document->setEntityDefinitions({definitionWithDefaults});

    auto* brushNode = createBrushNode("some_texture");
    document->addNodes({{document->parentForNodes(), {brushNode}}});

    document->selectNodes({brushNode});
    auto* entityNode = document->createBrushEntity(definitionWithDefaults);
    REQUIRE(entityNode != nullptr);
    CHECK_THAT(
      entityNode->entity().properties(),
      Catch::Matchers::UnorderedEquals(std::vector<Model::EntityProperty>{
        {Model::EntityPropertyKeys::Classname, "some_name"},
        {"some_default_prop", "value"},
      }));
  }
}

TEST_CASE_METHOD(MapDocumentTest, "resetDefaultProperties")
{
  document->selectAllNodes();
  document->deleteObjects();

  // Note: The test document does not automatically set the default properties
  auto* definitionWithDefaults = new Assets::PointEntityDefinition{
    "some_name",
    Color{},
    vm::bbox3{32.0},
    "",
    {
      std::make_shared<Assets::StringPropertyDefinition>(
        "some_prop", "", "", !true(readOnly)),
      std::make_shared<Assets::StringPropertyDefinition>(
        "default_prop_a", "", "", !true(readOnly), "default_value_a"),
      std::make_shared<Assets::StringPropertyDefinition>(
        "default_prop_b", "", "", !true(readOnly), "default_value_b"),
    },
    {}};
  document->setEntityDefinitions({definitionWithDefaults});

  auto* entityNodeWithoutDefinition = new Model::EntityNode{
    document->world()->entityPropertyConfig(),
    {
      {"classname", "some_class"},
    }};
  document->addNodes({{document->parentForNodes(), {entityNodeWithoutDefinition}}});
  document->selectNodes({entityNodeWithoutDefinition});
  document->setProperty("some_prop", "some_value");
  document->deselectAll();

  auto* entityNodeWithProp =
    document->createPointEntity(definitionWithDefaults, {0, 0, 0});
  REQUIRE(entityNodeWithProp != nullptr);
  REQUIRE(entityNodeWithProp->entity().definition() == definitionWithDefaults);
  document->selectNodes({entityNodeWithProp});
  document->setProperty("some_prop", "some_value");
  document->deselectAll();

  auto* entityNodeWithPropA =
    document->createPointEntity(definitionWithDefaults, {0, 0, 0});
  REQUIRE(entityNodeWithPropA != nullptr);
  REQUIRE(entityNodeWithPropA->entity().definition() == definitionWithDefaults);
  document->selectNodes({entityNodeWithPropA});
  document->setProperty("some_prop", "some_value");
  document->setProperty("default_prop_a", "default_value_a");
  document->deselectAll();

  auto* entityNodeWithPropAWithValueChanged =
    document->createPointEntity(definitionWithDefaults, {0, 0, 0});
  REQUIRE(entityNodeWithPropAWithValueChanged != nullptr);
  REQUIRE(
    entityNodeWithPropAWithValueChanged->entity().definition() == definitionWithDefaults);
  document->selectNodes({entityNodeWithPropAWithValueChanged});
  document->setProperty("default_prop_a", "some_other_value");
  document->deselectAll();

  auto* entityNodeWithPropsAB =
    document->createPointEntity(definitionWithDefaults, {0, 0, 0});
  REQUIRE(entityNodeWithPropsAB != nullptr);
  REQUIRE(entityNodeWithPropsAB->entity().definition() == definitionWithDefaults);
  document->selectNodes({entityNodeWithPropsAB});
  document->setProperty("some_prop", "some_value");
  document->setProperty("default_prop_a", "default_value_a");
  document->setProperty("default_prop_b", "yet_another_value");
  document->deselectAll();

  REQUIRE_THAT(
    entityNodeWithoutDefinition->entity().properties(),
    Catch::Matchers::UnorderedEquals(std::vector<Model::EntityProperty>{
      {"classname", "some_class"},
      {"some_prop", "some_value"},
    }));
  REQUIRE_THAT(
    entityNodeWithProp->entity().properties(),
    Catch::Matchers::UnorderedEquals(std::vector<Model::EntityProperty>{
      {"classname", "some_name"},
      {"some_prop", "some_value"},
    }));
  REQUIRE_THAT(
    entityNodeWithPropA->entity().properties(),
    Catch::Matchers::UnorderedEquals(std::vector<Model::EntityProperty>{
      {"classname", "some_name"},
      {"some_prop", "some_value"},
      {"default_prop_a", "default_value_a"},
    }));
  REQUIRE_THAT(
    entityNodeWithPropAWithValueChanged->entity().properties(),
    Catch::Matchers::UnorderedEquals(std::vector<Model::EntityProperty>{
      {"classname", "some_name"},
      {"default_prop_a", "some_other_value"},
    }));
  REQUIRE_THAT(
    entityNodeWithPropsAB->entity().properties(),
    Catch::Matchers::UnorderedEquals(std::vector<Model::EntityProperty>{
      {"classname", "some_name"},
      {"some_prop", "some_value"},
      {"default_prop_a", "default_value_a"},
      {"default_prop_b", "yet_another_value"},
    }));

  document->selectNodes(
    {entityNodeWithoutDefinition,
     entityNodeWithProp,
     entityNodeWithPropA,
     entityNodeWithPropAWithValueChanged,
     entityNodeWithPropsAB});

  SECTION("Set Existing Default Properties")
  {
    document->setDefaultProperties(Model::SetDefaultPropertyMode::SetExisting);

    CHECK_THAT(
      entityNodeWithoutDefinition->entity().properties(),
      Catch::Matchers::UnorderedEquals(std::vector<Model::EntityProperty>{
        {"classname", "some_class"},
        {"some_prop", "some_value"},
      }));
    CHECK_THAT(
      entityNodeWithProp->entity().properties(),
      Catch::Matchers::UnorderedEquals(std::vector<Model::EntityProperty>{
        {"classname", "some_name"},
        {"some_prop", "some_value"},
      }));
    CHECK_THAT(
      entityNodeWithPropA->entity().properties(),
      Catch::Matchers::UnorderedEquals(std::vector<Model::EntityProperty>{
        {"classname", "some_name"},
        {"some_prop", "some_value"},
        {"default_prop_a", "default_value_a"},
      }));
    CHECK_THAT(
      entityNodeWithPropAWithValueChanged->entity().properties(),
      Catch::Matchers::UnorderedEquals(std::vector<Model::EntityProperty>{
        {"classname", "some_name"},
        {"default_prop_a", "default_value_a"},
      }));
    CHECK_THAT(
      entityNodeWithPropsAB->entity().properties(),
      Catch::Matchers::UnorderedEquals(std::vector<Model::EntityProperty>{
        {"classname", "some_name"},
        {"some_prop", "some_value"},
        {"default_prop_a", "default_value_a"},
        {"default_prop_b", "default_value_b"},
      }));
  }

  SECTION("Set Missing Default Properties")
  {
    document->setDefaultProperties(Model::SetDefaultPropertyMode::SetMissing);

    CHECK_THAT(
      entityNodeWithoutDefinition->entity().properties(),
      Catch::Matchers::UnorderedEquals(std::vector<Model::EntityProperty>{
        {"classname", "some_class"},
        {"some_prop", "some_value"},
      }));
    CHECK_THAT(
      entityNodeWithProp->entity().properties(),
      Catch::Matchers::UnorderedEquals(std::vector<Model::EntityProperty>{
        {"classname", "some_name"},
        {"some_prop", "some_value"},
        {"default_prop_a", "default_value_a"},
        {"default_prop_b", "default_value_b"},
      }));
    CHECK_THAT(
      entityNodeWithPropA->entity().properties(),
      Catch::Matchers::UnorderedEquals(std::vector<Model::EntityProperty>{
        {"classname", "some_name"},
        {"some_prop", "some_value"},
        {"default_prop_a", "default_value_a"},
        {"default_prop_b", "default_value_b"},
      }));
    CHECK_THAT(
      entityNodeWithPropAWithValueChanged->entity().properties(),
      Catch::Matchers::UnorderedEquals(std::vector<Model::EntityProperty>{
        {"classname", "some_name"},
        {"default_prop_a", "some_other_value"},
        {"default_prop_b", "default_value_b"},
      }));
    CHECK_THAT(
      entityNodeWithPropsAB->entity().properties(),
      Catch::Matchers::UnorderedEquals(std::vector<Model::EntityProperty>{
        {"classname", "some_name"},
        {"some_prop", "some_value"},
        {"default_prop_a", "default_value_a"},
        {"default_prop_b", "yet_another_value"},
      }));
  }

  SECTION("Set All Default Properties")
  {
    document->setDefaultProperties(Model::SetDefaultPropertyMode::SetAll);

    CHECK_THAT(
      entityNodeWithoutDefinition->entity().properties(),
      Catch::Matchers::UnorderedEquals(std::vector<Model::EntityProperty>{
        {"classname", "some_class"},
        {"some_prop", "some_value"},
      }));
    CHECK_THAT(
      entityNodeWithProp->entity().properties(),
      Catch::Matchers::UnorderedEquals(std::vector<Model::EntityProperty>{
        {"classname", "some_name"},
        {"some_prop", "some_value"},
        {"default_prop_a", "default_value_a"},
        {"default_prop_b", "default_value_b"},
      }));
    CHECK_THAT(
      entityNodeWithPropA->entity().properties(),
      Catch::Matchers::UnorderedEquals(std::vector<Model::EntityProperty>{
        {"classname", "some_name"},
        {"some_prop", "some_value"},
        {"default_prop_a", "default_value_a"},
        {"default_prop_b", "default_value_b"},
      }));
    CHECK_THAT(
      entityNodeWithPropAWithValueChanged->entity().properties(),
      Catch::Matchers::UnorderedEquals(std::vector<Model::EntityProperty>{
        {"classname", "some_name"},
        {"default_prop_a", "default_value_a"},
        {"default_prop_b", "default_value_b"},
      }));
    CHECK_THAT(
      entityNodeWithPropsAB->entity().properties(),
      Catch::Matchers::UnorderedEquals(std::vector<Model::EntityProperty>{
        {"classname", "some_name"},
        {"some_prop", "some_value"},
        {"default_prop_a", "default_value_a"},
        {"default_prop_b", "default_value_b"},
      }));
  }
}

} // namespace View
} // namespace TrenchBroom
