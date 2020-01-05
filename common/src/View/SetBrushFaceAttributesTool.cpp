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

namespace TrenchBroom {
    namespace View {
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
            return performCopy(inputState, false);
        }

        bool SetBrushFaceAttributesTool::doMouseDoubleClick(const InputState& inputState) {
            return performCopy(inputState, true);
        }

        bool SetBrushFaceAttributesTool::performCopy(const InputState& inputState, const bool applyToBrush) {
            if (!applies(inputState))
                return false;

            auto document = kdl::mem_lock(m_document);

            const std::vector<Model::BrushFace*>& selectedFaces = document->selectedBrushFaces();
            if (selectedFaces.size() != 1)
                return false;

            const Model::Hit& hit = inputState.pickResult().query().pickable().type(Model::Brush::BrushHit).occluded().first();
            if (!hit.isMatch())
                return false;

            Model::BrushFace* source = selectedFaces.front();
            Model::BrushFace* targetFace = Model::hitToFace(hit);
            Model::Brush* targetBrush = targetFace->brush();
            const std::vector<Model::BrushFace*> targetList = applyToBrush ? targetBrush->faces() : std::vector<Model::BrushFace*>({ targetFace });

            const Model::WrapStyle wrapStyle = inputState.modifierKeysDown(ModifierKeys::MKShift) ? Model::WrapStyle::Rotation : Model::WrapStyle::Projection;

            const Transaction transaction(document);
            document->deselectAll();
            document->select(targetList);
            if (copyAttributes(inputState)) {
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
            return true;
        }

        bool SetBrushFaceAttributesTool::applies(const InputState& inputState) const {
            return inputState.checkModifierKeys(MK_DontCare, MK_Yes, MK_DontCare);
        }

        bool SetBrushFaceAttributesTool::copyAttributes(const InputState& inputState) const {
            return !inputState.modifierKeysDown(ModifierKeys::MKCtrlCmd);
        }

        bool SetBrushFaceAttributesTool::doCancel() {
            return false;
        }
    }
}
