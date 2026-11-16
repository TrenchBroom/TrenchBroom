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

#include <QAbstractButton>
#include <QComboBox>
#include <QLineEdit>

#include "mdl/BrushBuilder.h"
#include "mdl/BrushNode.h"
#include "mdl/EditorContext.h"
#include "mdl/LayerNode.h" // IWYU pragma: keep
#include "mdl/Map.h"
#include "mdl/Map_Nodes.h"
#include "mdl/Map_Selection.h"
#include "mdl/NodeHandleManager.h"
#include "mdl/NodeHandles.h"
#include "mdl/WorldNode.h"
#include "ui/MapDocument.h"
#include "ui/MapDocumentFixture.h"
#include "ui/QVecUtils.h"
#include "ui/ScaleTool.h"
#include "ui/ScaleToolPage.h"
#include "ui/VertexTool.h"

#include "kd/result.h"

#include "vm/vec.h"

#include <memory>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_vector.hpp>

namespace tb::ui
{
using namespace Catch::Matchers;

TEST_CASE("ScaleToolPage")
{
  auto fixture = MapDocumentFixture{};
  auto& document = fixture.create();
  auto& map = document.map();

  // a pyramid stump whose top face is off center along X
  auto builder = mdl::BrushBuilder{map.worldNode().mapFormat(), map.worldBounds()};
  auto* brushNode = mdl::addNode(
    map,
    *map.editorContext().currentLayer(),
    std::make_unique<mdl::BrushNode>(
      builder.createBrush(
        std::vector<vm::vec3d>{
          {-32, -32, -32},
          {-32, 32, -32},
          {32, -32, -32},
          {32, 32, -32},
          {0, -16, 32},
          {0, 16, 32},
          {16, -16, 32},
          {16, 16, 32},
        },
        "material")
      | kdl::value()));
  mdl::selectNodes(map, {brushNode});

  // the vertex tool keeps the vertex handles in sync with the brush
  auto vertexTool = VertexTool{document};
  REQUIRE(vertexTool.activate());
  map.nodeHandles().selectHandles<mdl::VertexHandle>(std::vector<mdl::VertexHandle>{
    {{0, -16, 32}},
    {{0, 16, 32}},
    {{16, -16, 32}},
    {{16, 16, 32}},
  });

  auto tool = ScaleTool{document};
  auto page = ScaleToolPage{document, tool};

  auto* modeChoice = page.findChild<QComboBox*>("ScaleToolPage_ModeChoice");
  auto* sizeText = page.findChild<QLineEdit*>("ScaleToolPage_SizeText");
  auto* factorsText = page.findChild<QLineEdit*>("ScaleToolPage_FactorsText");
  auto* applyButton = page.findChild<QAbstractButton*>("ScaleToolPage_ApplyButton");
  REQUIRE(modeChoice != nullptr);
  REQUIRE(sizeText != nullptr);
  REQUIRE(factorsText != nullptr);
  REQUIRE(applyButton != nullptr);

  REQUIRE(tool.activate());

  const auto selectedVertices = [&]() {
    return mdl::VertexHandle::getPositions(
      map.nodeHandles().selectedHandles<mdl::VertexHandle>());
  };

  // the top face doubled in size along X about its center
  const auto scaledTopVertices = std::vector<vm::vec3d>{
    {-8, -16, 32},
    {-8, 16, 32},
    {24, -16, 32},
    {24, 16, 32},
  };

  SECTION("activating the tool suggests the size of the selected vertices")
  {
    // along Z, the vertices have no extent, so their bounds extend to the bottom of the
    // brush
    CHECK(sizeText->text() == toString(vm::vec3d{16, 32, 64}));
  }

  SECTION("scaling to size")
  {
    modeChoice->setCurrentIndex(0);
    sizeText->setText(toString(vm::vec3d{32, 32, 64}));
    applyButton->click();

    CHECK_THAT(selectedVertices(), UnorderedEquals(scaledTopVertices));
  }

  SECTION("scaling by factors")
  {
    modeChoice->setCurrentIndex(1);
    factorsText->setText(toString(vm::vec3d{2, 1, 1}));
    applyButton->click();

    CHECK_THAT(selectedVertices(), UnorderedEquals(scaledTopVertices));
  }
}

} // namespace tb::ui
