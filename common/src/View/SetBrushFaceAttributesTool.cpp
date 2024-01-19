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

#include "SetBrushFaceAttributesTool.h"

#include "Ensure.h"
#include "Model/Brush.h"
#include "Model/BrushFace.h"
#include "Model/BrushFaceHandle.h"
#include "Model/BrushNode.h"
#include "Model/ChangeBrushFaceAttributesRequest.h"
#include "Model/Hit.h"
#include "Model/HitAdapter.h"
#include "Model/HitFilter.h"
#include "Model/LinkedGroupUtils.h"
#include "Model/ModelUtils.h"
#include "Model/TexCoordSystem.h"
#include "View/DragTracker.h"
#include "View/InputState.h"
#include "View/MapDocument.h"
#include "View/TransactionScope.h"

#include <kdl/memory_utils.h>

#include <vecmath/polygon.h>

#include <vector>

namespace TrenchBroom::View
{

static const std::string TransferFaceAttributesTransactionName =
  "Transfer Face Attributes";

SetBrushFaceAttributesTool::SetBrushFaceAttributesTool(
  std::weak_ptr<MapDocument> document)
  : ToolController{}
  , Tool{true}
  , m_document{std::move(document)}
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
    auto document = kdl::mem_lock(m_document);

    // The last click may not have been handled by this tool, see:
    // https://github.com/TrenchBroom/TrenchBroom/issues/3332
    if (
      document->canUndoCommand()
      && document->undoCommandName() == TransferFaceAttributesTransactionName)
    {
      document->undoCommand();

      copyAttributesFromSelection(inputState, true);
      return true;
    }
  }

  return false;
}

namespace
{

bool copyTextureOnlyModifiersDown(const InputState& inputState)
{
  return inputState.modifierKeys() == (ModifierKeys::MKAlt | ModifierKeys::MKCtrlCmd);
}

bool copyTextureAttribsProjectionModifiersDown(const InputState& inputState)
{
  return inputState.modifierKeys() == (ModifierKeys::MKAlt);
}

bool copyTextureAttribsRotationModifiersDown(const InputState& inputState)
{
  return inputState.modifierKeys() == (ModifierKeys::MKAlt | ModifierKeys::MKShift);
}

/**
 * Checks the mouse state. The requirements are the same whether this is a click, a double
 * click, or a drag.
 */
bool applies(const InputState& inputState)
{
  const auto textureOnly = copyTextureOnlyModifiersDown(inputState);
  const auto projection = copyTextureAttribsProjectionModifiersDown(inputState);
  const auto rotation = copyTextureAttribsRotationModifiersDown(inputState);

  return inputState.mouseButtonsPressed(MouseButtons::MBLeft)
         && (textureOnly || projection || rotation);
}

std::optional<Model::BrushFaceHandle> selectTargetFaceHandleForLinkedGroups(
  Model::GroupNode& containingSourceGroupNode,
  const Model::BrushFaceHandle& sourceFaceHandle,
  const Model::BrushFaceHandle& targetFaceHandle)
{
  const auto& sourceBrushNode = *sourceFaceHandle.node();
  const auto& targetBrushNode = *targetFaceHandle.node();
  const auto linkedBrushNodes =
    Model::collectLinkedNodes({&containingSourceGroupNode}, targetBrushNode);

  if (linkedBrushNodes.empty())
  {
    return targetFaceHandle;
  }

  auto* linkedBrushNodeInSourceGroup =
    dynamic_cast<Model::BrushNode*>(linkedBrushNodes.front());
  ensure(linkedBrushNodeInSourceGroup, "linked nodes are consistent");

  const auto* containingTargetGroupNode = targetBrushNode.containingGroup();
  assert(containingTargetGroupNode);

  const auto [invertible, targetTransformation] =
    vm::invert(containingTargetGroupNode->group().transformation());
  assert(invertible);
  unused(invertible);

  const auto transformation =
    containingSourceGroupNode.group().transformation() * targetTransformation;

  // Find the face in the source group that corresponds to the target face by
  // untransforming the normal and searching the linked brush node
  const auto targetNormal = targetFaceHandle.face().normal();
  const auto targetFaceInSourceGroupNormal =
    vm::strip_translation(transformation) * targetNormal;
  if (
    const auto targetFaceInLinkedBrushIndex =
      linkedBrushNodeInSourceGroup->brush().findFace(targetFaceInSourceGroupNormal))
  {
    // Can't apply to the same face
    if (
      linkedBrushNodeInSourceGroup != &sourceBrushNode
      || sourceFaceHandle.faceIndex() != targetFaceInLinkedBrushIndex)
    {
      return Model::BrushFaceHandle{
        linkedBrushNodeInSourceGroup, *targetFaceInLinkedBrushIndex};
    }
  }

  return std::nullopt;
}

auto selectTargetFaceHandlesForLinkedGroups(
  const Model::BrushFaceHandle& sourceFaceHandle,
  const std::vector<Model::BrushFaceHandle>& targetFaceHandles)
{
  auto* containingGroupNode = Model::findContainingGroup(sourceFaceHandle.node());
  if (!containingGroupNode)
  {
    return targetFaceHandles;
  }

  auto result = std::vector<Model::BrushFaceHandle>{};
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
  MapDocument& document,
  const InputState& inputState,
  const Model::BrushFaceHandle& sourceFaceHandle,
  const std::vector<Model::BrushFaceHandle>& targetFaceHandles,
  const Model::BrushFaceHandle& faceToSelectAfter)
{
  const auto targetFaceHandlesForLinkedGroups =
    selectTargetFaceHandlesForLinkedGroups(sourceFaceHandle, targetFaceHandles);

  const auto style = copyTextureAttribsRotationModifiersDown(inputState)
                       ? Model::WrapStyle::Rotation
                       : Model::WrapStyle::Projection;

  auto transaction = Transaction{document, TransferFaceAttributesTransactionName};
  document.deselectAll();
  document.selectBrushFaces(targetFaceHandlesForLinkedGroups);

  if (copyTextureOnlyModifiersDown(inputState))
  {
    auto request = Model::ChangeBrushFaceAttributesRequest{};
    request.setTextureName(sourceFaceHandle.face().attributes().textureName());
    document.setFaceAttributes(request);
  }
  else
  {
    auto snapshot = sourceFaceHandle.face().takeTexCoordSystemSnapshot();
    document.setFaceAttributesExceptContentFlags(sourceFaceHandle.face().attributes());
    if (snapshot)
    {
      document.copyTexCoordSystemFromFace(
        *snapshot,
        sourceFaceHandle.face().attributes(),
        sourceFaceHandle.face().boundary(),
        style);
    }
  }

  document.deselectAll();
  document.selectBrushFaces({faceToSelectAfter});
  transaction.commit();
}

class SetBrushFaceAttributesDragTracker : public DragTracker
{
private:
  MapDocument& m_document;
  Model::BrushFaceHandle m_initialSelectedFaceHandle;
  std::optional<Model::BrushFaceHandle> m_sourceFaceHandle;
  std::optional<Model::BrushFaceHandle> m_targetFaceHandle;

public:
  SetBrushFaceAttributesDragTracker(
    MapDocument& document, Model::BrushFaceHandle initialSelectedFaceHandle)
    : m_document{document}
    , m_initialSelectedFaceHandle{std::move(initialSelectedFaceHandle)}
  {
  }

  bool drag(const InputState& inputState) override
  {
    using namespace Model::HitFilters;

    const auto& hit = inputState.pickResult().first(type(Model::BrushNode::BrushHitType));
    const auto faceHandle = Model::hitToFaceHandle(hit);
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
      m_document,
      inputState,
      *m_sourceFaceHandle,
      {*m_targetFaceHandle},
      m_initialSelectedFaceHandle);

    return true;
  }

  void end(const InputState&) override { m_document.commitTransaction(); }

  void cancel() override { m_document.cancelTransaction(); }
};
} // namespace

std::unique_ptr<DragTracker> SetBrushFaceAttributesTool::acceptMouseDrag(
  const InputState& inputState)
{
  if (!applies(inputState))
  {
    return nullptr;
  }

  auto document = kdl::mem_lock(m_document);

  // Need to have a selected face to start painting alignment
  const auto& selectedFaces = document->selectedBrushFaces();
  if (selectedFaces.size() != 1)
  {
    return nullptr;
  }

  document->startTransaction("Drag Apply Face Attributes", TransactionScope::LongRunning);

  return std::make_unique<SetBrushFaceAttributesDragTracker>(
    *kdl::mem_lock(m_document), selectedFaces.front());
}

bool SetBrushFaceAttributesTool::cancel()
{
  return false;
}

void SetBrushFaceAttributesTool::copyAttributesFromSelection(
  const InputState& inputState, const bool applyToBrush)
{
  using namespace Model::HitFilters;

  assert(canCopyAttributesFromSelection(inputState));

  auto document = kdl::mem_lock(m_document);

  const auto selectedFaces = document->selectedBrushFaces();
  assert(!selectedFaces.empty());

  const auto& hit = inputState.pickResult().first(type(Model::BrushNode::BrushHitType));
  if (const auto targetFaceHandle = Model::hitToFaceHandle(hit))
  {
    const auto sourceFaceHandle = selectedFaces.front();
    const auto targetList = applyToBrush
                              ? Model::toHandles(targetFaceHandle->node())
                              : std::vector<Model::BrushFaceHandle>{*targetFaceHandle};

    transferFaceAttributes(
      *document, inputState, sourceFaceHandle, targetList, sourceFaceHandle);
  }
}

bool SetBrushFaceAttributesTool::canCopyAttributesFromSelection(
  const InputState& inputState) const
{
  using namespace Model::HitFilters;

  if (!applies(inputState))
  {
    return false;
  }

  auto document = kdl::mem_lock(m_document);

  const auto selectedFaces = document->selectedBrushFaces();
  if (selectedFaces.size() != 1)
  {
    return false;
  }

  const auto& hit = inputState.pickResult().first(type(Model::BrushNode::BrushHitType));
  return hit.isMatch();
}

} // namespace TrenchBroom::View
