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

#include "base/PreferenceManager.h"
#include "mdl/BrushBuilder.h"
#include "mdl/BrushNode.h"
#include "mdl/GroupNode.h"
#include "mdl/LayerNode.h"
#include "mdl/Map.h"
#include "mdl/Map_Nodes.h"
#include "mdl/Map_Selection.h"
#include "mdl/WorldNode.h"
#include "prefs/Preferences.h"
#include "ui/DrawShapeTool.h"
#include "ui/DrawShapeToolExtensionManager.h"
#include "ui/MapDocument.h"
#include "ui/MapDocumentFixture.h"

#include "kd/range_utils.h"
#include "kd/result.h"

#include "vm/bbox.h"

#include <algorithm>

#include <catch2/catch_test_macros.hpp>

namespace tb::ui
{

namespace
{

size_t indexOfExtensionNamed(
  DrawShapeToolExtensionManager& extensionManager, const std::string& name)
{
  return *kdl::index_of(extensionManager.extensions(), [&](const auto* extension) {
    return extension->name() == name;
  });
}

mdl::BrushNode* addBrushNode(mdl::Map& map, const vm::bbox3d& bounds)
{
  auto builder = mdl::BrushBuilder{map.worldNode().mapFormat(), map.worldBounds()};
  auto* brushNode =
    new mdl::BrushNode{builder.createCuboid(bounds, "material") | kdl::value()};
  mdl::addNodes(map, {{&mdl::parentForNodes(map), {brushNode}}});
  return brushNode;
}

} // namespace

TEST_CASE("DrawShapeTool")
{
  auto fixture = MapDocumentFixture{};
  auto& document = fixture.create();
  auto& map = document.map();

  auto tool = DrawShapeTool{document};

  constexpr auto bounds = vm::bbox3d{{0, 0, 0}, {64, 64, 64}};

  SECTION("createBrushes groups multiple brushes and names the group after the shape")
  {
    REQUIRE(pref(Preferences::GroupBrushesCreatedByShapeTool));

    const auto stairsIndex = indexOfExtensionNamed(tool.extensionManager(), "Stairs");
    REQUIRE(tool.extensionManager().setCurrentExtensionIndex(stairsIndex));

    tool.update(bounds);
    tool.createBrushes(tool.groupNameForCreatedBrushes());

    const auto& selectedNodes = map.selection().nodes;
    REQUIRE(selectedNodes.size() == 1);

    auto* groupNode = dynamic_cast<mdl::GroupNode*>(selectedNodes.front());
    REQUIRE(groupNode != nullptr);
    CHECK(groupNode->group().name() == "Stairs");
    CHECK(groupNode->childCount() > 1);
  }

  SECTION(
    "createBrushes does not group multiple brushes when "
    "GroupBrushesCreatedByShapeTool is disabled")
  {
    setPref(Preferences::GroupBrushesCreatedByShapeTool, false);

    const auto stairsIndex = indexOfExtensionNamed(tool.extensionManager(), "Stairs");
    REQUIRE(tool.extensionManager().setCurrentExtensionIndex(stairsIndex));

    tool.update(bounds);
    tool.createBrushes(tool.groupNameForCreatedBrushes());

    const auto& selectedNodes = map.selection().nodes;
    CHECK(selectedNodes.size() > 1);
    for (auto* node : selectedNodes)
    {
      CHECK(dynamic_cast<mdl::BrushNode*>(node) != nullptr);
    }

    PreferenceManager::instance().resetToDefault(
      Preferences::GroupBrushesCreatedByShapeTool);
  }

  SECTION("createBrushes does not create a group for a single brush")
  {
    // Cuboid is the default extension and always creates a single brush.
    REQUIRE(tool.extensionManager().currentExtension().name() == "Cuboid");

    tool.update(bounds);
    tool.createBrushes(tool.groupNameForCreatedBrushes());

    const auto& selectedNodes = map.selection().nodes;
    REQUIRE(selectedNodes.size() == 1);
    CHECK(dynamic_cast<mdl::BrushNode*>(selectedNodes.front()) != nullptr);
  }

  SECTION("createBrushes only increments the rock seed when a rock is created")
  {
    auto& parameters = tool.extensionManager().parameters();
    parameters.setIncrementRockSeed(true);

    REQUIRE(tool.extensionManager().currentExtension().name() == "Cuboid");

    tool.update(bounds);
    tool.createBrushes(tool.groupNameForCreatedBrushes());
    CHECK(parameters.rockSeed() == 0);

    const auto rockIndex = indexOfExtensionNamed(tool.extensionManager(), "Rock");
    REQUIRE(tool.extensionManager().setCurrentExtensionIndex(rockIndex));

    tool.update(bounds);
    tool.createBrushes(tool.groupNameForCreatedBrushes());
    CHECK(parameters.rockSeed() == 1);
  }

  SECTION("applying the parameters replaces the selection with the shape")
  {
    auto* brushNode = addBrushNode(map, bounds);
    mdl::selectNodes(map, {brushNode});

    tool.extensionManager().applyParametersNotifier();

    const auto& selectedNodes = map.selection().nodes;
    REQUIRE(selectedNodes.size() == 1);
    CHECK(selectedNodes.front() != brushNode);
    CHECK(map.worldNode().defaultLayer()->children() == selectedNodes);
  }

  SECTION("applying the parameters keeps the selection if the shape has no brushes")
  {
    // Every point of a rock formation in these bounds rounds to 0, so that all of its
    // pieces are dropped.
    auto* brushNode = addBrushNode(map, vm::bbox3d{{0, 0, 0}, {0.25, 0.25, 0.25}});
    mdl::selectNodes(map, {brushNode});

    const auto rockIndex = indexOfExtensionNamed(tool.extensionManager(), "Rock");
    REQUIRE(tool.extensionManager().setCurrentExtensionIndex(rockIndex));

    tool.extensionManager().applyParametersNotifier();

    CHECK(map.selection().nodes == std::vector<mdl::Node*>{brushNode});
    CHECK(
      map.worldNode().defaultLayer()->children() == std::vector<mdl::Node*>{brushNode});
  }
}

} // namespace tb::ui
