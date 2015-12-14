/*
 Copyright (C) 2010-2014 Kristian Duske
 
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

#ifndef TrenchBroom_CameraTool3D
#define TrenchBroom_CameraTool3D

#include "VecMath.h"
#include "View/Tool.h"
#include "View/ToolAdapter.h"
#include "View/ViewTypes.h"

namespace TrenchBroom {
    namespace Renderer {
        class PerspectiveCamera;
    }
    
    namespace View {
        class CameraTool3D : public ToolAdapterBase<NoPickingPolicy, NoKeyPolicy, MousePolicy, MouseDragPolicy, NoRenderPolicy, NoDropPolicy>, public Tool {
        private:
            MapDocumentWPtr m_document;
            Renderer::PerspectiveCamera& m_camera;
            bool m_orbit;
            Vec3f m_orbitCenter;
        public:
            CameraTool3D(MapDocumentWPtr document, Renderer::PerspectiveCamera& camera);
            void fly(int dx, int dy, bool forward, bool backward, bool left, bool right, unsigned int time);
        private:
            Tool* doGetTool();
            
            void doMouseScroll(const InputState& inputState);
            bool doStartMouseDrag(const InputState& inputState);
            bool doMouseDrag(const InputState& inputState);
            void doEndMouseDrag(const InputState& inputState);
            void doCancelMouseDrag();
            
            bool move(const InputState& inputState) const;
            bool look(const InputState& inputState) const;
            bool pan(const InputState& inputState) const;
            bool orbit(const InputState& inputState) const;
            
            float lookSpeedH() const;
            float lookSpeedV() const;
            float panSpeedH() const;
            float panSpeedV() const;
            float moveSpeed(bool altMode) const;
            
            bool doCancel();
        };
    }
}

#endif /* defined(TrenchBroom_CameraTool3D) */
