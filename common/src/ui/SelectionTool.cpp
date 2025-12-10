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

#include "SelectionTool.h"

#include "PreferenceManager.h"
#include "Preferences.h"
#include "mdl/BrushFace.h"
#include "mdl/BrushNode.h"
#include "mdl/EditorContext.h"
#include "mdl/Grid.h"
#include "mdl/GroupNode.h" // IWYU pragma: keep
#include "mdl/Hit.h"
#include "mdl/HitAdapter.h"
#include "mdl/HitFilter.h"
#include "mdl/Map.h"
#include "mdl/Map_Groups.h"
#include "mdl/Map_Selection.h"
#include "mdl/ModelUtils.h"
#include "mdl/Node.h"
#include "mdl/Transaction.h"
#include "mdl/TransactionScope.h"
#include "render/RenderContext.h"
#include "ui/GestureTracker.h"
#include "ui/InputState.h"
#include "ui/MapDocument.h"

#include "kd/contracts.h"

#include <algorithm>
#include <unordered_set>
#include <vector>

namespace tb::ui
{
namespace
{

mdl::HitFilter isNodeSelectable(const mdl::EditorContext& editorContext)
{
  return [&](const auto& hit) {
    if (const auto faceHandle = mdl::hitToFaceHandle(hit))
    {
      if (!editorContext.selectable(*faceHandle->node(), faceHandle->face()))
      {
        return false;
      }
    }
    if (const auto* node = mdl::hitToNode(hit))
    {
      return editorContext.selectable(*findOutermostClosedGroupOrNode(node));
    }
    return false;
  };
}

bool isFaceClick(const InputState& inputState)
{
  return inputState.modifierKeysDown(ModifierKeys::Shift);
}

bool isMultiClick(const InputState& inputState)
{
  return inputState.modifierKeysDown(ModifierKeys::CtrlCmd);
}

const mdl::Hit& firstHit(const InputState& inputState, const mdl::HitFilter& hitFilter)
{
  return inputState.pickResult().first(hitFilter);
}

std::vector<mdl::Node*> collectSelectableChildren(
  const mdl::EditorContext& editorContext, const mdl::Node* node)
{
  return mdl::collectSelectableNodes(node->children(), editorContext);
}

bool handleClick(const InputState& inputState, const mdl::EditorContext& editorContext)
{
  if (!inputState.mouseButtonsPressed(MouseButtons::Left))
  {
    return false;
  }
  if (!inputState.checkModifierKeys(
        ModifierKeyPressed::DontCare,
        ModifierKeyPressed::No,
        ModifierKeyPressed::DontCare))
  {
    return false;
  }

  return editorContext.canChangeSelection();
}

void adjustGrid(const InputState& inputState, mdl::Grid& grid)
{
  const auto factor = pref(Preferences::CameraMouseWheelInvert) ? -1.0f : 1.0f;
  if (factor * inputState.scrollY() < 0.0f)
  {
    grid.incSize();
  }
  else if (factor * inputState.scrollY() > 0.0f)
  {
    grid.decSize();
  }
}

/**
 * Returns a pair where:
 *  - first is the first node in the given list that's currently selected
 *  - second is the next selectable node in the list
 */
template <typename I>
std::pair<mdl::Node*, mdl::Node*> findSelectionPair(I it, I end)
{
  const auto first =
    std::find_if(it, end, [](const auto* node) { return node->selected(); });
  if (first == end)
  {
    return {nullptr, nullptr};
  }

  const auto next = std::next(first);
  if (next == end)
  {
    return {*first, nullptr};
  }

  return {*first, *next};
}

void drillSelection(const InputState& inputState, mdl::Map& map)
{
  using namespace mdl::HitFilters;

  const auto& editorContext = map.editorContext();

  const auto hits = inputState.pickResult().all(
    type(mdl::nodeHitType()) && isNodeSelectable(editorContext));

  // Hits may contain multiple brush/entity hits that are inside closed groups. These need
  // to be converted to group hits using findOutermostClosedGroupOrNode() and multiple
  // hits on the same Group need to be collapsed.
  const auto hitNodes = hitsToNodesWithGroupPicking(hits);

  const auto forward =
    (inputState.scrollY() > 0.0f) != (pref(Preferences::CameraMouseWheelInvert));
  const auto nodePair = forward
                          ? findSelectionPair(std::begin(hitNodes), std::end(hitNodes))
                          : findSelectionPair(std::rbegin(hitNodes), std::rend(hitNodes));

  auto* selectedNode = nodePair.first;
  auto* nextNode = nodePair.second;

  if (nextNode)
  {
    auto transaction = mdl::Transaction{map, "Drill Selection"};
    deselectNodes(map, {selectedNode});
    selectNodes(map, {nextNode});
    transaction.commit();
  }
}

class PaintSelectionDragTracker : public GestureTracker
{
private:
  mdl::Map& m_map;

public:
  explicit PaintSelectionDragTracker(mdl::Map& map)
    : m_map{map}
  {
  }

  bool update(const InputState& inputState) override
  {
    using namespace mdl::HitFilters;

    const auto& editorContext = m_map.editorContext();
    if (m_map.selection().hasBrushFaces())
    {
      const auto hit = firstHit(
        inputState,
        type(mdl::BrushNode::BrushHitType) && isNodeSelectable(editorContext));
      if (const auto faceHandle = mdl::hitToFaceHandle(hit))
      {
        const auto* brushNode = faceHandle->node();
        const auto& face = faceHandle->face();
        if (!face.selected() && editorContext.selectable(*brushNode, face))
        {
          selectBrushFaces(m_map, {*faceHandle});
        }
      }
    }
    else
    {
      contract_assert(m_map.selection().hasNodes());

      const auto hit =
        firstHit(inputState, type(mdl::nodeHitType()) && isNodeSelectable(editorContext));
      if (hit.isMatch())
      {
        auto* node = findOutermostClosedGroupOrNode(mdl::hitToNode(hit));
        if (!node->selected() && editorContext.selectable(*node))
        {
          selectNodes(m_map, {node});
        }
      }
    }
    return true;
  }

  void end(const InputState&) override { m_map.commitTransaction(); }

  void cancel() override { m_map.cancelTransaction(); }
};

} // namespace

SelectionTool::SelectionTool(MapDocument& document)
  : ToolController{}
  , Tool{true}
  , m_document{document}
{
}

Tool& SelectionTool::tool()
{
  return *this;
}

const Tool& SelectionTool::tool() const
{
  return *this;
}

bool SelectionTool::mouseClick(const InputState& inputState)
{
  using namespace mdl::HitFilters;

  auto& map = m_document.map();
  const auto& editorContext = map.editorContext();

  if (!handleClick(inputState, editorContext))
  {
    return false;
  }

  if (isFaceClick(inputState))
  {
    const auto hit = firstHit(
      inputState, type(mdl::BrushNode::BrushHitType) && isNodeSelectable(editorContext));
    if (const auto faceHandle = mdl::hitToFaceHandle(hit))
    {
      const auto* brushNode = faceHandle->node();
      const auto& face = faceHandle->face();
      if (editorContext.selectable(*brushNode, face))
      {
        if (isMultiClick(inputState))
        {
          const auto objects = map.selection().hasNodes();
          if (objects)
          {
            if (brushNode->selected())
            {
              deselectBrushFaces(map, {*faceHandle});
            }
            else
            {
              auto transaction = mdl::Transaction{map, "Select Brush Face"};
              convertToFaceSelection(map);
              selectBrushFaces(map, {*faceHandle});
              transaction.commit();
            }
          }
          else
          {
            if (face.selected())
            {
              deselectBrushFaces(map, {*faceHandle});
            }
            else
            {
              selectBrushFaces(map, {*faceHandle});
            }
          }
        }
        else
        {
          auto transaction = mdl::Transaction{map, "Select Brush Face"};
          deselectAll(map);
          selectBrushFaces(map, {*faceHandle});
          transaction.commit();
        }
      }
    }
    else
    {
      deselectAll(map);
    }
  }
  else
  {
    const auto hit =
      firstHit(inputState, type(mdl::nodeHitType()) && isNodeSelectable(editorContext));
    if (hit.isMatch())
    {
      auto* node = findOutermostClosedGroupOrNode(mdl::hitToNode(hit));
      if (editorContext.selectable(*node))
      {
        if (isMultiClick(inputState))
        {
          if (node->selected())
          {
            deselectNodes(map, {node});
          }
          else
          {
            auto transaction = mdl::Transaction{map, "Select Object"};
            if (map.selection().hasBrushFaces())
            {
              deselectAll(map);
            }
            selectNodes(map, {node});
            transaction.commit();
          }
        }
        else
        {
          auto transaction = mdl::Transaction{map, "Select Object"};
          deselectAll(map);
          selectNodes(map, {node});
          transaction.commit();
        }
      }
    }
    else
    {
      deselectAll(map);
    }
  }

  return true;
}

bool SelectionTool::mouseDoubleClick(const InputState& inputState)
{
  using namespace mdl::HitFilters;

  auto& map = m_document.map();
  const auto& editorContext = map.editorContext();

  if (!handleClick(inputState, editorContext))
  {
    return false;
  }

  if (isFaceClick(inputState))
  {
    const auto hit = firstHit(inputState, type(mdl::BrushNode::BrushHitType));
    if (const auto faceHandle = mdl::hitToFaceHandle(hit))
    {
      auto* brushNode = faceHandle->node();
      const auto& face = faceHandle->face();
      if (editorContext.selectable(*brushNode, face))
      {
        if (isMultiClick(inputState))
        {
          if (map.selection().hasNodes())
          {
            convertToFaceSelection(map);
          }
          selectBrushFaces(map, mdl::toHandles(brushNode));
        }
        else
        {
          auto transaction = mdl::Transaction{map, "Select Brush Faces"};
          deselectAll(map);
          selectBrushFaces(map, mdl::toHandles(brushNode));
          transaction.commit();
        }
      }
    }
  }
  else
  {
    const auto* currentGroup = map.editorContext().currentGroup();
    const auto inGroup = currentGroup != nullptr;
    const auto hit =
      firstHit(inputState, type(mdl::nodeHitType()) && isNodeSelectable(editorContext));
    if (hit.isMatch())
    {
      const auto hitInGroup =
        inGroup && mdl::hitToNode(hit)->isDescendantOf(currentGroup);
      if (!inGroup || hitInGroup)
      {
        // If the hit node is inside a closed group, treat it as a hit on the group insted
        auto* groupNode = findOutermostClosedGroup(mdl::hitToNode(hit));
        if (groupNode != nullptr)
        {
          if (editorContext.selectable(*groupNode))
          {
            openGroup(map, *groupNode);
          }
        }
        else
        {
          const auto* node = mdl::hitToNode(hit);
          if (editorContext.selectable(*node))
          {
            const auto* container = node->parent();
            const auto siblings = collectSelectableChildren(editorContext, container);
            if (isMultiClick(inputState))
            {
              if (map.selection().hasBrushFaces())
              {
                deselectAll(map);
              }
              selectNodes(map, siblings);
            }
            else
            {
              auto transaction = mdl::Transaction{map, "Select Brushes"};
              deselectAll(map);
              selectNodes(map, siblings);
              transaction.commit();
            }
          }
        }
      }
      else if (inGroup)
      {
        closeGroup(map);
      }
    }
    else if (inGroup)
    {
      closeGroup(map);
    }
  }

  return true;
}

void SelectionTool::mouseScroll(const InputState& inputState)
{
  auto& map = m_document.map();

  if (inputState.checkModifierKeys(
        ModifierKeyPressed::Yes, ModifierKeyPressed::Yes, ModifierKeyPressed::No))
  {
    adjustGrid(inputState, map.grid());
  }
  else if (inputState.checkModifierKeys(
             ModifierKeyPressed::Yes, ModifierKeyPressed::No, ModifierKeyPressed::No))
  {
    drillSelection(inputState, map);
  }
}

std::unique_ptr<GestureTracker> SelectionTool::acceptMouseDrag(
  const InputState& inputState)
{
  using namespace mdl::HitFilters;

  auto& map = m_document.map();
  const auto& editorContext = map.editorContext();

  if (!handleClick(inputState, editorContext) || !isMultiClick(inputState))
  {
    return nullptr;
  }

  if (isFaceClick(inputState))
  {
    const auto hit = firstHit(inputState, type(mdl::BrushNode::BrushHitType));
    if (const auto faceHandle = mdl::hitToFaceHandle(hit))
    {
      const auto* brushNode = faceHandle->node();
      const auto& face = faceHandle->face();
      if (editorContext.selectable(*brushNode, face))
      {
        map.startTransaction(
          "Drag Select Brush Faces", mdl::TransactionScope::LongRunning);
        if (map.selection().hasAny() && !map.selection().hasBrushFaces())
        {
          deselectAll(map);
        }
        if (!face.selected())
        {
          selectBrushFaces(map, {*faceHandle});
        }

        return std::make_unique<PaintSelectionDragTracker>(map);
      }
    }
  }
  else
  {
    const auto hit =
      firstHit(inputState, type(mdl::nodeHitType()) && isNodeSelectable(editorContext));
    if (!hit.isMatch())
    {
      return nullptr;
    }

    auto* node = findOutermostClosedGroupOrNode(mdl::hitToNode(hit));
    if (editorContext.selectable(*node))
    {
      map.startTransaction("Drag Select Objects", mdl::TransactionScope::LongRunning);
      if (map.selection().hasAny() && !map.selection().hasNodes())
      {
        deselectAll(map);
      }
      if (!node->selected())
      {
        selectNodes(map, {node});
      }

      return std::make_unique<PaintSelectionDragTracker>(map);
    }
  }

  return nullptr;
}

void SelectionTool::setRenderOptions(
  const InputState& inputState, render::RenderContext& renderContext) const
{
  using namespace mdl::HitFilters;

  if (const auto hit = firstHit(inputState, type(mdl::nodeHitType())); hit.isMatch())
  {
    auto* node = findOutermostClosedGroupOrNode(mdl::hitToNode(hit));
    if (node->selected())
    {
      renderContext.setShowSelectionGuide();
    }
  }
}

bool SelectionTool::cancel()
{
  // closing the current group is handled in MapViewBase
  return false;
}

} // namespace tb::ui
