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

#include <vecmath/ray.h>

namespace TrenchBroom {
    namespace Renderer {
        class Camera;
    }

    namespace View {
        class PickRequest {
        private:
            vm::ray3 m_pickRay;
            const Renderer::Camera* m_camera;
        public:
            PickRequest();
            PickRequest(const vm::ray3& pickRay, const Renderer::Camera& camera);

            const vm::ray3& pickRay() const;
            const Renderer::Camera& camera() const;
        };
    }
}

#endif /* defined(TrenchBroom_PickRequest) */
