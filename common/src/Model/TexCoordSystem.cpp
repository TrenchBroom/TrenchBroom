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

#include "TexCoordSystem.h"

#include "Assets/Texture.h"
#include "Model/BrushFace.h"

namespace TrenchBroom {
    namespace Model {
        TexCoordSystemSnapshot::~TexCoordSystemSnapshot() {}

        void TexCoordSystemSnapshot::restore() {
            doRestore();
        }

        TexCoordSystem::~TexCoordSystem() {}
        
        TexCoordSystem* TexCoordSystem::clone() const {
            return doClone();
        }

        TexCoordSystemSnapshot* TexCoordSystem::takeSnapshot() {
            return doTakeSnapshot();
        }

        Vec3 TexCoordSystem::xAxis() const {
            return getXAxis();
        }
        
        Vec3 TexCoordSystem::yAxis() const {
            return getYAxis();
        }

        void TexCoordSystem::resetTextureAxes(const Vec3& normal) {
            doResetTextureAxes(normal);
        }
        
        void TexCoordSystem::resetTextureAxesToParaxial(const Vec3& normal, const float angle) {
            doResetTextureAxesToParaxial(normal, angle);
        }
        
        void TexCoordSystem::resetTextureAxesToParallel(const Vec3& normal, const float angle) {
            doResetTextureAxesToParaxial(normal, angle);
        }
        
        Vec2f TexCoordSystem::getTexCoords(const Vec3& point, const BrushFaceAttributes& attribs) const {
            return doGetTexCoords(point, attribs);
        }
        
        void TexCoordSystem::setRotation(const Vec3& normal, const float oldAngle, const float newAngle) {
            doSetRotation(normal, oldAngle, newAngle);
        }
        
        void TexCoordSystem::transform(const Plane3& oldBoundary, const Mat4x4& transformation, BrushFaceAttributes& attribs, bool lockTexture, const Vec3& invariant) {
            doTransform(oldBoundary, transformation, attribs, lockTexture, invariant);
        }

        void TexCoordSystem::updateNormal(const Vec3& oldNormal, const Vec3& newNormal, const BrushFaceAttributes& attribs) {
            if (oldNormal != newNormal)
                doUpdateNormal(oldNormal, newNormal, attribs);
        }

        void TexCoordSystem::moveTexture(const Vec3& normal, const Vec3& up, const Vec3& right, const Vec2f& offset, BrushFaceAttributes& attribs) const {
            
            /*
            const Vec3 direction  = crossed(up, right);
            const Mat4x4 toPlane  = Mat4x4::ZerZ * planeProjectionMatrix(0.0, direction);
            const Vec3 upPlane    = (toPlane * up).normalized();
            const Vec3 rightPlane = (toPlane * right).normalized();
            const Vec3 xPlane     = (toPlane * xAxis()).normalized();
            const Vec3 yPlane     = (toPlane * yAxis()).normalized();
            
            size_t hIndex, vIndex;
            float hFactor, vFactor;
            if (rightPlane.firstComponent() == xPlane.firstComponent()) {
                hIndex = 0;
                vIndex = 1;
                if (rightPlane.dot(xPlane) > 0.0f)
                    hFactor = -1.0f;
                else
                    hFactor = +1.0f;
                if (upPlane.dot(yPlane) > 0.0f)
                    vFactor = -1.0f;
                else
                    vFactor = +1.0f;
            } else {
                hIndex = 1;
                vIndex = 0;
                if (rightPlane.dot(yPlane) > 0.0f)
                    hFactor = +1.0f;
                else
                    hFactor = -1.0f;
                if (upPlane.dot(xPlane) > 0.0f)
                    vFactor = +1.0f;
                else
                    vFactor = -1.0f;
            }
            
            Vec2f actualOffset;
            actualOffset[hIndex] = hFactor * offset.x();
            actualOffset[vIndex] = vFactor * offset.y();
             */

            const Mat4x4 toPlane = planeProjectionMatrix(0.0, normal);
            const Mat4x4 fromPlane = invertedMatrix(toPlane);
            const Mat4x4 transform = fromPlane * Mat4x4::ZerZ * toPlane;
            const Vec3 texX = (transform * getXAxis()).normalized();
            const Vec3 texY = (transform * getYAxis()).normalized();
            
            Vec3 vAxis, hAxis;
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
                
                if (Math::gt(std::abs(right.dot(texX)), std::abs(right.dot(texY)))) {
                    // the right view axis is closer to the X texture axis
                    hAxis = texX;
                    vAxis = texY;
                    xIndex = 0;
                    yIndex = 1;
                } else if (Math::gt(std::abs(right.dot(texY)), std::abs(right.dot(texX)))) {
                    // the right view axis is closer to the Y texture axis
                    hAxis = texY;
                    vAxis = texX;
                    xIndex = 1;
                    yIndex = 0;
                } else {
                    // the right axis is as close to the X texture axis as to the Y texture axis
                    // test the up axis
                    if (Math::gt(std::abs(up.dot(texY)), std::abs(up.dot(texX)))) {
                        // the up view axis is closer to the Y texture axis
                        hAxis = texX;
                        vAxis = texY;
                        xIndex = 0;
                        yIndex = 1;
                    } else if (Math::gt(std::abs(up.dot(texX)), std::abs(up.dot(texY)))) {
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
            
            Vec2f actualOffset;
            if (right.dot(hAxis) >= 0.0)
                actualOffset[xIndex] = -offset.x();
            else
                actualOffset[xIndex] = +offset.x();
            if (up.dot(vAxis) >= 0.0)
                actualOffset[yIndex] = -offset.y();
            else
                actualOffset[yIndex] = +offset.y();
            
            
            attribs.setOffset(attribs.offset() + actualOffset);
        }

        void TexCoordSystem::rotateTexture(const Vec3& normal, const float angle, BrushFaceAttributes& attribs) const {
            const float actualAngle = isRotationInverted(normal) ? - angle : angle;
            attribs.setRotation(attribs.rotation() + actualAngle);
        }

        void TexCoordSystem::shearTexture(const Vec3& normal, const Vec2f& factors) {
            doShearTexture(normal, factors);
        }

        Mat4x4 TexCoordSystem::toMatrix(const Vec2f& o, const Vec2f& s) const {
            const Vec3 x = safeScaleAxis(getXAxis(), s.x());
            const Vec3 y = safeScaleAxis(getYAxis(), s.y());
            const Vec3 z = getZAxis();
            
            return Mat4x4(x[0], x[1], x[2], o[0],
                          y[0], y[1], y[2], o[1],
                          z[0], z[1], z[2],  0.0,
                           0.0,  0.0,  0.0,  1.0);
/*
            const Vec3 xAxis(getXAxis() * scale.x());
            const Vec3 yAxis(getYAxis() * scale.y());
            const Vec3 zAxis(getZAxis());
            const Vec3 origin(xAxis * -offset.x() + yAxis * -offset.y());
            
            return coordinateSystemMatrix(xAxis, yAxis, zAxis, origin);
 */
        }

        Mat4x4 TexCoordSystem::fromMatrix(const Vec2f& offset, const Vec2f& scale) const {
            return invertedMatrix(toMatrix(offset, scale));
        }
        
        float TexCoordSystem::measureAngle(const float currentAngle, const Vec2f& center, const Vec2f& point) const {
            return doMeasureAngle(currentAngle, center, point);
        }

        Vec2f TexCoordSystem::computeTexCoords(const Vec3& point, const Vec2f& scale) const {
            return Vec2f(point.dot(safeScaleAxis(getXAxis(), scale.x())),
                         point.dot(safeScaleAxis(getYAxis(), scale.y())));
        }
        
    }
}
