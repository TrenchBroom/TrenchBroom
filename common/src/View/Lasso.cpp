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

#include "Lasso.h"

#include "Renderer/Camera.h"
#include "Renderer/RenderService.h"

namespace TrenchBroom {
    namespace View {
        Lasso::Lasso(const Renderer::Camera& camera, const FloatType distance, const Vec3& point) :
        m_transform(coordinateSystemMatrix(camera.right(), camera.up(), -camera.direction(), camera.defaultPoint(static_cast<float>(distance)))),
        m_start(point),
        m_cur(m_start) {}
        
        void Lasso::setPoint(const Vec3& point) {
            m_cur = point;
        }
        
        void Lasso::render(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) const {
            const Mat4x4 inverted = invertedMatrix(m_transform);
            
            const Vec3 start = m_transform * m_start;
            const Vec3 cur   = m_transform * m_cur;
            
            const Vec3 min = ::min(start, cur);
            const Vec3 max = ::max(start, cur);
            
            Vec3f::List polygon(4);
            polygon[0] = inverted * Vec3(min.x(), min.y(), min.z());
            polygon[1] = inverted * Vec3(min.x(), max.y(), min.z());
            polygon[2] = inverted * Vec3(max.x(), max.y(), min.z());
            polygon[3] = inverted * Vec3(max.x(), min.y(), min.z());
            
            Renderer::RenderService renderService(renderContext, renderBatch);
            renderService.setForegroundColor(Color(1.0f, 1.0f, 1.0f, 1.0f));
            renderService.setLineWidth(2.0f);
            renderService.renderPolygonOutline(polygon);

            renderService.setForegroundColor(Color(1.0f, 1.0f, 1.0f, 0.25f));
            renderService.renderFilledPolygon(polygon);
        }
    }
}
