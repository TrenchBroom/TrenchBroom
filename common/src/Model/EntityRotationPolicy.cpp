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

#include "EntityRotationPolicy.h"

#include "Macros.h"
#include "StringUtils.h"
#include "Model/Entity.h"

#include <vecmath/forward.h>
#include <vecmath/mat.h>

namespace TrenchBroom {
    namespace Model {
        EntityRotationPolicy::RotationInfo::RotationInfo(const RotationType i_type, const AttributeName& i_attribute) :
        type(i_type),
        attribute(i_attribute) {}

        EntityRotationPolicy::EntityRotationPolicy() {}

        vm::mat4x4 EntityRotationPolicy::getRotation(const Entity* entity) {
            const RotationInfo info = rotationInfo(entity);
            switch (info.type) {
                case RotationType_Angle: {
                    const auto angleValue = entity->attribute(info.attribute);
                    if (angleValue.empty()) {
                        return vm::mat4x4::identity;
                    } else {
                        const auto angle = static_cast<FloatType>(std::atof(angleValue.c_str()));
                        return vm::rotationMatrix(vm::vec3::pos_z, vm::radians(angle));
                    }
                }
                case RotationType_AngleUpDown: {
                    const auto angleValue = entity->attribute(info.attribute);
                    if (angleValue.empty()) {
                        return vm::mat4x4::identity;
                    }
                    const auto angle = static_cast<FloatType>(std::atof(angleValue.c_str()));
                    if (angle == -1.0) {
                        return vm::mat4x4::rot_90_y_cw;
                    } else if (angle == -2.0) {
                        return vm::mat4x4::rot_90_y_ccw;
                    } else {
                        return vm::rotationMatrix(vm::vec3::pos_z, vm::radians(angle));
                    }
                }
                case RotationType_Euler: {
                    const auto angleValue = entity->attribute(info.attribute);
                    const auto angles = angleValue.empty() ? vm::vec3::zero : vm::vec3::parse(angleValue);
                    
                    // x = -pitch
                    // y =  yaw
                    // z =  roll
                    // pitch is applied with an inverted sign
                    // see QuakeSpasm sources gl_rmain R_RotateForEntity function
                    const auto roll  = +vm::radians(angles.z());
                    const auto pitch = -vm::radians(angles.x());
                    const auto yaw   = +vm::radians(angles.y());
                    return vm::rotationMatrix(roll, pitch, yaw);
                }
                case RotationType_Euler_PositivePitchDown: {
                    const auto angleValue = entity->attribute(info.attribute);
                    const auto angles = angleValue.empty() ? vm::vec3::zero : vm::vec3::parse(angleValue);
                    
                    // x = pitch
                    // y = yaw
                    // z = roll
                    const auto roll  = +vm::radians(angles.z());
                    const auto pitch = +vm::radians(angles.x());
                    const auto yaw   = +vm::radians(angles.y());
                    return vm::rotationMatrix(roll, pitch, yaw);
                }
                case RotationType_Mangle: {
                    const auto angleValue = entity->attribute(info.attribute);
                    const auto angles = angleValue.empty() ? vm::vec3::zero : vm::vec3::parse(angleValue);
                    
                    // x = yaw
                    // y = -pitch
                    // z = roll
                    const auto roll  = +vm::radians(angles.z());
                    const auto pitch = -vm::radians(angles.y());
                    const auto yaw   = +vm::radians(angles.x());
                    return vm::rotationMatrix(roll, pitch, yaw);
                }
                case RotationType_None:
                    return vm::mat4x4::identity;
                switchDefault()
            }
        }

        void EntityRotationPolicy::applyRotation(Entity* entity, const vm::mat4x4& transformation) {
            const auto info = rotationInfo(entity);
            const auto rotation = getRotation(entity);
            
            switch (info.type) {
                case RotationType_Angle: {
                    const auto direction = normalize(transformation * rotation * vm::vec3::pos_x);
                    setAngle(entity, info.attribute, direction);
                    break;
                }
                case RotationType_AngleUpDown: {
                    const auto direction = normalize(transformation * rotation * vm::vec3::pos_x);
                    if (direction.z() > 0.9) {
                        entity->addOrUpdateAttribute(info.attribute, 1.0);
                    } else if (direction.z() < -0.9) {
                        entity->addOrUpdateAttribute(info.attribute, -1.0);
                    } else {
                        setAngle(entity, info.attribute, direction);
                    }
                    break;
                }
                case RotationType_Euler: {
                    const auto yawPitchRoll = getYawPitchRoll(transformation, rotation);
                    const auto nPitchYawRoll = vm::vec3(-yawPitchRoll.y(), yawPitchRoll.x(), yawPitchRoll.z());
                    entity->addOrUpdateAttribute(info.attribute, round(nPitchYawRoll));
                    break;
                }
                case RotationType_Euler_PositivePitchDown: {
                    const auto yawPitchRoll = getYawPitchRoll(transformation, rotation);
                    const auto nPitchYawRoll = vm::vec3(yawPitchRoll.y(), yawPitchRoll.x(), yawPitchRoll.z());
                    entity->addOrUpdateAttribute(info.attribute, round(nPitchYawRoll));
                    break;
                }
                case RotationType_Mangle: {
                    const auto yawPitchRoll = getYawPitchRoll(transformation, rotation);
                    const auto yawNPitchRoll = vm::vec3(yawPitchRoll.x(), -yawPitchRoll.y(), yawPitchRoll.z());
                    entity->addOrUpdateAttribute(info.attribute, round(yawNPitchRoll));
                    break;
                }
                case RotationType_None:
                    break;
                switchDefault()
            }
        }
        
        AttributeName EntityRotationPolicy::getAttribute(const Entity* entity) {
            const auto info = rotationInfo(entity);
            return info.attribute;
        }

        EntityRotationPolicy::RotationInfo EntityRotationPolicy::rotationInfo(const Entity* entity) {
            auto type = RotationType_None;
            AttributeName attribute;
            
            // determine the type of rotation to apply to this entity
            const auto classname = entity->classname();
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
                        const auto offset = entity->origin() - entity->bounds().center();
                        if (offset.x() == 0.0 && offset.y() == 0.0) {
                            if (entity->hasAttribute(AttributeNames::Angles)) {
                                type = RotationType_Euler;
                                attribute = AttributeNames::Angles;
                            } else if (entity->hasAttribute(AttributeNames::Mangle)) {
                                if (StringUtils::caseSensitiveEqual(classname, "info_intermission")) {
                                    type = RotationType_Euler_PositivePitchDown;
                                } else {
                                    type = RotationType_Mangle;
                                }
                                attribute = AttributeNames::Mangle;
                            } else {
                                type = RotationType_AngleUpDown;
                                attribute = AttributeNames::Angle;
                            }
                        }
                    }
                }
            }
            
            return RotationInfo(type, attribute);
        }

        void EntityRotationPolicy::setAngle(Entity* entity, const AttributeName& attribute, const vm::vec3& direction) {
            const auto angle = getAngle(direction);
            entity->addOrUpdateAttribute(attribute, vm::round(angle));
        }

        FloatType EntityRotationPolicy::getAngle(vm::vec3 direction) {
            direction[2] = 0.0;
            direction = normalize(direction);

            auto angle = vm::round(vm::degrees(std::acos(direction.x())));
            if (vm::isNegative(direction.y())) {
                angle = 360.0 - angle;
            }
            angle = vm::normalizeDegrees(angle);
            return angle;
        }

        vm::vec3 EntityRotationPolicy::getYawPitchRoll(const vm::mat4x4& transformation, const vm::mat4x4& rotation) {
            FloatType yaw = 0.0, pitch = 0.0, roll = 0.0;
            vm::vec3 newX, newY, newZ;
            
            newX = transformation * rotation * vm::vec3::pos_x;
            newY = transformation * rotation * vm::vec3::pos_y;
            
            if (std::abs(newX.z()) < std::abs(newY.z())) {
                newX = normalize(vm::vec3(newX.x(), newX.y(), 0.0));
                yaw = measureAngle(newX, vm::vec3::pos_x, vm::vec3::pos_z); // CCW yaw angle in radians
            } else {
                newY = normalize(vm::vec3(newY.x(), newY.y(), 0.0));
                yaw = measureAngle(newY, vm::vec3::pos_y, vm::vec3::pos_z);
            }
            
            // Now we know the yaw rotation angle. We have to correct for it to get the pitch angle.
            const auto invYaw = vm::rotationMatrix(vm::vec3::pos_z, -yaw);
            newX = invYaw * transformation * rotation * vm::vec3::pos_x;
            newZ = invYaw * transformation * rotation * vm::vec3::pos_z;
            
            if (std::abs(newX.y()) < std::abs(newZ.y())) {
                newX = normalize(vm::vec3(newX.x(), 0.0, newX.z()));
                pitch = measureAngle(newX, vm::vec3::pos_x, vm::vec3::pos_y);
            } else {
                newZ = normalize(vm::vec3(newZ.x(), 0.0, newZ.z()));
                pitch = measureAngle(newZ, vm::vec3::pos_z, vm::vec3::pos_y);
            }
            
            // Now we know the pitch rotation angle. We have to correct for it to get the roll angle.
            const auto invPitch = vm::rotationMatrix(vm::vec3::pos_y, -pitch);
            newY = invPitch * invYaw * transformation * rotation * vm::vec3::pos_y;
            newZ = invPitch * invYaw * transformation * rotation * vm::vec3::pos_z;
            
            if (std::abs(newY.x()) < std::abs(newY.x())) {
                newY = normalize(vm::vec3(0.0, newY.y(), newY.z()));
                roll = measureAngle(newY, vm::vec3::pos_y, vm::vec3::pos_x);
            } else {
                newZ = normalize(vm::vec3(0.0, newZ.y(), newZ.z()));
                roll = measureAngle(newZ, vm::vec3::pos_z, vm::vec3::pos_x);
            }
            
            return vm::vec3(vm::degrees(yaw), vm::degrees(pitch), vm::degrees(roll));
        }
    }
}
