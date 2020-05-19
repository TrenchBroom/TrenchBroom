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

#include "Model/BrushFace.h"
#include "Model/BrushNode.h"
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
        SetBrushFaceAttributesTool::SetBrushFaceAttributesTool(std::weak_ptr<MapDocument> document) :
        ToolControllerBase(),
        Tool(true),
        m_document(document),
        m_dragInitialSelectedFace(nullptr),
        m_dragTargetFace(nullptr),
        m_dragSourceFace(nullptr) {}

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
                // A double click is always preceeded by a single click, so we already done some work which is now
                // superseded by what is done next. To avoid inconsistencies with undo, we undo the work done by the
                // single click now:
                auto document = kdl::mem_lock(m_document);
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

            const std::vector<Model::BrushFace*>& selectedFaces = document->selectedBrushFaces();
            const Model::Hit& hit = inputState.pickResult().query().pickable().type(Model::BrushNode::BrushHit).occluded().first();

            Model::BrushFace* source = selectedFaces.front();
            Model::BrushFace* targetFace = Model::hitToFace(hit);
            Model::BrushNode* targetBrush = targetFace->brush();
            const std::vector<Model::BrushFace*> targetList = applyToBrush ? targetBrush->faces() : std::vector<Model::BrushFace*>({ targetFace });

            transferFaceAttributes(inputState, source, targetList, source);
        }

        bool SetBrushFaceAttributesTool::canCopyAttributesFromSelection(const InputState& inputState) const {
            if (!applies(inputState)) {
                return false;
            }

            auto document = kdl::mem_lock(m_document);

            const std::vector<Model::BrushFace*>& selectedFaces = document->selectedBrushFaces();
            if (selectedFaces.size() != 1) {
                return false;
            }

            const Model::Hit& hit = inputState.pickResult().query().pickable().type(Model::BrushNode::BrushHit).occluded().first();
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
            const std::vector<Model::BrushFace*>& selectedFaces = document->selectedBrushFaces();
            if (selectedFaces.size() != 1) {
                return false;
            }

            resetDragState();
            m_dragInitialSelectedFace = selectedFaces[0];

            document->startTransaction("Drag Apply Face Attributes");

            return true;
        }

        bool SetBrushFaceAttributesTool::doMouseDrag(const InputState& inputState) {            
            const Model::Hit& hit = inputState.pickResult().query().pickable().type(Model::BrushNode::BrushHit).occluded().first();
            if (!hit.isMatch()) {
                // Dragging over void
                return true;
            }

            Model::BrushFace* face = Model::hitToFace(hit);
            if (face == m_dragTargetFace) {
                // Dragging on the same face as last frame
                return true;
            }

            if (m_dragSourceFace == nullptr && m_dragTargetFace == nullptr) {
                // Start drag
                m_dragSourceFace = m_dragInitialSelectedFace;
                m_dragTargetFace = face;                
            } else {
                // Continuing drag onto new face
                m_dragSourceFace = m_dragTargetFace;
                m_dragTargetFace = face;
            }

            transferFaceAttributes(inputState, m_dragSourceFace, { m_dragTargetFace }, m_dragInitialSelectedFace);

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
            m_dragInitialSelectedFace = nullptr;
            m_dragTargetFace = nullptr;
            m_dragSourceFace = nullptr;
        }

        void SetBrushFaceAttributesTool::transferFaceAttributes(const InputState& inputState, Model::BrushFace* sourceFace,
                                                                const std::vector<Model::BrushFace*>& targetFaces, Model::BrushFace* faceToSelectAfter) {
            ensure(sourceFace != nullptr, "no source face");
            ensure(faceToSelectAfter != nullptr, "no face to select after");

            auto document = kdl::mem_lock(m_document);

            const Model::WrapStyle style =
                    copyTextureAttribsRotationModifiersDown(inputState) ? Model::WrapStyle::Rotation : Model::WrapStyle::Projection;

            const Transaction transaction(document);
            document->deselectAll();
            document->select(targetFaces);

            if (copyTextureOnlyModifiersDown(inputState)) {
                document->setTexture(sourceFace->texture(), false);
            } else {
                auto snapshot = sourceFace->takeTexCoordSystemSnapshot();
                document->setFaceAttributes(sourceFace->attribs());
                if (snapshot != nullptr) {
                    document->copyTexCoordSystemFromFace(*snapshot, sourceFace->attribs().takeSnapshot(), sourceFace->boundary(), style);
                }
            }

            document->deselectAll();
            document->select(faceToSelectAfter);
        }
    }
}
