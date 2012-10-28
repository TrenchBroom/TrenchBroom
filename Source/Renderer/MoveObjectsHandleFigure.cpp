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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "MoveObjectsHandleFigure.h"

#include "Renderer/Camera.h"
#include "Renderer/RenderContext.h"
#include "Renderer/VertexArray.h"

namespace TrenchBroom {
    namespace Renderer {
        void MoveObjectsHandleFigure::axes(const Vec3f& origin, Vec3f& xAxis, Vec3f& yAxis, Vec3f& zAxis) {
            if (!m_locked) {
                Vec3f view = m_position - origin;
                view.normalize();
                
                if (eq(fabsf(view.z), 1.0f)) {
                    m_xAxis = Vec3f::PosX;
                    m_yAxis = Vec3f::PosY;
                } else {
                    m_xAxis = view.x > 0.0f ? Vec3f::NegX : Vec3f::PosX;
                    m_yAxis = view.y > 0.0f ? Vec3f::NegY : Vec3f::PosY;
                }
                
                if (view.z >= 0.0f)
                    m_zAxis = Vec3f::NegZ;
                else
                    m_zAxis = Vec3f::PosZ;
            }
            
            xAxis = m_xAxis;
            yAxis = m_yAxis;
            zAxis = m_zAxis;
        }

        MoveObjectsHandleFigure::Hit MoveObjectsHandleFigure::pickAxis(const Ray& ray, const Vec3f& axis, Hit::Type type) {
            float distance1, distance2, missDistance1, missDistance2;
            missDistance1 = ray.squaredDistanceToSegment(m_position + m_planeSize * axis, m_position + m_axisLength * axis, distance1);
            missDistance2 = ray.squaredDistanceToSegment(m_position - m_planeSize * axis, m_position - m_axisLength * axis, distance2);
            
            if (!isnan(missDistance1) && missDistance1 <= 9.0f) {
                if (!isnan(missDistance2) && missDistance2 <= 9.0f) {
                    if (distance1 <= distance2)
                        return Hit::hit(type, ray.pointAtDistance(distance1), distance1);
                    return Hit::hit(type, ray.pointAtDistance(distance2), distance2);
                }
                return Hit::hit(type, ray.pointAtDistance(distance1), distance1);
            } else if (!isnan(missDistance2) && missDistance2 <= 9.0f) {
                return Hit::hit(type, ray.pointAtDistance(distance2), distance2);
            }
            
            return Hit::noHit();
        }

        MoveObjectsHandleFigure::MoveObjectsHandleFigure(float axisLength, float planeSize) :
        m_axisLength(axisLength),
        m_planeSize(planeSize),
        m_lastHit(Hit::TNone),
        m_locked(false) {
            assert(m_axisLength > 0.0f);
            assert(m_planeSize > 0.0f);
        }

        MoveObjectsHandleFigure::Hit MoveObjectsHandleFigure::pick(const Ray& ray) {
            Vec3f xAxis, yAxis, zAxis;
            axes(ray.origin, xAxis, yAxis, zAxis);
            Hit closestHit = Hit::noHit();

            Hit hit = pickAxis(ray, xAxis, Hit::TXAxis);
            if (hit.distance() < closestHit.distance())
                closestHit = hit;
            
            hit = pickAxis(ray, yAxis, Hit::TYAxis);
            if (hit.distance() < closestHit.distance())
                closestHit = hit;

            hit = pickAxis(ray, zAxis, Hit::TZAxis);
            if (hit.distance() < closestHit.distance())
                closestHit = hit;

            Plane plane(Vec3f::PosZ, m_position);
            float distance = plane.intersectWithRay(ray);
            if (!isnan(distance) && distance < closestHit.distance()) {
                Vec3f hitPoint = ray.pointAtDistance(distance);
                if (between(hitPoint.x, m_position.x, (m_position + m_planeSize * xAxis).x) &&
                    between(hitPoint.y, m_position.y, (m_position + m_planeSize * yAxis).y))
                    closestHit = Hit::hit(Hit::TXYPlane, hitPoint, distance);
            }
            
            plane = Plane(Vec3f::PosY, m_position);
            distance = plane.intersectWithRay(ray);
            if (!isnan(distance) && distance < closestHit.distance()) {
                Vec3f hitPoint = ray.pointAtDistance(distance);
                if (between(hitPoint.x, m_position.x, (m_position + m_planeSize * xAxis).x) &&
                    between(hitPoint.z, m_position.z, (m_position + m_planeSize * zAxis).z))
                    closestHit = Hit::hit(Hit::TXZPlane, hitPoint, distance);
            }
            
            plane = Plane(Vec3f::PosX, m_position);
            distance = plane.intersectWithRay(ray);
            if (!isnan(distance) && distance < closestHit.distance()) {
                Vec3f hitPoint = ray.pointAtDistance(distance);
                if (between(hitPoint.y, m_position.y, (m_position + m_planeSize * yAxis).y) &&
                    between(hitPoint.z, m_position.z, (m_position + m_planeSize * zAxis).z))
                    closestHit = Hit::hit(Hit::TYZPlane, hitPoint, distance);
            }

            return closestHit;
        }

        void MoveObjectsHandleFigure::render(Vbo& vbo, RenderContext& context) {
            VertexArray axisArray(vbo, GL_LINES, 6,
                                  VertexAttribute(3, GL_FLOAT, VertexAttribute::Position),
                                  VertexAttribute(4, GL_FLOAT, VertexAttribute::Color));
            VertexArray quadArray(vbo, GL_QUADS, 12,
                                  VertexAttribute(3, GL_FLOAT, VertexAttribute::Position),
                                  VertexAttribute(4, GL_FLOAT, VertexAttribute::Color));
            VertexArray quadLinesArray(vbo, GL_LINES, 18,
                                       VertexAttribute(3, GL_FLOAT, VertexAttribute::Position),
                                       VertexAttribute(4, GL_FLOAT, VertexAttribute::Color));
            
            Color color;
            
            vbo.activate();
            vbo.map();
            
            color = m_lastHit == Hit::TXAxis ? Color(1.0f, 1.0f, 1.0f, 1.0f) : Color(1.0f, 0.0f, 0.0f, 1.0f);
            axisArray.addAttribute(Vec3f(m_position.x - m_axisLength, m_position.y, m_position.z));
            axisArray.addAttribute(color);
            axisArray.addAttribute(Vec3f(m_position.x + m_axisLength, m_position.y, m_position.z));
            axisArray.addAttribute(color);
            
            color = m_lastHit == Hit::TYAxis ? Color(1.0f, 1.0f, 1.0f, 1.0f) : Color(0.0f, 1.0f, 0.0f, 1.0f);
            axisArray.addAttribute(Vec3f(m_position.x, m_position.y - m_axisLength, m_position.z));
            axisArray.addAttribute(color);
            axisArray.addAttribute(Vec3f(m_position.x, m_position.y + m_axisLength, m_position.z));
            axisArray.addAttribute(color);
            
            color = m_lastHit == Hit::TZAxis ? Color(1.0f, 1.0f, 1.0f, 1.0f) : Color(0.0f, 0.0f, 1.0f, 1.0f);
            axisArray.addAttribute(Vec3f(m_position.x, m_position.y, m_position.z - m_axisLength));
            axisArray.addAttribute(color);
            axisArray.addAttribute(Vec3f(m_position.x, m_position.y, m_position.z + m_axisLength));
            axisArray.addAttribute(color);

            Vec3f xAxis, yAxis, zAxis;
            axes(context.camera().position(), xAxis, yAxis, zAxis);

            color = m_lastHit == Hit::TXYPlane ? Color(1.0f, 1.0f, 1.0f, 0.35f) : Color(1.0f, 1.0f, 0.0f, 0.35f);
            quadArray.addAttribute(m_position);
            quadArray.addAttribute(color);
            quadArray.addAttribute(m_position + m_planeSize * xAxis);
            quadArray.addAttribute(color);
            quadArray.addAttribute(m_position + m_planeSize * (xAxis + yAxis));
            quadArray.addAttribute(color);
            quadArray.addAttribute(m_position + m_planeSize * yAxis);
            quadArray.addAttribute(color);
            
            color = m_lastHit == Hit::TXZPlane ? Color(1.0f, 1.0f, 1.0f, 0.35f) : Color(1.0f, 0.0f, 1.0f, 0.35f);
            quadArray.addAttribute(m_position);
            quadArray.addAttribute(color);
            quadArray.addAttribute(m_position + m_planeSize * xAxis);
            quadArray.addAttribute(color);
            quadArray.addAttribute(m_position + m_planeSize * (xAxis + zAxis));
            quadArray.addAttribute(color);
            quadArray.addAttribute(m_position + m_planeSize * zAxis);
            quadArray.addAttribute(color);
            
            color = m_lastHit == Hit::TYZPlane ? Color(1.0f, 1.0f, 1.0f, 0.35f) : Color(0.0f, 1.0f, 1.0f, 0.35f);
            quadArray.addAttribute(m_position);
            quadArray.addAttribute(color);
            quadArray.addAttribute(m_position + m_planeSize * yAxis);
            quadArray.addAttribute(color);
            quadArray.addAttribute(m_position + m_planeSize * (yAxis + zAxis));
            quadArray.addAttribute(color);
            quadArray.addAttribute(m_position + m_planeSize * zAxis);
            quadArray.addAttribute(color);

            quadLinesArray.addAttribute(m_position);
            quadLinesArray.addAttribute(Color(1.0f, 1.0f, 1.0f, 0.7f));
            quadLinesArray.addAttribute(m_position + m_planeSize * xAxis);
            quadLinesArray.addAttribute(Color(1.0f, 1.0f, 1.0f, 0.7f));
            quadLinesArray.addAttribute(m_position);
            quadLinesArray.addAttribute(Color(1.0f, 1.0f, 1.0f, 0.7f));
            quadLinesArray.addAttribute(m_position + m_planeSize * yAxis);
            quadLinesArray.addAttribute(Color(1.0f, 1.0f, 1.0f, 0.7f));
            quadLinesArray.addAttribute(m_position);
            quadLinesArray.addAttribute(Color(1.0f, 1.0f, 1.0f, 0.7f));
            quadLinesArray.addAttribute(m_position + m_planeSize * zAxis);
            quadLinesArray.addAttribute(Color(1.0f, 1.0f, 1.0f, 0.7f));
            
            quadLinesArray.addAttribute(m_position + m_planeSize * xAxis);
            quadLinesArray.addAttribute(Color(1.0f, 1.0f, 1.0f, 0.7f));
            quadLinesArray.addAttribute(m_position + m_planeSize * (xAxis + yAxis));
            quadLinesArray.addAttribute(Color(1.0f, 1.0f, 1.0f, 0.7f));
            quadLinesArray.addAttribute(m_position + m_planeSize * (xAxis + yAxis));
            quadLinesArray.addAttribute(Color(1.0f, 1.0f, 1.0f, 0.7f));
            quadLinesArray.addAttribute(m_position + m_planeSize * yAxis);
            quadLinesArray.addAttribute(Color(1.0f, 1.0f, 1.0f, 0.7f));
            
            quadLinesArray.addAttribute(m_position + m_planeSize * xAxis);
            quadLinesArray.addAttribute(Color(1.0f, 1.0f, 1.0f, 0.7f));
            quadLinesArray.addAttribute(m_position + m_planeSize * (xAxis + zAxis));
            quadLinesArray.addAttribute(Color(1.0f, 1.0f, 1.0f, 0.7f));
            quadLinesArray.addAttribute(m_position + m_planeSize * (xAxis + zAxis));
            quadLinesArray.addAttribute(Color(1.0f, 1.0f, 1.0f, 0.7f));
            quadLinesArray.addAttribute(m_position + m_planeSize * zAxis);
            quadLinesArray.addAttribute(Color(1.0f, 1.0f, 1.0f, 0.7f));

            quadLinesArray.addAttribute(m_position + m_planeSize * yAxis);
            quadLinesArray.addAttribute(Color(1.0f, 1.0f, 1.0f, 0.7f));
            quadLinesArray.addAttribute(m_position + m_planeSize * (yAxis + zAxis));
            quadLinesArray.addAttribute(Color(1.0f, 1.0f, 1.0f, 0.7f));
            quadLinesArray.addAttribute(m_position + m_planeSize * (yAxis + zAxis));
            quadLinesArray.addAttribute(Color(1.0f, 1.0f, 1.0f, 0.7f));
            quadLinesArray.addAttribute(m_position + m_planeSize * zAxis);
            quadLinesArray.addAttribute(Color(1.0f, 1.0f, 1.0f, 0.7f));

            vbo.unmap();
            
            glDisable(GL_DEPTH_TEST);
            axisArray.render();
            glDisable(GL_CULL_FACE);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            quadArray.render();
            glEnable(GL_CULL_FACE);
            quadLinesArray.render();
            glEnable(GL_DEPTH_TEST);
            
            vbo.deactivate();
        }
    }
}