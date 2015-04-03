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
                        return Mat4x4::Rot90XCCW;
                    if (angle == -2.0)
                        return Mat4x4::Rot90XCW;
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
                case RotationType_None:
                    return Mat4x4::Identity;
                DEFAULT_SWITCH()
            }
        }

        void EntityRotationPolicy::applyRotation(Entity* entity, const Mat4x4& transformation) {
            const RotationInfo info = rotationInfo(entity);
            const Mat4x4 rotation = getRotation(entity);

            Vec3 direction = rotation * Vec3::PosX;
            direction = transformation * direction;
            direction.normalize();
            
            switch (info.type) {
                case RotationType_Angle:
                    setAngle(entity, info.attribute, direction);
                    break;
                case RotationType_AngleUpDown:
                    if (direction.z() > 0.9)
                        entity->addOrUpdateAttribute(info.attribute, 1.0);
                    else if (direction.z() < -0.9)
                        entity->addOrUpdateAttribute(info.attribute, -1.0);
                    else
                        setAngle(entity, info.attribute, direction);
                    break;
                case RotationType_Euler: {
                    const Vec3 newAngles = eulerAngles(transformation * rotation).normalizeRadians();
                    const FloatType roll  = Math::degrees(newAngles.x());
                    const FloatType pitch = Math::degrees(newAngles.y());
                    const FloatType yaw   = Math::degrees(newAngles.z());
                    
                    entity->addOrUpdateAttribute(info.attribute, Vec3(-pitch, yaw, roll).round());
                    break;
                }
                case RotationType_None:
                    break;
                DEFAULT_SWITCH()
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
                        type = RotationType_Euler;
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
    }
}
