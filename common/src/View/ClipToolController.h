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

#ifndef TrenchBroom_ClipToolController
#define TrenchBroom_ClipToolController

#include "FloatType.h"
#include "View/ToolController.h"

#include <vecmath/forward.h>

#include <vector>

namespace TrenchBroom {
    namespace Model {
        class BrushFace;
        class BrushNode;
        class PickResult;
    }

    namespace Renderer {
        class RenderBatch;
        class RenderContext;
    }

    namespace View {
        class ClipTool;

        class ClipToolController : public ToolControllerGroup {
        protected:
            class Callback {
            protected:
                ClipTool* m_tool;
            public:
                explicit Callback(ClipTool* tool);
                virtual ~Callback();
                ClipTool* tool() const;

                bool addClipPoint(const InputState& inputState, vm::vec3& position);
                bool setClipFace(const InputState& inputState);

                virtual DragRestricter* createDragRestricter(const InputState& inputState, const vm::vec3& initialPoint) const = 0;
                virtual DragSnapper* createDragSnapper(const InputState& inputState) const = 0;
                virtual std::vector<vm::vec3> getHelpVectors(const InputState& inputState, const vm::vec3& clipPoint) const = 0;

                void renderFeedback(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch);
            private:
                virtual bool doGetNewClipPointPosition(const InputState& inputState, vm::vec3& position) const = 0;
            };

            class PartBase {
            protected:
                Callback* m_callback;
                explicit PartBase(Callback* callback);
            public:
                virtual ~PartBase();
            };

            class AddClipPointPart : public ToolControllerBase<NoPickingPolicy, NoKeyPolicy, MousePolicy, RestrictedDragPolicy, RenderPolicy, NoDropPolicy>, protected PartBase {
            private:
                bool m_secondPointSet;
            public:
                explicit AddClipPointPart(Callback* callback);
            private:
                Tool* doGetTool() override;
                const Tool* doGetTool() const override;

                bool doMouseClick(const InputState& inputState) override;
                bool doMouseDoubleClick(const InputState& inputState) override;
                DragInfo doStartDrag(const InputState& inputState) override;
                DragResult doDrag(const InputState& inputState, const vm::vec3& lastHandlePosition, const vm::vec3& nextHandlePosition) override;
                void doEndDrag(const InputState& inputState) override;
                void doCancelDrag() override;
                void doRender(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) override;
                bool doCancel() override;
            };

            class MoveClipPointPart : public ToolControllerBase<NoPickingPolicy, NoKeyPolicy, NoMousePolicy, RestrictedDragPolicy, NoRenderPolicy, NoDropPolicy>, protected PartBase {
            public:
                explicit MoveClipPointPart(Callback* callback);
            private:
                Tool* doGetTool() override;
                const Tool* doGetTool() const override;
                DragInfo doStartDrag(const InputState& inputState) override;
                DragResult doDrag(const InputState& inputState, const vm::vec3& lastHandlePosition, const vm::vec3& nextHandlePosition) override;
                void doEndDrag(const InputState& inputState) override;
                void doCancelDrag() override;
                bool doCancel() override;
            };
        protected:
            ClipTool* m_tool;
        protected:
            explicit ClipToolController(ClipTool* tool);
            virtual ~ClipToolController() override;
        private:
            Tool* doGetTool() override;
            const Tool* doGetTool() const override;

            void doPick(const InputState& inputState, Model::PickResult& pickResult) override;

            void doSetRenderOptions(const InputState& inputState, Renderer::RenderContext& renderContext) const override;
            void doRender(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) override;

            bool doCancel() override;
        };

        class ClipToolController2D : public ClipToolController {
        private:
            class Callback2D;
        public:
            explicit ClipToolController2D(ClipTool* tool);
        };

        class ClipToolController3D : public ClipToolController {
        private:
            static std::vector<vm::vec3> selectHelpVectors(const Model::BrushNode* brushNode, const Model::BrushFace* face, const vm::vec3& hitPoint);
            static std::vector<const Model::BrushFace*> selectIncidentFaces(const Model::BrushNode* brushNode, const Model::BrushFace* face, const vm::vec3& hitPoint);
        private:
            class Callback3D;
        public:
            explicit ClipToolController3D(ClipTool* tool);
        };
    }
}

#endif /* defined(TrenchBroom_ClipToolController) */
