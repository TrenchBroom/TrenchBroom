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

#ifndef TrenchBroom_ResizeBrushesToolController
#define TrenchBroom_ResizeBrushesToolController

#include "Renderer/EdgeRenderer.h"
#include "View/ToolController.h"

namespace TrenchBroom {
    namespace Renderer {
        class DirectEdgeRenderer;
        class RenderBatch;
        class RenderContext;
    }

    namespace View {
        class ResizeBrushesTool;

        class ResizeBrushesToolController : public ToolControllerBase<PickingPolicy, KeyPolicy, MousePolicy, MouseDragPolicy, RenderPolicy, NoDropPolicy> {
        protected:
            ResizeBrushesTool* m_tool;
        private:
            enum class Mode {
                Resize,
                MoveFace
            };
            Mode m_mode;
        protected:
            explicit ResizeBrushesToolController(ResizeBrushesTool* tool);
        public:
            ~ResizeBrushesToolController() override;
        private:
            Tool* doGetTool() override;
            const Tool* doGetTool() const override;

            void doPick(const InputState& inputState, Model::PickResult& pickResult) override;

            void doModifierKeyChange(const InputState& inputState) override;

            void doMouseMove(const InputState& inputState) override;

            bool doStartMouseDrag(const InputState& inputState) override;
            bool doMouseDrag(const InputState& inputState) override;
            void doEndMouseDrag(const InputState& inputState) override;
            void doCancelMouseDrag() override;

            void doSetRenderOptions(const InputState& inputState, Renderer::RenderContext& renderContext) const override;
            void doRender(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) override;
            Renderer::DirectEdgeRenderer buildEdgeRenderer();

            bool doCancel() override;

            bool handleInput(const InputState& inputState) const;
            virtual bool doHandleInput(const InputState& inputState) const = 0;
        private:
            virtual Model::Hit doPick(const vm::ray3& pickRay, const Model::PickResult& pickResult) = 0;
        };

        class ResizeBrushesToolController2D : public ResizeBrushesToolController {
        public:
            explicit ResizeBrushesToolController2D(ResizeBrushesTool* tool);
        private:
            Model::Hit doPick(const vm::ray3& pickRay, const Model::PickResult& pickResult) override;
            bool doHandleInput(const InputState& inputState) const override;
        };

        class ResizeBrushesToolController3D : public ResizeBrushesToolController {
        public:
            explicit ResizeBrushesToolController3D(ResizeBrushesTool* tool);
        private:
            Model::Hit doPick(const vm::ray3& pickRay, const Model::PickResult& pickResult) override;
            bool doHandleInput(const InputState& inputState) const override;
        };
    }
}

#endif /* defined(TrenchBroom_ResizeBrushesToolController) */
