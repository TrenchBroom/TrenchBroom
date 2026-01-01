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

#include "mdl/BrushNode.h" // IWYU pragma: keep
#include "mdl/Entity.h"
#include "mdl/EntityNode.h"
#include "mdl/Map.h"
#include "mdl/Map_Nodes.h"
#include "mdl/Map_Selection.h"
#include "mdl/Node.h"
#include "mdl/PatchNode.h" // IWYU pragma: keep
#include "mdl/TestFactory.h"
#include "ui/CatchConfig.h"
#include "ui/MapDocument.h"
#include "ui/MapDocumentFixture.h"
#include "ui/ShearTool.h"

#include "kd/ranges/to.h"

#include <ranges>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

namespace tb::ui
{

TEST_CASE("ShearTool")
{
  auto fixture = MapDocumentFixture{};
  auto& document = fixture.create();
  auto& map = document.map();

  auto* entityNode = new mdl::EntityNode{mdl::Entity{}};
  auto* brushNode = mdl::createBrushNode(map);
  auto* patchNode = mdl::createPatchNode("some_material");
  mdl::addNodes(map, {{mdl::parentForNodes(map), {brushNode, entityNode, patchNode}}});

  auto nodes = std::vector<mdl::Node*>{entityNode, brushNode, patchNode};
  constexpr size_t iEntityNode = 0;
  constexpr size_t iBrushNode = 1;
  constexpr size_t iPatchNode = 2;

  auto tool = ShearTool{document};

  SECTION("applies")
  {
    using T = std::tuple<std::vector<size_t>, bool>;

    const auto [nodesIndicesToSelect, expectedApplies] = GENERATE(values<T>({
      {std::vector<size_t>{}, false},
      {std::vector<size_t>{iEntityNode}, true},
      {std::vector<size_t>{iBrushNode}, true},
      {std::vector<size_t>{iPatchNode}, true},
      {std::vector<size_t>{iEntityNode, iBrushNode, iPatchNode}, true},
    }));

    const auto nodesToSelect =
      nodesIndicesToSelect
      | std::views::transform([&](const auto i) -> mdl::Node* { return nodes[i]; })
      | kdl::ranges::to<std::vector>();

    mdl::selectNodes(map, nodesToSelect);
    CHECK(tool.applies() == expectedApplies);
  }
}

} // namespace tb::ui
