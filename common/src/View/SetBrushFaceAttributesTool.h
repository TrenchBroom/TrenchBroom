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

#ifndef TrenchBroom_SetBrushFaceAttributesTool
#define TrenchBroom_SetBrushFaceAttributesTool

#include "View/Tool.h"
#include "View/ToolController.h"

#include <memory>

namespace TrenchBroom {
    namespace Model {
        class BrushFace;
    }
    namespace View {
        class MapDocument;

        /**
         * Functionality summary:
         *
         * Modifier combinations:
         * - Alt:       transfer texture and alignment from selected
         * - Alt+Shift: transfer texture and alignment (rotation method) from selected
         * - Alt+Ctrl:  transfer texture (but not alignment) from selected
         *
         * Actions:
         * - LMB Click: applies to clicked faces
         * - LMB Drag: applies to all faces dragged over
         * - LMB Double click: applies to all faces of target brush
         */
        class SetBrushFaceAttributesTool : public ToolControllerBase<NoPickingPolicy, NoKeyPolicy, MousePolicy, MouseDragPolicy, NoRenderPolicy, NoDropPolicy>, public Tool {
        private:
            std::weak_ptr<MapDocument> m_document;
        private: // drag state
            Model::BrushFace* m_dragInitialSelectedFace;
            Model::BrushFace* m_dragTargetFace;
            Model::BrushFace* m_dragSourceFace;
        public:
            SetBrushFaceAttributesTool(std::weak_ptr<MapDocument> document);
        private:
            Tool* doGetTool() override;
            const Tool* doGetTool() const override;

            bool doMouseClick(const InputState& inputState) override;
            bool doMouseDoubleClick(const InputState& inputState) override;
            
            void copyAttributesFromSelection(const InputState& inputState, bool applyToBrush);
            bool canCopyAttributesFromSelection(const InputState& inputState) const;
            bool applies(const InputState& inputState) const;
            bool copyTextureOnlyModifiersDown(const InputState& inputState) const;
            bool copyTextureAttribsProjectionModifiersDown(const InputState& inputState) const;
            bool copyTextureAttribsRotationModifiersDown(const InputState& inputState) const;

            bool doCancel() override;
            
            bool doStartMouseDrag(const InputState& inputState) override;
            bool doMouseDrag(const InputState& inputState) override;
            void doEndMouseDrag(const InputState& inputState) override;
            void doCancelMouseDrag() override;

            void resetDragState();
            void transferFaceAttributes(const InputState& inputState, Model::BrushFace* sourceFace,
                                        const std::vector<Model::BrushFace*>& targetFaces, Model::BrushFace* faceToSelectAfter);
        };
    }
}

#endif /* defined(TrenchBroom_SetBrushFaceAttributesTool) */
