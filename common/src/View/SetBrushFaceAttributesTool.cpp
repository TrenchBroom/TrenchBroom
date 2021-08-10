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
#include "Model/Hit.h"
#include "Model/HitAdapter.h"
#include "Model/HitFilter.h"
#include "Model/TexCoordSystem.h"
#include "View/DragTracker.h"
#include "View/InputState.h"
#include "View/MapDocument.h"

#include <kdl/memory_utils.h>

#include <vector>

#include "Ensure.h"

namespace TrenchBroom {
    namespace View {
        static const std::string TransferFaceAttributesTransactionName = "Transfer Face Attributes";

        SetBrushFaceAttributesTool::SetBrushFaceAttributesTool(std::weak_ptr<MapDocument> document) :
        ToolController{},
        Tool{true},
        m_document{document} {}

        Tool& SetBrushFaceAttributesTool::tool() {
            return *this;
        }

        const Tool& SetBrushFaceAttributesTool::tool() const {
            return *this;
        }

        bool SetBrushFaceAttributesTool::mouseClick(const InputState& inputState) {
            if (canCopyAttributesFromSelection(inputState)) {
                copyAttributesFromSelection(inputState, false);
                return true;
            }
            
            return false;
        }

        bool SetBrushFaceAttributesTool::mouseDoubleClick(const InputState& inputState) {
            if (canCopyAttributesFromSelection(inputState)) {
                // The typical use case is, mouseClick() previously copied the selected attributes to the clicked face,
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
            }
            
            return false;
        }

        static bool copyTextureOnlyModifiersDown(const InputState& inputState) {
            return inputState.modifierKeys() == (ModifierKeys::MKAlt | ModifierKeys::MKCtrlCmd);
        }

        static bool copyTextureAttribsProjectionModifiersDown(const InputState& inputState) {
            return inputState.modifierKeys() == (ModifierKeys::MKAlt);
        }

        static bool copyTextureAttribsRotationModifiersDown(const InputState& inputState) {
            return inputState.modifierKeys() == (ModifierKeys::MKAlt | ModifierKeys::MKShift);
        }

        /**
         * Checks the mouse state. The requirements are the same whether this is a click, a double click, or a drag.
         */
        static bool applies(const InputState& inputState) {
            const bool textureOnly = copyTextureOnlyModifiersDown(inputState);
            const bool projection = copyTextureAttribsProjectionModifiersDown(inputState);
            const bool rotation = copyTextureAttribsRotationModifiersDown(inputState);

            return inputState.mouseButtonsPressed(MouseButtons::MBLeft)
                   && (textureOnly || projection || rotation);
        }

        static void transferFaceAttributes(MapDocument& document, const InputState& inputState, const Model::BrushFaceHandle& sourceFaceHandle, const std::vector<Model::BrushFaceHandle>& targetFaceHandles, const Model::BrushFaceHandle& faceToSelectAfter) {
            const Model::WrapStyle style =
                    copyTextureAttribsRotationModifiersDown(inputState) ? Model::WrapStyle::Rotation : Model::WrapStyle::Projection;

            const Transaction transaction(&document, TransferFaceAttributesTransactionName);
            document.deselectAll();
            document.select(targetFaceHandles);

            if (copyTextureOnlyModifiersDown(inputState)) {
                Model::ChangeBrushFaceAttributesRequest request;
                request.setTextureName(sourceFaceHandle.face().attributes().textureName());
                document.setFaceAttributes(request);
            } else {
                auto snapshot = sourceFaceHandle.face().takeTexCoordSystemSnapshot();
                document.setFaceAttributesExceptContentFlags(sourceFaceHandle.face().attributes());
                if (snapshot != nullptr) {
                    document.copyTexCoordSystemFromFace(*snapshot, sourceFaceHandle.face().attributes(), sourceFaceHandle.face().boundary(), style);
                }
            }

            document.deselectAll();
            document.select(faceToSelectAfter);
        }

        namespace {
            class SetBrushFaceAttributesDragTracker : public DragTracker {
            private:
                MapDocument& m_document;
                Model::BrushFaceHandle m_initialSelectedFaceHandle;
                std::optional<Model::BrushFaceHandle> m_sourceFaceHandle;
                std::optional<Model::BrushFaceHandle> m_targetFaceHandle;
            public:
                SetBrushFaceAttributesDragTracker(MapDocument& document, Model::BrushFaceHandle initialSelectedFaceHandle) :
                m_document{document},
                m_initialSelectedFaceHandle{std::move(initialSelectedFaceHandle)} {}

                bool drag(const InputState& inputState) {
                    using namespace Model::HitFilters;

                    const Model::Hit& hit = inputState.pickResult().first(type(Model::BrushNode::BrushHitType));
                    const auto faceHandle = Model::hitToFaceHandle(hit);
                    if (!faceHandle) {
                        // Dragging over void
                        return true;
                    }

                    if (faceHandle == m_targetFaceHandle) {
                        // Dragging on the same face as last frame
                        return true;
                    }

                    if (!m_sourceFaceHandle && !m_targetFaceHandle) {
                        // Start drag
                        m_sourceFaceHandle = m_initialSelectedFaceHandle;
                        m_targetFaceHandle = faceHandle;
                    } else {
                        // Continuing drag onto new face
                        m_sourceFaceHandle = m_targetFaceHandle;
                        m_targetFaceHandle = faceHandle;
                    }

                    transferFaceAttributes(m_document, inputState, *m_sourceFaceHandle, { *m_targetFaceHandle }, m_initialSelectedFaceHandle);

                    return true;
                }

                void end(const InputState&) {
                    m_document.commitTransaction();
                }

                void cancel() {
                    m_document.cancelTransaction();
                }
            };
        }

        std::unique_ptr<DragTracker> SetBrushFaceAttributesTool::acceptMouseDrag(const InputState& inputState) {
            if (!applies(inputState)) {
                return nullptr;
            }

            auto document = kdl::mem_lock(m_document);

            // Need to have a selected face to start painting alignment
            const std::vector<Model::BrushFaceHandle>& selectedFaces = document->selectedBrushFaces();
            if (selectedFaces.size() != 1) {
                return nullptr;
            }

            document->startTransaction("Drag Apply Face Attributes");

            return std::make_unique<SetBrushFaceAttributesDragTracker>(*kdl::mem_lock(m_document), selectedFaces.front());
        }

        bool SetBrushFaceAttributesTool::cancel() {
            return false;
        }

        void SetBrushFaceAttributesTool::copyAttributesFromSelection(const InputState& inputState, const bool applyToBrush) {
            using namespace Model::HitFilters;

            assert(canCopyAttributesFromSelection(inputState));

            auto document = kdl::mem_lock(m_document);

            const auto selectedFaces = document->selectedBrushFaces();
            assert(!selectedFaces.empty());

            const Model::Hit& hit = inputState.pickResult().first(type(Model::BrushNode::BrushHitType));
            if (const auto targetFaceHandle = Model::hitToFaceHandle(hit)) {
                const auto sourceFaceHandle = selectedFaces.front();
                const auto targetList = applyToBrush ? Model::toHandles(targetFaceHandle->node()) : std::vector<Model::BrushFaceHandle>{*targetFaceHandle};

                transferFaceAttributes(*document, inputState, sourceFaceHandle, targetList, sourceFaceHandle);
            }
        }

        bool SetBrushFaceAttributesTool::canCopyAttributesFromSelection(const InputState& inputState) const {
            using namespace Model::HitFilters;

            if (!applies(inputState)) {
                return false;
            }

            auto document = kdl::mem_lock(m_document);

            const auto selectedFaces = document->selectedBrushFaces();
            if (selectedFaces.size() != 1) {
                return false;
            }

            const Model::Hit& hit = inputState.pickResult().first(type(Model::BrushNode::BrushHitType));
            if (!hit.isMatch()) {
                return false;
            }

            return true;
        }
    }
}
