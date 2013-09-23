/*
 Copyright (C) 2010-2012 Kristian Duske
 
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
        class CameraTool : public Tool<NoActivationPolicy, NoPickingPolicy, MousePolicy, MouseDragPolicy, NoRenderPolicy> {
        private:
            Renderer::Camera& m_camera;
            bool m_orbit;
            Vec3f m_orbitCenter;
        public:
            CameraTool(BaseTool* next, MapDocumentPtr document, ControllerFacade& controller, Renderer::Camera& camera);
        private:
            void doScroll(const InputState& inputState);
            bool doStartMouseDrag(const InputState& inputState);
            bool doMouseDrag(const InputState& inputState);
            void doEndMouseDrag(const InputState& inputState);
            void doCancelMouseDrag(const InputState& inputState);
            
            bool move(const InputState& inputState) const;
            bool look(const InputState& inputState) const;
            bool pan(const InputState& inputState) const;
            bool orbit(const InputState& inputState) const;
            
            float lookSpeedH() const;
            float lookSpeedV() const;
            float panSpeedH() const;
            float panSpeedV() const;
            float moveSpeed() const;
        };
    }
}

#endif /* defined(__TrenchBroom__CameraTool__) */