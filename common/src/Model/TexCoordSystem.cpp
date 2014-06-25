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
        TexCoordSystem::~TexCoordSystem() {}
        
        TexCoordSystem* TexCoordSystem::clone() const {
            return doClone();
        }

        Vec3 TexCoordSystem::xAxis() const {
            return getXAxis();
        }
        
        Vec3 TexCoordSystem::yAxis() const {
            return getYAxis();
        }

        Vec2f TexCoordSystem::getTexCoords(const Vec3& point, const BrushFaceAttribs& attribs) const {
            return doGetTexCoords(point, attribs);
        }
        
        void TexCoordSystem::setRotation(const Vec3& normal, const float oldAngle, const float newAngle) {
            doSetRotation(normal, oldAngle, newAngle);
        }
        
        void TexCoordSystem::transform(const Vec3& oldNormal, const Mat4x4& transformation, BrushFaceAttribs& attribs, bool lockTexture) {
            doTransform(oldNormal, transformation, attribs, lockTexture);
        }

        void TexCoordSystem::moveTexture(const Vec3& normal, const Vec3& up, const Vec3& right, const Math::Direction direction, const float distance, BrushFaceAttribs& attribs) const {
            assert(direction != Math::Direction_Forward && direction != Math::Direction_Backward);
            
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
            
            Vec2f offset;
            switch (direction) {
                case Math::Direction_Up:
                    if (up.dot(vAxis) >= 0.0)
                        offset[yIndex] -= distance;
                    else
                        offset[yIndex] += distance;
                    break;
                case Math::Direction_Right:
                    if (right.dot(hAxis) >= 0.0)
                        offset[xIndex] -= distance;
                    else
                        offset[xIndex] += distance;
                    break;
                case Math::Direction_Down:
                    if (up.dot(vAxis) >= 0.0)
                        offset[yIndex] += distance;
                    else
                        offset[yIndex] -= distance;
                    break;
                case Math::Direction_Left:
                    if (right.dot(hAxis) >= 0.0f)
                        offset[xIndex] += distance;
                    else
                        offset[xIndex] -= distance;
                    break;
                case Math::Direction_Forward:
                case Math::Direction_Backward:
                    break;
            }
            
            attribs.setXOffset(attribs.xOffset() + offset.x());
            attribs.setYOffset(attribs.yOffset() + offset.y());
        }

        void TexCoordSystem::rotateTexture(const Vec3& normal, const float angle, BrushFaceAttribs& attribs) const {
            const float actualAngle = isRotationInverted(normal) ? - angle : angle;
            attribs.setRotation(attribs.rotation() + actualAngle);
        }

        Mat4x4 TexCoordSystem::toMatrix(const Vec2f& offset, const Vec2f& scale) const {
            const Vec3 xAxis(getXAxis() * scale.x());
            const Vec3 yAxis(getYAxis() * scale.y());
            const Vec3 zAxis(getZAxis());
            const Vec3 origin(xAxis * -offset.x() + yAxis * -offset.y());
            
            return coordinateSystemMatrix(xAxis, yAxis, zAxis, origin);
        }

        Mat4x4 TexCoordSystem::fromMatrix(const Vec2f& offset, const Vec2f& scale) const {
            return invertedMatrix(toMatrix(offset, scale));
        }
        
        float TexCoordSystem::measureAngle(const float currentAngle, const Vec2f& center, const Vec2f& point) const {
            return doMeasureAngle(currentAngle, center, point);
        }

        void TexCoordSystem::modOffset(Vec2f& offset, const Assets::Texture* texture) const {
            const float w = texture != NULL && texture->width() != 0.0f  ? static_cast<float>(texture->width()) : 1.0f;
            const float h = texture != NULL && texture->height() != 0.0f ? static_cast<float>(texture->height()) : 1.0f;
            offset[0] -= Math::round(offset[0] / w) * w;
            offset[1] -= Math::round(offset[1] / h) * h;
        }
    }
}
