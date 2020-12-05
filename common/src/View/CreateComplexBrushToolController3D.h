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

#include "View/ToolController.h"

namespace TrenchBroom {
    namespace View {
        class CreateComplexBrushTool;

        class CreateComplexBrushToolController3D : public ToolControllerGroup {
        private:
            class Part;
            class DrawFacePart;
            class DuplicateFacePart;

            CreateComplexBrushTool* m_tool;
        public:
            explicit CreateComplexBrushToolController3D(CreateComplexBrushTool* tool);
        private:
            Tool* doGetTool() override;
            const Tool* doGetTool() const override;

            bool doMouseClick(const InputState& inputState) override;
            bool doMouseDoubleClick(const InputState& inputState) override;

            bool doShouldHandleMouseDrag(const InputState& inputState) const override;

            void doRender(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) override;

            bool doCancel() override;
        };
    }
}

#endif /* defined(TrenchBroom_CreateComplexBrushToolController3D) */
