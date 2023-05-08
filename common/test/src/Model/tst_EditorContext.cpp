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

#include "Catch2.h"
#include "Exceptions.h"
#include "Model/BezierPatch.h"
#include "Model/BrushBuilder.h"
#include "Model/BrushNode.h"
#include "Model/EditorContext.h"
#include "Model/Entity.h"
#include "Model/EntityNode.h"
#include "Model/GroupNode.h"
#include "Model/LayerNode.h"
#include "Model/LockState.h"
#include "Model/MapFormat.h"
#include "Model/PatchNode.h"
#include "Model/VisibilityState.h"
#include "Model/WorldNode.h"
#include "PreferenceManager.h"
#include "Preferences.h"

#include <kdl/result.h>
#include <kdl/result_io.h>

#include <vecmath/bbox.h>
#include <vecmath/bbox_io.h>

#include <functional>
#include <tuple>

namespace TrenchBroom
{
namespace Model
{
class EditorContextTest
{
protected:
  vm::bbox3d worldBounds;
  WorldNode worldNode;
  EditorContext context;

  EditorContextTest()
    : worldBounds{8192.0}
    , worldNode{{}, {}, MapFormat::Quake3}
  {
  }

public:
  GroupNode* createTopLevelGroup()
  {
    GroupNode* group;
    std::tie(group, std::ignore) = createGroupedBrush();
    return group;
  }

  EntityNode* createTopLevelPointEntity()
  {
    auto* entityNode = new EntityNode{Entity{}};
    worldNode.defaultLayer()->addChild(entityNode);
    return entityNode;
  }

  std::tuple<EntityNode*, BrushNode*> createTopLevelBrushEntity()
  {
    BrushBuilder builder(worldNode.mapFormat(), worldBounds);
    auto* brushNode = new BrushNode{builder.createCube(32.0, "sometex").value()};
    auto* entityNode = new EntityNode{Entity{}};
    entityNode->addChild(brushNode);
    worldNode.defaultLayer()->addChild(entityNode);
    return std::make_tuple(entityNode, brushNode);
  }

  std::tuple<EntityNode*, PatchNode*> createTopLevelPatchEntity()
  {
    // clang-format off
    auto* patchNode = new PatchNode{BezierPatch{3, 3, {
      {0, 0, 0}, {1, 0, 1}, {2, 0, 0},
      {0, 1, 1}, {1, 1, 2}, {2, 1, 1},
      {0, 2, 0}, {1, 2, 1}, {2, 2, 0} }, "texture"}};
    // clang-format on

    auto* entityNode = new EntityNode{Entity{}};
    entityNode->addChild(patchNode);
    worldNode.defaultLayer()->addChild(entityNode);
    return std::make_tuple(entityNode, patchNode);
  }

  BrushNode* createTopLevelBrush()
  {
    BrushBuilder builder(worldNode.mapFormat(), worldBounds);
    auto* brushNode = new BrushNode{builder.createCube(32.0, "sometex").value()};
    worldNode.defaultLayer()->addChild(brushNode);
    return brushNode;
  }

  PatchNode* createTopLevelPatch()
  {
    // clang-format off
    auto* patchNode = new PatchNode{BezierPatch{3, 3, {
      {0, 0, 0}, {1, 0, 1}, {2, 0, 0},
      {0, 1, 1}, {1, 1, 2}, {2, 1, 1},
      {0, 2, 0}, {1, 2, 1}, {2, 2, 0} }, "texture"}};
    // clang-format on

    worldNode.defaultLayer()->addChild(patchNode);
    return patchNode;
  }

  std::tuple<GroupNode*, GroupNode*> createNestedGroup()
  {
    GroupNode* outerGroup;
    GroupNode* innerGroup;
    std::tie(outerGroup, innerGroup, std::ignore) = createdNestedGroupedBrush();

    return std::make_tuple(outerGroup, innerGroup);
  }

  std::tuple<GroupNode*, BrushNode*> createGroupedBrush()
  {
    BrushBuilder builder(worldNode.mapFormat(), worldBounds);
    auto* brushNode = new BrushNode{builder.createCube(32.0, "sometex").value()};
    auto* groupNode = new GroupNode{Group{"somegroup"}};

    groupNode->addChild(brushNode);
    worldNode.defaultLayer()->addChild(groupNode);

    return std::make_tuple(groupNode, brushNode);
  }

  std::tuple<GroupNode*, EntityNode*> createGroupedPointEntity()
  {
    auto* entityNode = new EntityNode{Entity{}};
    auto* groupNode = new GroupNode{Group{"somegroup"}};

    groupNode->addChild(entityNode);
    worldNode.defaultLayer()->addChild(groupNode);

    return std::make_tuple(groupNode, entityNode);
  }

  std::tuple<GroupNode*, PatchNode*> createGroupedPatch()
  {
    // clang-format off
    auto* patchNode = new PatchNode{BezierPatch{3, 3, {
      {0, 0, 0}, {1, 0, 1}, {2, 0, 0},
      {0, 1, 1}, {1, 1, 2}, {2, 1, 1},
      {0, 2, 0}, {1, 2, 1}, {2, 2, 0} }, "texture"}};
    // clang-format on

    auto* groupNode = new GroupNode{Group{"somegroup"}};

    groupNode->addChild(patchNode);
    worldNode.defaultLayer()->addChild(groupNode);

    return std::make_tuple(groupNode, patchNode);
  }

  std::tuple<GroupNode*, EntityNode*, BrushNode*> createGroupedBrushEntity()
  {
    BrushBuilder builder(worldNode.mapFormat(), worldBounds);
    auto* brushNode = new BrushNode{builder.createCube(32.0, "sometex").value()};
    auto* entityNode = new EntityNode{Entity{}};
    auto* groupNode = new GroupNode{Group{"somegroup"}};

    entityNode->addChild(brushNode);
    groupNode->addChild(entityNode);
    worldNode.defaultLayer()->addChild(groupNode);

    return std::make_tuple(groupNode, entityNode, brushNode);
  }

  std::tuple<GroupNode*, EntityNode*, PatchNode*> createGroupedPatchEntity()
  {
    // clang-format off
    auto* patchNode = new PatchNode{BezierPatch{3, 3, {
      {0, 0, 0}, {1, 0, 1}, {2, 0, 0},
      {0, 1, 1}, {1, 1, 2}, {2, 1, 1},
      {0, 2, 0}, {1, 2, 1}, {2, 2, 0} }, "texture"}};
    // clang-format on

    auto* entityNode = new EntityNode{Entity{}};
    auto* groupNode = new GroupNode{Group{"somegroup"}};

    entityNode->addChild(patchNode);
    groupNode->addChild(entityNode);
    worldNode.defaultLayer()->addChild(groupNode);

    return std::make_tuple(groupNode, entityNode, patchNode);
  }

  std::tuple<GroupNode*, GroupNode*, BrushNode*> createdNestedGroupedBrush()
  {
    BrushBuilder builder(worldNode.mapFormat(), worldBounds);
    auto* innerBrushNode = new BrushNode{builder.createCube(32.0, "sometex").value()};
    auto* innerGroupNode = new GroupNode{Group{"inner"}};
    auto* outerGroupNode = new GroupNode{Group{"outer"}};

    innerGroupNode->addChild(innerBrushNode);
    outerGroupNode->addChild(innerGroupNode);
    worldNode.defaultLayer()->addChild(outerGroupNode);

    return std::make_tuple(outerGroupNode, innerGroupNode, innerBrushNode);
  }
};

constexpr auto V_Inherited = VisibilityState::Inherited;
constexpr auto V_Hidden = VisibilityState::Hidden;
constexpr auto V_Shown = VisibilityState::Shown;

constexpr auto L_Inherited = LockState::Inherited;
constexpr auto L_Locked = LockState::Locked;
constexpr auto L_Unlocked = LockState::Unlocked;

TEST_CASE_METHOD(EditorContextTest, "EditorContextTest.testTopLevelNodes")
{

  SECTION("World")
  {
    using T = std::tuple<VisibilityState, LockState, bool, bool, bool>;

    // clang-format off
    const auto
    [wrldVisState, wrldLckState, visible, editable, selectable] = GENERATE(values<T>({
    {V_Shown,      L_Locked,     true,    false,    false     },
    {V_Shown,      L_Unlocked,   true,    true,     false     },
    {V_Hidden,     L_Locked,     false,   false,    false     },
    {V_Hidden,     L_Unlocked,   false,   true,     false     },
    }));
    // clang-format on

    CAPTURE(wrldVisState, wrldLckState);

    worldNode.setVisibilityState(wrldVisState);
    worldNode.setLockState(wrldLckState);

    CHECK(context.visible(&worldNode) == visible);
    CHECK(context.editable(&worldNode) == editable);
    CHECK(context.selectable(&worldNode) == selectable);
  }

  SECTION("Layer")
  {
    using T = std::
      tuple<VisibilityState, LockState, VisibilityState, LockState, bool, bool, bool>;

    // clang-format off
    const auto
    [wrldVisState, wrldLckState, layrVisState, layrLckState,   visible, editable, selectable] = GENERATE(values<T>({
    {V_Shown,      L_Unlocked,   V_Inherited,  L_Inherited,    true,    true,     false     },
    {V_Shown,      L_Unlocked,   V_Inherited,  L_Locked,       true,    false,    false     },
    {V_Shown,      L_Unlocked,   V_Inherited,  L_Unlocked,     true,    true,     false     },
    {V_Shown,      L_Unlocked,   V_Shown,      L_Inherited,    true,    true,     false     },
    {V_Shown,      L_Unlocked,   V_Shown,      L_Locked,       true,    false,    false     },
    {V_Shown,      L_Unlocked,   V_Shown,      L_Unlocked,     true,    true,     false     },
    {V_Shown,      L_Unlocked,   V_Hidden,     L_Inherited,    false,   true,     false     },
    {V_Shown,      L_Unlocked,   V_Hidden,     L_Locked,       false,   false,    false     },
    {V_Shown,      L_Unlocked,   V_Hidden,     L_Unlocked,     false,   true,     false     },

    {V_Shown,      L_Locked,     V_Inherited,  L_Inherited,    true,    false,    false     },
    {V_Shown,      L_Locked,     V_Inherited,  L_Locked,       true,    false,    false     },
    {V_Shown,      L_Locked,     V_Inherited,  L_Unlocked,     true,    true,     false     },
    {V_Shown,      L_Locked,     V_Shown,      L_Inherited,    true,    false,    false     },
    {V_Shown,      L_Locked,     V_Shown,      L_Locked,       true,    false,    false     },
    {V_Shown,      L_Locked,     V_Shown,      L_Unlocked,     true,    true,     false     },
    {V_Shown,      L_Locked,     V_Hidden,     L_Inherited,    false,   false,    false     },
    {V_Shown,      L_Locked,     V_Hidden,     L_Locked,       false,   false,    false     },
    {V_Shown,      L_Locked,     V_Hidden,     L_Unlocked,     false,   true,     false     },

    {V_Hidden,     L_Unlocked,   V_Inherited,  L_Inherited,    false,   true,     false     },
    {V_Hidden,     L_Unlocked,   V_Inherited,  L_Locked,       false,   false,    false     },
    {V_Hidden,     L_Unlocked,   V_Inherited,  L_Unlocked,     false,   true,     false     },
    {V_Hidden,     L_Unlocked,   V_Shown,      L_Inherited,    true,    true,     false     },
    {V_Hidden,     L_Unlocked,   V_Shown,      L_Locked,       true,    false,    false     },
    {V_Hidden,     L_Unlocked,   V_Shown,      L_Unlocked,     true,    true,     false     },
    {V_Hidden,     L_Unlocked,   V_Hidden,     L_Inherited,    false,   true,     false     },
    {V_Hidden,     L_Unlocked,   V_Hidden,     L_Locked,       false,   false,    false     },
    {V_Hidden,     L_Unlocked,   V_Hidden,     L_Unlocked,     false,   true,     false     },

    {V_Hidden,     L_Locked,     V_Inherited,  L_Inherited,    false,   false,    false     },
    {V_Hidden,     L_Locked,     V_Inherited,  L_Locked,       false,   false,    false     },
    {V_Hidden,     L_Locked,     V_Inherited,  L_Unlocked,     false,   true,     false     },
    {V_Hidden,     L_Locked,     V_Shown,      L_Inherited,    true,    false,    false     },
    {V_Hidden,     L_Locked,     V_Shown,      L_Locked,       true,    false,    false     },
    {V_Hidden,     L_Locked,     V_Shown,      L_Unlocked,     true,    true,     false     },
    {V_Hidden,     L_Locked,     V_Hidden,     L_Inherited,    false,   false,    false     },
    {V_Hidden,     L_Locked,     V_Hidden,     L_Locked,       false,   false,    false     },
    {V_Hidden,     L_Locked,     V_Hidden,     L_Unlocked,     false,   true,     false     },
    }));
    // clang-format on

    auto* layerNode = worldNode.defaultLayer();

    CAPTURE(wrldVisState, wrldLckState, layrVisState, layrLckState);

    worldNode.setVisibilityState(wrldVisState);
    worldNode.setLockState(wrldLckState);

    layerNode->setVisibilityState(layrVisState);
    layerNode->setLockState(layrLckState);

    CHECK(context.visible(layerNode) == visible);
    CHECK(context.editable(layerNode) == editable);
    CHECK(context.selectable(layerNode) == selectable);
  }

  SECTION("Top Level Group")
  {
    using T = std::tuple<
      VisibilityState,
      LockState,
      bool,
      bool,
      VisibilityState,
      LockState,
      VisibilityState,
      bool,
      bool,
      bool>;

    // clang-format off
    const auto
    [wrldVisState, wrldLckState, open,  selected, grpVisState, grpLckState, childVisState, visible, editable, selectable] = GENERATE(values<T>({
    {V_Shown,      L_Unlocked,   false, false,    V_Inherited, L_Inherited, V_Inherited,   true,    true,     true      },
    {V_Shown,      L_Unlocked,   false, false,    V_Inherited, L_Inherited, V_Hidden,      false,   true,     false     },
    {V_Shown,      L_Unlocked,   false, false,    V_Inherited, L_Inherited, V_Shown,       true,    true,     true      },
    {V_Shown,      L_Unlocked,   false, false,    V_Inherited, L_Locked,    V_Inherited,   true,    false,    false     },
    {V_Shown,      L_Unlocked,   false, false,    V_Inherited, L_Locked,    V_Hidden,      false,   false,    false     },
    {V_Shown,      L_Unlocked,   false, false,    V_Inherited, L_Locked,    V_Shown,       true,    false,    false     },
    {V_Shown,      L_Unlocked,   false, false,    V_Inherited, L_Unlocked,  V_Inherited,   true,    true,     true      },
    {V_Shown,      L_Unlocked,   false, false,    V_Inherited, L_Unlocked,  V_Hidden,      false,   true,     false     },
    {V_Shown,      L_Unlocked,   false, false,    V_Inherited, L_Unlocked,  V_Shown,       true,    true,     true      },
    {V_Shown,      L_Unlocked,   false, false,    V_Hidden,    L_Inherited, V_Inherited,   false,   true,     false     },
    {V_Shown,      L_Unlocked,   false, false,    V_Hidden,    L_Inherited, V_Hidden,      false,   true,     false     },
    {V_Shown,      L_Unlocked,   false, false,    V_Hidden,    L_Inherited, V_Shown,       false,   true,     false     },
    {V_Shown,      L_Unlocked,   false, false,    V_Hidden,    L_Locked,    V_Inherited,   false,   false,    false     },
    {V_Shown,      L_Unlocked,   false, false,    V_Hidden,    L_Locked,    V_Hidden,      false,   false,    false     },
    {V_Shown,      L_Unlocked,   false, false,    V_Hidden,    L_Locked,    V_Shown,       false,   false,    false     },
    {V_Shown,      L_Unlocked,   false, false,    V_Hidden,    L_Unlocked,  V_Inherited,   false,   true,     false     },
    {V_Shown,      L_Unlocked,   false, false,    V_Hidden,    L_Unlocked,  V_Hidden,      false,   true,     false     },
    {V_Shown,      L_Unlocked,   false, false,    V_Hidden,    L_Unlocked,  V_Shown,       false,   true,     false     },
    {V_Shown,      L_Unlocked,   false, false,    V_Shown,     L_Inherited, V_Inherited,   true,    true,     true      },
    {V_Shown,      L_Unlocked,   false, false,    V_Shown,     L_Inherited, V_Hidden,      false,   true,     false     },
    {V_Shown,      L_Unlocked,   false, false,    V_Shown,     L_Inherited, V_Shown,       true,    true,     true      },
    {V_Shown,      L_Unlocked,   false, false,    V_Shown,     L_Locked,    V_Inherited,   true,    false,    false     },
    {V_Shown,      L_Unlocked,   false, false,    V_Shown,     L_Locked,    V_Hidden,      false,   false,    false     },
    {V_Shown,      L_Unlocked,   false, false,    V_Shown,     L_Locked,    V_Shown,       true,    false,    false     },
    {V_Shown,      L_Unlocked,   false, false,    V_Shown,     L_Unlocked,  V_Inherited,   true,    true,     true      },
    {V_Shown,      L_Unlocked,   false, false,    V_Shown,     L_Unlocked,  V_Hidden,      false,   true,     false     },
    {V_Shown,      L_Unlocked,   false, false,    V_Shown,     L_Unlocked,  V_Shown,       true,    true,     true      },
    {V_Shown,      L_Unlocked,   false, true,     V_Shown,     L_Unlocked,  V_Inherited,   true,    true,     true      },
    {V_Shown,      L_Unlocked,   false, true,     V_Shown,     L_Unlocked,  V_Hidden,      true,    true,     true      },
    {V_Shown,      L_Unlocked,   false, true,     V_Shown,     L_Unlocked,  V_Shown,       true,    true,     true      },
    {V_Shown,      L_Unlocked,   true,  false,    V_Hidden,    L_Locked,    V_Inherited,   false,   false,    false     },
    {V_Shown,      L_Unlocked,   true,  false,    V_Hidden,    L_Locked,    V_Hidden,      false,   false,    false     },
    {V_Shown,      L_Unlocked,   true,  false,    V_Hidden,    L_Locked,    V_Shown,       false,   false,    false     },
    {V_Shown,      L_Unlocked,   true,  false,    V_Hidden,    L_Unlocked,  V_Inherited,   false,   true,     false     },
    {V_Shown,      L_Unlocked,   true,  false,    V_Hidden,    L_Unlocked,  V_Hidden,      false,   true,     false     },
    {V_Shown,      L_Unlocked,   true,  false,    V_Hidden,    L_Unlocked,  V_Shown,       false,   true,     false     },
    {V_Shown,      L_Unlocked,   true,  false,    V_Shown,     L_Locked,    V_Inherited,   true,    false,    false     },
    {V_Shown,      L_Unlocked,   true,  false,    V_Shown,     L_Locked,    V_Hidden,      false,   false,    false     },
    {V_Shown,      L_Unlocked,   true,  false,    V_Shown,     L_Locked,    V_Shown,       true,    false,    false     },
    {V_Shown,      L_Unlocked,   true,  false,    V_Shown,     L_Unlocked,  V_Inherited,   true,    true,     false     },
    {V_Shown,      L_Unlocked,   true,  false,    V_Shown,     L_Unlocked,  V_Hidden,      false,   true,     false     },
    {V_Shown,      L_Unlocked,   true,  false,    V_Shown,     L_Unlocked,  V_Shown,       true,    true,     false     },

    {V_Shown,      L_Locked,     false, false,    V_Inherited, L_Inherited, V_Inherited,   true,    false,    false     },
    {V_Shown,      L_Locked,     false, false,    V_Inherited, L_Inherited, V_Hidden,      false,   false,    false     },
    {V_Shown,      L_Locked,     false, false,    V_Inherited, L_Inherited, V_Shown,       true,    false,    false     },
    {V_Shown,      L_Locked,     false, false,    V_Inherited, L_Locked,    V_Inherited,   true,    false,    false     },
    {V_Shown,      L_Locked,     false, false,    V_Inherited, L_Locked,    V_Hidden,      false,   false,    false     },
    {V_Shown,      L_Locked,     false, false,    V_Inherited, L_Locked,    V_Shown,       true,    false,    false     },
    {V_Shown,      L_Locked,     false, false,    V_Inherited, L_Unlocked,  V_Inherited,   true,    true,     true      },
    {V_Shown,      L_Locked,     false, false,    V_Inherited, L_Unlocked,  V_Hidden,      false,   true,     false     },
    {V_Shown,      L_Locked,     false, false,    V_Inherited, L_Unlocked,  V_Shown,       true,    true,     true      },
    {V_Shown,      L_Locked,     false, false,    V_Hidden,    L_Inherited, V_Inherited,   false,   false,    false     },
    {V_Shown,      L_Locked,     false, false,    V_Hidden,    L_Inherited, V_Hidden,      false,   false,    false     },
    {V_Shown,      L_Locked,     false, false,    V_Hidden,    L_Inherited, V_Shown,       false,   false,    false     },
    {V_Shown,      L_Locked,     false, false,    V_Hidden,    L_Locked,    V_Inherited,   false,   false,    false     },
    {V_Shown,      L_Locked,     false, false,    V_Hidden,    L_Locked,    V_Hidden,      false,   false,    false     },
    {V_Shown,      L_Locked,     false, false,    V_Hidden,    L_Locked,    V_Shown,       false,   false,    false     },
    {V_Shown,      L_Locked,     false, false,    V_Hidden,    L_Unlocked,  V_Inherited,   false,   true,     false     },
    {V_Shown,      L_Locked,     false, false,    V_Hidden,    L_Unlocked,  V_Hidden,      false,   true,     false     },
    {V_Shown,      L_Locked,     false, false,    V_Hidden,    L_Unlocked,  V_Shown,       false,   true,     false     },
    {V_Shown,      L_Locked,     false, false,    V_Shown,     L_Inherited, V_Inherited,   true,    false,    false     },
    {V_Shown,      L_Locked,     false, false,    V_Shown,     L_Inherited, V_Hidden,      false,   false,    false     },
    {V_Shown,      L_Locked,     false, false,    V_Shown,     L_Inherited, V_Shown,       true,    false,    false     },
    {V_Shown,      L_Locked,     false, false,    V_Shown,     L_Locked,    V_Inherited,   true,    false,    false     },
    {V_Shown,      L_Locked,     false, false,    V_Shown,     L_Locked,    V_Hidden,      false,   false,    false     },
    {V_Shown,      L_Locked,     false, false,    V_Shown,     L_Locked,    V_Shown,       true,    false,    false     },
    {V_Shown,      L_Locked,     false, false,    V_Shown,     L_Unlocked,  V_Inherited,   true,    true,     true      },
    {V_Shown,      L_Locked,     false, false,    V_Shown,     L_Unlocked,  V_Hidden,      false,   true,     false     },
    {V_Shown,      L_Locked,     false, false,    V_Shown,     L_Unlocked,  V_Shown,       true,    true,     true      },
    {V_Shown,      L_Locked,     false, true,     V_Shown,     L_Unlocked,  V_Inherited,   true,    true,     true      },
    {V_Shown,      L_Locked,     false, true,     V_Shown,     L_Unlocked,  V_Hidden,      true,    true,     true      },
    {V_Shown,      L_Locked,     false, true,     V_Shown,     L_Unlocked,  V_Shown,       true,    true,     true      },
    {V_Shown,      L_Locked,     true,  false,    V_Hidden,    L_Locked,    V_Inherited,   false,   false,    false     },
    {V_Shown,      L_Locked,     true,  false,    V_Hidden,    L_Locked,    V_Hidden,      false,   false,    false     },
    {V_Shown,      L_Locked,     true,  false,    V_Hidden,    L_Locked,    V_Shown,       false,   false,    false     },
    {V_Shown,      L_Locked,     true,  false,    V_Hidden,    L_Unlocked,  V_Inherited,   false,   true,     false     },
    {V_Shown,      L_Locked,     true,  false,    V_Hidden,    L_Unlocked,  V_Hidden,      false,   true,     false     },
    {V_Shown,      L_Locked,     true,  false,    V_Hidden,    L_Unlocked,  V_Shown,       false,   true,     false     },
    {V_Shown,      L_Locked,     true,  false,    V_Shown,     L_Locked,    V_Inherited,   true,    false,    false     },
    {V_Shown,      L_Locked,     true,  false,    V_Shown,     L_Locked,    V_Hidden,      false,   false,    false     },
    {V_Shown,      L_Locked,     true,  false,    V_Shown,     L_Locked,    V_Shown,       true,    false,    false     },
    {V_Shown,      L_Locked,     true,  false,    V_Shown,     L_Unlocked,  V_Inherited,   true,    true,     false     },
    {V_Shown,      L_Locked,     true,  false,    V_Shown,     L_Unlocked,  V_Hidden,      false,   true,     false     },
    {V_Shown,      L_Locked,     true,  false,    V_Shown,     L_Unlocked,  V_Shown,       true,    true,     false     },

    {V_Hidden,     L_Unlocked,   false, false,    V_Inherited, L_Inherited, V_Inherited,   false,   true,     false     },
    {V_Hidden,     L_Unlocked,   false, false,    V_Inherited, L_Inherited, V_Hidden,      false,   true,     false     },
    {V_Hidden,     L_Unlocked,   false, false,    V_Inherited, L_Inherited, V_Shown,       false,   true,     false     },
    {V_Hidden,     L_Unlocked,   false, false,    V_Inherited, L_Locked,    V_Inherited,   false,   false,    false     },
    {V_Hidden,     L_Unlocked,   false, false,    V_Inherited, L_Locked,    V_Hidden,      false,   false,    false     },
    {V_Hidden,     L_Unlocked,   false, false,    V_Inherited, L_Locked,    V_Shown,       false,   false,    false     },
    {V_Hidden,     L_Unlocked,   false, false,    V_Inherited, L_Unlocked,  V_Inherited,   false,   true,     false     },
    {V_Hidden,     L_Unlocked,   false, false,    V_Inherited, L_Unlocked,  V_Hidden,      false,   true,     false     },
    {V_Hidden,     L_Unlocked,   false, false,    V_Inherited, L_Unlocked,  V_Shown,       false,   true,     false     },
    {V_Hidden,     L_Unlocked,   false, false,    V_Hidden,    L_Inherited, V_Inherited,   false,   true,     false     },
    {V_Hidden,     L_Unlocked,   false, false,    V_Hidden,    L_Inherited, V_Hidden,      false,   true,     false     },
    {V_Hidden,     L_Unlocked,   false, false,    V_Hidden,    L_Inherited, V_Shown,       false,   true,     false     },
    {V_Hidden,     L_Unlocked,   false, false,    V_Hidden,    L_Locked,    V_Inherited,   false,   false,    false     },
    {V_Hidden,     L_Unlocked,   false, false,    V_Hidden,    L_Locked,    V_Hidden,      false,   false,    false     },
    {V_Hidden,     L_Unlocked,   false, false,    V_Hidden,    L_Locked,    V_Shown,       false,   false,    false     },
    {V_Hidden,     L_Unlocked,   false, false,    V_Hidden,    L_Unlocked,  V_Inherited,   false,   true,     false     },
    {V_Hidden,     L_Unlocked,   false, false,    V_Hidden,    L_Unlocked,  V_Hidden,      false,   true,     false     },
    {V_Hidden,     L_Unlocked,   false, false,    V_Hidden,    L_Unlocked,  V_Shown,       false,   true,     false     },
    {V_Hidden,     L_Unlocked,   false, false,    V_Shown,     L_Inherited, V_Inherited,   true,    true,     true      },
    {V_Hidden,     L_Unlocked,   false, false,    V_Shown,     L_Inherited, V_Hidden,      false,   true,     false     },
    {V_Hidden,     L_Unlocked,   false, false,    V_Shown,     L_Inherited, V_Shown,       true,    true,     true      },
    {V_Hidden,     L_Unlocked,   false, false,    V_Shown,     L_Locked,    V_Inherited,   true,    false,    false     },
    {V_Hidden,     L_Unlocked,   false, false,    V_Shown,     L_Locked,    V_Hidden,      false,   false,    false     },
    {V_Hidden,     L_Unlocked,   false, false,    V_Shown,     L_Locked,    V_Shown,       true,    false,    false     },
    {V_Hidden,     L_Unlocked,   false, false,    V_Shown,     L_Unlocked,  V_Inherited,   true,    true,     true      },
    {V_Hidden,     L_Unlocked,   false, false,    V_Shown,     L_Unlocked,  V_Hidden,      false,   true,     false     },
    {V_Hidden,     L_Unlocked,   false, false,    V_Shown,     L_Unlocked,  V_Shown,       true,    true,     true      },
    {V_Hidden,     L_Unlocked,   false, true,     V_Shown,     L_Unlocked,  V_Inherited,   true,    true,     true      },
    {V_Hidden,     L_Unlocked,   false, true,     V_Shown,     L_Unlocked,  V_Hidden,      true,    true,     true      },
    {V_Hidden,     L_Unlocked,   false, true,     V_Shown,     L_Unlocked,  V_Shown,       true,    true,     true      },
    {V_Hidden,     L_Unlocked,   true,  false,    V_Hidden,    L_Locked,    V_Inherited,   false,   false,    false     },
    {V_Hidden,     L_Unlocked,   true,  false,    V_Hidden,    L_Locked,    V_Hidden,      false,   false,    false     },
    {V_Hidden,     L_Unlocked,   true,  false,    V_Hidden,    L_Locked,    V_Shown,       false,   false,    false     },
    {V_Hidden,     L_Unlocked,   true,  false,    V_Hidden,    L_Unlocked,  V_Inherited,   false,   true,     false     },
    {V_Hidden,     L_Unlocked,   true,  false,    V_Hidden,    L_Unlocked,  V_Hidden,      false,   true,     false     },
    {V_Hidden,     L_Unlocked,   true,  false,    V_Hidden,    L_Unlocked,  V_Shown,       false,   true,     false     },
    {V_Hidden,     L_Unlocked,   true,  false,    V_Shown,     L_Locked,    V_Inherited,   true,    false,    false     },
    {V_Hidden,     L_Unlocked,   true,  false,    V_Shown,     L_Locked,    V_Hidden,      false,   false,    false     },
    {V_Hidden,     L_Unlocked,   true,  false,    V_Shown,     L_Locked,    V_Shown,       true,    false,    false     },
    {V_Hidden,     L_Unlocked,   true,  false,    V_Shown,     L_Unlocked,  V_Inherited,   true,    true,     false     },
    {V_Hidden,     L_Unlocked,   true,  false,    V_Shown,     L_Unlocked,  V_Hidden,      false,   true,     false     },
    {V_Hidden,     L_Unlocked,   true,  false,    V_Shown,     L_Unlocked,  V_Shown,       true,    true,     false     },
    }));
    // clang-format on

    auto [groupNode, brushNode] = createGroupedBrush();

    CAPTURE(
      wrldVisState,
      wrldLckState,
      open,
      selected,
      grpVisState,
      grpLckState,
      childVisState);

    worldNode.setVisibilityState(wrldVisState);
    worldNode.setLockState(wrldLckState);

    if (open)
    {
      context.pushGroup(groupNode);
    }

    if (selected)
    {
      groupNode->select();
    }

    groupNode->setVisibilityState(grpVisState);
    groupNode->setLockState(grpLckState);

    brushNode->setVisibilityState(childVisState);

    CHECK(context.visible(groupNode) == visible);
    CHECK(context.editable(groupNode) == editable);
    CHECK(context.selectable(groupNode) == selectable);
  }

  SECTION("Top Level Brush Entity, Patch Entity")
  {
    using T = std::tuple<
      VisibilityState,
      LockState,
      VisibilityState,
      LockState,
      VisibilityState,
      bool,
      bool,
      bool>;

    // clang-format off
    const auto
    [wrldVisState, wrldLckState, entVisState, entLockState, childVisState, visible, editable, selectable] = GENERATE(values<T>({
    {V_Shown,      L_Unlocked,   V_Inherited, L_Inherited,  V_Inherited,   true,    true,     false     },
    {V_Shown,      L_Unlocked,   V_Inherited, L_Inherited,  V_Hidden,      false,   true,     false     },
    {V_Shown,      L_Unlocked,   V_Inherited, L_Inherited,  V_Shown,       true,    true,     false     },
    {V_Shown,      L_Unlocked,   V_Inherited, L_Locked,     V_Inherited,   true,    false,    false     },
    {V_Shown,      L_Unlocked,   V_Inherited, L_Locked,     V_Hidden,      false,   false,    false     },
    {V_Shown,      L_Unlocked,   V_Inherited, L_Locked,     V_Shown,       true,    false,    false     },
    {V_Shown,      L_Unlocked,   V_Inherited, L_Unlocked,   V_Inherited,   true,    true,     false     },
    {V_Shown,      L_Unlocked,   V_Inherited, L_Unlocked,   V_Hidden,      false,   true,     false     },
    {V_Shown,      L_Unlocked,   V_Inherited, L_Unlocked,   V_Shown,       true,    true,     false     },
    {V_Shown,      L_Unlocked,   V_Hidden,    L_Inherited,  V_Inherited,   false,   true,     false     },
    {V_Shown,      L_Unlocked,   V_Hidden,    L_Inherited,  V_Hidden,      false,   true,     false     },
    {V_Shown,      L_Unlocked,   V_Hidden,    L_Inherited,  V_Shown,       true,    true,     false     },
    {V_Shown,      L_Unlocked,   V_Hidden,    L_Locked,     V_Inherited,   false,   false,    false     },
    {V_Shown,      L_Unlocked,   V_Hidden,    L_Locked,     V_Hidden,      false,   false,    false     },
    {V_Shown,      L_Unlocked,   V_Hidden,    L_Locked,     V_Shown,       true,    false,    false     },
    {V_Shown,      L_Unlocked,   V_Hidden,    L_Unlocked,   V_Inherited,   false,   true,     false     },
    {V_Shown,      L_Unlocked,   V_Hidden,    L_Unlocked,   V_Hidden,      false,   true,     false     },
    {V_Shown,      L_Unlocked,   V_Hidden,    L_Unlocked,   V_Shown,       true,    true,     false     },
    {V_Shown,      L_Unlocked,   V_Shown,     L_Inherited,  V_Inherited,   true,    true,     false     },
    {V_Shown,      L_Unlocked,   V_Shown,     L_Inherited,  V_Hidden,      false,   true,     false     },
    {V_Shown,      L_Unlocked,   V_Shown,     L_Inherited,  V_Shown,       true,    true,     false     },
    {V_Shown,      L_Unlocked,   V_Shown,     L_Locked,     V_Inherited,   true,    false,    false     },
    {V_Shown,      L_Unlocked,   V_Shown,     L_Locked,     V_Hidden,      false,   false,    false     },
    {V_Shown,      L_Unlocked,   V_Shown,     L_Locked,     V_Shown,       true,    false,    false     },
    {V_Shown,      L_Unlocked,   V_Shown,     L_Unlocked,   V_Inherited,   true,    true,     false     },
    {V_Shown,      L_Unlocked,   V_Shown,     L_Unlocked,   V_Hidden,      false,   true,     false     },
    {V_Shown,      L_Unlocked,   V_Shown,     L_Unlocked,   V_Shown,       true,    true,     false     },

    {V_Shown,      L_Locked,     V_Inherited, L_Inherited,  V_Inherited,   true,    false,    false     },
    {V_Shown,      L_Locked,     V_Inherited, L_Inherited,  V_Hidden,      false,   false,    false     },
    {V_Shown,      L_Locked,     V_Inherited, L_Inherited,  V_Shown,       true,    false,    false     },
    {V_Shown,      L_Locked,     V_Inherited, L_Locked,     V_Inherited,   true,    false,    false     },
    {V_Shown,      L_Locked,     V_Inherited, L_Locked,     V_Hidden,      false,   false,    false     },
    {V_Shown,      L_Locked,     V_Inherited, L_Locked,     V_Shown,       true,    false,    false     },
    {V_Shown,      L_Locked,     V_Inherited, L_Unlocked,   V_Inherited,   true,    true,     false     },
    {V_Shown,      L_Locked,     V_Inherited, L_Unlocked,   V_Hidden,      false,   true,     false     },
    {V_Shown,      L_Locked,     V_Inherited, L_Unlocked,   V_Shown,       true,    true,     false     },
    {V_Shown,      L_Locked,     V_Hidden,    L_Inherited,  V_Inherited,   false,   false,    false     },
    {V_Shown,      L_Locked,     V_Hidden,    L_Inherited,  V_Hidden,      false,   false,    false     },
    {V_Shown,      L_Locked,     V_Hidden,    L_Inherited,  V_Shown,       true,    false,    false     },
    {V_Shown,      L_Locked,     V_Hidden,    L_Locked,     V_Inherited,   false,   false,    false     },
    {V_Shown,      L_Locked,     V_Hidden,    L_Locked,     V_Hidden,      false,   false,    false     },
    {V_Shown,      L_Locked,     V_Hidden,    L_Locked,     V_Shown,       true,    false,    false     },
    {V_Shown,      L_Locked,     V_Hidden,    L_Unlocked,   V_Inherited,   false,   true,     false     },
    {V_Shown,      L_Locked,     V_Hidden,    L_Unlocked,   V_Hidden,      false,   true,     false     },
    {V_Shown,      L_Locked,     V_Hidden,    L_Unlocked,   V_Shown,       true,    true,     false     },
    {V_Shown,      L_Locked,     V_Shown,     L_Inherited,  V_Inherited,   true,    false,    false     },
    {V_Shown,      L_Locked,     V_Shown,     L_Inherited,  V_Hidden,      false,   false,    false     },
    {V_Shown,      L_Locked,     V_Shown,     L_Inherited,  V_Shown,       true,    false,    false     },
    {V_Shown,      L_Locked,     V_Shown,     L_Locked,     V_Inherited,   true,    false,    false     },
    {V_Shown,      L_Locked,     V_Shown,     L_Locked,     V_Hidden,      false,   false,    false     },
    {V_Shown,      L_Locked,     V_Shown,     L_Locked,     V_Shown,       true,    false,    false     },
    {V_Shown,      L_Locked,     V_Shown,     L_Unlocked,   V_Inherited,   true,    true,     false     },
    {V_Shown,      L_Locked,     V_Shown,     L_Unlocked,   V_Hidden,      false,   true,     false     },
    {V_Shown,      L_Locked,     V_Shown,     L_Unlocked,   V_Shown,       true,    true,     false     },

    {V_Hidden,     L_Unlocked,   V_Inherited, L_Inherited,  V_Inherited,   false,   true,     false     },
    {V_Hidden,     L_Unlocked,   V_Inherited, L_Inherited,  V_Hidden,      false,   true,     false     },
    {V_Hidden,     L_Unlocked,   V_Inherited, L_Inherited,  V_Shown,       true,    true,     false     },
    {V_Hidden,     L_Unlocked,   V_Inherited, L_Locked,     V_Inherited,   false,   false,    false     },
    {V_Hidden,     L_Unlocked,   V_Inherited, L_Locked,     V_Hidden,      false,   false,    false     },
    {V_Hidden,     L_Unlocked,   V_Inherited, L_Locked,     V_Shown,       true,    false,    false     },
    {V_Hidden,     L_Unlocked,   V_Inherited, L_Unlocked,   V_Inherited,   false,   true,     false     },
    {V_Hidden,     L_Unlocked,   V_Inherited, L_Unlocked,   V_Hidden,      false,   true,     false     },
    {V_Hidden,     L_Unlocked,   V_Inherited, L_Unlocked,   V_Shown,       true,    true,     false     },
    {V_Hidden,     L_Unlocked,   V_Hidden,    L_Inherited,  V_Inherited,   false,   true,     false     },
    {V_Hidden,     L_Unlocked,   V_Hidden,    L_Inherited,  V_Hidden,      false,   true,     false     },
    {V_Hidden,     L_Unlocked,   V_Hidden,    L_Inherited,  V_Shown,       true,    true,     false     },
    {V_Hidden,     L_Unlocked,   V_Hidden,    L_Locked,     V_Inherited,   false,   false,    false     },
    {V_Hidden,     L_Unlocked,   V_Hidden,    L_Locked,     V_Hidden,      false,   false,    false     },
    {V_Hidden,     L_Unlocked,   V_Hidden,    L_Locked,     V_Shown,       true,    false,    false     },
    {V_Hidden,     L_Unlocked,   V_Hidden,    L_Unlocked,   V_Inherited,   false,   true,     false     },
    {V_Hidden,     L_Unlocked,   V_Hidden,    L_Unlocked,   V_Hidden,      false,   true,     false     },
    {V_Hidden,     L_Unlocked,   V_Hidden,    L_Unlocked,   V_Shown,       true,    true,     false     },
    {V_Hidden,     L_Unlocked,   V_Shown,     L_Inherited,  V_Inherited,   true,    true,     false     },
    {V_Hidden,     L_Unlocked,   V_Shown,     L_Inherited,  V_Hidden,      false,   true,     false     },
    {V_Hidden,     L_Unlocked,   V_Shown,     L_Inherited,  V_Shown,       true,    true,     false     },
    {V_Hidden,     L_Unlocked,   V_Shown,     L_Locked,     V_Inherited,   true,    false,    false     },
    {V_Hidden,     L_Unlocked,   V_Shown,     L_Locked,     V_Hidden,      false,   false,    false     },
    {V_Hidden,     L_Unlocked,   V_Shown,     L_Locked,     V_Shown,       true,    false,    false     },
    {V_Hidden,     L_Unlocked,   V_Shown,     L_Unlocked,   V_Inherited,   true,    true,     false     },
    {V_Hidden,     L_Unlocked,   V_Shown,     L_Unlocked,   V_Hidden,      false,   true,     false     },
    {V_Hidden,     L_Unlocked,   V_Shown,     L_Unlocked,   V_Shown,       true,    true,     false     },
    }));
    // clang-format on

    using GetNodes = std::function<std::tuple<EntityNode*, Node*>(EditorContextTest&)>;
    const GetNodes getNodes = GENERATE_COPY(
      GetNodes{[](auto& test) { return test.createTopLevelBrushEntity(); }},
      GetNodes{[](auto& test) { return test.createTopLevelPatchEntity(); }});

    auto [entityNode, childNode] = getNodes(*this);

    CAPTURE(
      childNode->name(),
      wrldVisState,
      wrldLckState,
      entVisState,
      entLockState,
      childVisState);

    worldNode.setVisibilityState(wrldVisState);
    worldNode.setLockState(wrldLckState);

    entityNode->setVisibilityState(entVisState);
    entityNode->setLockState(entLockState);

    childNode->setVisibilityState(childVisState);

    CHECK(context.visible(entityNode) == visible);
    CHECK(context.editable(entityNode) == editable);
    CHECK(context.selectable(entityNode) == selectable);
  }

  SECTION("Top Level Point Entity")
  {
    using T = std::tuple<
      VisibilityState,
      LockState,
      bool,
      VisibilityState,
      LockState,
      bool,
      bool,
      bool>;

    // clang-format off
    const auto
    [wrldVisState, wrldLckState, prefValue, entVisState, entLckState, visible, editable, selectable] = GENERATE(values<T>({
    {V_Shown,      L_Unlocked,   true,      V_Inherited, L_Inherited, true,    true,     true      },
    {V_Shown,      L_Unlocked,   true,      V_Inherited, L_Locked,    true,    false,    false     },
    {V_Shown,      L_Unlocked,   true,      V_Inherited, L_Unlocked,  true,    true,     true      },
    {V_Shown,      L_Unlocked,   true,      V_Shown,     L_Inherited, true,    true,     true      },
    {V_Shown,      L_Unlocked,   true,      V_Shown,     L_Locked,    true,    false,    false     },
    {V_Shown,      L_Unlocked,   true,      V_Shown,     L_Unlocked,  true,    true,     true      },
    {V_Shown,      L_Unlocked,   true,      V_Hidden,    L_Inherited, false,   true,     false     },
    {V_Shown,      L_Unlocked,   true,      V_Hidden,    L_Locked,    false,   false,    false     },
    {V_Shown,      L_Unlocked,   true,      V_Hidden,    L_Unlocked,  false,   true,     false     },
    {V_Shown,      L_Unlocked,   false,     V_Inherited, L_Inherited, false,   true,     false     },
    {V_Shown,      L_Unlocked,   false,     V_Inherited, L_Locked,    false,   false,    false     },
    {V_Shown,      L_Unlocked,   false,     V_Inherited, L_Unlocked,  false,   true,     false     },
    {V_Shown,      L_Unlocked,   false,     V_Shown,     L_Inherited, false,   true,     false     },
    {V_Shown,      L_Unlocked,   false,     V_Shown,     L_Locked,    false,   false,    false     },
    {V_Shown,      L_Unlocked,   false,     V_Shown,     L_Unlocked,  false,   true,     false     },
    {V_Shown,      L_Unlocked,   false,     V_Hidden,    L_Inherited, false,   true,     false     },
    {V_Shown,      L_Unlocked,   false,     V_Hidden,    L_Locked,    false,   false,    false     },
    {V_Shown,      L_Unlocked,   false,     V_Hidden,    L_Unlocked,  false,   true,     false     },

    {V_Shown,      L_Locked,     true,      V_Inherited, L_Inherited, true,    false,    false     },
    {V_Shown,      L_Locked,     true,      V_Inherited, L_Locked,    true,    false,    false     },
    {V_Shown,      L_Locked,     true,      V_Inherited, L_Unlocked,  true,    true,     true      },
    {V_Shown,      L_Locked,     true,      V_Shown,     L_Inherited, true,    false,    false     },
    {V_Shown,      L_Locked,     true,      V_Shown,     L_Locked,    true,    false,    false     },
    {V_Shown,      L_Locked,     true,      V_Shown,     L_Unlocked,  true,    true,     true      },
    {V_Shown,      L_Locked,     true,      V_Hidden,    L_Inherited, false,   false,    false     },
    {V_Shown,      L_Locked,     true,      V_Hidden,    L_Locked,    false,   false,    false     },
    {V_Shown,      L_Locked,     true,      V_Hidden,    L_Unlocked,  false,   true,     false     },
    {V_Shown,      L_Locked,     false,     V_Inherited, L_Inherited, false,   false,    false     },
    {V_Shown,      L_Locked,     false,     V_Inherited, L_Locked,    false,   false,    false     },
    {V_Shown,      L_Locked,     false,     V_Inherited, L_Unlocked,  false,   true,     false     },
    {V_Shown,      L_Locked,     false,     V_Shown,     L_Inherited, false,   false,    false     },
    {V_Shown,      L_Locked,     false,     V_Shown,     L_Locked,    false,   false,    false     },
    {V_Shown,      L_Locked,     false,     V_Shown,     L_Unlocked,  false,   true,     false     },
    {V_Shown,      L_Locked,     false,     V_Hidden,    L_Inherited, false,   false,    false     },
    {V_Shown,      L_Locked,     false,     V_Hidden,    L_Locked,    false,   false,    false     },
    {V_Shown,      L_Locked,     false,     V_Hidden,    L_Unlocked,  false,   true,     false     },

    {V_Hidden,     L_Unlocked,   true,      V_Inherited, L_Inherited, false,   true,     false     },
    {V_Hidden,     L_Unlocked,   true,      V_Inherited, L_Locked,    false,   false,    false     },
    {V_Hidden,     L_Unlocked,   true,      V_Inherited, L_Unlocked,  false,   true,     false     },
    {V_Hidden,     L_Unlocked,   true,      V_Shown,     L_Inherited, true,    true,     true      },
    {V_Hidden,     L_Unlocked,   true,      V_Shown,     L_Locked,    true,    false,    false     },
    {V_Hidden,     L_Unlocked,   true,      V_Shown,     L_Unlocked,  true,    true,     true      },
    {V_Hidden,     L_Unlocked,   true,      V_Hidden,    L_Inherited, false,   true,     false     },
    {V_Hidden,     L_Unlocked,   true,      V_Hidden,    L_Locked,    false,   false,    false     },
    {V_Hidden,     L_Unlocked,   true,      V_Hidden,    L_Unlocked,  false,   true,     false     },
    {V_Hidden,     L_Unlocked,   false,     V_Inherited, L_Inherited, false,   true,     false     },
    {V_Hidden,     L_Unlocked,   false,     V_Inherited, L_Locked,    false,   false,    false     },
    {V_Hidden,     L_Unlocked,   false,     V_Inherited, L_Unlocked,  false,   true,     false     },
    {V_Hidden,     L_Unlocked,   false,     V_Shown,     L_Inherited, false,   true,     false     },
    {V_Hidden,     L_Unlocked,   false,     V_Shown,     L_Locked,    false,   false,    false     },
    {V_Hidden,     L_Unlocked,   false,     V_Shown,     L_Unlocked,  false,   true,     false     },
    {V_Hidden,     L_Unlocked,   false,     V_Hidden,    L_Inherited, false,   true,     false     },
    {V_Hidden,     L_Unlocked,   false,     V_Hidden,    L_Locked,    false,   false,    false     },
    {V_Hidden,     L_Unlocked,   false,     V_Hidden,    L_Unlocked,  false,   true,     false     },
    }));
    // clang-format on

    auto* entityNode = createTopLevelPointEntity();

    CAPTURE(wrldVisState, wrldLckState, prefValue, entVisState, entLckState);

    worldNode.setVisibilityState(wrldVisState);
    worldNode.setLockState(wrldLckState);

    const auto setPref = TemporarilySetPref{Preferences::ShowPointEntities, prefValue};

    entityNode->setVisibilityState(entVisState);
    entityNode->setLockState(entLckState);

    CHECK(context.visible(entityNode) == visible);
    CHECK(context.editable(entityNode) == editable);
    CHECK(context.selectable(entityNode) == selectable);
  }

  SECTION("Top Level Brush, Patch")
  {
    using T = std::
      tuple<VisibilityState, LockState, VisibilityState, LockState, bool, bool, bool>;

    // clang-format off
    const auto
    [wrldVisState, wrldLckState, nodeVisState, nodeLckState, visible, editable, selectable] = GENERATE(values<T>({
    {V_Shown,      L_Unlocked,   V_Inherited,  L_Inherited,  true,    true,     true      },
    {V_Shown,      L_Unlocked,   V_Inherited,  L_Locked,     true,    false,    false     },
    {V_Shown,      L_Unlocked,   V_Inherited,  L_Unlocked,   true,    true,     true      },
    {V_Shown,      L_Unlocked,   V_Shown,      L_Inherited,  true,    true,     true      },
    {V_Shown,      L_Unlocked,   V_Shown,      L_Locked,     true,    false,    false     },
    {V_Shown,      L_Unlocked,   V_Shown,      L_Unlocked,   true,    true,     true      },
    {V_Shown,      L_Unlocked,   V_Hidden,     L_Inherited,  false,   true,     false     },
    {V_Shown,      L_Unlocked,   V_Hidden,     L_Locked,     false,   false,    false     },
    {V_Shown,      L_Unlocked,   V_Hidden,     L_Unlocked,   false,   true,     false     },

    {V_Shown,      L_Locked,     V_Inherited,  L_Inherited,  true,    false,    false     },
    {V_Shown,      L_Locked,     V_Inherited,  L_Locked,     true,    false,    false     },
    {V_Shown,      L_Locked,     V_Inherited,  L_Unlocked,   true,    true,     true      },
    {V_Shown,      L_Locked,     V_Shown,      L_Inherited,  true,    false,    false     },
    {V_Shown,      L_Locked,     V_Shown,      L_Locked,     true,    false,    false     },
    {V_Shown,      L_Locked,     V_Shown,      L_Unlocked,   true,    true,     true      },
    {V_Shown,      L_Locked,     V_Hidden,     L_Inherited,  false,   false,    false     },
    {V_Shown,      L_Locked,     V_Hidden,     L_Locked,     false,   false,    false     },
    {V_Shown,      L_Locked,     V_Hidden,     L_Unlocked,   false,   true,     false     },

    {V_Hidden,     L_Unlocked,   V_Inherited,  L_Inherited,  false,   true,     false     },
    {V_Hidden,     L_Unlocked,   V_Inherited,  L_Locked,     false,   false,    false     },
    {V_Hidden,     L_Unlocked,   V_Inherited,  L_Unlocked,   false,   true,     false     },
    {V_Hidden,     L_Unlocked,   V_Shown,      L_Inherited,  true,    true,     true      },
    {V_Hidden,     L_Unlocked,   V_Shown,      L_Locked,     true,    false,    false     },
    {V_Hidden,     L_Unlocked,   V_Shown,      L_Unlocked,   true,    true,     true      },
    {V_Hidden,     L_Unlocked,   V_Hidden,     L_Inherited,  false,   true,     false     },
    {V_Hidden,     L_Unlocked,   V_Hidden,     L_Locked,     false,   false,    false     },
    {V_Hidden,     L_Unlocked,   V_Hidden,     L_Unlocked,   false,   true,     false     },
    }));
    // clang-format on

    using GetNode = std::function<Node*(EditorContextTest&)>;
    const GetNode getNode = GENERATE_COPY(
      GetNode{[](auto& test) { return test.createTopLevelBrush(); }},
      GetNode{[](auto& test) { return test.createTopLevelPatch(); }});

    auto* node = getNode(*this);

    CAPTURE(node->name(), wrldVisState, wrldLckState, nodeVisState, nodeLckState);

    worldNode.setVisibilityState(wrldVisState);
    worldNode.setLockState(wrldLckState);

    node->setVisibilityState(nodeVisState);
    node->setLockState(nodeLckState);

    CHECK(context.visible(node) == visible);
    CHECK(context.editable(node) == editable);
    CHECK(context.selectable(node) == selectable);
  }
}

TEST_CASE_METHOD(EditorContextTest, "EditorContextTest.testGroupedNodes")
{
  SECTION("Nested group")
  {
    using T = std::tuple<
      bool,
      bool,
      bool,
      bool,
      VisibilityState,
      LockState,
      VisibilityState,
      LockState,
      bool,
      bool,
      bool>;

    // clang-format off
    const auto
    [outOpen, innOpen, outSel, innSel, outVisState, outLckState, innVisState, innLckState, visible, editable, selectable] = GENERATE(values<T>({
    {false,   false,   false,  false,  V_Hidden,    L_Locked,    V_Inherited, L_Inherited, false,   false,    false      },
    {false,   false,   false,  false,  V_Hidden,    L_Locked,    V_Inherited, L_Locked,    false,   false,    false      },
    {false,   false,   false,  false,  V_Hidden,    L_Locked,    V_Inherited, L_Unlocked,  false,   true,     false      },
    {false,   false,   false,  false,  V_Hidden,    L_Locked,    V_Hidden,    L_Inherited, false,   false,    false      },
    {false,   false,   false,  false,  V_Hidden,    L_Locked,    V_Hidden,    L_Locked,    false,   false,    false      },
    {false,   false,   false,  false,  V_Hidden,    L_Locked,    V_Hidden,    L_Unlocked,  false,   true,     false      },
    {false,   false,   false,  false,  V_Hidden,    L_Locked,    V_Shown,     L_Inherited, true,    false,    false      },
    {false,   false,   false,  false,  V_Hidden,    L_Locked,    V_Shown,     L_Locked,    true,    false,    false      },
    {false,   false,   false,  false,  V_Hidden,    L_Locked,    V_Shown,     L_Unlocked,  true,    true,     false      },
    {false,   false,   false,  false,  V_Hidden,    L_Unlocked,  V_Inherited, L_Inherited, false,   true,     false      },
    {false,   false,   false,  false,  V_Hidden,    L_Unlocked,  V_Inherited, L_Locked,    false,   false,    false      },
    {false,   false,   false,  false,  V_Hidden,    L_Unlocked,  V_Inherited, L_Unlocked,  false,   true,     false      },
    {false,   false,   false,  false,  V_Hidden,    L_Unlocked,  V_Hidden,    L_Inherited, false,   true,     false      },
    {false,   false,   false,  false,  V_Hidden,    L_Unlocked,  V_Hidden,    L_Locked,    false,   false,    false      },
    {false,   false,   false,  false,  V_Hidden,    L_Unlocked,  V_Hidden,    L_Unlocked,  false,   true,     false      },
    {false,   false,   false,  false,  V_Hidden,    L_Unlocked,  V_Shown,     L_Inherited, true,    true,     false      },
    {false,   false,   false,  false,  V_Hidden,    L_Unlocked,  V_Shown,     L_Locked,    true,    false,    false      },
    {false,   false,   false,  false,  V_Hidden,    L_Unlocked,  V_Shown,     L_Unlocked,  true,    true,     false      },
    {false,   false,   false,  false,  V_Shown,     L_Locked,    V_Inherited, L_Inherited, true,    false,    false      },
    {false,   false,   false,  false,  V_Shown,     L_Locked,    V_Inherited, L_Locked,    true,    false,    false      },
    {false,   false,   false,  false,  V_Shown,     L_Locked,    V_Inherited, L_Unlocked,  true,    true,     false      },
    {false,   false,   false,  false,  V_Shown,     L_Locked,    V_Hidden,    L_Inherited, false,   false,    false      },
    {false,   false,   false,  false,  V_Shown,     L_Locked,    V_Hidden,    L_Locked,    false,   false,    false      },
    {false,   false,   false,  false,  V_Shown,     L_Locked,    V_Hidden,    L_Unlocked,  false,   true,     false      },
    {false,   false,   false,  false,  V_Shown,     L_Locked,    V_Shown,     L_Inherited, true,    false,    false      },
    {false,   false,   false,  false,  V_Shown,     L_Locked,    V_Shown,     L_Locked,    true,    false,    false      },
    {false,   false,   false,  false,  V_Shown,     L_Locked,    V_Shown,     L_Unlocked,  true,    true,     false      },
    {false,   false,   false,  false,  V_Shown,     L_Unlocked,  V_Inherited, L_Inherited, true,    true,     false      },
    {false,   false,   false,  false,  V_Shown,     L_Unlocked,  V_Inherited, L_Locked,    true,    false,    false      },
    {false,   false,   false,  false,  V_Shown,     L_Unlocked,  V_Inherited, L_Unlocked,  true,    true,     false      },
    {false,   false,   false,  false,  V_Shown,     L_Unlocked,  V_Hidden,    L_Inherited, false,   true,     false      },
    {false,   false,   false,  false,  V_Shown,     L_Unlocked,  V_Hidden,    L_Locked,    false,   false,    false      },
    {false,   false,   false,  false,  V_Shown,     L_Unlocked,  V_Hidden,    L_Unlocked,  false,   true,     false      },
    {false,   false,   false,  false,  V_Shown,     L_Unlocked,  V_Shown,     L_Inherited, true,    true,     false      },
    {false,   false,   false,  false,  V_Shown,     L_Unlocked,  V_Shown,     L_Locked,    true,    false,    false      },
    {false,   false,   false,  false,  V_Shown,     L_Unlocked,  V_Shown,     L_Unlocked,  true,    true,     false      },

    {false,   false,   true,   false,  V_Hidden,    L_Unlocked,  V_Inherited, L_Inherited, false,   true,     false      },
    {false,   false,   true,   false,  V_Hidden,    L_Unlocked,  V_Inherited, L_Locked,    false,   false,    false      },
    {false,   false,   true,   false,  V_Hidden,    L_Unlocked,  V_Inherited, L_Unlocked,  false,   true,     false      },
    {false,   false,   true,   false,  V_Hidden,    L_Unlocked,  V_Hidden,    L_Inherited, false,   true,     false      },
    {false,   false,   true,   false,  V_Hidden,    L_Unlocked,  V_Hidden,    L_Locked,    false,   false,    false      },
    {false,   false,   true,   false,  V_Hidden,    L_Unlocked,  V_Hidden,    L_Unlocked,  false,   true,     false      },
    {false,   false,   true,   false,  V_Hidden,    L_Unlocked,  V_Shown,     L_Inherited, true,    true,     false      },
    {false,   false,   true,   false,  V_Hidden,    L_Unlocked,  V_Shown,     L_Locked,    true,    false,    false      },
    {false,   false,   true,   false,  V_Hidden,    L_Unlocked,  V_Shown,     L_Unlocked,  true,    true,     false      },
    {false,   false,   true,   false,  V_Shown,     L_Unlocked,  V_Inherited, L_Inherited, true,    true,     false      },
    {false,   false,   true,   false,  V_Shown,     L_Unlocked,  V_Inherited, L_Locked,    true,    false,    false      },
    {false,   false,   true,   false,  V_Shown,     L_Unlocked,  V_Inherited, L_Unlocked,  true,    true,     false      },
    {false,   false,   true,   false,  V_Shown,     L_Unlocked,  V_Hidden,    L_Inherited, false,   true,     false      },
    {false,   false,   true,   false,  V_Shown,     L_Unlocked,  V_Hidden,    L_Locked,    false,   false,    false      },
    {false,   false,   true,   false,  V_Shown,     L_Unlocked,  V_Hidden,    L_Unlocked,  false,   true,     false      },
    {false,   false,   true,   false,  V_Shown,     L_Unlocked,  V_Shown,     L_Inherited, true,    true,     false      },
    {false,   false,   true,   false,  V_Shown,     L_Unlocked,  V_Shown,     L_Locked,    true,    false,    false      },
    {false,   false,   true,   false,  V_Shown,     L_Unlocked,  V_Shown,     L_Unlocked,  true,    true,     false      },

    {true,    false,   false,  false,  V_Hidden,    L_Unlocked,  V_Inherited, L_Inherited, false,   true,     false      },
    {true,    false,   false,  false,  V_Hidden,    L_Unlocked,  V_Inherited, L_Locked,    false,   false,    false      },
    {true,    false,   false,  false,  V_Hidden,    L_Unlocked,  V_Inherited, L_Unlocked,  false,   true,     false      },
    {true,    false,   false,  false,  V_Hidden,    L_Unlocked,  V_Hidden,    L_Inherited, false,   true,     false      },
    {true,    false,   false,  false,  V_Hidden,    L_Unlocked,  V_Hidden,    L_Locked,    false,   false,    false      },
    {true,    false,   false,  false,  V_Hidden,    L_Unlocked,  V_Hidden,    L_Unlocked,  false,   true,     false      },
    {true,    false,   false,  false,  V_Hidden,    L_Unlocked,  V_Shown,     L_Inherited, true,    true,     true       },
    {true,    false,   false,  false,  V_Hidden,    L_Unlocked,  V_Shown,     L_Locked,    true,    false,    false      },
    {true,    false,   false,  false,  V_Hidden,    L_Unlocked,  V_Shown,     L_Unlocked,  true,    true,     true       },
    {true,    false,   false,  false,  V_Shown,     L_Unlocked,  V_Inherited, L_Inherited, true,    true,     true       },
    {true,    false,   false,  false,  V_Shown,     L_Unlocked,  V_Inherited, L_Locked,    true,    false,    false      },
    {true,    false,   false,  false,  V_Shown,     L_Unlocked,  V_Inherited, L_Unlocked,  true,    true,     true       },
    {true,    false,   false,  false,  V_Shown,     L_Unlocked,  V_Hidden,    L_Inherited, false,   true,     false      },
    {true,    false,   false,  false,  V_Shown,     L_Unlocked,  V_Hidden,    L_Locked,    false,   false,    false      },
    {true,    false,   false,  false,  V_Shown,     L_Unlocked,  V_Hidden,    L_Unlocked,  false,   true,     false      },
    {true,    false,   false,  false,  V_Shown,     L_Unlocked,  V_Shown,     L_Inherited, true,    true,     true       },
    {true,    false,   false,  false,  V_Shown,     L_Unlocked,  V_Shown,     L_Locked,    true,    false,    false      },
    {true,    false,   false,  false,  V_Shown,     L_Unlocked,  V_Shown,     L_Unlocked,  true,    true,     true       },

    {true,    true,    false,  false,  V_Hidden,    L_Unlocked,  V_Inherited, L_Inherited, false,   true,     false      },
    {true,    true,    false,  false,  V_Hidden,    L_Unlocked,  V_Inherited, L_Locked,    false,   false,    false      },
    {true,    true,    false,  false,  V_Hidden,    L_Unlocked,  V_Inherited, L_Unlocked,  false,   true,     false      },
    {true,    true,    false,  false,  V_Hidden,    L_Unlocked,  V_Hidden,    L_Inherited, false,   true,     false      },
    {true,    true,    false,  false,  V_Hidden,    L_Unlocked,  V_Hidden,    L_Locked,    false,   false,    false      },
    {true,    true,    false,  false,  V_Hidden,    L_Unlocked,  V_Hidden,    L_Unlocked,  false,   true,     false      },
    {true,    true,    false,  false,  V_Hidden,    L_Unlocked,  V_Shown,     L_Inherited, true,    true,     false      },
    {true,    true,    false,  false,  V_Hidden,    L_Unlocked,  V_Shown,     L_Locked,    true,    false,    false      },
    {true,    true,    false,  false,  V_Hidden,    L_Unlocked,  V_Shown,     L_Unlocked,  true,    true,     false      },
    {true,    true,    false,  false,  V_Shown,     L_Unlocked,  V_Inherited, L_Inherited, true,    true,     false      },
    {true,    true,    false,  false,  V_Shown,     L_Unlocked,  V_Inherited, L_Locked,    true,    false,    false      },
    {true,    true,    false,  false,  V_Shown,     L_Unlocked,  V_Inherited, L_Unlocked,  true,    true,     false      },
    {true,    true,    false,  false,  V_Shown,     L_Unlocked,  V_Hidden,    L_Inherited, false,   true,     false      },
    {true,    true,    false,  false,  V_Shown,     L_Unlocked,  V_Hidden,    L_Locked,    false,   false,    false      },
    {true,    true,    false,  false,  V_Shown,     L_Unlocked,  V_Hidden,    L_Unlocked,  false,   true,     false      },
    {true,    true,    false,  false,  V_Shown,     L_Unlocked,  V_Shown,     L_Inherited, true,    true,     false      },
    {true,    true,    false,  false,  V_Shown,     L_Unlocked,  V_Shown,     L_Locked,    true,    false,    false      },
    {true,    true,    false,  false,  V_Shown,     L_Unlocked,  V_Shown,     L_Unlocked,  true,    true,     false      },

    {true,    false,   false,  true,   V_Hidden,    L_Unlocked,  V_Inherited, L_Inherited, true,    true,     true       },
    {true,    false,   false,  true,   V_Hidden,    L_Unlocked,  V_Inherited, L_Unlocked,  true,    true,     true       },
    {true,    false,   false,  true,   V_Hidden,    L_Unlocked,  V_Hidden,    L_Inherited, true,    true,     true       },
    {true,    false,   false,  true,   V_Hidden,    L_Unlocked,  V_Hidden,    L_Unlocked,  true,    true,     true       },
    {true,    false,   false,  true,   V_Hidden,    L_Unlocked,  V_Shown,     L_Inherited, true,    true,     true       },
    {true,    false,   false,  true,   V_Hidden,    L_Unlocked,  V_Shown,     L_Unlocked,  true,    true,     true       },
    {true,    false,   false,  true,   V_Shown,     L_Unlocked,  V_Inherited, L_Inherited, true,    true,     true       },
    {true,    false,   false,  true,   V_Shown,     L_Unlocked,  V_Inherited, L_Unlocked,  true,    true,     true       },
    {true,    false,   false,  true,   V_Shown,     L_Unlocked,  V_Hidden,    L_Inherited, true,    true,     true       },
    {true,    false,   false,  true,   V_Shown,     L_Unlocked,  V_Hidden,    L_Unlocked,  true,    true,     true       },
    {true,    false,   false,  true,   V_Shown,     L_Unlocked,  V_Shown,     L_Inherited, true,    true,     true       },
    {true,    false,   false,  true,   V_Shown,     L_Unlocked,  V_Shown,     L_Unlocked,  true,    true,     true       },
    }));
    // clang-format on

    auto [outerGroupNode, innerGroupNode, brushNode] = createdNestedGroupedBrush();

    CAPTURE(
      outOpen,
      innOpen,
      outSel,
      innSel,
      outVisState,
      outLckState,
      innVisState,
      innLckState);

    REQUIRE((!innOpen || outOpen)); // inner group open implies outer group open
    REQUIRE((!outSel || !innSel));  // both cannot be selected
    REQUIRE((!outSel || !outOpen)); // outer group selected implies it's closed
    REQUIRE((!innSel || !innOpen)); // inner group selected implies it's closed
    REQUIRE((!innSel || outOpen));  // inner group selected implies outer group is open

    if (outOpen)
    {
      context.pushGroup(outerGroupNode);
    }

    if (innOpen)
    {
      context.pushGroup(innerGroupNode);
    }

    if (outSel)
    {
      outerGroupNode->select();
    }

    if (innSel)
    {
      innerGroupNode->select();
    }

    outerGroupNode->setVisibilityState(outVisState);
    outerGroupNode->setLockState(outLckState);
    innerGroupNode->setVisibilityState(innVisState);
    innerGroupNode->setLockState(innLckState);

    CHECK(context.visible(innerGroupNode) == visible);
    CHECK(context.editable(innerGroupNode) == editable);
    CHECK(context.selectable(innerGroupNode) == selectable);
  }

  SECTION("Grouped Point Entity, Grouped Brush, Grouped Patch")
  {
    using T = std::tuple<
      bool,
      VisibilityState,
      LockState,
      VisibilityState,
      LockState,
      bool,
      bool,
      bool>;

    // clang-format off
    const auto
    [grpOpen, grpVisState, grpLckState, entVisState, entLckState, visible, editable, selectable] = GENERATE(values<T>({
    {false,   V_Shown,     L_Unlocked,  V_Inherited, L_Inherited, true,    true,     false      },
    {false,   V_Shown,     L_Unlocked,  V_Inherited, L_Locked,    true,    false,    false      },
    {false,   V_Shown,     L_Unlocked,  V_Inherited, L_Unlocked,  true,    true,     false      },
    {false,   V_Shown,     L_Unlocked,  V_Shown,     L_Inherited, true,    true,     false      },
    {false,   V_Shown,     L_Unlocked,  V_Shown,     L_Locked,    true,    false,    false      },
    {false,   V_Shown,     L_Unlocked,  V_Shown,     L_Unlocked,  true,    true,     false      },
    {false,   V_Shown,     L_Unlocked,  V_Hidden,    L_Inherited, false,   true,     false      },
    {false,   V_Shown,     L_Unlocked,  V_Hidden,    L_Locked,    false,   false,    false      },
    {false,   V_Shown,     L_Unlocked,  V_Hidden,    L_Unlocked,  false,   true,     false      },

    {false,   V_Shown,     L_Locked,    V_Inherited, L_Inherited, true,    false,    false      },
    {false,   V_Shown,     L_Locked,    V_Inherited, L_Locked,    true,    false,    false      },
    {false,   V_Shown,     L_Locked,    V_Inherited, L_Unlocked,  true,    true,     false      },
    {false,   V_Shown,     L_Locked,    V_Shown,     L_Inherited, true,    false,    false      },
    {false,   V_Shown,     L_Locked,    V_Shown,     L_Locked,    true,    false,    false      },
    {false,   V_Shown,     L_Locked,    V_Shown,     L_Unlocked,  true,    true,     false      },
    {false,   V_Shown,     L_Locked,    V_Hidden,    L_Inherited, false,   false,    false      },
    {false,   V_Shown,     L_Locked,    V_Hidden,    L_Locked,    false,   false,    false      },
    {false,   V_Shown,     L_Locked,    V_Hidden,    L_Unlocked,  false,   true,     false      },

    {false,   V_Hidden,    L_Unlocked,  V_Inherited, L_Inherited, false,   true,     false      },
    {false,   V_Hidden,    L_Unlocked,  V_Inherited, L_Locked,    false,   false,    false      },
    {false,   V_Hidden,    L_Unlocked,  V_Inherited, L_Unlocked,  false,   true,     false      },
    {false,   V_Hidden,    L_Unlocked,  V_Shown,     L_Inherited, true,    true,     false      },
    {false,   V_Hidden,    L_Unlocked,  V_Shown,     L_Locked,    true,    false,    false      },
    {false,   V_Hidden,    L_Unlocked,  V_Shown,     L_Unlocked,  true,    true,     false      },
    {false,   V_Hidden,    L_Unlocked,  V_Hidden,    L_Inherited, false,   true,     false      },
    {false,   V_Hidden,    L_Unlocked,  V_Hidden,    L_Locked,    false,   false,    false      },
    {false,   V_Hidden,    L_Unlocked,  V_Hidden,    L_Unlocked,  false,   true,     false      },

    {true,    V_Shown,     L_Unlocked,  V_Inherited, L_Inherited, true,    true,     true       },
    {true,    V_Shown,     L_Unlocked,  V_Inherited, L_Locked,    true,    false,    false      },
    {true,    V_Shown,     L_Unlocked,  V_Inherited, L_Unlocked,  true,    true,     true       },
    {true,    V_Shown,     L_Unlocked,  V_Shown,     L_Inherited, true,    true,     true       },
    {true,    V_Shown,     L_Unlocked,  V_Shown,     L_Locked,    true,    false,    false      },
    {true,    V_Shown,     L_Unlocked,  V_Shown,     L_Unlocked,  true,    true,     true       },
    {true,    V_Shown,     L_Unlocked,  V_Hidden,    L_Inherited, false,   true,     false      },
    {true,    V_Shown,     L_Unlocked,  V_Hidden,    L_Locked,    false,   false,    false      },
    {true,    V_Shown,     L_Unlocked,  V_Hidden,    L_Unlocked,  false,   true,     false      },
    }));
    // clang-format on

    using GetNodes = std::function<std::tuple<GroupNode*, Node*>(EditorContextTest&)>;
    const GetNodes getNodes = GENERATE_COPY(
      GetNodes{[](auto& test) { return test.createGroupedPointEntity(); }},
      GetNodes{[](auto& test) { return test.createGroupedBrush(); }},
      GetNodes{[](auto& test) { return test.createGroupedPatch(); }});

    auto [groupNode, childNode] = getNodes(*this);

    CAPTURE(
      childNode->name(), grpOpen, grpVisState, grpLckState, entVisState, entLckState);

    if (grpOpen)
    {
      context.pushGroup(groupNode);
    }

    groupNode->setVisibilityState(grpVisState);
    groupNode->setLockState(grpLckState);
    childNode->setVisibilityState(entVisState);
    childNode->setLockState(entLckState);

    CHECK(context.visible(childNode) == visible);
    CHECK(context.editable(childNode) == editable);
    CHECK(context.selectable(childNode) == selectable);
  }

  SECTION("Grouped Brush Entity, Patch Entity")
  {
    using T = std::tuple<
      bool,
      VisibilityState,
      LockState,
      VisibilityState,
      LockState,
      VisibilityState,
      bool,
      bool,
      bool>;

    // clang-format off
    const auto
    [grpOpen, grpVisState, grpLckState, entVisState, entLockState, childVisState, visible, editable, selectable] = GENERATE(values<T>({
    {false,   V_Shown,     L_Unlocked,  V_Inherited, L_Inherited,  V_Inherited,   true,    true,     false     },
    {false,   V_Shown,     L_Unlocked,  V_Inherited, L_Inherited,  V_Hidden,      false,   true,     false     },
    {false,   V_Shown,     L_Unlocked,  V_Inherited, L_Inherited,  V_Shown,       true,    true,     false     },
    {false,   V_Shown,     L_Unlocked,  V_Inherited, L_Locked,     V_Inherited,   true,    false,    false     },
    {false,   V_Shown,     L_Unlocked,  V_Inherited, L_Locked,     V_Hidden,      false,   false,    false     },
    {false,   V_Shown,     L_Unlocked,  V_Inherited, L_Locked,     V_Shown,       true,    false,    false     },
    {false,   V_Shown,     L_Unlocked,  V_Inherited, L_Unlocked,   V_Inherited,   true,    true,     false     },
    {false,   V_Shown,     L_Unlocked,  V_Inherited, L_Unlocked,   V_Hidden,      false,   true,     false     },
    {false,   V_Shown,     L_Unlocked,  V_Inherited, L_Unlocked,   V_Shown,       true,    true,     false     },
    {false,   V_Shown,     L_Unlocked,  V_Hidden,    L_Inherited,  V_Inherited,   false,   true,     false     },
    {false,   V_Shown,     L_Unlocked,  V_Hidden,    L_Inherited,  V_Hidden,      false,   true,     false     },
    {false,   V_Shown,     L_Unlocked,  V_Hidden,    L_Inherited,  V_Shown,       true,    true,     false     },
    {false,   V_Shown,     L_Unlocked,  V_Hidden,    L_Locked,     V_Inherited,   false,   false,    false     },
    {false,   V_Shown,     L_Unlocked,  V_Hidden,    L_Locked,     V_Hidden,      false,   false,    false     },
    {false,   V_Shown,     L_Unlocked,  V_Hidden,    L_Locked,     V_Shown,       true,    false,    false     },
    {false,   V_Shown,     L_Unlocked,  V_Hidden,    L_Unlocked,   V_Inherited,   false,   true,     false     },
    {false,   V_Shown,     L_Unlocked,  V_Hidden,    L_Unlocked,   V_Hidden,      false,   true,     false     },
    {false,   V_Shown,     L_Unlocked,  V_Hidden,    L_Unlocked,   V_Shown,       true,    true,     false     },
    {false,   V_Shown,     L_Unlocked,  V_Shown,     L_Inherited,  V_Inherited,   true,    true,     false     },
    {false,   V_Shown,     L_Unlocked,  V_Shown,     L_Inherited,  V_Hidden,      false,   true,     false     },
    {false,   V_Shown,     L_Unlocked,  V_Shown,     L_Inherited,  V_Shown,       true,    true,     false     },
    {false,   V_Shown,     L_Unlocked,  V_Shown,     L_Locked,     V_Inherited,   true,    false,    false     },
    {false,   V_Shown,     L_Unlocked,  V_Shown,     L_Locked,     V_Hidden,      false,   false,    false     },
    {false,   V_Shown,     L_Unlocked,  V_Shown,     L_Locked,     V_Shown,       true,    false,    false     },
    {false,   V_Shown,     L_Unlocked,  V_Shown,     L_Unlocked,   V_Inherited,   true,    true,     false     },
    {false,   V_Shown,     L_Unlocked,  V_Shown,     L_Unlocked,   V_Hidden,      false,   true,     false     },
    {false,   V_Shown,     L_Unlocked,  V_Shown,     L_Unlocked,   V_Shown,       true,    true,     false     },

    {false,   V_Shown,     L_Locked,    V_Inherited, L_Inherited,  V_Inherited,   true,    false,    false     },
    {false,   V_Shown,     L_Locked,    V_Inherited, L_Inherited,  V_Hidden,      false,   false,    false     },
    {false,   V_Shown,     L_Locked,    V_Inherited, L_Inherited,  V_Shown,       true,    false,    false     },
    {false,   V_Shown,     L_Locked,    V_Inherited, L_Locked,     V_Inherited,   true,    false,    false     },
    {false,   V_Shown,     L_Locked,    V_Inherited, L_Locked,     V_Hidden,      false,   false,    false     },
    {false,   V_Shown,     L_Locked,    V_Inherited, L_Locked,     V_Shown,       true,    false,    false     },
    {false,   V_Shown,     L_Locked,    V_Inherited, L_Unlocked,   V_Inherited,   true,    true,     false     },
    {false,   V_Shown,     L_Locked,    V_Inherited, L_Unlocked,   V_Hidden,      false,   true,     false     },
    {false,   V_Shown,     L_Locked,    V_Inherited, L_Unlocked,   V_Shown,       true,    true,     false     },
    {false,   V_Shown,     L_Locked,    V_Hidden,    L_Inherited,  V_Inherited,   false,   false,    false     },
    {false,   V_Shown,     L_Locked,    V_Hidden,    L_Inherited,  V_Hidden,      false,   false,    false     },
    {false,   V_Shown,     L_Locked,    V_Hidden,    L_Inherited,  V_Shown,       true,    false,    false     },
    {false,   V_Shown,     L_Locked,    V_Hidden,    L_Locked,     V_Inherited,   false,   false,    false     },
    {false,   V_Shown,     L_Locked,    V_Hidden,    L_Locked,     V_Hidden,      false,   false,    false     },
    {false,   V_Shown,     L_Locked,    V_Hidden,    L_Locked,     V_Shown,       true,    false,    false     },
    {false,   V_Shown,     L_Locked,    V_Hidden,    L_Unlocked,   V_Inherited,   false,   true,     false     },
    {false,   V_Shown,     L_Locked,    V_Hidden,    L_Unlocked,   V_Hidden,      false,   true,     false     },
    {false,   V_Shown,     L_Locked,    V_Hidden,    L_Unlocked,   V_Shown,       true,    true,     false     },
    {false,   V_Shown,     L_Locked,    V_Shown,     L_Inherited,  V_Inherited,   true,    false,    false     },
    {false,   V_Shown,     L_Locked,    V_Shown,     L_Inherited,  V_Hidden,      false,   false,    false     },
    {false,   V_Shown,     L_Locked,    V_Shown,     L_Inherited,  V_Shown,       true,    false,    false     },
    {false,   V_Shown,     L_Locked,    V_Shown,     L_Locked,     V_Inherited,   true,    false,    false     },
    {false,   V_Shown,     L_Locked,    V_Shown,     L_Locked,     V_Hidden,      false,   false,    false     },
    {false,   V_Shown,     L_Locked,    V_Shown,     L_Locked,     V_Shown,       true,    false,    false     },
    {false,   V_Shown,     L_Locked,    V_Shown,     L_Unlocked,   V_Inherited,   true,    true,     false     },
    {false,   V_Shown,     L_Locked,    V_Shown,     L_Unlocked,   V_Hidden,      false,   true,     false     },
    {false,   V_Shown,     L_Locked,    V_Shown,     L_Unlocked,   V_Shown,       true,    true,     false     },

    {false,   V_Hidden,    L_Unlocked,  V_Inherited, L_Inherited,  V_Inherited,   false,   true,     false     },
    {false,   V_Hidden,    L_Unlocked,  V_Inherited, L_Inherited,  V_Hidden,      false,   true,     false     },
    {false,   V_Hidden,    L_Unlocked,  V_Inherited, L_Inherited,  V_Shown,       true,    true,     false     },
    {false,   V_Hidden,    L_Unlocked,  V_Inherited, L_Locked,     V_Inherited,   false,   false,    false     },
    {false,   V_Hidden,    L_Unlocked,  V_Inherited, L_Locked,     V_Hidden,      false,   false,    false     },
    {false,   V_Hidden,    L_Unlocked,  V_Inherited, L_Locked,     V_Shown,       true,    false,    false     },
    {false,   V_Hidden,    L_Unlocked,  V_Inherited, L_Unlocked,   V_Inherited,   false,   true,     false     },
    {false,   V_Hidden,    L_Unlocked,  V_Inherited, L_Unlocked,   V_Hidden,      false,   true,     false     },
    {false,   V_Hidden,    L_Unlocked,  V_Inherited, L_Unlocked,   V_Shown,       true,    true,     false     },
    {false,   V_Hidden,    L_Unlocked,  V_Hidden,    L_Inherited,  V_Inherited,   false,   true,     false     },
    {false,   V_Hidden,    L_Unlocked,  V_Hidden,    L_Inherited,  V_Hidden,      false,   true,     false     },
    {false,   V_Hidden,    L_Unlocked,  V_Hidden,    L_Inherited,  V_Shown,       true,    true,     false     },
    {false,   V_Hidden,    L_Unlocked,  V_Hidden,    L_Locked,     V_Inherited,   false,   false,    false     },
    {false,   V_Hidden,    L_Unlocked,  V_Hidden,    L_Locked,     V_Hidden,      false,   false,    false     },
    {false,   V_Hidden,    L_Unlocked,  V_Hidden,    L_Locked,     V_Shown,       true,    false,    false     },
    {false,   V_Hidden,    L_Unlocked,  V_Hidden,    L_Unlocked,   V_Inherited,   false,   true,     false     },
    {false,   V_Hidden,    L_Unlocked,  V_Hidden,    L_Unlocked,   V_Hidden,      false,   true,     false     },
    {false,   V_Hidden,    L_Unlocked,  V_Hidden,    L_Unlocked,   V_Shown,       true,    true,     false     },
    {false,   V_Hidden,    L_Unlocked,  V_Shown,     L_Inherited,  V_Inherited,   true,    true,     false     },
    {false,   V_Hidden,    L_Unlocked,  V_Shown,     L_Inherited,  V_Hidden,      false,   true,     false     },
    {false,   V_Hidden,    L_Unlocked,  V_Shown,     L_Inherited,  V_Shown,       true,    true,     false     },
    {false,   V_Hidden,    L_Unlocked,  V_Shown,     L_Locked,     V_Inherited,   true,    false,    false     },
    {false,   V_Hidden,    L_Unlocked,  V_Shown,     L_Locked,     V_Hidden,      false,   false,    false     },
    {false,   V_Hidden,    L_Unlocked,  V_Shown,     L_Locked,     V_Shown,       true,    false,    false     },
    {false,   V_Hidden,    L_Unlocked,  V_Shown,     L_Unlocked,   V_Inherited,   true,    true,     false     },
    {false,   V_Hidden,    L_Unlocked,  V_Shown,     L_Unlocked,   V_Hidden,      false,   true,     false     },
    {false,   V_Hidden,    L_Unlocked,  V_Shown,     L_Unlocked,   V_Shown,       true,    true,     false     },

    {true,    V_Shown,     L_Unlocked,  V_Inherited, L_Inherited,  V_Inherited,   true,    true,     false     },
    {true,    V_Shown,     L_Unlocked,  V_Inherited, L_Inherited,  V_Hidden,      false,   true,     false     },
    {true,    V_Shown,     L_Unlocked,  V_Inherited, L_Inherited,  V_Shown,       true,    true,     false     },
    {true,    V_Shown,     L_Unlocked,  V_Inherited, L_Locked,     V_Inherited,   true,    false,    false     },
    {true,    V_Shown,     L_Unlocked,  V_Inherited, L_Locked,     V_Hidden,      false,   false,    false     },
    {true,    V_Shown,     L_Unlocked,  V_Inherited, L_Locked,     V_Shown,       true,    false,    false     },
    {true,    V_Shown,     L_Unlocked,  V_Inherited, L_Unlocked,   V_Inherited,   true,    true,     false     },
    {true,    V_Shown,     L_Unlocked,  V_Inherited, L_Unlocked,   V_Hidden,      false,   true,     false     },
    {true,    V_Shown,     L_Unlocked,  V_Inherited, L_Unlocked,   V_Shown,       true,    true,     false     },
    {true,    V_Shown,     L_Unlocked,  V_Hidden,    L_Inherited,  V_Inherited,   false,   true,     false     },
    {true,    V_Shown,     L_Unlocked,  V_Hidden,    L_Inherited,  V_Hidden,      false,   true,     false     },
    {true,    V_Shown,     L_Unlocked,  V_Hidden,    L_Inherited,  V_Shown,       true,    true,     false     },
    {true,    V_Shown,     L_Unlocked,  V_Hidden,    L_Locked,     V_Inherited,   false,   false,    false     },
    {true,    V_Shown,     L_Unlocked,  V_Hidden,    L_Locked,     V_Hidden,      false,   false,    false     },
    {true,    V_Shown,     L_Unlocked,  V_Hidden,    L_Locked,     V_Shown,       true,    false,    false     },
    {true,    V_Shown,     L_Unlocked,  V_Hidden,    L_Unlocked,   V_Inherited,   false,   true,     false     },
    {true,    V_Shown,     L_Unlocked,  V_Hidden,    L_Unlocked,   V_Hidden,      false,   true,     false     },
    {true,    V_Shown,     L_Unlocked,  V_Hidden,    L_Unlocked,   V_Shown,       true,    true,     false     },
    {true,    V_Shown,     L_Unlocked,  V_Shown,     L_Inherited,  V_Inherited,   true,    true,     false     },
    {true,    V_Shown,     L_Unlocked,  V_Shown,     L_Inherited,  V_Hidden,      false,   true,     false     },
    {true,    V_Shown,     L_Unlocked,  V_Shown,     L_Inherited,  V_Shown,       true,    true,     false     },
    {true,    V_Shown,     L_Unlocked,  V_Shown,     L_Locked,     V_Inherited,   true,    false,    false     },
    {true,    V_Shown,     L_Unlocked,  V_Shown,     L_Locked,     V_Hidden,      false,   false,    false     },
    {true,    V_Shown,     L_Unlocked,  V_Shown,     L_Locked,     V_Shown,       true,    false,    false     },
    {true,    V_Shown,     L_Unlocked,  V_Shown,     L_Unlocked,   V_Inherited,   true,    true,     false     },
    {true,    V_Shown,     L_Unlocked,  V_Shown,     L_Unlocked,   V_Hidden,      false,   true,     false     },
    {true,    V_Shown,     L_Unlocked,  V_Shown,     L_Unlocked,   V_Shown,       true,    true,     false     },
    }));
    // clang-format on

    using GetNodes =
      std::function<std::tuple<GroupNode*, EntityNode*, Node*>(EditorContextTest&)>;
    const GetNodes getNodes = GENERATE_COPY(
      GetNodes{[](auto& test) { return test.createGroupedBrushEntity(); }},
      GetNodes{[](auto& test) { return test.createGroupedPatchEntity(); }});

    auto [groupNode, entityNode, childNode] = getNodes(*this);

    CAPTURE(
      childNode->name(),
      grpVisState,
      grpLckState,
      entVisState,
      entLockState,
      childVisState);

    if (grpOpen)
    {
      context.pushGroup(groupNode);
    }

    groupNode->setVisibilityState(grpVisState);
    groupNode->setLockState(grpLckState);

    entityNode->setVisibilityState(entVisState);
    entityNode->setLockState(entLockState);

    childNode->setVisibilityState(childVisState);

    CHECK(context.visible(entityNode) == visible);
    CHECK(context.editable(entityNode) == editable);
    CHECK(context.selectable(entityNode) == selectable);
  }
}
} // namespace Model
} // namespace TrenchBroom
