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

#include "ParaxialTexCoordSystem.h"

#include "Model/BrushFace.h"

namespace TrenchBroom {
    namespace Model {
        const Vec3 ParaxialTexCoordSystem::BaseAxes[] = {
            Vec3( 0.0,  0.0,  1.0), Vec3( 1.0,  0.0,  0.0), Vec3( 0.0, -1.0,  0.0),
            Vec3( 0.0,  0.0, -1.0), Vec3( 1.0,  0.0,  0.0), Vec3( 0.0, -1.0,  0.0),
            Vec3( 1.0,  0.0,  0.0), Vec3( 0.0,  1.0,  0.0), Vec3( 0.0,  0.0, -1.0),
            Vec3(-1.0,  0.0,  0.0), Vec3( 0.0,  1.0,  0.0), Vec3( 0.0,  0.0, -1.0),
            Vec3( 0.0,  1.0,  0.0), Vec3( 1.0,  0.0,  0.0), Vec3( 0.0,  0.0, -1.0),
            Vec3( 0.0, -1.0,  0.0), Vec3( 1.0,  0.0,  0.0), Vec3( 0.0,  0.0, -1.0),
        };

        ParaxialTexCoordSystem::ParaxialTexCoordSystem(const Vec3& point0, const Vec3& point1, const Vec3& point2) {
            const Vec3 normal = crossed(point2 - point0, point1 - point0).normalized();
            update(normal, 0.0f);
        }

        const Vec3& ParaxialTexCoordSystem::xAxis() const {
            return m_xAxis;
        }
        
        const Vec3& ParaxialTexCoordSystem::yAxis() const {
            return m_yAxis;
        }
        
        Vec3 ParaxialTexCoordSystem::projectedXAxis(const Vec3& normal) const {
            return projectAxis(normal, xAxis());
        }
        
        Vec3 ParaxialTexCoordSystem::projectedYAxis(const Vec3& normal) const {
            return projectAxis(normal, yAxis());
        }

        void ParaxialTexCoordSystem::update(const Vec3& normal, const float rotation) {
            const size_t index = planeNormalIndex(normal);
            axes(index, m_xAxis, m_yAxis);
            rotateAxes(m_xAxis, m_yAxis, Math::radians(rotation), (index / 2) * 6);
        }

        size_t ParaxialTexCoordSystem::planeNormalIndex(const Vec3& normal) {
            size_t bestIndex = 0;
            FloatType bestDot = static_cast<FloatType>(0.0);
            for (size_t i = 0; i < 6; ++i) {
                const FloatType dot = normal.dot(BaseAxes[i * 3]);
                if (dot > bestDot) { // no need to use -altaxis for qbsp
                    bestDot = dot;
                    bestIndex = i;
                }
            }
            return bestIndex;
//            return (bestIndex / 2) * 6;
        }

        void ParaxialTexCoordSystem::axes(const size_t index, Vec3& xAxis, Vec3& yAxis) {
            xAxis = BaseAxes[index * 3 + 1];
            yAxis = BaseAxes[index * 3 + 2];
        }
        
        bool ParaxialTexCoordSystem::invertRotation(const Vec3& normal) {
            const size_t index = planeNormalIndex(normal);
            switch (index) {
                case 0:
                    return true;
                case 1:
                    return false;
                case 2:
                    return true;
                case 3:
                    return false;
                case 4: // Y axis rotation is the other way around (see next method, too)
                    return false;
                default:
                    return true;
            }
        }

        void ParaxialTexCoordSystem::rotateAxes(Vec3& xAxis, Vec3& yAxis, const FloatType angle, const size_t planeNormIndex) {
            // for some reason, when the texture plane normal is the Y axis, we must rotation clockwise
            const Quat3 rot(BaseAxes[planeNormIndex], planeNormIndex == 12 ? -angle : angle);
            xAxis = rot * xAxis;
            yAxis = rot * yAxis;
        }

        Vec3 ParaxialTexCoordSystem::projectAxis(const Vec3& normal, const Vec3& axis) const {
            const Plane3 plane(0.0, normal);
            const size_t index = planeNormalIndex(normal);
            switch (index) {
                case 0:
                case 1: // z != 0
                    return Vec3(axis.x(), axis.y(), plane.zAt(Vec2(axis.x(), axis.y())));
                case 2:
                case 3: // x != 0
                    return Vec3(plane.xAt(Vec2(axis.y(), axis.z())), axis.y(), axis.z());
                default: // y != 0
                    return Vec3(axis.x(), plane.yAt(Vec2(axis.x(), axis.z())), axis.z());
            }
        }
    }
}
