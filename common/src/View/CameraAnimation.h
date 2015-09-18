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
            
            const Vec3f m_startPosition;
            const Vec3f m_startDirection;
            const Vec3f m_startUp;
            const Vec3f m_targetPosition;
            const Vec3f m_targetDirection;
            const Vec3f m_targetUp;
        public:
            CameraAnimation(Renderer::Camera& camera, const Vec3f& targetPosition, const Vec3f& targetDirection, const Vec3f& targetUp, wxLongLong duration);
        private:
            void doUpdate(double progress);
        };
    }
}

#endif /* defined(TrenchBroom_CameraAnimation) */
