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

#include "Model/Brush.h"
#include "Model/BrushFace.h"
#include "Model/BrushFaceHandle.h"
#include "Model/BrushNode.h"
#include "Model/ChangeBrushFaceAttributesRequest.h"
#include "Model/HitAdapter.h"
#include "Model/HitQuery.h"
#include "Model/TexCoordSystem.h"
#include "View/InputState.h"
#include "View/MapDocument.h"

#include <kdl/memory_utils.h>

#include <vector>

#include "Ensure.h"

namespace TrenchBroom {
    namespace View {
        static const std::string TransferFaceAttributesTransactionName = "Transfer Face Attributes";

        SetBrushFaceAttributesTool::SetBrushFaceAttributesTool(std::weak_ptr<MapDocument> document) :
        ToolControllerBase(),
        Tool(true),
        m_document(document) {}

        Tool* SetBrushFaceAttributesTool::doGetTool() {
            return this;
        }

        const Tool* SetBrushFaceAttributesTool::doGetTool() const {
            return this;
        }

        bool SetBrushFaceAttributesTool::doMouseClick(const InputState& inputState) {
            if (canCopyAttributesFromSelection(inputState)) {
                copyAttributesFromSelection(inputState, false);
                return true;
            } else {
                return false;
            }
        }

        bool SetBrushFaceAttributesTool::doMouseDoubleClick(const InputState& inputState) {
            if (canCopyAttributesFromSelection(inputState)) {
                // The typical use case is, doMouseClick() previously copied the selected attributes to the clicked face,
                // and now the second click has arrived so we're about to copy the selected attributes to the whole brush.
                // To make undo/redo more intuitivie, undo the application to the single face now, so that if the
                // double click is later undone/redone, it appears as one atomic action.
                auto document = kdl::mem_lock(m_document);

                if (!document->canUndoCommand() || document->undoCommandName() != TransferFaceAttributesTransactionName) {
                    // The last click may not have been handled by this tool, see:
                    // https://github.com/TrenchBroom/TrenchBroom/issues/3332
                    return false;
                }
                document->undoCommand();

                copyAttributesFromSelection(inputState, true);
                return true;
            } else {
                return false;
            }
        }

        void SetBrushFaceAttributesTool::copyAttributesFromSelection(const InputState& inputState, const bool applyToBrush) {
            assert(canCopyAttributesFromSelection(inputState));

            auto document = kdl::mem_lock(m_document);

            const auto selectedFaces = document->selectedBrushFaces();
            assert(!selectedFaces.empty());

            const Model::Hit& hit = inputState.pickResult().query().pickable().type(Model::BrushNode::BrushHitType).occluded().first();
            if (const auto targetFaceHandle = Model::hitToFaceHandle(hit)) {
                const auto sourceFaceHandle = selectedFaces.front();
                const auto targetList = applyToBrush ? Model::toHandles(targetFaceHandle->node()) : std::vector<Model::BrushFaceHandle>{*targetFaceHandle};

                transferFaceAttributes(inputState, sourceFaceHandle, targetList, sourceFaceHandle);
            }
        }

        bool SetBrushFaceAttributesTool::canCopyAttributesFromSelection(const InputState& inputState) const {
            if (!applies(inputState)) {
                return false;
            }

            auto document = kdl::mem_lock(m_document);

            const auto selectedFaces = document->selectedBrushFaces();
            if (selectedFaces.size() != 1) {
                return false;
            }

            const Model::Hit& hit = inputState.pickResult().query().pickable().type(Model::BrushNode::BrushHitType).occluded().first();
            if (!hit.isMatch()) {
                return false;
            }

            return true;
        }

        /**
         * Checks the mouse state. The requirements are the same whether this is a click, a double click, or a drag.
         */
        bool SetBrushFaceAttributesTool::applies(const InputState& inputState) const {
            const bool textureOnly = copyTextureOnlyModifiersDown(inputState);
            const bool projection = copyTextureAttribsProjectionModifiersDown(inputState);
            const bool rotation = copyTextureAttribsRotationModifiersDown(inputState);

            return inputState.mouseButtonsPressed(MouseButtons::MBLeft)
                   && (textureOnly || projection || rotation);
        }

        bool SetBrushFaceAttributesTool::copyTextureOnlyModifiersDown(const InputState& inputState) const {
            return inputState.modifierKeys() == (ModifierKeys::MKAlt | ModifierKeys::MKCtrlCmd);
        }

        bool SetBrushFaceAttributesTool::copyTextureAttribsProjectionModifiersDown(const InputState& inputState) const {
            return inputState.modifierKeys() == (ModifierKeys::MKAlt);
        }

        bool SetBrushFaceAttributesTool::copyTextureAttribsRotationModifiersDown(const InputState& inputState) const {
            return inputState.modifierKeys() == (ModifierKeys::MKAlt | ModifierKeys::MKShift);
        }

        bool SetBrushFaceAttributesTool::doCancel() {
            return false;
        }

        bool SetBrushFaceAttributesTool::doStartMouseDrag(const InputState& inputState) {
            if (!applies(inputState)) {
                return false;
            }

            auto document = kdl::mem_lock(m_document);

            // Need to have a selected face to start painting alignment
            const std::vector<Model::BrushFaceHandle>& selectedFaces = document->selectedBrushFaces();
            if (selectedFaces.size() != 1) {
                return false;
            }

            resetDragState();
            m_dragInitialSelectedFaceHandle = selectedFaces[0];

            document->startTransaction("Drag Apply Face Attributes");

            return true;
        }

        bool SetBrushFaceAttributesTool::doMouseDrag(const InputState& inputState) {            
            const Model::Hit& hit = inputState.pickResult().query().pickable().type(Model::BrushNode::BrushHitType).occluded().first();
            const auto faceHandle = Model::hitToFaceHandle(hit);
            if (!faceHandle) {
                // Dragging over void
                return true;
            }

            if (faceHandle == m_dragTargetFaceHandle) {
                // Dragging on the same face as last frame
                return true;
            }

            if (!m_dragTargetFaceHandle && !m_dragTargetFaceHandle) {
                // Start drag
                m_dragSourceFaceHandle = m_dragInitialSelectedFaceHandle;
                m_dragTargetFaceHandle = faceHandle;
            } else {
                // Continuing drag onto new face
                m_dragSourceFaceHandle = m_dragTargetFaceHandle;
                m_dragTargetFaceHandle = faceHandle;
            }

            transferFaceAttributes(inputState, *m_dragSourceFaceHandle, { *m_dragTargetFaceHandle }, *m_dragInitialSelectedFaceHandle);

            return true;
        }

        void SetBrushFaceAttributesTool::doEndMouseDrag(const InputState&) {
            auto document = kdl::mem_lock(m_document);
            document->commitTransaction();

            resetDragState();
        }

        void SetBrushFaceAttributesTool::doCancelMouseDrag() {
            auto document = kdl::mem_lock(m_document);
            document->cancelTransaction();

            resetDragState();
        }

        void SetBrushFaceAttributesTool::resetDragState() {
            m_dragInitialSelectedFaceHandle = std::nullopt;
            m_dragTargetFaceHandle = std::nullopt;
            m_dragSourceFaceHandle = std::nullopt;
        }

        void SetBrushFaceAttributesTool::transferFaceAttributes(const InputState& inputState, const Model::BrushFaceHandle& sourceFaceHandle, const std::vector<Model::BrushFaceHandle>& targetFaceHandles, const Model::BrushFaceHandle& faceToSelectAfter) {
            auto document = kdl::mem_lock(m_document);

            const Model::WrapStyle style =
                    copyTextureAttribsRotationModifiersDown(inputState) ? Model::WrapStyle::Rotation : Model::WrapStyle::Projection;

            const Transaction transaction(document, TransferFaceAttributesTransactionName);
            document->deselectAll();
            document->select(targetFaceHandles);

            if (copyTextureOnlyModifiersDown(inputState)) {
                Model::ChangeBrushFaceAttributesRequest request;
                request.setTextureName(sourceFaceHandle.face().attributes().textureName());
                document->setFaceAttributes(request);
            } else {
                auto snapshot = sourceFaceHandle.face().takeTexCoordSystemSnapshot();
                document->setFaceAttributesExceptContentFlags(sourceFaceHandle.face().attributes());
                if (snapshot != nullptr) {
                    document->copyTexCoordSystemFromFace(*snapshot, sourceFaceHandle.face().attributes(), sourceFaceHandle.face().boundary(), style);
                }
            }

            document->deselectAll();
            document->select(faceToSelectAfter);
        }
    }
}
