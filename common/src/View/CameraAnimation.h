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

#ifndef TrenchBroom_CameraAnimation
#define TrenchBroom_CameraAnimation

#include "VecMath.h"
#include "View/Animation.h"

namespace TrenchBroom {
    namespace Renderer {
        class Camera;
    }
    
    namespace View {
        class CameraAnimation : public Animation {
        private:
            static const Type AnimationType;

            Renderer::Camera& m_camera;
            
            const vec3f m_startPosition;
            const vec3f m_startDirection;
            const vec3f m_startUp;
            const vec3f m_targetPosition;
            const vec3f m_targetDirection;
            const vec3f m_targetUp;
        public:
            CameraAnimation(Renderer::Camera& camera, const vec3f& targetPosition, const vec3f& targetDirection, const vec3f& targetUp, wxLongLong duration);
        private:
            void doUpdate(double progress) override;
        };
    }
}

#endif /* defined(TrenchBroom_CameraAnimation) */
