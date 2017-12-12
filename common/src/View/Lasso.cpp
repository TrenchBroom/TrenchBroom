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

#include "Lasso.h"

#include "Renderer/Camera.h"
#include "Renderer/RenderService.h"

namespace TrenchBroom {
    namespace View {
        Lasso::Lasso(const Renderer::Camera& camera, const FloatType distance, const Vec3& point) :
        m_camera(camera),
        m_distance(distance),
        m_transform(coordinateSystemMatrix(m_camera.right(), m_camera.up(), -m_camera.direction(),
                                           m_camera.defaultPoint(static_cast<float>(m_distance)))),
        m_start(point),
        m_cur(m_start) {}
        
        void Lasso::update(const Vec3& point) {
            m_cur = point;
        }

        bool Lasso::selects(const Vec3& point, const Plane3& plane, const BBox2& box) const {
            const Ray3 ray(m_camera.pickRay(point));
            const FloatType hitDistance = plane.intersectWithRay(ray);
            if (Math::isnan(hitDistance))
                return false;
            
            const Vec3 hitPoint = ray.pointAtDistance(hitDistance);
            const Vec3 projected = m_transform * hitPoint;
            return box.contains(projected);
        }
        
        bool Lasso::selects(const Edge3& edge, const Plane3& plane, const BBox2& box) const {
            const Vec3 prStart = project(edge.start(), plane);
            const Vec3 prEnd = project(edge.end(), plane);
            if (prStart.nan() || prEnd.nan())
                return false;
            
            return box.touches(prStart, prEnd);
        }
        
        bool Lasso::selects(const Polygon3& polygon, const Plane3& plane, const BBox2& box) const {
            if (polygon.vertexCount() == 0)
                return false;
            if (polygon.vertexCount() == 1)
                return selects(polygon.vertices().front());
            
            Vec3 prStart = project(polygon.vertices().back(), plane);
            for (const Vec3& cur : polygon) {
                Vec3 prEnd = project(cur, plane);
                if (!prStart.nan() && !prEnd.nan() && box.touches(prStart, prEnd)) {
                    return true;
                }
                
                prStart = prEnd;
            }
            
            return false;
        }
        
        Vec3 Lasso::project(const Vec3& point, const Plane3& plane) const {
            const Ray3 ray(m_camera.pickRay(point));
            const FloatType hitDistance = plane.intersectWithRay(ray);
            if (Math::isnan(hitDistance))
                return Vec3::NaN;
            
            const Vec3 hitPoint = ray.pointAtDistance(hitDistance);
            return m_transform * hitPoint;
        }

        void Lasso::render(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) const {
            const BBox2 box = this->box();
            const Mat4x4 inverted = invertedMatrix(m_transform);
            
            Vec3f::List polygon(4);
            polygon[0] = inverted * Vec3(box.min.x(), box.min.y(), 0.0);
            polygon[1] = inverted * Vec3(box.min.x(), box.max.y(), 0.0);
            polygon[2] = inverted * Vec3(box.max.x(), box.max.y(), 0.0);
            polygon[3] = inverted * Vec3(box.max.x(), box.min.y(), 0.0);
            
            Renderer::RenderService renderService(renderContext, renderBatch);
            renderService.setForegroundColor(Color(1.0f, 1.0f, 1.0f, 1.0f));
            renderService.setLineWidth(2.0f);
            renderService.renderPolygonOutline(polygon);

            renderService.setForegroundColor(Color(1.0f, 1.0f, 1.0f, 0.25f));
            renderService.renderFilledPolygon(polygon);
        }

        Plane3 Lasso::plane() const {
            return Plane3(m_camera.defaultPoint(static_cast<float>(m_distance)), m_camera.direction());
        }
        
        BBox2 Lasso::box() const {
            const Vec3 start = m_transform * m_start;
            const Vec3 cur   = m_transform * m_cur;
            
            const Vec3 min = ::min(start, cur);
            const Vec3 max = ::max(start, cur);
            return BBox2(min, max);
        }
    }
}
