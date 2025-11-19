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

#include "SetBrushFaceAttributesTool.h"

#include "Ensure.h"
#include "mdl/Brush.h"
#include "mdl/BrushFace.h"
#include "mdl/BrushFaceHandle.h"
#include "mdl/BrushNode.h"
#include "mdl/Hit.h"
#include "mdl/HitAdapter.h"
#include "mdl/HitFilter.h"
#include "mdl/LinkedGroupUtils.h"
#include "mdl/Map.h"
#include "mdl/Map_Brushes.h"
#include "mdl/Map_Selection.h"
#include "mdl/ModelUtils.h"
#include "mdl/Transaction.h"
#include "mdl/TransactionScope.h"
#include "mdl/UVCoordSystem.h"
#include "mdl/UpdateBrushFaceAttributes.h"
#include "ui/GestureTracker.h"
#include "ui/InputState.h"

#include <vector>

namespace tb::ui
{

static const std::string TransferFaceAttributesTransactionName =
  "Transfer Face Attributes";

SetBrushFaceAttributesTool::SetBrushFaceAttributesTool(mdl::Map& map)
  : ToolController{}
  , Tool{true}
  , m_map{map}
{
}

Tool& SetBrushFaceAttributesTool::tool()
{
  return *this;
}

const Tool& SetBrushFaceAttributesTool::tool() const
{
  return *this;
}

bool SetBrushFaceAttributesTool::mouseClick(const InputState& inputState)
{
  if (canCopyAttributesFromSelection(inputState))
  {
    copyAttributesFromSelection(inputState, false);
    return true;
  }

  return false;
}

bool SetBrushFaceAttributesTool::mouseDoubleClick(const InputState& inputState)
{
  if (canCopyAttributesFromSelection(inputState))
  {
    // The typical use case is, mouseClick() previously copied the selected attributes to
    // the clicked face, and now the second click has arrived so we're about to copy the
    // selected attributes to the whole brush. To make undo/redo more intuitivie, undo the
    // application to the single face now, so that if the double click is later
    // undone/redone, it appears as one atomic action.

    // The last click may not have been handled by this tool, see:
    // https://github.com/TrenchBroom/TrenchBroom/issues/3332
    if (const auto* undoCommandName = m_map.undoCommandName();
        undoCommandName && *undoCommandName == TransferFaceAttributesTransactionName)
    {
      m_map.undoCommand();

      copyAttributesFromSelection(inputState, true);
      return true;
    }
  }

  return false;
}

namespace
{

bool copyMaterialOnlyModifiersDown(const InputState& inputState)
{
  return inputState.modifierKeys() == (ModifierKeys::Alt | ModifierKeys::CtrlCmd);
}

bool copyMaterialAttribsProjectionModifiersDown(const InputState& inputState)
{
  return inputState.modifierKeys() == (ModifierKeys::Alt);
}

bool copyMaterialAttribsRotationModifiersDown(const InputState& inputState)
{
  return inputState.modifierKeys() == (ModifierKeys::Alt | ModifierKeys::Shift);
}

/**
 * Checks the mouse state. The requirements are the same whether this is a click, a double
 * click, or a drag.
 */
bool applies(const InputState& inputState)
{
  const auto materialOnly = copyMaterialOnlyModifiersDown(inputState);
  const auto projection = copyMaterialAttribsProjectionModifiersDown(inputState);
  const auto rotation = copyMaterialAttribsRotationModifiersDown(inputState);

  return inputState.mouseButtonsPressed(MouseButtons::Left)
         && (materialOnly || projection || rotation);
}

size_t findClosestFace(const mdl::Brush& brush, const vm::vec3d& normal)
{
  size_t best = 0;
  for (size_t i = 1; i < brush.faceCount(); ++i)
  {
    const auto& bestFace = brush.face(best);
    const auto& face = brush.face(i);
    if (vm::dot(face.normal(), normal) > vm::dot(bestFace.normal(), normal))
    {
      best = i;
    }
  }
  return best;
}

/**
 * If the source face and the target face are in different linked groups with identical
 * link IDs, then applying a change to the target face will lead to the group containing
 * the source face to be deleted and replaced by the replicated group that contained the
 * target face. We want to avoid this.
 *
 * Instead, we want to find a face in the same group that contains the source face to
 * which we can apply the change, and achieve the same effect. For this, the new target
 * face must be linked to the old target face.
 *
 * Nested linked groups further complicate matters. We must make sure that we select the
 * innermost containing linked groups for both the old and new targets!
 */
std::optional<mdl::BrushFaceHandle> selectTargetFaceHandleForLinkedGroups(
  mdl::GroupNode& containingSourceGroupNode,
  const mdl::BrushFaceHandle& sourceFaceHandle,
  const mdl::BrushFaceHandle& oldTargetFaceHandle)
{
  const auto& sourceBrushNode = *sourceFaceHandle.node();
  const auto& oldTargetBrushNode = *oldTargetFaceHandle.node();

  // The target is already in the same linked group as the source
  if (containingSourceGroupNode.isAncestorOf(&oldTargetBrushNode))
  {
    return oldTargetFaceHandle;
  }

  const auto linkedTargetBrushNodesInSourceGroup =
    mdl::collectLinkedNodes({&containingSourceGroupNode}, oldTargetBrushNode);

  if (linkedTargetBrushNodesInSourceGroup.empty())
  {
    return oldTargetFaceHandle;
  }

  auto* newTargetBrushNode =
    dynamic_cast<mdl::BrushNode*>(linkedTargetBrushNodesInSourceGroup.front());
  ensure(newTargetBrushNode, "linked nodes are consistent");

  const auto* oldTargetContainingGroupNode = oldTargetBrushNode.containingGroup();
  assert(oldTargetContainingGroupNode);

  const auto* newTargetContainingGroupNode = newTargetBrushNode->containingGroup();
  assert(newTargetContainingGroupNode);

  ensure(
    oldTargetContainingGroupNode->linkId() == newTargetContainingGroupNode->linkId(),
    "containing groups are linked");

  const auto oldTargetTransformation =
    vm::invert(oldTargetContainingGroupNode->group().transformation());

  const auto newTargetTransformation =
    newTargetContainingGroupNode->group().transformation();
  const auto oldToNewTargetTransformation =
    newTargetTransformation * *oldTargetTransformation;

  // Find the face in the source group that corresponds to the target face by
  // untransforming the normal and searching the linked brush node
  const auto oldTargetNormal = oldTargetFaceHandle.face().normal();
  const auto newTargetNormal =
    vm::strip_translation(oldToNewTargetTransformation) * oldTargetNormal;

  const auto newTargetFaceIndex =
    findClosestFace(newTargetBrushNode->brush(), newTargetNormal);

  // Can't apply to the same face
  if (
    newTargetBrushNode != &sourceBrushNode
    || sourceFaceHandle.faceIndex() != newTargetFaceIndex)
  {
    return mdl::BrushFaceHandle{newTargetBrushNode, newTargetFaceIndex};
  }

  return std::nullopt;
}

auto selectTargetFaceHandlesForLinkedGroups(
  const mdl::BrushFaceHandle& sourceFaceHandle,
  const std::vector<mdl::BrushFaceHandle>& targetFaceHandles)
{
  auto* containingGroupNode = mdl::findContainingGroup(sourceFaceHandle.node());
  if (!containingGroupNode)
  {
    return targetFaceHandles;
  }

  auto result = std::vector<mdl::BrushFaceHandle>{};
  result.reserve(targetFaceHandles.size());

  for (const auto& targetFaceHandle : targetFaceHandles)
  {
    if (
      auto newTargetFaceHandle = selectTargetFaceHandleForLinkedGroups(
        *containingGroupNode, sourceFaceHandle, targetFaceHandle))
    {
      result.push_back(std::move(*newTargetFaceHandle));
    }
  }
  return result;
}

void transferFaceAttributes(
  mdl::Map& map,
  const InputState& inputState,
  const mdl::BrushFaceHandle& sourceFaceHandle,
  const std::vector<mdl::BrushFaceHandle>& targetFaceHandles,
  const mdl::BrushFaceHandle& faceToSelectAfter)
{
  const auto targetFaceHandlesForLinkedGroups =
    selectTargetFaceHandlesForLinkedGroups(sourceFaceHandle, targetFaceHandles);

  const auto style = copyMaterialAttribsRotationModifiersDown(inputState)
                       ? mdl::WrapStyle::Rotation
                       : mdl::WrapStyle::Projection;

  auto transaction = mdl::Transaction{map, TransferFaceAttributesTransactionName};
  deselectAll(map);
  selectBrushFaces(map, targetFaceHandlesForLinkedGroups);

  if (copyMaterialOnlyModifiersDown(inputState))
  {
    setBrushFaceAttributes(
      map, {.materialName = sourceFaceHandle.face().attributes().materialName()});
  }
  else
  {
    setBrushFaceAttributes(
      map, mdl::copyAllExceptContentFlags(sourceFaceHandle.face().attributes()));

    if (auto snapshot = sourceFaceHandle.face().takeUVCoordSystemSnapshot())
    {
      copyUV(
        map,
        *snapshot,
        sourceFaceHandle.face().attributes(),
        sourceFaceHandle.face().boundary(),
        style);
    }
  }

  deselectAll(map);
  selectBrushFaces(map, {faceToSelectAfter});
  transaction.commit();
}

class SetBrushFaceAttributesDragTracker : public GestureTracker
{
private:
  mdl::Map& m_map;
  mdl::BrushFaceHandle m_initialSelectedFaceHandle;
  std::optional<mdl::BrushFaceHandle> m_sourceFaceHandle;
  std::optional<mdl::BrushFaceHandle> m_targetFaceHandle;

public:
  SetBrushFaceAttributesDragTracker(
    mdl::Map& map, mdl::BrushFaceHandle initialSelectedFaceHandle)
    : m_map{map}
    , m_initialSelectedFaceHandle{std::move(initialSelectedFaceHandle)}
  {
  }

  bool update(const InputState& inputState) override
  {
    using namespace mdl::HitFilters;

    const auto& hit = inputState.pickResult().first(type(mdl::BrushNode::BrushHitType));
    const auto faceHandle = mdl::hitToFaceHandle(hit);
    if (!faceHandle)
    {
      // Dragging over void
      return true;
    }

    if (faceHandle == m_targetFaceHandle)
    {
      // Dragging on the same face as last frame
      return true;
    }

    if (!m_sourceFaceHandle && !m_targetFaceHandle)
    {
      // Start drag
      m_sourceFaceHandle = m_initialSelectedFaceHandle;
      m_targetFaceHandle = faceHandle;
    }
    else
    {
      // Continuing drag onto new face
      m_sourceFaceHandle = m_targetFaceHandle;
      m_targetFaceHandle = faceHandle;
    }

    transferFaceAttributes(
      m_map,
      inputState,
      *m_sourceFaceHandle,
      {*m_targetFaceHandle},
      m_initialSelectedFaceHandle);

    return true;
  }

  void end(const InputState&) override { m_map.commitTransaction(); }

  void cancel() override { m_map.cancelTransaction(); }
};
} // namespace

std::unique_ptr<GestureTracker> SetBrushFaceAttributesTool::acceptMouseDrag(
  const InputState& inputState)
{
  if (!applies(inputState))
  {
    return nullptr;
  }

  // Need to have a selected face to start painting alignment
  const auto& selectedFaces = m_map.selection().brushFaces;
  if (selectedFaces.size() != 1)
  {
    return nullptr;
  }

  m_map.startTransaction(
    "Drag Apply Face Attributes", mdl::TransactionScope::LongRunning);
  return std::make_unique<SetBrushFaceAttributesDragTracker>(
    m_map, selectedFaces.front());
}

bool SetBrushFaceAttributesTool::cancel()
{
  return false;
}

void SetBrushFaceAttributesTool::copyAttributesFromSelection(
  const InputState& inputState, const bool applyToBrush)
{
  using namespace mdl::HitFilters;

  assert(canCopyAttributesFromSelection(inputState));

  const auto selectedFaces = m_map.selection().brushFaces;
  assert(!selectedFaces.empty());

  const auto& hit = inputState.pickResult().first(type(mdl::BrushNode::BrushHitType));
  if (const auto targetFaceHandle = mdl::hitToFaceHandle(hit))
  {
    const auto sourceFaceHandle = selectedFaces.front();
    const auto targetList = applyToBrush
                              ? mdl::toHandles(targetFaceHandle->node())
                              : std::vector<mdl::BrushFaceHandle>{*targetFaceHandle};

    transferFaceAttributes(
      m_map, inputState, sourceFaceHandle, targetList, sourceFaceHandle);
  }
}

bool SetBrushFaceAttributesTool::canCopyAttributesFromSelection(
  const InputState& inputState) const
{
  using namespace mdl::HitFilters;

  if (!applies(inputState))
  {
    return false;
  }

  const auto selectedFaces = m_map.selection().brushFaces;
  if (selectedFaces.size() != 1)
  {
    return false;
  }

  const auto& hit = inputState.pickResult().first(type(mdl::BrushNode::BrushHitType));
  return hit.isMatch();
}

} // namespace tb::ui
