/*
 Copyright (C) 2010-2016 Kristian Duske
 
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

#include "PickRequest.h"

#include <cassert>

namespace TrenchBroom {
    namespace View {
        PickRequest::PickRequest() :
        m_camera(NULL) {}
        
        PickRequest::PickRequest(const Ray3& pickRay, const Renderer::Camera& camera) :
        m_pickRay(pickRay),
        m_camera(&camera) {}
        
        const Ray3& PickRequest::pickRay() const {
            return m_pickRay;
        }
        
        const Renderer::Camera& PickRequest::camera() const {
            assert(m_camera != NULL);
            return *m_camera;
        }
    }
}
