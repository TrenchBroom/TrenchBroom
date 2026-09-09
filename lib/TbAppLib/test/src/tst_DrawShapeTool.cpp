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
#include "mdl/BrushNode.h"
#include "mdl/GroupNode.h"
#include "mdl/Map.h"
#include "mdl/Map_Selection.h"
#include "prefs/Preferences.h"
#include "ui/DrawShapeTool.h"
#include "ui/DrawShapeToolExtensionManager.h"
#include "ui/MapDocument.h"
#include "ui/MapDocumentFixture.h"

#include "kd/range_utils.h"

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
}

} // namespace tb::ui
