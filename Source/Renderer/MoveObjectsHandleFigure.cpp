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
#include "Renderer/CircleFigure.h"
#include "Renderer/PushMatrix.h"
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
            float distance;
            float missDistance = ray.squaredDistanceToSegment(m_position - m_axisLength * axis, m_position + m_axisLength * axis, distance);
            if (isnan(missDistance) || missDistance > 5.0f)
                return Hit::noHit();
            
            return Hit::hit(type, ray.pointAtDistance(distance), distance);
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
                float missDistance = (hitPoint - m_position).lengthSquared();
                if (missDistance < (m_planeSize - 5.0f) * (m_planeSize - 5.0f))
                    closestHit = Hit::hit(Hit::TXYPlane, hitPoint, distance);
                else if (missDistance <= (m_planeSize + 5.0f) * (m_planeSize + 5.0f))
                    closestHit = Hit::hit(Hit::TZRotation, hitPoint, distance);
            }
            
            plane = Plane(Vec3f::PosY, m_position);
            distance = plane.intersectWithRay(ray);
            if (!isnan(distance) && distance < closestHit.distance()) {
                Vec3f hitPoint = ray.pointAtDistance(distance);
                float missDistance = (hitPoint - m_position).lengthSquared();
                if (missDistance < (m_planeSize - 5.0f) * (m_planeSize - 5.0f))
                    closestHit = Hit::hit(Hit::TXZPlane, hitPoint, distance);
                else if (missDistance <= (m_planeSize + 5.0f) * (m_planeSize + 5.0f))
                    closestHit = Hit::hit(Hit::TYRotation, hitPoint, distance);
            }
            
            plane = Plane(Vec3f::PosX, m_position);
            distance = plane.intersectWithRay(ray);
            if (!isnan(distance) && distance < closestHit.distance()) {
                Vec3f hitPoint = ray.pointAtDistance(distance);
                float missDistance = (hitPoint - m_position).lengthSquared();
                if (missDistance < (m_planeSize - 5.0f) * (m_planeSize - 5.0f))
                    closestHit = Hit::hit(Hit::TYZPlane, hitPoint, distance);
                else if (missDistance <= (m_planeSize + 5.0f) * (m_planeSize + 5.0f))
                    closestHit = Hit::hit(Hit::TXRotation, hitPoint, distance);
            }

            return closestHit;
        }

        void MoveObjectsHandleFigure::render(Vbo& vbo, RenderContext& context) {
            SetVboState mapVbo(vbo, Vbo::VboMapped);
            
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_CULL_FACE);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            
            VertexArray axisArray(vbo, GL_LINES, 6,
                                  VertexAttribute(3, GL_FLOAT, VertexAttribute::Position),
                                  VertexAttribute(4, GL_FLOAT, VertexAttribute::Color));

            Color color;
            if (m_lastHit == Hit::TXAxis || m_lastHit == Hit::TXYPlane || m_lastHit == Hit::TXZPlane || m_lastHit == Hit::TXRotation)
                color = Color(1.0f, 1.0f, 1.0f, 1.0f);
            else
                color = Color(1.0f, 0.0f, 0.0f, 1.0f);
            axisArray.addAttribute(Vec3f(m_position.x - m_axisLength, m_position.y, m_position.z));
            axisArray.addAttribute(color);
            axisArray.addAttribute(Vec3f(m_position.x + m_axisLength, m_position.y, m_position.z));
            axisArray.addAttribute(color);
            
            if (m_lastHit == Hit::TYAxis || m_lastHit == Hit::TXYPlane || m_lastHit == Hit::TYZPlane || m_lastHit == Hit::TYRotation)
                color = Color(1.0f, 1.0f, 1.0f, 1.0f);
            else
                color = Color(0.0f, 1.0f, 0.0f, 1.0f);
            axisArray.addAttribute(Vec3f(m_position.x, m_position.y - m_axisLength, m_position.z));
            axisArray.addAttribute(color);
            axisArray.addAttribute(Vec3f(m_position.x, m_position.y + m_axisLength, m_position.z));
            axisArray.addAttribute(color);
            
            if (m_lastHit == Hit::TZAxis || m_lastHit == Hit::TXZPlane || m_lastHit == Hit::TYZPlane || m_lastHit == Hit::TZRotation)
                color = Color(1.0f, 1.0f, 1.0f, 1.0f);
            else
                color = Color(0.0f, 0.0f, 1.0f, 1.0f);
            axisArray.addAttribute(Vec3f(m_position.x, m_position.y, m_position.z - m_axisLength));
            axisArray.addAttribute(color);
            axisArray.addAttribute(Vec3f(m_position.x, m_position.y, m_position.z + m_axisLength));
            axisArray.addAttribute(color);

            Vec3f xAxis, yAxis, zAxis;
            axes(context.camera().position(), xAxis, yAxis, zAxis);

            if (m_lastHit == Hit::TXYPlane || m_lastHit == Hit::TXZPlane || m_lastHit == Hit::TYZPlane ||
                m_lastHit == Hit::TXRotation || m_lastHit == Hit::TYRotation || m_lastHit == Hit::TZRotation) {
                PushMatrix pushMatrix(context.transformation());
                Mat4f matrix = pushMatrix.matrix();
                matrix.translate(m_position);
                
                if (m_lastHit == Hit::TXZPlane || m_lastHit == Hit::TYRotation) {
                    color = m_lastHit == Hit::TXZPlane ? Color(0.0f, 1.0f, 0.0f, 1.0f) : Color(1.0f, 1.0f, 1.0f, 1.0f);
                    matrix.rotate(Math::Pi / 2.0f, Vec3f::PosX);
                } else if (m_lastHit == Hit::TYZPlane || m_lastHit == Hit::TXRotation) {
                    color = m_lastHit == Hit::TYZPlane ? Color(1.0f, 0.0f, 0.0f, 1.0f) : Color(1.0f, 1.0f, 1.0f, 1.0f);
                    matrix.rotate(Math::Pi / 2.0f, Vec3f::PosY);
                } else if (m_lastHit == Hit::TXYPlane || m_lastHit == Hit::TZRotation) {
                    color = m_lastHit == Hit::TXYPlane ? Color(0.0f, 0.0f, 1.0f, 1.0f) : Color(1.0f, 1.0f, 1.0f, 1.0f);
                }
                pushMatrix.load(matrix);

                CircleFigure filledCircle(m_planeSize, 24, Color(1.0f, 1.0f, 1.0f, 0.25f), true);
                filledCircle.render(vbo, context);
                
                CircleFigure outlinedCircle(m_planeSize, 24, color, false);
                outlinedCircle.render(vbo, context);
            }
            
            SetVboState activateVbo(vbo, Vbo::VboActive);
            axisArray.render();
            glEnable(GL_CULL_FACE);
            glEnable(GL_DEPTH_TEST);
        }
    }
}