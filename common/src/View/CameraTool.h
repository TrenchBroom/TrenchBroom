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

#ifndef __TrenchBroom__CameraTool__
#define __TrenchBroom__CameraTool__

#include "VecMath.h"
#include "Model/Picker.h"
#include "View/Tool.h"
#include "View/ViewTypes.h"

namespace TrenchBroom {
    namespace Renderer {
        class Camera;
    }
    
    namespace View {
        class CameraTool : public ToolImpl<NoActivationPolicy, NoPickingPolicy, MousePolicy, MouseDragPolicy, NoDropPolicy, NoRenderPolicy> {
        private:
            Renderer::Camera* m_camera;
            bool m_orbit;
            Vec3f m_orbitCenter;
        public:
            CameraTool(MapDocumentWPtr document);
            void setCamera(Renderer::Camera* camera);
            void fly(int dx, int dy, bool forward, bool backward, bool left, bool right, unsigned int time);
        private:
            void doScroll(const InputState& inputState);
            bool doStartMouseDrag(const InputState& inputState);
            bool doMouseDrag(const InputState& inputState);
            void doEndMouseDrag(const InputState& inputState);
            void doCancelMouseDrag();
            
            bool move(const InputState& inputState) const;
            bool move2D(const InputState& inputState) const;
            bool move3D(const InputState& inputState) const;
            bool zoom(const InputState& inputState) const;
            
            bool look(const InputState& inputState) const;
            bool look2D(const InputState& inputState) const;
            bool look3D(const InputState& inputState) const;
            
            bool pan(const InputState& inputState) const;
            bool pan2D(const InputState& inputState) const;
            bool pan3D(const InputState& inputState) const;
            
            bool orbit(const InputState& inputState) const;
            
            float lookSpeedH() const;
            float lookSpeedV() const;
            float panSpeedH() const;
            float panSpeedV() const;
            float moveSpeed(const bool altMode) const;
        };
    }
}

#endif /* defined(__TrenchBroom__CameraTool__) */
