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

#include <memory>

namespace TrenchBroom {
    namespace Renderer {
        class PerspectiveCamera;
    }

    namespace View {
        class MapDocument;

        class CameraTool3D : public ToolControllerBase<NoPickingPolicy, NoKeyPolicy, MousePolicy, MouseDragPolicy, NoRenderPolicy, NoDropPolicy>, public Tool {
        private:
            std::weak_ptr<MapDocument> m_document;
            Renderer::PerspectiveCamera& m_camera;
            bool m_orbit;
            vm::vec3f m_orbitCenter;
        public:
            CameraTool3D(std::weak_ptr<MapDocument> document, Renderer::PerspectiveCamera& camera);
            void fly(int dx, int dy, bool forward, bool backward, bool left, bool right, unsigned int time);
        private:
            Tool* doGetTool() override;
            const Tool* doGetTool() const override;

            void doMouseScroll(const InputState& inputState) override;
            void doMouseUp(const InputState& inputState) override;
            bool doStartMouseDrag(const InputState& inputState) override;
            bool doMouseDrag(const InputState& inputState) override;
            void doEndMouseDrag(const InputState& inputState) override;
            void doCancelMouseDrag() override;

            bool move(const InputState& inputState) const;
            bool look(const InputState& inputState) const;
            bool pan(const InputState& inputState) const;
            bool orbit(const InputState& inputState) const;
            bool adjustFlySpeed(const InputState& inputState) const;

            float lookSpeedH() const;
            float lookSpeedV() const;
            float panSpeedH() const;
            float panSpeedV() const;
            float moveSpeed(bool altMode) const;

            bool doCancel() override;
        };
    }
}

#endif /* defined(TrenchBroom_CameraTool3D) */
