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

#include "View/Tool.h"
#include "View/ToolController.h"

#include <vecmath/forward.h>
#include <vecmath/vec.h>

namespace TrenchBroom {
    namespace Renderer {
        class OrthographicCamera;
    }

    namespace View {
        class CameraTool2D : public ToolControllerBase<NoPickingPolicy, NoKeyPolicy, MousePolicy, MouseDragPolicy, NoRenderPolicy, NoDropPolicy>, public Tool {
        private:
            Renderer::OrthographicCamera& m_camera;
            vm::vec2f m_lastMousePos;
        public:
            explicit CameraTool2D(Renderer::OrthographicCamera& camera);
        private:
            Tool* doGetTool() override;
            const Tool* doGetTool() const override;

            void doMouseScroll(const InputState& inputState) override;
            bool doStartMouseDrag(const InputState& inputState) override;
            bool doMouseDrag(const InputState& inputState) override;
            void doEndMouseDrag(const InputState& inputState) override;
            void doCancelMouseDrag() override;

            bool zoom(const InputState& inputState) const;
            bool look(const InputState& inputState) const;
            bool pan(const InputState& inputState) const;

            bool dragZoom(const InputState& inputState) const;

            void zoom(const InputState& inputState, const vm::vec2f& mousePos, float factor);

            bool doCancel() override;
        };
    }
}


