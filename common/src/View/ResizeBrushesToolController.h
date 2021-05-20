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

#pragma once

#include "Renderer/EdgeRenderer.h"
#include "View/ToolController.h"

namespace TrenchBroom {
    namespace Renderer {
        class DirectEdgeRenderer;
        class RenderBatch;
        class RenderContext;
    }

    namespace View {
        class DragTracker;
        class ResizeBrushesTool;

        class ResizeBrushesToolController : public ToolControllerBase<PickingPolicy, KeyPolicy, MousePolicy, NoMouseDragPolicy, RenderPolicy, NoDropPolicy> {
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

            std::unique_ptr<DragTracker> acceptMouseDrag(const InputState& inputState) override;

            void doRender(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) override;

            bool doCancel() override;

            bool handleInput(const InputState& inputState) const;
        private:
            virtual bool doHandleInput(const InputState& inputState) const = 0;
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

