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

#include "TexCoordSystem.h"

#include "Model/BrushFace.h"

namespace TrenchBroom {
    namespace Model {
        TexCoordSystemSnapshot::~TexCoordSystemSnapshot() {}

        void TexCoordSystemSnapshot::restore(TexCoordSystem* coordSystem) const {
            coordSystem->doRestoreSnapshot(*this);
        }

        TexCoordSystemSnapshot* TexCoordSystemSnapshot::clone() const {
            return doClone();
        }
        
        TexCoordSystem::TexCoordSystem() {}

        TexCoordSystem::~TexCoordSystem() {}
        
        TexCoordSystem* TexCoordSystem::clone() const {
            return doClone();
        }

        TexCoordSystemSnapshot* TexCoordSystem::takeSnapshot() {
            return doTakeSnapshot();
        }

        vec3 TexCoordSystem::xAxis() const {
            return getXAxis();
        }
        
        vec3 TexCoordSystem::yAxis() const {
            return getYAxis();
        }

        void TexCoordSystem::resetCache(const vec3& point0, const vec3& point1, const vec3& point2, const BrushFaceAttributes& attribs) {
            doResetCache(point0, point1, point2, attribs);
        }

        void TexCoordSystem::resetTextureAxes(const vec3& normal) {
            doResetTextureAxes(normal);
        }
        
        void TexCoordSystem::resetTextureAxesToParaxial(const vec3& normal, const float angle) {
            doResetTextureAxesToParaxial(normal, angle);
        }
        
        void TexCoordSystem::resetTextureAxesToParallel(const vec3& normal, const float angle) {
            doResetTextureAxesToParaxial(normal, angle);
        }
        
        vec2f TexCoordSystem::getTexCoords(const vec3& point, const BrushFaceAttributes& attribs) const {
            return doGetTexCoords(point, attribs);
        }
        
        void TexCoordSystem::setRotation(const vec3& normal, const float oldAngle, const float newAngle) {
            doSetRotation(normal, oldAngle, newAngle);
        }
        
        void TexCoordSystem::transform(const plane3& oldBoundary, const plane3& newBoundary, const mat4x4& transformation, BrushFaceAttributes& attribs, bool lockTexture, const vec3& invariant) {
            doTransform(oldBoundary, newBoundary, transformation, attribs, lockTexture, invariant);
        }

        void TexCoordSystem::updateNormal(const vec3& oldNormal, const vec3& newNormal, const BrushFaceAttributes& attribs, const WrapStyle style) {
            if (oldNormal != newNormal) {
                switch (style) {
                    case WrapStyle::Rotation:
                        doUpdateNormalWithRotation(oldNormal, newNormal, attribs);
                        break;
                    case WrapStyle::Projection:
                        doUpdateNormalWithProjection(oldNormal, newNormal, attribs);
                        break;
                }
            }
        }

        void TexCoordSystem::moveTexture(const vec3& normal, const vec3& up, const vec3& right, const vec2f& offset, BrushFaceAttributes& attribs) const {
            const auto toPlane = planeProjectionMatrix(0.0, normal);
            const auto [invertible, fromPlane] = invert(toPlane);
            const auto transform = fromPlane * mat4x4::zero_z * toPlane;
            const auto texX = normalize(transform * getXAxis());
            const auto texY = normalize(transform * getYAxis());
            assert(invertible); unused(invertible);

            vec3 vAxis, hAxis;
            size_t xIndex = 0;
            size_t yIndex = 0;
            
            // we prefer to use the texture axis which is closer to the XY plane for horizontal movement
            if (Math::lt(std::abs(texX.z()), std::abs(texY.z()))) {
                hAxis = texX;
                vAxis = texY;
                xIndex = 0;
                yIndex = 1;
            } else if (Math::lt(std::abs(texY.z()), std::abs(texX.z()))) {
                hAxis = texY;
                vAxis = texX;
                xIndex = 1;
                yIndex = 0;
            } else {
                // both texture axes have the same absolute angle towards the XY plane, prefer the one that is closer
                // to the right view axis for horizontal movement
                
                if (Math::gt(std::abs(dot(right, texX)), std::abs(dot(right, texY)))) {
                    // the right view axis is closer to the X texture axis
                    hAxis = texX;
                    vAxis = texY;
                    xIndex = 0;
                    yIndex = 1;
                } else if (Math::gt(std::abs(dot(right, texY)), std::abs(dot(right, texX)))) {
                    // the right view axis is closer to the Y texture axis
                    hAxis = texY;
                    vAxis = texX;
                    xIndex = 1;
                    yIndex = 0;
                } else {
                    // the right axis is as close to the X texture axis as to the Y texture axis
                    // test the up axis
                    if (Math::gt(std::abs(dot(up, texY)), std::abs(dot(up, texX)))) {
                        // the up view axis is closer to the Y texture axis
                        hAxis = texX;
                        vAxis = texY;
                        xIndex = 0;
                        yIndex = 1;
                    } else if (Math::gt(std::abs(dot(up, texX)), std::abs(dot(up, texY)))) {
                        // the up view axis is closer to the X texture axis
                        hAxis = texY;
                        vAxis = texX;
                        xIndex = 1;
                        yIndex = 0;
                    } else {
                        // this is just bad, better to do nothing
                        return;
                    }
                }
            }
            
            vec2f actualOffset;
            if (dot(right, hAxis) >= 0.0) {
                actualOffset[xIndex] = -offset.x();
            } else {
                actualOffset[xIndex] = +offset.x();
            }
            if (dot(up, vAxis) >= 0.0) {
                actualOffset[yIndex] = -offset.y();
            } else {
                actualOffset[yIndex] = +offset.y();
            }


            attribs.setOffset(attribs.offset() + actualOffset);
        }

        void TexCoordSystem::rotateTexture(const vec3& normal, const float angle, BrushFaceAttributes& attribs) const {
            const float actualAngle = isRotationInverted(normal) ? - angle : angle;
            attribs.setRotation(attribs.rotation() + actualAngle);
        }

        void TexCoordSystem::shearTexture(const vec3& normal, const vec2f& factors) {
            doShearTexture(normal, factors);
        }

        mat4x4 TexCoordSystem::toMatrix(const vec2f& o, const vec2f& s) const {
            const vec3 x = safeScaleAxis(getXAxis(), s.x());
            const vec3 y = safeScaleAxis(getYAxis(), s.y());
            const vec3 z = getZAxis();
            
            return mat4x4(x[0], x[1], x[2], o[0],
                          y[0], y[1], y[2], o[1],
                          z[0], z[1], z[2],  0.0,
                           0.0,  0.0,  0.0,  1.0);
/*
            const vec3 xAxis(getXAxis() * scale.x());
            const vec3 yAxis(getYAxis() * scale.y());
            const vec3 zAxis(getZAxis());
            const vec3 origin(xAxis * -offset.x() + yAxis * -offset.y());
            
            return coordinateSystemMatrix(xAxis, yAxis, zAxis, origin);
 */
        }

        mat4x4 TexCoordSystem::fromMatrix(const vec2f& offset, const vec2f& scale) const {
            const auto [invertible, result] = invert(toMatrix(offset, scale));
            assert(invertible); unused(invertible);
            return result;
        }
        
        float TexCoordSystem::measureAngle(const float currentAngle, const vec2f& center, const vec2f& point) const {
            return doMeasureAngle(currentAngle, center, point);
        }

        vec2f TexCoordSystem::computeTexCoords(const vec3& point, const vec2f& scale) const {
            return vec2f(dot(point, safeScaleAxis(getXAxis(), scale.x())),
                         dot(point, safeScaleAxis(getYAxis(), scale.y())));
        }
        
    }
}
