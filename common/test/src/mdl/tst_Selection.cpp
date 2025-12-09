/*
 Copyright (C) 2021 Kristian Duske

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

#include "TestUtils.h"
#include "mdl/BezierPatch.h"
#include "mdl/BrushBuilder.h"
#include "mdl/BrushFaceHandle.h"
#include "mdl/BrushNode.h"
#include "mdl/Entity.h"
#include "mdl/EntityNode.h"
#include "mdl/Group.h"
#include "mdl/GroupNode.h"
#include "mdl/Map.h"
#include "mdl/MapFixture.h"
#include "mdl/Map_Nodes.h"
#include "mdl/Map_Selection.h"
#include "mdl/PatchNode.h"
#include "mdl/Selection.h"
#include "mdl/WorldNode.h"

#include "kd/result.h"

#include <ostream>
#include <vector>

#include "catch/CatchConfig.h"

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers_vector.hpp>

namespace tb::mdl
{
namespace
{

enum class SelectionItem
{
  nothing,
  outerGroupNode,
  entityNode,
  brushNode,
  patchNode,
  brushFace,
};

[[maybe_unused]] std::ostream& operator<<(std::ostream& lhs, const SelectionItem rhs)
{
  switch (rhs)
  {
  case SelectionItem::nothing:
    lhs << "nothing";
    break;
  case SelectionItem::outerGroupNode:
    lhs << "outerGroupNode";
    break;
  case SelectionItem::entityNode:
    lhs << "entityNode";
    break;
  case SelectionItem::brushNode:
    lhs << "brushNode";
    break;
  case SelectionItem::patchNode:
    lhs << "patchNode";
    break;
  case SelectionItem::brushFace:
    lhs << "brushFace";
    break;
  }
  return lhs;
}

} // namespace

using namespace Catch::Matchers;

TEST_CASE("Selection")
{
  auto fixture = MapFixture{};
  auto& map = fixture.create();

  auto& worldNode = map.worldNode();
  auto brushBuilder = BrushBuilder{worldNode.mapFormat(), map.worldBounds()};

  auto* outerGroupNode = new GroupNode{Group{"outer"}};
  auto* innerGroupNode = new GroupNode{Group{"inner"}};

  auto* entityNode = new EntityNode{{}};
  auto* brushNode =
    brushBuilder.createCube(64.0, "material")
      .transform([](auto brush) { return std::make_unique<BrushNode>(std::move(brush)); })
      .value()
      .release();

  // clang-format off
  auto* patchNode = new PatchNode{BezierPatch{3, 3, {
    {0, 0, 0}, {1, 0, 1}, {2, 0, 0},
    {0, 1, 1}, {1, 1, 2}, {2, 1, 1},
    {0, 2, 0}, {1, 2, 1}, {2, 2, 0} }, "material"}};
  // clang-format on

  auto* brushEntityNode = new EntityNode{{}};
  auto* entityBrushNode =
    brushBuilder.createCube(64.0, "material")
      .transform([](auto brush) { return std::make_unique<BrushNode>(std::move(brush)); })
      .value()
      .release();

  auto* otherGroupNode = new GroupNode{Group{"other"}};
  auto* groupedEntityNode = new EntityNode{{}};

  /*
   worldNode
     outerGroupNode
       innerGroupNode
         patchNode
       brushNode
     entityNode
     brushEntityNode
       entityBrushNode
     otherGroupNode
       groupedEntityNode
  */

  innerGroupNode->addChildren({patchNode});
  outerGroupNode->addChildren({innerGroupNode, brushNode});
  brushEntityNode->addChildren({entityBrushNode});
  otherGroupNode->addChildren({groupedEntityNode});

  addNodes(
    map,
    {{parentForNodes(map),
      {outerGroupNode, entityNode, brushEntityNode, otherGroupNode}}});

  const auto selectItem = [&](const auto selectionItem) {
    switch (selectionItem)
    {
    case SelectionItem::nothing:
      deselectAll(map);
      break;
    case SelectionItem::outerGroupNode:
      selectNodes(map, {outerGroupNode});
      break;
    case SelectionItem::entityNode:
      selectNodes(map, {entityNode});
      break;
    case SelectionItem::brushNode:
      selectNodes(map, {brushNode});
      break;
    case SelectionItem::patchNode:
      selectNodes(map, {patchNode});
      break;
    case SelectionItem::brushFace:
      selectBrushFaces(map, {{brushNode, 0}});
      break;
    }
  };

  const auto selectItems = [&](const auto& selectionItems) {
    for (const auto& selectionItem : selectionItems)
    {
      selectItem(selectionItem);
    }
  };

  SECTION("hasAny")
  {
    using T = std::tuple<SelectionItem, bool>;
    const auto [selectionItem, expectedHasAny] = GENERATE(values<T>({
      {SelectionItem::nothing, false},
      {SelectionItem::outerGroupNode, true},
      {SelectionItem::entityNode, true},
      {SelectionItem::brushNode, true},
      {SelectionItem::patchNode, true},
      {SelectionItem::brushFace, true},
    }));

    CAPTURE(selectionItem);

    selectItem(selectionItem);
    CHECK(map.selection().hasAny() == expectedHasAny);
  }

  SECTION("hasNodes")
  {
    using T = std::tuple<SelectionItem, bool>;
    const auto [selectionItem, expected] = GENERATE(values<T>({
      {SelectionItem::nothing, false},
      {SelectionItem::outerGroupNode, true},
      {SelectionItem::entityNode, true},
      {SelectionItem::brushNode, true},
      {SelectionItem::patchNode, true},
      {SelectionItem::brushFace, false},
    }));

    CAPTURE(selectionItem);

    selectItem(selectionItem);
    CHECK(map.selection().hasNodes() == expected);
  }

  SECTION("hasGroups")
  {
    using T = std::tuple<std::vector<SelectionItem>, bool>;
    const auto [selectionItems, expected] = GENERATE(values<T>({
      {{SelectionItem::nothing}, false},
      {{SelectionItem::outerGroupNode}, true},
      {{SelectionItem::outerGroupNode, SelectionItem::entityNode}, true},
      {{SelectionItem::entityNode}, false},
      {{SelectionItem::brushFace}, false},
    }));

    CAPTURE(selectionItems);

    selectItems(selectionItems);
    CHECK(map.selection().hasGroups() == expected);
  }

  SECTION("hasOnlyGroups")
  {
    using T = std::tuple<std::vector<SelectionItem>, bool>;
    const auto [selectionItems, expected] = GENERATE(values<T>({
      {{SelectionItem::nothing}, false},
      {{SelectionItem::outerGroupNode}, true},
      {{SelectionItem::outerGroupNode, SelectionItem::entityNode}, false},
      {{SelectionItem::entityNode}, false},
      {{SelectionItem::brushFace}, false},
    }));

    CAPTURE(selectionItems);

    selectItems(selectionItems);
    CHECK(map.selection().hasOnlyGroups() == expected);
  }

  SECTION("hasEntities")
  {
    using T = std::tuple<std::vector<SelectionItem>, bool>;
    const auto [selectionItems, expected] = GENERATE(values<T>({
      {{SelectionItem::nothing}, false},
      {{SelectionItem::entityNode}, true},
      {{SelectionItem::entityNode, SelectionItem::brushNode}, true},
      {{SelectionItem::brushNode}, false},
      {{SelectionItem::brushFace}, false},
    }));

    CAPTURE(selectionItems);

    selectItems(selectionItems);
    CHECK(map.selection().hasEntities() == expected);
  }

  SECTION("hasOnlyEntities")
  {
    using T = std::tuple<std::vector<SelectionItem>, bool>;
    const auto [selectionItems, expected] = GENERATE(values<T>({
      {{SelectionItem::nothing}, false},
      {{SelectionItem::entityNode}, true},
      {{SelectionItem::entityNode, SelectionItem::brushNode}, false},
      {{SelectionItem::brushNode}, false},
      {{SelectionItem::brushFace}, false},
    }));

    CAPTURE(selectionItems);

    selectItems(selectionItems);
    CHECK(map.selection().hasOnlyEntities() == expected);
  }

  SECTION("hasBrushes")
  {
    using T = std::tuple<std::vector<SelectionItem>, bool>;
    const auto [selectionItems, expected] = GENERATE(values<T>({
      {{SelectionItem::nothing}, false},
      {{SelectionItem::brushNode}, true},
      {{SelectionItem::entityNode, SelectionItem::brushNode}, true},
      {{SelectionItem::entityNode}, false},
      {{SelectionItem::brushFace}, false},
    }));

    CAPTURE(selectionItems);

    selectItems(selectionItems);
    CHECK(map.selection().hasBrushes() == expected);
  }

  SECTION("hasOnlyBrushes")
  {
    using T = std::tuple<std::vector<SelectionItem>, bool>;
    const auto [selectionItems, expected] = GENERATE(values<T>({
      {{SelectionItem::nothing}, false},
      {{SelectionItem::brushNode}, true},
      {{SelectionItem::entityNode, SelectionItem::brushNode}, false},
      {{SelectionItem::entityNode}, false},
      {{SelectionItem::brushFace}, false},
    }));

    CAPTURE(selectionItems);

    selectItems(selectionItems);
    CHECK(map.selection().hasOnlyBrushes() == expected);
  }

  SECTION("hasPatches")
  {
    using T = std::tuple<std::vector<SelectionItem>, bool>;
    const auto [selectionItems, expected] = GENERATE(values<T>({
      {{SelectionItem::nothing}, false},
      {{SelectionItem::patchNode}, true},
      {{SelectionItem::entityNode, SelectionItem::patchNode}, true},
      {{SelectionItem::entityNode}, false},
      {{SelectionItem::brushFace}, false},
    }));

    CAPTURE(selectionItems);

    selectItems(selectionItems);
    CHECK(map.selection().hasPatches() == expected);
  }

  SECTION("hasOnlyPatches")
  {
    using T = std::tuple<std::vector<SelectionItem>, bool>;
    const auto [selectionItems, expected] = GENERATE(values<T>({
      {{SelectionItem::nothing}, false},
      {{SelectionItem::patchNode}, true},
      {{SelectionItem::entityNode, SelectionItem::patchNode}, false},
      {{SelectionItem::entityNode}, false},
      {{SelectionItem::brushFace}, false},
    }));

    CAPTURE(selectionItems);

    selectItems(selectionItems);
    CHECK(map.selection().hasOnlyPatches() == expected);
  }

  SECTION("hasBrushFaces")
  {
    using T = std::tuple<std::vector<SelectionItem>, bool>;
    const auto [selectionItems, expected] = GENERATE(values<T>({
      {{SelectionItem::nothing}, false},
      {{SelectionItem::entityNode}, false},
      {{SelectionItem::entityNode, SelectionItem::patchNode}, false},
      {{SelectionItem::brushNode}, false},
      {{SelectionItem::brushFace}, true},
    }));

    CAPTURE(selectionItems);

    selectItems(selectionItems);
    CHECK(map.selection().hasBrushFaces() == expected);
  }

  SECTION("hasAnyBrushFaces")
  {
    using T = std::tuple<std::vector<SelectionItem>, bool>;
    const auto [selectionItems, expected] = GENERATE(values<T>({
      {{SelectionItem::nothing}, false},
      {{SelectionItem::entityNode}, false},
      {{SelectionItem::entityNode, SelectionItem::brushNode}, true},
      {{SelectionItem::brushNode}, true},
      {{SelectionItem::brushFace}, true},
    }));

    CAPTURE(selectionItems);

    selectItems(selectionItems);
    CHECK(map.selection().hasAnyBrushFaces() == expected);
  }

  SECTION("allEntities")
  {
    SECTION("nothing selected")
    {
      CHECK_THAT(
        map.selection().allEntities(),
        UnorderedEquals(std::vector<EntityNodeBase*>{&worldNode}));
    }

    SECTION("outer group node selected")
    {
      selectNodes(map, {outerGroupNode});
      CHECK_THAT(
        map.selection().allEntities(),
        UnorderedEquals(std::vector<EntityNodeBase*>{&worldNode}));
    }

    SECTION("entity node selected")
    {
      selectNodes(map, {entityNode});
      CHECK_THAT(
        map.selection().allEntities(),
        UnorderedEquals(std::vector<EntityNodeBase*>{entityNode}));
    }

    SECTION("mixed selection")
    {
      selectNodes(map, {entityNode, brushNode});
      CHECK_THAT(
        map.selection().allEntities(),
        UnorderedEquals(std::vector<EntityNodeBase*>{entityNode}));
    }

    SECTION("other group selected")
    {
      selectNodes(map, {otherGroupNode});
      CHECK(
        map.selection().allEntities() == std::vector<EntityNodeBase*>{groupedEntityNode});
    }

    SECTION("nested entity selected")
    {
      selectNodes(map, {groupedEntityNode});
      CHECK_THAT(
        map.selection().allEntities(),
        UnorderedEquals(std::vector<EntityNodeBase*>{groupedEntityNode}));
    }

    SECTION("face selected")
    {
      selectBrushFaces(map, {{brushNode, 0}});
      CHECK_THAT(
        map.selection().allEntities(),
        UnorderedEquals(std::vector<EntityNodeBase*>{&worldNode}));
    }
  }

  SECTION("allBrushes")
  {
    SECTION("selection is empty")
    {
      CHECK_THAT(
        map.selection().allBrushes(), UnorderedEquals(std::vector<BrushNode*>{}));
    }

    SECTION("nothing selected")
    {
      CHECK_THAT(
        map.selection().allBrushes(), UnorderedEquals(std::vector<BrushNode*>{}));
    }

    SECTION("outer group node selected")
    {
      selectNodes(map, {outerGroupNode});
      CHECK_THAT(
        map.selection().allBrushes(),
        UnorderedEquals(std::vector<BrushNode*>{brushNode}));
    }

    SECTION("entity node selected")
    {
      selectNodes(map, {entityNode});
      CHECK_THAT(
        map.selection().allBrushes(), UnorderedEquals(std::vector<BrushNode*>{}));
    }

    SECTION("mixed selection")
    {
      selectNodes(map, {entityNode, brushNode});
      CHECK_THAT(
        map.selection().allBrushes(),
        UnorderedEquals(std::vector<BrushNode*>{brushNode}));
    }

    SECTION("entity brush selected")
    {
      selectNodes(map, {entityBrushNode});
      CHECK_THAT(
        map.selection().allBrushes(),
        UnorderedEquals(std::vector<BrushNode*>{entityBrushNode}));
    }

    SECTION("face selected")
    {
      selectBrushFaces(map, {{brushNode, 0}});
      CHECK_THAT(
        map.selection().allBrushes(), UnorderedEquals(std::vector<BrushNode*>{}));
    }
  }

  SECTION("allBrushFaces")
  {
    SECTION("face selected")
    {
      selectBrushFaces(map, {{brushNode, 0}});
      CHECK(
        map.selection().allBrushFaces()
        == std::vector<BrushFaceHandle>{BrushFaceHandle{brushNode, 0}});
    }

    SECTION("brush selected")
    {
      selectNodes(map, {brushNode});
      CHECK(map.selection().allBrushFaces().size() == 6);
    }

    SECTION("mixed selection")
    {
      selectNodes(map, {entityNode, brushNode});
      CHECK(map.selection().allBrushFaces().size() == 6);
    }
  }
}

} // namespace tb::mdl
