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

#include "EntityRotationPolicy.h"

#include "Macros.h"
#include "VecMath.h"
#include "StringUtils.h"
#include "Model/Entity.h"

namespace TrenchBroom {
    namespace Model {
        EntityRotationPolicy::RotationInfo::RotationInfo(const RotationType i_type, const AttributeName& i_attribute) :
        type(i_type),
        attribute(i_attribute) {}

        EntityRotationPolicy::EntityRotationPolicy() {}

        Mat4x4 EntityRotationPolicy::getRotation(const Entity* entity) {
            const RotationInfo info = rotationInfo(entity);
            switch (info.type) {
                case RotationType_Angle: {
                    const AttributeValue angleValue = entity->attribute(info.attribute);
                    if (angleValue.empty())
                        return Mat4x4::Identity;
                    const FloatType angle = static_cast<FloatType>(std::atof(angleValue.c_str()));
                    return rotationMatrix(Vec3::PosZ, Math::radians(angle));
                }
                case RotationType_AngleUpDown: {
                    const AttributeValue angleValue = entity->attribute(info.attribute);
                    if (angleValue.empty())
                        return Mat4x4::Identity;
                    const FloatType angle = static_cast<FloatType>(std::atof(angleValue.c_str()));
                    if (angle == -1.0)
                        return Mat4x4::Rot90YCW;
                    if (angle == -2.0)
                        return Mat4x4::Rot90YCCW;
                    return rotationMatrix(Vec3::PosZ, Math::radians(angle));
                }
                case RotationType_Euler: {
                    const AttributeValue angleValue = entity->attribute(info.attribute);
                    const Vec3 angles = angleValue.empty() ? Vec3::Null : Vec3::parse(angleValue);
                    
                    // x = -pitch
                    // y =  yaw
                    // z =  roll
                    // pitch is applied with an inverted sign
                    // see QuakeSpasm sources gl_rmain R_RotateForEntity function
                    const FloatType roll  = +Math::radians(angles.z());
                    const FloatType pitch = -Math::radians(angles.x());
                    const FloatType yaw   = +Math::radians(angles.y());
                    return rotationMatrix(roll, pitch, yaw);
                }
                case RotationType_Mangle: {
                    const AttributeValue angleValue = entity->attribute(info.attribute);
                    const Vec3 angles = angleValue.empty() ? Vec3::Null : Vec3::parse(angleValue);
                    
                    // x = yaw
                    // y = pitch
                    // z = roll
                    const FloatType roll  = +Math::radians(angles.z());
                    const FloatType pitch = +Math::radians(angles.y());
                    const FloatType yaw   = +Math::radians(angles.x());
                    return rotationMatrix(roll, pitch, yaw);
                }
                case RotationType_None:
                    return Mat4x4::Identity;
                switchDefault()
            }
        }

        void EntityRotationPolicy::applyRotation(Entity* entity, const Mat4x4& transformation) {
            const RotationInfo info = rotationInfo(entity);
            const Mat4x4 rotation = getRotation(entity);
            
            switch (info.type) {
                case RotationType_Angle: {
                    const Vec3 direction = (transformation * rotation * Vec3::PosX).normalized();
                    setAngle(entity, info.attribute, direction);
                    break;
                }
                case RotationType_AngleUpDown: {
                    const Vec3 direction = (transformation * rotation * Vec3::PosX).normalized();
                    if (direction.z() > 0.9)
                        entity->addOrUpdateAttribute(info.attribute, 1.0);
                    else if (direction.z() < -0.9)
                        entity->addOrUpdateAttribute(info.attribute, -1.0);
                    else
                        setAngle(entity, info.attribute, direction);
                    break;
                }
                case RotationType_Euler: {
                    const Vec3 yawPitchRoll = getYawPitchRoll(transformation, rotation);
                    const Vec3 pitchYawRoll(-yawPitchRoll.y(), yawPitchRoll.x(), yawPitchRoll.z());
                    entity->addOrUpdateAttribute(info.attribute, pitchYawRoll.rounded());
                    break;
                }
                case RotationType_Mangle: {
                    const Vec3 yawPitchRoll = getYawPitchRoll(transformation, rotation);
                    entity->addOrUpdateAttribute(info.attribute, yawPitchRoll.rounded());
                    break;
                }
                case RotationType_None:
                    break;
                switchDefault()
            }
        }
        
        AttributeName EntityRotationPolicy::getAttribute(const Entity* entity) {
            const RotationInfo info = rotationInfo(entity);
            return info.attribute;
        }

        EntityRotationPolicy::RotationInfo EntityRotationPolicy::rotationInfo(const Entity* entity) {
            RotationType type = RotationType_None;
            AttributeName attribute;
            
            // determine the type of rotation to apply to this entity
            const AttributeValue classname = entity->classname();
            if (classname != AttributeValues::NoClassname) {
                if (StringUtils::isPrefix(classname, "light")) {
                    if (entity->hasAttribute(AttributeNames::Mangle)) {
                        // spotlight without a target, update mangle
                        type = RotationType_Mangle;
                        attribute = AttributeNames::Mangle;
                    } else if (!entity->hasAttribute(AttributeNames::Target)) {
                        // not a spotlight, but might have a rotatable model, so change angle or angles
                        if (entity->hasAttribute(AttributeNames::Angles)) {
                            type = RotationType_Euler;
                            attribute = AttributeNames::Angles;
                        } else {
                            type = RotationType_Angle;
                            attribute = AttributeNames::Angle;
                        }
                    } else {
                        // spotlight with target, don't modify
                    }
                } else {
                    if (!entity->pointEntity()) {
                        if (entity->hasAttribute(AttributeNames::Angles)) {
                            type = RotationType_Euler;
                            attribute = AttributeNames::Angles;
                        } else if (entity->hasAttribute(AttributeNames::Mangle)) {
                            type = RotationType_Mangle;
                            attribute = AttributeNames::Mangle;
                        } else if (entity->hasAttribute(AttributeNames::Angle)) {
                            type = RotationType_AngleUpDown;
                            attribute = AttributeNames::Angle;
                        }
                    } else {
                        // point entity
                        
                        // if the origin of the definition's bounding box is not in its center, don't apply the rotation
                        const Vec3 offset = entity->origin() - entity->bounds().center();
                        if (offset.x() == 0.0 && offset.y() == 0.0) {
                            if (entity->hasAttribute(AttributeNames::Angles)) {
                                type = RotationType_Euler;
                                attribute = AttributeNames::Angles;
                            } else if (entity->hasAttribute(AttributeNames::Mangle)) {
                                type = RotationType_Mangle;
                                attribute = AttributeNames::Mangle;
                            } else {
                                type = RotationType_Angle;
                                attribute = AttributeNames::Angle;
                            }
                        }
                    }
                }
            }
            
            return RotationInfo(type, attribute);
        }

        void EntityRotationPolicy::setAngle(Entity* entity, const AttributeName& attribute, const Vec3& direction) {
            const FloatType angle = getAngle(direction);
            entity->addOrUpdateAttribute(attribute, Math::round(angle));
        }

        FloatType EntityRotationPolicy::getAngle(Vec3 direction) {
            direction[2] = 0.0;
            direction.normalize();
            
            FloatType angle = Math::round(Math::degrees(std::acos(direction.x())));
            if (Math::neg(direction.y()))
                angle = 360.0 - angle;
            while (Math::neg(angle))
                angle += 360.0;
            return angle;
        }

        Vec3 EntityRotationPolicy::getYawPitchRoll(const Mat4x4& transformation, const Mat4x4& rotation) {
            FloatType yaw = 0.0, pitch = 0.0, roll = 0.0;
            Vec3 newX, newY, newZ;
            
            newX = transformation * rotation * Vec3::PosX;
            newY = transformation * rotation * Vec3::PosY;
            
            if (std::abs(newX.z()) < std::abs(newY.z())) {
                newX = Vec3(newX.x(), newX.y(), 0.0).normalized();
                yaw = angleBetween(newX, Vec3::PosX, Vec3::PosZ); // CCW yaw angle in radians
            } else {
                newY = Vec3(newY.x(), newY.y(), 0.0).normalized();
                yaw = angleBetween(newY, Vec3::PosY, Vec3::PosZ);
            }
            
            // Now we know the yaw rotation angle. We have to correct for it to get the pitch angle.
            const Mat4x4 invYaw = rotationMatrix(Vec3::PosZ, -yaw);
            newX = invYaw * transformation * rotation * Vec3::PosX;
            newZ = invYaw * transformation * rotation * Vec3::PosZ;
            
            if (std::abs(newX.y()) < std::abs(newZ.y())) {
                newX = Vec3(newX.x(), 0.0, newX.z()).normalized();
                pitch = angleBetween(newX, Vec3::PosX, Vec3::PosY);
            } else {
                newZ = Vec3(newZ.x(), 0.0, newZ.z()).normalized();
                pitch = angleBetween(newZ, Vec3::PosZ, Vec3::PosY);
            }
            
            // Now we know the pitch rotation angle. We have to correct for it to get the roll angle.
            const Mat4x4 invPitch = rotationMatrix(Vec3::PosY, -pitch);
            newY = invPitch * invYaw * transformation * rotation * Vec3::PosY;
            newZ = invPitch * invYaw * transformation * rotation * Vec3::PosZ;
            
            if (std::abs(newY.x()) < std::abs(newY.x())) {
                newY = Vec3(0.0, newY.y(), newY.z()).normalized();
                roll = angleBetween(newY, Vec3::PosY, Vec3::PosX);
            } else {
                newZ = Vec3(0.0, newZ.y(), newZ.z()).normalized();
                roll = angleBetween(newZ, Vec3::PosZ, Vec3::PosX);
            }
            
            return Vec3(Math::degrees(yaw),
                        Math::degrees(pitch),
                        Math::degrees(roll));
        }
    }
}
