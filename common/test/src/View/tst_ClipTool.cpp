/*
 Copyright (C) 2024 Kristian Duske
 Copyright (C) 2019 Eric Wasylishen

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
#include "Model/BrushNode.h"
#include "Model/LayerNode.h"
#include "Model/WorldNode.h"
#include "Renderer/PerspectiveCamera.h"
#include "View/ClipTool.h"
#include "View/ClipToolController.h"
#include "View/Grid.h"
#include "View/PasteType.h"
#include "View/Tool.h"

#include "Catch2.h"

namespace TrenchBroom::View
{

TEST_CASE_METHOD(ValveMapDocumentTest, "ClipToolTest")
{
  // https://github.com/TrenchBroom/TrenchBroom/issues/4461
  SECTION("Clipped brushes get new link IDs")
  {
    const auto data = R"(// entity 0
{
"mapversion" "220"
"wad" ""
"classname" "worldspawn"
// brush 0
{
( -64 -64 -16 ) ( -64 -63 -16 ) ( -64 -64 -15 ) __TB_empty [ 0 -1 0 0 ] [ 0 0 -1 0 ] 0 1 1
( -64 -64 -16 ) ( -64 -64 -15 ) ( -63 -64 -16 ) __TB_empty [ 1 0 0 0 ] [ 0 0 -1 0 ] 0 1 1
( -64 -64 -16 ) ( -63 -64 -16 ) ( -64 -63 -16 ) __TB_empty [ -1 0 0 0 ] [ 0 -1 0 0 ] 0 1 1
( 64 64 16 ) ( 64 65 16 ) ( 65 64 16 ) __TB_empty [ 1 0 0 0 ] [ 0 -1 0 0 ] 0 1 1
( 64 64 16 ) ( 65 64 16 ) ( 64 64 17 ) __TB_empty [ -1 0 0 0 ] [ 0 0 -1 0 ] 0 1 1
( 64 64 16 ) ( 64 64 17 ) ( 64 65 16 ) __TB_empty [ 0 1 0 0 ] [ 0 0 -1 0 ] 0 1 1
}
}
)";
    REQUIRE(document->paste(data) == PasteType::Node);

    const auto* defaultLayer = document->world()->defaultLayer();

    const auto* originalBrushNode =
      dynamic_cast<const Model::BrushNode*>(defaultLayer->children().front());
    REQUIRE(originalBrushNode);

    const auto originalLinkId = originalBrushNode->brush().linkId();

    auto tool = ClipTool{document};
    REQUIRE(tool.activate());

    tool.addPoint(vm::vec3{0, 16, 16}, {});
    tool.addPoint(vm::vec3{0, -16, 16}, {});
    tool.addPoint(vm::vec3{0, -64, 0}, {});

    REQUIRE(tool.canClip());
    tool.toggleSide();
    tool.performClip();

    REQUIRE(defaultLayer->childCount() == 2);
    const auto* clippedBrushNode1 =
      dynamic_cast<const Model::BrushNode*>(defaultLayer->children().front());
    const auto* clippedBrushNode2 =
      dynamic_cast<const Model::BrushNode*>(defaultLayer->children().back());

    REQUIRE(clippedBrushNode1);
    REQUIRE(clippedBrushNode2);

    CHECK(clippedBrushNode1->brush().linkId() != originalLinkId);
    CHECK(clippedBrushNode2->brush().linkId() != originalLinkId);
    CHECK(clippedBrushNode1->brush().linkId() != clippedBrushNode2->brush().linkId());
  }
}

} // namespace TrenchBroom::View
