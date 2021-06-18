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

        class ScaleObjectsToolController : public ToolController {
        protected:
            ScaleObjectsTool& m_tool;
        private:
            std::weak_ptr<MapDocument> m_document;
        public:
            explicit ScaleObjectsToolController(ScaleObjectsTool& tool, std::weak_ptr<MapDocument> document);
            ~ScaleObjectsToolController() override;
        private:
            Tool& tool() override;
            const Tool& tool() const override;

            void pick(const InputState& inputState, Model::PickResult& pickResult) override;

            void modifierKeyChange(const InputState& inputState) override;

            void mouseMove(const InputState& inputState) override;

            std::unique_ptr<DragTracker> acceptMouseDrag(const InputState& inputState) override;

            void setRenderOptions(const InputState& inputState, Renderer::RenderContext& renderContext) const override;
            void render(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) override;

            bool cancel() override;
        private:
            virtual void doPick(const vm::ray3 &pickRay, const Renderer::Camera &camera, Model::PickResult &pickResult) const = 0;
        };

        class ScaleObjectsToolController2D : public ScaleObjectsToolController {
        public:
            explicit ScaleObjectsToolController2D(ScaleObjectsTool& tool, std::weak_ptr<MapDocument> document);
        private:
            void doPick(const vm::ray3& pickRay, const Renderer::Camera& camera, Model::PickResult& pickResult) const override;
        };

        class ScaleObjectsToolController3D : public ScaleObjectsToolController {
        public:
            explicit ScaleObjectsToolController3D(ScaleObjectsTool& tool, std::weak_ptr<MapDocument> document);
        private:
            void doPick(const vm::ray3& pickRay, const Renderer::Camera& camera, Model::PickResult& pickResult) const override;
        };
    }
}


