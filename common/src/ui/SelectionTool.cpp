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
#include "mdl/ModelUtils.h"
#include "mdl/Node.h"
#include "render/RenderContext.h"
#include "ui/GestureTracker.h"
#include "ui/InputState.h"
#include "ui/MapDocument.h"
#include "ui/Transaction.h"
#include "ui/TransactionScope.h"

#include "kdl/memory_utils.h"
#include "kdl/range_to_vector.h"
#include "kdl/stable_remove_duplicates.h"

#include <algorithm>
#include <unordered_set>
#include <vector>

namespace tb::ui
{
namespace
{

/**
 * Implements the Group picking logic: if `node` is inside a (possibly nested chain of)
 * closed group(s), the outermost closed group is returned. Otherwise, `node` itself is
 * returned.
 *
 * This is used to implement the UI where clicking on a brush inside a group selects the
 * group.
 */
mdl::Node* findOutermostClosedGroupOrNode(mdl::Node* node)
{
  if (auto* group = findOutermostClosedGroup(node))
  {
    return group;
  }

  return node;
}

const mdl::Node* findOutermostClosedGroupOrNode(const mdl::Node* node)
{
  return findOutermostClosedGroupOrNode(const_cast<mdl::Node*>(node));
}

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

void drillSelection(const InputState& inputState, MapDocument& document)
{
  using namespace mdl::HitFilters;

  const auto& editorContext = document.editorContext();

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
    auto transaction = Transaction{document, "Drill Selection"};
    document.deselectNodes({selectedNode});
    document.selectNodes({nextNode});
    transaction.commit();
  }
}

class PaintSelectionDragTracker : public GestureTracker
{
private:
  std::shared_ptr<MapDocument> m_document;

public:
  explicit PaintSelectionDragTracker(std::shared_ptr<MapDocument> document)
    : m_document{std::move(document)}
  {
  }

  bool update(const InputState& inputState) override
  {
    using namespace mdl::HitFilters;

    const auto& editorContext = m_document->editorContext();
    if (m_document->selection().hasBrushFaces())
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
          m_document->selectBrushFaces({*faceHandle});
        }
      }
    }
    else
    {
      assert(m_document->selection().hasNodes());
      const auto hit =
        firstHit(inputState, type(mdl::nodeHitType()) && isNodeSelectable(editorContext));
      if (hit.isMatch())
      {
        auto* node = findOutermostClosedGroupOrNode(mdl::hitToNode(hit));
        if (!node->selected() && editorContext.selectable(*node))
        {
          m_document->selectNodes({node});
        }
      }
    }
    return true;
  }

  void end(const InputState&) override { m_document->commitTransaction(); }

  void cancel() override { m_document->cancelTransaction(); }
};

} // namespace

std::vector<mdl::Node*> hitsToNodesWithGroupPicking(const std::vector<mdl::Hit>& hits)
{
  return kdl::col_stable_remove_duplicates(
    hits | std::views::transform([](const auto& hit) {
      return findOutermostClosedGroupOrNode(mdl::hitToNode(hit));
    })
    | kdl::to_vector);
}

SelectionTool::SelectionTool(std::weak_ptr<MapDocument> document)
  : ToolController{}
  , Tool{true}
  , m_document{std::move(document)}
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

  auto document = kdl::mem_lock(m_document);
  const auto& editorContext = document->editorContext();

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
          const auto objects = document->selection().hasNodes();
          if (objects)
          {
            if (brushNode->selected())
            {
              document->deselectBrushFaces({*faceHandle});
            }
            else
            {
              auto transaction = Transaction{document, "Select Brush Face"};
              document->convertToFaceSelection();
              document->selectBrushFaces({*faceHandle});
              transaction.commit();
            }
          }
          else
          {
            if (face.selected())
            {
              document->deselectBrushFaces({*faceHandle});
            }
            else
            {
              document->selectBrushFaces({*faceHandle});
            }
          }
        }
        else
        {
          auto transaction = Transaction{document, "Select Brush Face"};
          document->deselectAll();
          document->selectBrushFaces({*faceHandle});
          transaction.commit();
        }
      }
    }
    else
    {
      document->deselectAll();
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
            document->deselectNodes({node});
          }
          else
          {
            auto transaction = Transaction{document, "Select Object"};
            if (document->selection().hasBrushFaces())
            {
              document->deselectAll();
            }
            document->selectNodes({node});
            transaction.commit();
          }
        }
        else
        {
          auto transaction = Transaction{document, "Select Object"};
          document->deselectAll();
          document->selectNodes({node});
          transaction.commit();
        }
      }
    }
    else
    {
      document->deselectAll();
    }
  }

  return true;
}

bool SelectionTool::mouseDoubleClick(const InputState& inputState)
{
  using namespace mdl::HitFilters;

  auto document = kdl::mem_lock(m_document);
  const auto& editorContext = document->editorContext();

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
          if (document->selection().hasNodes())
          {
            document->convertToFaceSelection();
          }
          document->selectBrushFaces(mdl::toHandles(brushNode));
        }
        else
        {
          auto transaction = Transaction{document, "Select Brush Faces"};
          document->deselectAll();
          document->selectBrushFaces(mdl::toHandles(brushNode));
          transaction.commit();
        }
      }
    }
  }
  else
  {
    const auto inGroup = document->currentGroup() != nullptr;
    const auto hit =
      firstHit(inputState, type(mdl::nodeHitType()) && isNodeSelectable(editorContext));
    if (hit.isMatch())
    {
      const auto hitInGroup =
        inGroup && mdl::hitToNode(hit)->isDescendantOf(document->currentGroup());
      if (!inGroup || hitInGroup)
      {
        // If the hit node is inside a closed group, treat it as a hit on the group insted
        auto* groupNode = findOutermostClosedGroup(mdl::hitToNode(hit));
        if (groupNode != nullptr)
        {
          if (editorContext.selectable(*groupNode))
          {
            document->openGroup(groupNode);
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
              if (document->selection().hasBrushFaces())
              {
                document->deselectAll();
              }
              document->selectNodes(siblings);
            }
            else
            {
              auto transaction = Transaction{document, "Select Brushes"};
              document->deselectAll();
              document->selectNodes(siblings);
              transaction.commit();
            }
          }
        }
      }
      else if (inGroup)
      {
        document->closeGroup();
      }
    }
    else if (inGroup)
    {
      document->closeGroup();
    }
  }

  return true;
}

void SelectionTool::mouseScroll(const InputState& inputState)
{
  const auto document = kdl::mem_lock(m_document);

  if (inputState.checkModifierKeys(
        ModifierKeyPressed::Yes, ModifierKeyPressed::Yes, ModifierKeyPressed::No))
  {
    adjustGrid(inputState, document->grid());
  }
  else if (inputState.checkModifierKeys(
             ModifierKeyPressed::Yes, ModifierKeyPressed::No, ModifierKeyPressed::No))
  {
    drillSelection(inputState, *document);
  }
}

std::unique_ptr<GestureTracker> SelectionTool::acceptMouseDrag(
  const InputState& inputState)
{
  using namespace mdl::HitFilters;

  auto document = kdl::mem_lock(m_document);
  const auto& editorContext = document->editorContext();

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
        document->startTransaction(
          "Drag Select Brush Faces", TransactionScope::LongRunning);
        if (document->selection().hasAny() && !document->selection().hasBrushFaces())
        {
          document->deselectAll();
        }
        if (!face.selected())
        {
          document->selectBrushFaces({*faceHandle});
        }

        return std::make_unique<PaintSelectionDragTracker>(std::move(document));
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
      document->startTransaction("Drag Select Objects", TransactionScope::LongRunning);
      if (document->selection().hasAny() && !document->selection().hasNodes())
      {
        document->deselectAll();
      }
      if (!node->selected())
      {
        document->selectNodes({node});
      }

      return std::make_unique<PaintSelectionDragTracker>(std::move(document));
    }
  }

  return nullptr;
}

void SelectionTool::setRenderOptions(
  const InputState& inputState, render::RenderContext& renderContext) const
{
  using namespace mdl::HitFilters;

  auto document = kdl::mem_lock(m_document);
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
