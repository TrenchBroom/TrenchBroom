/*
Copyright (C) 2010-2017 Kristian Duske
Copyright (C) 2018 Eric Wasylishen

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

#include "View/ToolController.h"

#include <vecmath/forward.h>

#include <memory>

namespace TrenchBroom {
    namespace Renderer {
        class Camera;
        class RenderBatch;
        class RenderContext;
    }

    namespace View {
        class MapDocument;
        class ScaleObjectsTool;

        class ScaleObjectsToolController : public ToolControllerBase<PickingPolicy, KeyPolicy, MousePolicy, RestrictedDragPolicy, RenderPolicy, NoDropPolicy> {
        protected:
            ScaleObjectsTool* m_tool;
        private:
            std::weak_ptr<MapDocument> m_document;
        public:
            explicit ScaleObjectsToolController(ScaleObjectsTool* tool, std::weak_ptr<MapDocument> document);
            ~ScaleObjectsToolController() override;
        private:
            Tool* doGetTool() override;
            const Tool* doGetTool() const override;

            void doPick(const InputState& inputState, Model::PickResult& pickResult) override;

            void doModifierKeyChange(const InputState& inputState) override;

            void doMouseMove(const InputState& inputState) override;

            // RestrictedDragPolicy
            DragInfo doStartDrag(const InputState& inputState) override;
            DragResult doDrag(const InputState& inputState, const vm::vec3& lastHandlePosition, const vm::vec3& nextHandlePosition) override;
            void doEndDrag(const InputState& inputState) override;
            void doCancelDrag() override;

            void doSetRenderOptions(const InputState& inputState, Renderer::RenderContext& renderContext) const override;
            void doRender(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) override;

            bool doCancel() override;
        private:
            virtual void doPick(const vm::ray3 &pickRay, const Renderer::Camera &camera, Model::PickResult &pickResult) const = 0;
        };

        class ScaleObjectsToolController2D : public ScaleObjectsToolController {
        public:
            explicit ScaleObjectsToolController2D(ScaleObjectsTool* tool, std::weak_ptr<MapDocument> document);
        private:
            void doPick(const vm::ray3& pickRay, const Renderer::Camera& camera, Model::PickResult& pickResult) const override;
        };

        class ScaleObjectsToolController3D : public ScaleObjectsToolController {
        public:
            explicit ScaleObjectsToolController3D(ScaleObjectsTool* tool, std::weak_ptr<MapDocument> document);
        private:
            void doPick(const vm::ray3& pickRay, const Renderer::Camera& camera, Model::PickResult& pickResult) const override;
        };
    }
}


