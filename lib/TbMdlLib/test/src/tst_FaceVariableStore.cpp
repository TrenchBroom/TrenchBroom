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

#include "el/Value.h"
#include "mdl/Brush.h"
#include "mdl/BrushFace.h"
#include "mdl/BrushFaceHandle.h"
#include "mdl/BrushNode.h"
#include "mdl/Entity.h"
#include "mdl/EntityNode.h"
#include "mdl/EntityProperties.h"
#include "mdl/FaceVariableStore.h"
#include "mdl/GroupNode.h"
#include "mdl/Map.h"
#include "mdl/MapFixture.h"
#include "mdl/Map_Nodes.h"
#include "mdl/Map_Selection.h"
#include "mdl/TestFactory.h"
#include "mdl/WorldNode.h"

#include "kd/result.h"

#include <catch2/catch_test_macros.hpp>

namespace tb::mdl
{

TEST_CASE("FaceVariableStore")
{
  auto fixture = MapFixture{};
  auto& map = fixture.create();

  auto* brushNode = createBrushNode(map, "material");
  addNodes(map, {{&parentForNodes(map), {brushNode}}});

  const auto& brush = brushNode->brush();
  const auto& face = brush.face(0);

  SECTION("material, normal, bounds, center")
  {
    auto store = FaceVariableStore{map, *brushNode, face};

    CHECK(store.value("material") == el::Value{"material"});
    CHECK(store.value("normal") == el::Value{face.normal()});
    CHECK(store.value("bounds") == el::Value{face.bounds()});
    CHECK(store.value("center") == el::Value{face.center()});
  }

  SECTION("entity: structural face belongs to worldspawn")
  {
    auto store = FaceVariableStore{map, *brushNode, face};

    CHECK(
      store.value("entity")
      == el::Value{el::MapType{
        {"classname", el::Value{EntityPropertyValues::WorldspawnClassname}},
        {"properties",
         el::Value{el::MapType{
           {"classname", el::Value{EntityPropertyValues::WorldspawnClassname}},
         }}},
      }});
  }

  SECTION("entity: brush entity's face")
  {
    auto* brushEntityNode = new EntityNode{Entity{{
      {"classname", "func_detail"},
    }}};
    auto* entityBrushNode = createBrushNode(map);
    addNodes(map, {{&parentForNodes(map), {brushEntityNode}}});
    addNodes(map, {{brushEntityNode, {entityBrushNode}}});

    const auto& entityFace = entityBrushNode->brush().face(0);
    auto store = FaceVariableStore{map, *entityBrushNode, entityFace};

    CHECK(
      store.value("entity")
      == el::Value{el::MapType{
        {"classname", el::Value{"func_detail"}},
        {"properties",
         el::Value{el::MapType{
           {"classname", el::Value{"func_detail"}},
         }}},
      }});
  }

  SECTION("layerName and groupName")
  {
    // added directly under the default layer, not inside any group
    CHECK(
      FaceVariableStore{map, *brushNode, face}.value("layerName")
      == el::Value{"Default Layer"});
    CHECK(
      FaceVariableStore{map, *brushNode, face}.value("groupName")
      == el::Value::Undefined);

    auto* groupNode = new GroupNode{Group{"CustomGroup"}};
    auto* groupedBrushNode = createBrushNode(map);
    addNodes(map, {{&parentForNodes(map), {groupNode}}});
    addNodes(map, {{groupNode, {groupedBrushNode}}});

    const auto& groupedFace = groupedBrushNode->brush().face(0);
    CHECK(
      FaceVariableStore{map, *groupedBrushNode, groupedFace}.value("groupName")
      == el::Value{"CustomGroup"});
  }

  SECTION("tags")
  {
    // the default test fixture registers no smart tags
    CHECK(
      FaceVariableStore{map, *brushNode, face}.value("tags")
      == el::Value{el::ArrayType{}});
  }

  SECTION("visible, locked: inherited from the owning brush node")
  {
    CHECK(FaceVariableStore{map, *brushNode, face}.value("visible") == el::Value{true});
    CHECK(FaceVariableStore{map, *brushNode, face}.value("locked") == el::Value{false});
  }

  SECTION("selected: the face's own state, independent of the brush node")
  {
    auto store = FaceVariableStore{map, *brushNode, face};
    CHECK(store.value("selected") == el::Value{false});

    selectBrushFaces(map, {BrushFaceHandle{brushNode, 0}});

    CHECK(store.value("selected") == el::Value{true});
    CHECK(brushNode->selected() == false);
  }

  SECTION("names and size")
  {
    auto store = FaceVariableStore{map, *brushNode, face};
    CHECK(store.names().size() == store.size());
    CHECK(store.size() == 11u);
  }
}

} // namespace tb::mdl
