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

#include "Exceptions.h"
#include "MapDocumentTest.h"
#include "TestUtils.h"
#include "io/MapHeader.h"
#include "io/TestEnvironment.h"
#include "io/WorldReader.h"
#include "mdl/BrushBuilder.h"
#include "mdl/BrushNode.h"
#include "mdl/Entity.h"
#include "mdl/EntityDefinition.h"
#include "mdl/EntityNode.h"
#include "mdl/Group.h"
#include "mdl/GroupNode.h"
#include "mdl/LayerNode.h"
#include "mdl/PatchNode.h"
#include "mdl/PropertyDefinition.h"
#include "mdl/WorldNode.h"

#include "kdl/k.h"
#include "kdl/map_utils.h"
#include "kdl/result.h"
#include "kdl/vector_utils.h"

#include <filesystem>
#include <sstream>

#include "Catch2.h"

namespace tb::ui
{

TEST_CASE_METHOD(MapDocumentTest, "MapDocumentTestFixture")
{
  SECTION("Brush Node Selection")
  {
    auto* brushNodeInDefaultLayer = createBrushNode("brushNodeInDefaultLayer");
    auto* brushNodeInCustomLayer = createBrushNode("brushNodeInCustomLayer");
    auto* brushNodeInEntity = createBrushNode("brushNodeInEntity");
    auto* brushNodeInGroup = createBrushNode("brushNodeInGroup");
    auto* brushNodeInNestedGroup = createBrushNode("brushNodeInNestedGroup");

    auto* customLayerNode = new mdl::LayerNode{mdl::Layer{"customLayerNode"}};
    auto* brushEntityNode = new mdl::EntityNode{mdl::Entity{}};
    auto* pointEntityNode = new mdl::EntityNode{mdl::Entity{}};
    auto* outerGroupNode = new mdl::GroupNode{mdl::Group{"outerGroupNode"}};
    auto* innerGroupNode = new mdl::GroupNode{mdl::Group{"outerGroupNode"}};

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

    const auto getPath = [&](const mdl::Node* node) {
      return node->pathFrom(*document->world());
    };
    const auto resolvePaths = [&](const std::vector<mdl::NodePath>& paths) {
      auto result = std::vector<mdl::Node*>{};
      for (const auto& path : paths)
      {
        result.push_back(document->world()->resolvePath(path));
      }
      return result;
    };

    SECTION("allSelectedBrushNodes")
    {
      using T = std::vector<mdl::NodePath>;

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
      const auto brushNodes = kdl::vec_static_cast<mdl::BrushNode*>(nodes);

      document->selectNodes(nodes);

      CHECK_THAT(
        document->allSelectedBrushNodes(), Catch::Matchers::UnorderedEquals(brushNodes));
    }

    SECTION("hasAnySelectedBrushNodes")
    {
      using T = std::tuple<std::vector<mdl::NodePath>, bool>;

      // clang-format off
    const auto 
    [pathsToSelect,                      expectedResult] = GENERATE_COPY(values<T>({
    {std::vector<mdl::NodePath>{},     false},
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

  SECTION("selectByLineNumber")
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
    auto* pointEntity = new mdl::EntityNode{mdl::Entity{}};
    auto* patch = createPatchNode("patch");

    auto* brushEntity = new mdl::EntityNode{mdl::Entity{}};
    auto* brushInEntity1 = createBrushNode("brushInEntity1");
    auto* brushInEntity2 = createBrushNode("brushInEntity2");

    auto* outerGroup = new mdl::GroupNode{mdl::Group{"outerGroup"}};
    auto* brushInOuterGroup = createBrushNode("brushInOuterGroup");
    auto* innerGroup = new mdl::GroupNode{mdl::Group{"innerGroup"}};
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

    const auto map = std::map<const mdl::Node*, std::string>{
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
      return kdl::vec_transform(nodes, [&](const mdl::Node* node) {
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

  SECTION("canUpdateLinkedGroups")
  {
    auto* innerGroupNode = new mdl::GroupNode{mdl::Group{"inner"}};
    auto* entityNode = new mdl::EntityNode{mdl::Entity{}};
    innerGroupNode->addChild(entityNode);

    auto* linkedInnerGroupNode = static_cast<mdl::GroupNode*>(
      innerGroupNode->cloneRecursively(document->worldBounds()));

    auto* linkedEntityNode =
      dynamic_cast<mdl::EntityNode*>(linkedInnerGroupNode->children().front());
    REQUIRE(linkedEntityNode != nullptr);

    auto* outerGroupNode = new mdl::GroupNode{mdl::Group{"outer"}};
    outerGroupNode->addChildren({innerGroupNode, linkedInnerGroupNode});

    document->addNodes({{document->parentForNodes(), {outerGroupNode}}});
    document->selectNodes({outerGroupNode});

    const auto entityNodes = document->allSelectedEntityNodes();
    REQUIRE_THAT(
      entityNodes,
      Catch::UnorderedEquals(
        std::vector<mdl::EntityNodeBase*>{entityNode, linkedEntityNode}));

    CHECK(document->canUpdateLinkedGroups({entityNode}));
    CHECK(document->canUpdateLinkedGroups({linkedEntityNode}));
    CHECK_FALSE(
      document->canUpdateLinkedGroups(kdl::vec_static_cast<mdl::Node*>(entityNodes)));
  }

  SECTION("createPointEntity")
  {
    document->selectAllNodes();
    document->deleteObjects();

    SECTION("Point entity is created and selected")
    {
      auto* entityNode =
        document->createPointEntity(m_pointEntityDef, vm::vec3d{16.0, 32.0, 48.0});
      CHECK(entityNode != nullptr);
      CHECK(entityNode->entity().definition() == m_pointEntityDef);
      CHECK(entityNode->entity().origin() == vm::vec3d{16.0, 32.0, 48.0});
      CHECK(document->selectedNodes().nodes() == std::vector<mdl::Node*>{entityNode});
    }

    SECTION("Selected objects are deselect and not translated")
    {
      auto* existingNode =
        document->createPointEntity(m_pointEntityDef, vm::vec3d{0, 0, 0});
      document->selectNodes({existingNode});

      const auto origin = existingNode->entity().origin();
      document->createPointEntity(m_pointEntityDef, {16, 16, 16});

      CHECK(existingNode->entity().origin() == origin);
    }

    SECTION("Default entity properties")
    {
      // set up a document with an entity config having setDefaultProperties set to true
      game->config().entityConfig.setDefaultProperties = true;
      document->newDocument(mdl::MapFormat::Standard, document->worldBounds(), game)
        | kdl::transform_error([](auto e) { throw std::runtime_error{e.msg}; });

      auto definitionWithDefaultsOwner = std::make_unique<mdl::PointEntityDefinition>(
        "some_name",
        Color{},
        vm::bbox3d{32.0},
        "",
        std::vector<std::shared_ptr<mdl::PropertyDefinition>>{
          std::make_shared<mdl::StringPropertyDefinition>(
            "some_default_prop", "", "", !K(readOnly), "value"),
        },
        mdl::ModelDefinition{},
        mdl::DecalDefinition{});
      auto* definitionWithDefaults = definitionWithDefaultsOwner.get();
      document->setEntityDefinitions(
        kdl::vec_from<std::unique_ptr<mdl::EntityDefinition>>(
          std::move(definitionWithDefaultsOwner)));

      auto* entityNode = document->createPointEntity(definitionWithDefaults, {0, 0, 0});
      REQUIRE(entityNode != nullptr);
      CHECK_THAT(
        entityNode->entity().properties(),
        Catch::Matchers::UnorderedEquals(std::vector<mdl::EntityProperty>{
          {mdl::EntityPropertyKeys::Classname, "some_name"},
          {"some_default_prop", "value"},
        }));
    }
  }

  SECTION("createBrushEntity")
  {
    document->selectAllNodes();
    document->deleteObjects();

    SECTION("Brush entity is created and selected")
    {
      auto* brushNode = createBrushNode("some_material");
      document->addNodes({{document->parentForNodes(), {brushNode}}});

      document->selectNodes({brushNode});
      auto* entityNode = document->createBrushEntity(m_brushEntityDef);
      CHECK(entityNode != nullptr);
      CHECK(entityNode->entity().definition() == m_brushEntityDef);
      CHECK(document->selectedNodes().nodes() == std::vector<mdl::Node*>{brushNode});
    }

    SECTION("Copies properties from existing brush entity")
    {
      auto* brushNode1 = createBrushNode("some_material");
      auto* brushNode2 = createBrushNode("some_material");
      auto* brushNode3 = createBrushNode("some_material");
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
      game->config().entityConfig.setDefaultProperties = true;
      document->newDocument(mdl::MapFormat::Standard, document->worldBounds(), game)
        | kdl::transform_error([](auto e) { throw std::runtime_error{e.msg}; });

      auto definitionWithDefaultsOwner = std::make_unique<mdl::BrushEntityDefinition>(
        "some_name",
        Color{},
        "",
        std::vector<std::shared_ptr<mdl::PropertyDefinition>>{
          std::make_shared<mdl::StringPropertyDefinition>(
            "some_default_prop", "", "", !K(readOnly), "value"),
        });
      auto* definitionWithDefaults = definitionWithDefaultsOwner.get();

      document->setEntityDefinitions(
        kdl::vec_from<std::unique_ptr<mdl::EntityDefinition>>(
          std::move(definitionWithDefaultsOwner)));

      auto* brushNode = createBrushNode("some_material");
      document->addNodes({{document->parentForNodes(), {brushNode}}});

      document->selectNodes({brushNode});
      auto* entityNode = document->createBrushEntity(definitionWithDefaults);
      REQUIRE(entityNode != nullptr);
      CHECK_THAT(
        entityNode->entity().properties(),
        Catch::Matchers::UnorderedEquals(std::vector<mdl::EntityProperty>{
          {mdl::EntityPropertyKeys::Classname, "some_name"},
          {"some_default_prop", "value"},
        }));
    }
  }

  SECTION("resetDefaultProperties")
  {
    document->selectAllNodes();
    document->deleteObjects();

    // Note: The test document does not automatically set the default properties
    auto definitionWithDefaultsOwner = std::make_unique<mdl::PointEntityDefinition>(
      "some_name",
      Color{},
      vm::bbox3d{32.0},
      "",
      std::vector<std::shared_ptr<mdl::PropertyDefinition>>{
        std::make_shared<mdl::StringPropertyDefinition>(
          "some_prop", "", "", !K(readOnly)),
        std::make_shared<mdl::StringPropertyDefinition>(
          "default_prop_a", "", "", !K(readOnly), "default_value_a"),
        std::make_shared<mdl::StringPropertyDefinition>(
          "default_prop_b", "", "", !K(readOnly), "default_value_b"),
      },
      mdl::ModelDefinition{},
      mdl::DecalDefinition{});
    auto* definitionWithDefaults = definitionWithDefaultsOwner.get();

    document->setEntityDefinitions(kdl::vec_from<std::unique_ptr<mdl::EntityDefinition>>(
      std::move(definitionWithDefaultsOwner)));

    auto* entityNodeWithoutDefinition = new mdl::EntityNode{mdl::Entity{{
      {"classname", "some_class"},
    }}};
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
      entityNodeWithPropAWithValueChanged->entity().definition()
      == definitionWithDefaults);
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
      Catch::Matchers::UnorderedEquals(std::vector<mdl::EntityProperty>{
        {"classname", "some_class"},
        {"some_prop", "some_value"},
      }));
    REQUIRE_THAT(
      entityNodeWithProp->entity().properties(),
      Catch::Matchers::UnorderedEquals(std::vector<mdl::EntityProperty>{
        {"classname", "some_name"},
        {"some_prop", "some_value"},
      }));
    REQUIRE_THAT(
      entityNodeWithPropA->entity().properties(),
      Catch::Matchers::UnorderedEquals(std::vector<mdl::EntityProperty>{
        {"classname", "some_name"},
        {"some_prop", "some_value"},
        {"default_prop_a", "default_value_a"},
      }));
    REQUIRE_THAT(
      entityNodeWithPropAWithValueChanged->entity().properties(),
      Catch::Matchers::UnorderedEquals(std::vector<mdl::EntityProperty>{
        {"classname", "some_name"},
        {"default_prop_a", "some_other_value"},
      }));
    REQUIRE_THAT(
      entityNodeWithPropsAB->entity().properties(),
      Catch::Matchers::UnorderedEquals(std::vector<mdl::EntityProperty>{
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
      document->setDefaultProperties(mdl::SetDefaultPropertyMode::SetExisting);

      CHECK_THAT(
        entityNodeWithoutDefinition->entity().properties(),
        Catch::Matchers::UnorderedEquals(std::vector<mdl::EntityProperty>{
          {"classname", "some_class"},
          {"some_prop", "some_value"},
        }));
      CHECK_THAT(
        entityNodeWithProp->entity().properties(),
        Catch::Matchers::UnorderedEquals(std::vector<mdl::EntityProperty>{
          {"classname", "some_name"},
          {"some_prop", "some_value"},
        }));
      CHECK_THAT(
        entityNodeWithPropA->entity().properties(),
        Catch::Matchers::UnorderedEquals(std::vector<mdl::EntityProperty>{
          {"classname", "some_name"},
          {"some_prop", "some_value"},
          {"default_prop_a", "default_value_a"},
        }));
      CHECK_THAT(
        entityNodeWithPropAWithValueChanged->entity().properties(),
        Catch::Matchers::UnorderedEquals(std::vector<mdl::EntityProperty>{
          {"classname", "some_name"},
          {"default_prop_a", "default_value_a"},
        }));
      CHECK_THAT(
        entityNodeWithPropsAB->entity().properties(),
        Catch::Matchers::UnorderedEquals(std::vector<mdl::EntityProperty>{
          {"classname", "some_name"},
          {"some_prop", "some_value"},
          {"default_prop_a", "default_value_a"},
          {"default_prop_b", "default_value_b"},
        }));
    }

    SECTION("Set Missing Default Properties")
    {
      document->setDefaultProperties(mdl::SetDefaultPropertyMode::SetMissing);

      CHECK_THAT(
        entityNodeWithoutDefinition->entity().properties(),
        Catch::Matchers::UnorderedEquals(std::vector<mdl::EntityProperty>{
          {"classname", "some_class"},
          {"some_prop", "some_value"},
        }));
      CHECK_THAT(
        entityNodeWithProp->entity().properties(),
        Catch::Matchers::UnorderedEquals(std::vector<mdl::EntityProperty>{
          {"classname", "some_name"},
          {"some_prop", "some_value"},
          {"default_prop_a", "default_value_a"},
          {"default_prop_b", "default_value_b"},
        }));
      CHECK_THAT(
        entityNodeWithPropA->entity().properties(),
        Catch::Matchers::UnorderedEquals(std::vector<mdl::EntityProperty>{
          {"classname", "some_name"},
          {"some_prop", "some_value"},
          {"default_prop_a", "default_value_a"},
          {"default_prop_b", "default_value_b"},
        }));
      CHECK_THAT(
        entityNodeWithPropAWithValueChanged->entity().properties(),
        Catch::Matchers::UnorderedEquals(std::vector<mdl::EntityProperty>{
          {"classname", "some_name"},
          {"default_prop_a", "some_other_value"},
          {"default_prop_b", "default_value_b"},
        }));
      CHECK_THAT(
        entityNodeWithPropsAB->entity().properties(),
        Catch::Matchers::UnorderedEquals(std::vector<mdl::EntityProperty>{
          {"classname", "some_name"},
          {"some_prop", "some_value"},
          {"default_prop_a", "default_value_a"},
          {"default_prop_b", "yet_another_value"},
        }));
    }

    SECTION("Set All Default Properties")
    {
      document->setDefaultProperties(mdl::SetDefaultPropertyMode::SetAll);

      CHECK_THAT(
        entityNodeWithoutDefinition->entity().properties(),
        Catch::Matchers::UnorderedEquals(std::vector<mdl::EntityProperty>{
          {"classname", "some_class"},
          {"some_prop", "some_value"},
        }));
      CHECK_THAT(
        entityNodeWithProp->entity().properties(),
        Catch::Matchers::UnorderedEquals(std::vector<mdl::EntityProperty>{
          {"classname", "some_name"},
          {"some_prop", "some_value"},
          {"default_prop_a", "default_value_a"},
          {"default_prop_b", "default_value_b"},
        }));
      CHECK_THAT(
        entityNodeWithPropA->entity().properties(),
        Catch::Matchers::UnorderedEquals(std::vector<mdl::EntityProperty>{
          {"classname", "some_name"},
          {"some_prop", "some_value"},
          {"default_prop_a", "default_value_a"},
          {"default_prop_b", "default_value_b"},
        }));
      CHECK_THAT(
        entityNodeWithPropAWithValueChanged->entity().properties(),
        Catch::Matchers::UnorderedEquals(std::vector<mdl::EntityProperty>{
          {"classname", "some_name"},
          {"default_prop_a", "default_value_a"},
          {"default_prop_b", "default_value_b"},
        }));
      CHECK_THAT(
        entityNodeWithPropsAB->entity().properties(),
        Catch::Matchers::UnorderedEquals(std::vector<mdl::EntityProperty>{
          {"classname", "some_name"},
          {"some_prop", "some_value"},
          {"default_prop_a", "default_value_a"},
          {"default_prop_b", "default_value_b"},
        }));
    }
  }

  SECTION("throwExceptionDuringCommand")
  {
    CHECK_THROWS_AS(document->throwExceptionDuringCommand(), CommandProcessorException);
  }
}

TEST_CASE("MapDocumentTest")
{
  SECTION("saveDocumentTo")
  {
    SECTION("Writing map header")
    {
      auto [document, game, gameConfig, taskManager] = ui::loadMapDocument(
        "fixture/test/ui/MapDocumentTest/valveFormatMapWithoutFormatTag.map",
        "Quake",
        mdl::MapFormat::Unknown);
      REQUIRE(document->world()->mapFormat() == mdl::MapFormat::Valve);

      auto env = io::TestEnvironment{};

      const auto newDocumentPath = std::filesystem::path{"test.map"};
      document->saveDocumentTo(env.dir() / newDocumentPath);
      REQUIRE(env.fileExists(newDocumentPath));

      const auto newDocumentContent = env.loadFile(newDocumentPath);
      auto istr = std::istringstream{newDocumentContent};

      CHECK(
        io::readMapHeader(istr)
        == Result<std::pair<std::optional<std::string>, mdl::MapFormat>>{
          std::pair{"Quake", mdl::MapFormat::Valve}});
    }
  }

  SECTION("loadDocument")
  {
    SECTION("Format detection")
    {
      SECTION("detectValveFormatMap")
      {
        auto [document, game, gameConfig, taskManager] = ui::loadMapDocument(
          "fixture/test/ui/MapDocumentTest/valveFormatMapWithoutFormatTag.map",
          "Quake",
          mdl::MapFormat::Unknown);
        CHECK(document->world()->mapFormat() == mdl::MapFormat::Valve);
        CHECK(document->world()->defaultLayer()->childCount() == 1);
      }

      SECTION("detectStandardFormatMap")
      {
        auto [document, game, gameConfig, taskManager] = ui::loadMapDocument(
          "fixture/test/ui/MapDocumentTest/standardFormatMapWithoutFormatTag.map",
          "Quake",
          mdl::MapFormat::Unknown);
        CHECK(document->world()->mapFormat() == mdl::MapFormat::Standard);
        CHECK(document->world()->defaultLayer()->childCount() == 1);
      }

      SECTION("detectEmptyMap")
      {
        auto [document, game, gameConfig, taskManager] = ui::loadMapDocument(
          "fixture/test/ui/MapDocumentTest/emptyMapWithoutFormatTag.map",
          "Quake",
          mdl::MapFormat::Unknown);
        // an empty map detects as Valve because Valve is listed first in the Quake game
        // config
        CHECK(document->world()->mapFormat() == mdl::MapFormat::Valve);
        CHECK(document->world()->defaultLayer()->childCount() == 0);
      }

      SECTION("mixedFormats")
      {
        // map has both Standard and Valve brushes
        CHECK_THROWS_AS(
          ui::loadMapDocument(
            "fixture/test/ui/MapDocumentTest/mixedFormats.map",
            "Quake",
            mdl::MapFormat::Unknown),
          io::WorldReaderException);
      }
    }
  }

  SECTION("exportDocumentAs")
  {
    auto env = io::TestEnvironment{};

    SECTION("omit layers from export")
    {
      const auto newDocumentPath = std::filesystem::path{"test.map"};

      {
        auto [document, game, gameConfig, taskManager] =
          ui::newMapDocument("Quake", mdl::MapFormat::Valve);

        auto layer = mdl::Layer{"Layer"};
        layer.setOmitFromExport(true);

        auto* layerNode = new mdl::LayerNode{std::move(layer)};
        document->addNodes({{document->world(), {layerNode}}});

        REQUIRE(
          document->exportDocumentAs(io::MapExportOptions{env.dir() / newDocumentPath})
            .is_success());
        REQUIRE(env.fileExists(newDocumentPath));
      }

      auto [document, game, gameConfig, taskManager] =
        ui::loadMapDocument(env.dir() / newDocumentPath, "Quake", mdl::MapFormat::Valve);
      CHECK(document->world()->customLayers().empty());
    }
  }

  SECTION("reloadMaterialCollections")
  {
    auto [document, game, gameConfig, taskManager] = ui::loadMapDocument(
      "fixture/test/ui/MapDocumentTest/reloadMaterialCollectionsQ2.map",
      "Quake2",
      mdl::MapFormat::Quake2);


    const auto faces = kdl::vec_transform(
      document->world()->defaultLayer()->children(), [&](const auto* node) {
        const auto* brushNode = dynamic_cast<const mdl::BrushNode*>(node);
        REQUIRE(brushNode);
        return &brushNode->brush().faces().front();
      });

    REQUIRE(faces.size() == 4);
    REQUIRE(
      kdl::vec_transform(
        faces, [](const auto* face) { return face->attributes().materialName(); })
      == std::vector<std::string>{
        "b_pv_v1a1", "e1m1/b_pv_v1a2", "e1m1/f1/b_rc_v4", "lavatest"});

    REQUIRE(
      kdl::none_of(faces, [](const auto* face) { return face->material() == nullptr; }));

    CHECK_NOTHROW(document->reloadMaterialCollections());

    REQUIRE(
      kdl::none_of(faces, [](const auto* face) { return face->material() == nullptr; }));
  }
}

} // namespace tb::ui
