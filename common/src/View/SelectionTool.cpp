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

#include "SelectionTool.h"

#include "Model/BrushFace.h"
#include "Model/BrushNode.h"
#include "Model/EditorContext.h"
#include "Model/GroupNode.h"
#include "Model/Hit.h"
#include "Model/HitAdapter.h"
#include "Model/HitFilter.h"
#include "Model/ModelUtils.h"
#include "Model/Node.h"
#include "PreferenceManager.h"
#include "Preferences.h"
#include "Renderer/RenderContext.h"
#include "View/DragTracker.h"
#include "View/Grid.h"
#include "View/InputState.h"
#include "View/MapDocument.h"
#include "View/TransactionScope.h"

#include <kdl/memory_utils.h>

#include <algorithm>
#include <unordered_set>
#include <vector>

namespace TrenchBroom
{
namespace View
{
/**
 * Implements the Group picking logic: if `node` is inside a (possibly nested chain of)
 * closed group(s), the outermost closed group is returned. Otherwise, `node` itself is
 * returned.
 *
 * This is used to implement the UI where clicking on a brush inside a group selects the
 * group.
 */
static Model::Node* findOutermostClosedGroupOrNode(Model::Node* node)
{
  Model::GroupNode* group = findOutermostClosedGroup(node);
  if (group != nullptr)
  {
    return group;
  }

  return node;
}

static const Model::Node* findOutermostClosedGroupOrNode(const Model::Node* node)
{
  const Model::GroupNode* group = findOutermostClosedGroup(node);
  if (group != nullptr)
  {
    return group;
  }

  return node;
}

static Model::HitFilter isNodeSelectable(const Model::EditorContext& editorContext)
{
  return [&](const auto& hit) {
    if (const auto faceHandle = Model::hitToFaceHandle(hit))
    {
      if (!editorContext.selectable(faceHandle->node(), faceHandle->face()))
      {
        return false;
      }
    }
    if (const auto* node = Model::hitToNode(hit))
    {
      return editorContext.selectable(findOutermostClosedGroupOrNode(node));
    }
    return false;
  };
}

std::vector<Model::Node*> hitsToNodesWithGroupPicking(const std::vector<Model::Hit>& hits)
{
  auto hitNodes = std::vector<Model::Node*>{};
  hitNodes.reserve(hits.size());

  auto duplicateCheck = std::unordered_set<Model::Node*>{};

  for (const auto& hit : hits)
  {
    Model::Node* node = findOutermostClosedGroupOrNode(Model::hitToNode(hit));
    if (!duplicateCheck.insert(node).second)
    {
      continue;
    }

    // Note that the order of the input hits are preserved, although duplicates later in
    // the list are dropped
    hitNodes.push_back(node);
  }

  return hitNodes;
}

static bool isFaceClick(const InputState& inputState)
{
  return inputState.modifierKeysDown(ModifierKeys::MKShift);
}

static bool isMultiClick(const InputState& inputState)
{
  return inputState.modifierKeysDown(ModifierKeys::MKCtrlCmd);
}

static const Model::Hit& firstHit(
  const InputState& inputState, const Model::HitFilter& hitFilter)
{
  return inputState.pickResult().first(hitFilter);
}

static std::vector<Model::Node*> collectSelectableChildren(
  const Model::EditorContext& editorContext, const Model::Node* node)
{
  return Model::collectSelectableNodes(node->children(), editorContext);
}

static bool handleClick(
  const InputState& inputState, const Model::EditorContext& editorContext)
{
  if (!inputState.mouseButtonsPressed(MouseButtons::MBLeft))
  {
    return false;
  }
  if (!inputState.checkModifierKeys(MK_DontCare, MK_No, MK_DontCare))
  {
    return false;
  }

  return editorContext.canChangeSelection();
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
  using namespace Model::HitFilters;

  auto document = kdl::mem_lock(m_document);
  const auto& editorContext = document->editorContext();

  if (!handleClick(inputState, editorContext))
  {
    return false;
  }

  if (isFaceClick(inputState))
  {
    const auto& hit = firstHit(inputState, type(Model::BrushNode::BrushHitType));
    if (const auto faceHandle = Model::hitToFaceHandle(hit))
    {
      const auto* brush = faceHandle->node();
      const auto& face = faceHandle->face();
      if (editorContext.selectable(brush, face))
      {
        if (isMultiClick(inputState))
        {
          const auto objects = document->hasSelectedNodes();
          if (objects)
          {
            if (brush->selected())
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
    const auto& hit =
      firstHit(inputState, type(Model::nodeHitType()) && isNodeSelectable(editorContext));
    if (hit.isMatch())
    {
      auto* node = findOutermostClosedGroupOrNode(Model::hitToNode(hit));
      if (editorContext.selectable(node))
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
            if (document->hasSelectedBrushFaces())
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
  using namespace Model::HitFilters;

  auto document = kdl::mem_lock(m_document);
  const auto& editorContext = document->editorContext();

  if (!handleClick(inputState, editorContext))
  {
    return false;
  }

  if (isFaceClick(inputState))
  {
    const auto& hit = firstHit(inputState, type(Model::BrushNode::BrushHitType));
    if (const auto faceHandle = Model::hitToFaceHandle(hit))
    {
      auto* brush = faceHandle->node();
      const auto& face = faceHandle->face();
      if (editorContext.selectable(brush, face))
      {
        if (isMultiClick(inputState))
        {
          if (document->hasSelectedNodes())
          {
            document->convertToFaceSelection();
          }
          document->selectBrushFaces(Model::toHandles(brush));
        }
        else
        {
          auto transaction = Transaction{document, "Select Brush Faces"};
          document->deselectAll();
          document->selectBrushFaces(Model::toHandles(brush));
          transaction.commit();
        }
      }
    }
  }
  else
  {
    const auto inGroup = document->currentGroup() != nullptr;
    const auto& hit =
      firstHit(inputState, type(Model::nodeHitType()) && isNodeSelectable(editorContext));
    if (hit.isMatch())
    {
      const auto hitInGroup =
        inGroup && Model::hitToNode(hit)->isDescendantOf(document->currentGroup());
      if (!inGroup || hitInGroup)
      {
        // If the hit node is inside a closed group, treat it as a hit on the group insted
        auto* group = findOutermostClosedGroup(Model::hitToNode(hit));
        if (group != nullptr)
        {
          if (editorContext.selectable(group))
          {
            document->openGroup(group);
          }
        }
        else
        {
          const auto* node = Model::hitToNode(hit);
          if (editorContext.selectable(node))
          {
            const auto* container = node->parent();
            const auto siblings = collectSelectableChildren(editorContext, container);
            if (isMultiClick(inputState))
            {
              if (document->hasSelectedBrushFaces())
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

static void adjustGrid(const InputState& inputState, Grid& grid)
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
static std::pair<Model::Node*, Model::Node*> findSelectionPair(I it, I end)
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

static void drillSelection(const InputState& inputState, MapDocument& document)
{
  using namespace Model::HitFilters;

  const auto& editorContext = document.editorContext();

  const auto hits = inputState.pickResult().all(
    type(Model::nodeHitType()) && isNodeSelectable(editorContext));

  // Hits may contain multiple brush/entity hits that are inside closed groups. These need
  // to be converted to group hits using findOutermostClosedGroupOrNode() and multiple
  // hits on the same Group need to be collapsed.
  const std::vector<Model::Node*> hitNodes = hitsToNodesWithGroupPicking(hits);

  const auto forward =
    (inputState.scrollY() > 0.0f) != (pref(Preferences::CameraMouseWheelInvert));
  const auto nodePair = forward
                          ? findSelectionPair(std::begin(hitNodes), std::end(hitNodes))
                          : findSelectionPair(std::rbegin(hitNodes), std::rend(hitNodes));

  auto* selectedNode = nodePair.first;
  auto* nextNode = nodePair.second;

  if (nextNode != nullptr)
  {
    auto transaction = Transaction{document, "Drill Selection"};
    document.deselectNodes({selectedNode});
    document.selectNodes({nextNode});
    transaction.commit();
  }
}

void SelectionTool::mouseScroll(const InputState& inputState)
{
  const auto document = kdl::mem_lock(m_document);

  if (inputState.checkModifierKeys(MK_Yes, MK_Yes, MK_No))
  {
    adjustGrid(inputState, document->grid());
  }
  else if (inputState.checkModifierKeys(MK_Yes, MK_No, MK_No))
  {
    drillSelection(inputState, *document);
  }
}

namespace
{
class PaintSelectionDragTracker : public DragTracker
{
private:
  std::shared_ptr<MapDocument> m_document;

public:
  PaintSelectionDragTracker(std::shared_ptr<MapDocument> document)
    : m_document{std::move(document)}
  {
  }

  bool drag(const InputState& inputState) override
  {
    using namespace Model::HitFilters;

    const auto& editorContext = m_document->editorContext();
    if (m_document->hasSelectedBrushFaces())
    {
      const auto& hit = firstHit(inputState, type(Model::BrushNode::BrushHitType));
      if (const auto faceHandle = Model::hitToFaceHandle(hit))
      {
        const auto* brush = faceHandle->node();
        const auto& face = faceHandle->face();
        if (!face.selected() && editorContext.selectable(brush, face))
        {
          m_document->selectBrushFaces({*faceHandle});
        }
      }
    }
    else
    {
      assert(m_document->hasSelectedNodes());
      const auto& hit = firstHit(
        inputState, type(Model::nodeHitType()) && isNodeSelectable(editorContext));
      if (hit.isMatch())
      {
        auto* node = findOutermostClosedGroupOrNode(Model::hitToNode(hit));
        if (!node->selected() && editorContext.selectable(node))
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

std::unique_ptr<DragTracker> SelectionTool::acceptMouseDrag(const InputState& inputState)
{
  using namespace Model::HitFilters;

  auto document = kdl::mem_lock(m_document);
  const auto& editorContext = document->editorContext();

  if (!handleClick(inputState, editorContext) || !isMultiClick(inputState))
  {
    return nullptr;
  }

  if (isFaceClick(inputState))
  {
    const auto& hit = firstHit(inputState, type(Model::BrushNode::BrushHitType));
    if (const auto faceHandle = Model::hitToFaceHandle(hit))
    {
      const auto* brush = faceHandle->node();
      const auto& face = faceHandle->face();
      if (editorContext.selectable(brush, face))
      {
        document->startTransaction(
          "Drag Select Brush Faces", TransactionScope::LongRunning);
        if (document->hasSelection() && !document->hasSelectedBrushFaces())
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
    const auto& hit =
      firstHit(inputState, type(Model::nodeHitType()) && isNodeSelectable(editorContext));
    if (!hit.isMatch())
    {
      return nullptr;
    }

    auto* node = findOutermostClosedGroupOrNode(Model::hitToNode(hit));
    if (editorContext.selectable(node))
    {
      document->startTransaction("Drag Select Objects", TransactionScope::LongRunning);
      if (document->hasSelection() && !document->hasSelectedNodes())
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
  const InputState& inputState, Renderer::RenderContext& renderContext) const
{
  using namespace Model::HitFilters;

  auto document = kdl::mem_lock(m_document);
  const auto& hit = firstHit(inputState, type(Model::nodeHitType()));
  if (hit.isMatch())
  {
    Model::Node* node = findOutermostClosedGroupOrNode(Model::hitToNode(hit));

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
} // namespace View
} // namespace TrenchBroom
