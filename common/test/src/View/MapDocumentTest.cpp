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

#include <kdl/result.h>
#include <kdl/vector_utils.h>

#include "Catch2.h"

namespace TrenchBroom {
namespace View {
MapDocumentTest::MapDocumentTest()
  : MapDocumentTest(Model::MapFormat::Standard) {}

MapDocumentTest::MapDocumentTest(const Model::MapFormat mapFormat)
  : m_mapFormat(mapFormat)
  , m_pointEntityDef(nullptr)
  , m_brushEntityDef(nullptr) {
  SetUp();
}

void MapDocumentTest::SetUp() {
  game = std::make_shared<Model::TestGame>();
  document = MapDocumentCommandFacade::newMapDocument();
  document->newDocument(m_mapFormat, vm::bbox3(8192.0), game);

  // create two entity definitions
  m_pointEntityDef = new Assets::PointEntityDefinition(
    "point_entity", Color(), vm::bbox3(16.0), "this is a point entity", {}, {});
  m_brushEntityDef =
    new Assets::BrushEntityDefinition("brush_entity", Color(), "this is a brush entity", {});

  document->setEntityDefinitions(
    std::vector<Assets::EntityDefinition*>{m_pointEntityDef, m_brushEntityDef});
}

MapDocumentTest::~MapDocumentTest() {
  m_pointEntityDef = nullptr;
  m_brushEntityDef = nullptr;
}

Model::BrushNode* MapDocumentTest::createBrushNode(
  const std::string& textureName, const std::function<void(Model::Brush&)>& brushFunc) const {
  const Model::WorldNode* world = document->world();
  Model::BrushBuilder builder(
    world->mapFormat(), document->worldBounds(), document->game()->defaultFaceAttribs());
  Model::Brush brush = builder.createCube(32.0, textureName).value();
  brushFunc(brush);
  return new Model::BrushNode(std::move(brush));
}

Model::PatchNode* MapDocumentTest::createPatchNode(const std::string& textureName) const {
  // clang-format off
  return new Model::PatchNode{Model::BezierPatch{3, 3, {
    {0, 0, 0}, {1, 0, 1}, {2, 0, 0},
    {0, 1, 1}, {1, 1, 2}, {2, 1, 1},
    {0, 2, 0}, {1, 2, 1}, {2, 2, 0} }, textureName}};
  // clang-format on
}

ValveMapDocumentTest::ValveMapDocumentTest()
  : MapDocumentTest(Model::MapFormat::Valve) {}

Quake3MapDocumentTest::Quake3MapDocumentTest()
  : MapDocumentTest{Model::MapFormat::Quake3} {}

TEST_CASE_METHOD(MapDocumentTest, "MapDocumentTest.throwExceptionDuringCommand") {
  CHECK_THROWS_AS(document->throwExceptionDuringCommand(), CommandProcessorException);
}

TEST_CASE("MapDocumentTest.detectValveFormatMap", "[MapDocumentTest]") {
  auto [document, game, gameConfig] = View::loadMapDocument(
    IO::Path("fixture/test/View/MapDocumentTest/valveFormatMapWithoutFormatTag.map"), "Quake",
    Model::MapFormat::Unknown);
  CHECK(document->world()->mapFormat() == Model::MapFormat::Valve);
  CHECK(document->world()->defaultLayer()->childCount() == 1);
}

TEST_CASE("MapDocumentTest.detectStandardFormatMap", "[MapDocumentTest]") {
  auto [document, game, gameConfig] = View::loadMapDocument(
    IO::Path("fixture/test/View/MapDocumentTest/standardFormatMapWithoutFormatTag.map"), "Quake",
    Model::MapFormat::Unknown);
  CHECK(document->world()->mapFormat() == Model::MapFormat::Standard);
  CHECK(document->world()->defaultLayer()->childCount() == 1);
}

TEST_CASE("MapDocumentTest.detectEmptyMap", "[MapDocumentTest]") {
  auto [document, game, gameConfig] = View::loadMapDocument(
    IO::Path("fixture/test/View/MapDocumentTest/emptyMapWithoutFormatTag.map"), "Quake",
    Model::MapFormat::Unknown);
  // an empty map detects as Valve because Valve is listed first in the Quake game config
  CHECK(document->world()->mapFormat() == Model::MapFormat::Valve);
  CHECK(document->world()->defaultLayer()->childCount() == 0);
}

TEST_CASE("MapDocumentTest.mixedFormats", "[MapDocumentTest]") {
  // map has both Standard and Valve brushes
  CHECK_THROWS_AS(
    View::loadMapDocument(
      IO::Path("fixture/test/View/MapDocumentTest/mixedFormats.map"), "Quake",
      Model::MapFormat::Unknown),
    IO::WorldReaderException);
}

TEST_CASE_METHOD(MapDocumentTest, "Brush Node Selection") {
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
    for (const auto& path : paths) {
      result.push_back(document->world()->resolvePath(path));
    }
    return result;
  };

  SECTION("allSelectedBrushNodes") {
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

    CHECK_THAT(document->allSelectedBrushNodes(), Catch::Matchers::UnorderedEquals(brushNodes));
  }

  SECTION("hasAnySelectedBrushNodes") {
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
} // namespace View
} // namespace TrenchBroom
