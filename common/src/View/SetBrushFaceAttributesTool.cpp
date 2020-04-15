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
#include "Model/Brush.h"
#include "Model/HitAdapter.h"
#include "Model/HitQuery.h"
#include "Model/TexCoordSystem.h"
#include "View/InputState.h"
#include "View/MapDocument.h"

#include <kdl/memory_utils.h>

#include <vector>

#include <QDebug>

namespace TrenchBroom {
    namespace View {
        SetBrushFaceAttributesTool::SetBrushFaceAttributesTool(std::weak_ptr<MapDocument> document) :
        ToolControllerBase(),
        Tool(true),
        m_document(document),
        m_lastDraggedBrushFace(nullptr),
        m_secondLastDraggedBrushFace(nullptr) {}

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

        bool SetBrushFaceAttributesTool::doStartMouseDrag(const InputState& inputState) {
            if (!applies(inputState))
                return false;

            auto document = kdl::mem_lock(m_document);

            // Need to have a selected face to start painting alignment
            const std::vector<Model::BrushFace*>& selectedFaces = document->selectedBrushFaces();
            if (selectedFaces.size() != 1) {
                return false;
            }
            Model::BrushFace* selectedFace = selectedFaces[0];
            m_secondLastDraggedBrushFace = selectedFace;

            // Optionally clicking on a face
            const auto& hit = firstHit(inputState, Model::Brush::BrushHit);
            if (hit.isMatch()) {
                Model::BrushFace* clickedFace = Model::hitToFace(hit);
                m_lastDraggedBrushFace = clickedFace;
            } else {
                m_lastDraggedBrushFace = nullptr;
            }

            qDebug() << "start drag";           
            document->startTransaction("Drag Select Brush Faces");
            transferFaceAttributes(inputState,
                m_secondLastDraggedBrushFace, 
                m_lastDraggedBrushFace);

            return true;
        }

        bool SetBrushFaceAttributesTool::doMouseDrag(const InputState& inputState) {            
            const auto& hit = firstHit(inputState, Model::Brush::BrushHit);
            if (!hit.isMatch()) {
                // Dragging over void
                return true;
            }

            Model::BrushFace* face = Model::hitToFace(hit);
            if (face == m_lastDraggedBrushFace) {
                // Dragging on the same face as last frame
                return true;
            }

            // Dragged onto a new face
            m_secondLastDraggedBrushFace = m_lastDraggedBrushFace;
            m_lastDraggedBrushFace = face;

            transferFaceAttributes(inputState,
                m_secondLastDraggedBrushFace, 
                m_lastDraggedBrushFace);

            return true;
        }

        void SetBrushFaceAttributesTool::doEndMouseDrag(const InputState& inputState) {
            auto document = kdl::mem_lock(m_document);
            document->commitTransaction();

            m_lastDraggedBrushFace = nullptr;
            m_secondLastDraggedBrushFace = nullptr;
        }

        void SetBrushFaceAttributesTool::doCancelMouseDrag() {
            auto document = kdl::mem_lock(m_document);
            document->cancelTransaction();

            m_lastDraggedBrushFace = nullptr;
            m_secondLastDraggedBrushFace = nullptr;
        }

        void SetBrushFaceAttributesTool::transferFaceAttributes(const InputState& inputState, Model::BrushFace* from, Model::BrushFace* to) {
            qDebug() << "transfer from " << from << " to " << to;

            auto document = kdl::mem_lock(m_document);

            const Model::WrapStyle wrapStyle = inputState.modifierKeysDown(ModifierKeys::MKShift) ? Model::WrapStyle::Rotation : Model::WrapStyle::Projection;

            const Transaction transaction(document);
            document->deselectAll();
            document->select(to);

            auto snapshot = from->takeTexCoordSystemSnapshot();
            document->setFaceAttributes(from->attribs());
            if (snapshot != nullptr) {
                document->copyTexCoordSystemFromFace(*snapshot, from->attribs().takeSnapshot(), from->boundary(), wrapStyle);
            }

            document->deselectAll();
            document->select(from);
        }

        void SetBrushFaceAttributesTool::copyAttributesFromSelection(const InputState& inputState, const bool applyToBrush) {
            assert(canCopyAttributesFromSelection(inputState));
            
            auto document = kdl::mem_lock(m_document);

            const std::vector<Model::BrushFace*>& selectedFaces = document->selectedBrushFaces();
            const Model::Hit& hit = inputState.pickResult().query().pickable().type(Model::Brush::BrushHit).occluded().first();

            Model::BrushFace* source = selectedFaces.front();
            Model::BrushFace* targetFace = Model::hitToFace(hit);
            Model::Brush* targetBrush = targetFace->brush();
            const std::vector<Model::BrushFace*> targetList = applyToBrush ? targetBrush->faces() : std::vector<Model::BrushFace*>({ targetFace });

            const Model::WrapStyle wrapStyle = inputState.modifierKeysDown(ModifierKeys::MKShift) ? Model::WrapStyle::Rotation : Model::WrapStyle::Projection;

            const Transaction transaction(document);
            document->deselectAll();
            document->select(targetList);
            if (copyAllAttributes(inputState)) {
                auto snapshot = source->takeTexCoordSystemSnapshot();
                document->setFaceAttributes(source->attribs());
                if (snapshot != nullptr) {
                    document->copyTexCoordSystemFromFace(*snapshot, source->attribs().takeSnapshot(), source->boundary(), wrapStyle);
                }
            } else {
                document->setTexture(source->texture());
            }
            document->deselectAll();
            document->select(source);
        }

        bool SetBrushFaceAttributesTool::canCopyAttributesFromSelection(const InputState& inputState) const {
            if (!applies(inputState))
                return false;

            auto document = kdl::mem_lock(m_document);

            const std::vector<Model::BrushFace*>& selectedFaces = document->selectedBrushFaces();
            if (selectedFaces.size() != 1)
                return false;

            const Model::Hit& hit = inputState.pickResult().query().pickable().type(Model::Brush::BrushHit).occluded().first();
            if (!hit.isMatch())
                return false;
            
            return true;
        }

        bool SetBrushFaceAttributesTool::applies(const InputState& inputState) const {
            return inputState.checkModifierKeys(MK_DontCare, MK_Yes, MK_DontCare);
        }

        bool SetBrushFaceAttributesTool::copyAllAttributes(const InputState& inputState) const {
            return !inputState.modifierKeysDown(ModifierKeys::MKCtrlCmd);
        }

        bool SetBrushFaceAttributesTool::doCancel() {
            return false;
        }

        /**
         * FIXME: Copied from SelectionTool, factor out
         */
        const Model::Hit& SetBrushFaceAttributesTool::firstHit(const InputState& inputState, const Model::HitType::Type type) const {
            return inputState.pickResult().query().pickable().type(type).occluded().first();
        }
    }
}
