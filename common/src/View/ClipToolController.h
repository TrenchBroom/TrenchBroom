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

#include "FloatType.h"
#include "View/ToolController.h"

#include <vecmath/forward.h>

#include <memory>
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
        public:
            explicit ClipToolController2D(ClipTool* tool);
        };

        class ClipToolController3D : public ClipToolController {
        public:
            explicit ClipToolController3D(ClipTool* tool);
        };
    }
}

