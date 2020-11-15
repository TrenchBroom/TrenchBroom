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
#include "Assets/EntityModel.h"
#include "Model/Entity.h"
#include "Model/EntityNode.h"

#include <kdl/string_compare.h>
#include <kdl/string_utils.h>

#include <vecmath/forward.h>
#include <vecmath/mat.h>
#include <vecmath/mat_ext.h>
#include <vecmath/vec.h>
#include <vecmath/vec_io.h>

namespace TrenchBroom {
    namespace Model {
        vm::mat4x4 EntityRotationPolicy::getRotation(const EntityNode* entityNode) {
            return getRotation(entityNode->entity());
        }

        void EntityRotationPolicy::applyRotation(EntityNode* entityNode, const vm::mat4x4& transformation) {
            auto entity = entityNode->entity();
            applyRotation(entity, transformation);
            entityNode->setEntity(std::move(entity));
        }

        std::string EntityRotationPolicy::getAttribute(const EntityNode* entityNode) {
            return getAttribute(entityNode->entity());
        }

        vm::mat4x4 EntityRotationPolicy::getRotation(const Entity& entity) {
            const RotationInfo info = rotationInfo(entity);
            switch (info.type) {
                case RotationType::Angle: {
                    const auto* angleValue = entity.attribute(info.attribute);
                    if (!angleValue || angleValue->empty()) {
                        return vm::mat4x4::identity();
                    } else {
                        const auto angle = static_cast<FloatType>(std::atof(angleValue->c_str()));
                        return vm::rotation_matrix(vm::vec3::pos_z(), vm::to_radians(angle));
                    }
                }
                case RotationType::AngleUpDown: {
                    const auto* angleValue = entity.attribute(info.attribute);
                    if (!angleValue || angleValue->empty()) {
                        return vm::mat4x4::identity();
                    }
                    const auto angle = static_cast<FloatType>(std::atof(angleValue->c_str()));
                    if (angle == -1.0) {
                        return vm::mat4x4::rot_90_y_cw();
                    } else if (angle == -2.0) {
                        return vm::mat4x4::rot_90_y_ccw();
                    } else {
                        return vm::rotation_matrix(vm::vec3::pos_z(), vm::to_radians(angle));
                    }
                }
                case RotationType::Euler: {
                    const auto* angleValue = entity.attribute(info.attribute);
                    const auto angles = angleValue ? vm::parse<FloatType, 3>(*angleValue, vm::vec3::zero()) : vm::vec3::zero();

                    // x = -pitch
                    // y =  yaw
                    // z =  roll
                    // pitch is applied with an inverted sign
                    // see QuakeSpasm sources gl_rmain R_RotateForEntity function
                    const auto roll  = +vm::to_radians(angles.z());
                    const auto pitch = -vm::to_radians(angles.x());
                    const auto yaw   = +vm::to_radians(angles.y());
                    return vm::rotation_matrix(roll, pitch, yaw);
                }
                case RotationType::Euler_PositivePitchDown: {
                    const auto* angleValue = entity.attribute(info.attribute);
                    const auto angles = angleValue ? vm::parse<FloatType, 3>(*angleValue, vm::vec3::zero()) : vm::vec3::zero();

                    // x = pitch
                    // y = yaw
                    // z = roll
                    const auto roll  = +vm::to_radians(angles.z());
                    const auto pitch = +vm::to_radians(angles.x());
                    const auto yaw   = +vm::to_radians(angles.y());
                    return vm::rotation_matrix(roll, pitch, yaw);
                }
                case RotationType::Mangle: {
                    const auto* angleValue = entity.attribute(info.attribute);
                    const auto angles = angleValue ? vm::parse<FloatType, 3>(*angleValue, vm::vec3::zero()) : vm::vec3::zero();

                    // x = yaw
                    // y = -pitch
                    // z = roll
                    const auto roll  = +vm::to_radians(angles.z());
                    const auto pitch = -vm::to_radians(angles.y());
                    const auto yaw   = +vm::to_radians(angles.x());
                    return vm::rotation_matrix(roll, pitch, yaw);
                }
                case RotationType::None:
                    return vm::mat4x4::identity();
                switchDefault()
            }
        }

        void EntityRotationPolicy::applyRotation(Entity& entity, const vm::mat4x4& transformation) {
            const auto info = rotationInfo(entity);

            if (info.usage == RotationUsage::BlockRotation) {
                return;
            }

            const auto rotation = getRotation(entity);

            switch (info.type) {
                case RotationType::Angle: {
                    const auto direction = normalize(transformation * rotation * vm::vec3::pos_x());
                    setAngle(entity, info.attribute, direction);
                    break;
                }
                case RotationType::AngleUpDown: {
                    const auto direction = normalize(transformation * rotation * vm::vec3::pos_x());
                    if (direction.z() > 0.9) {
                        entity.addOrUpdateAttribute(info.attribute, "1.0");
                    } else if (direction.z() < -0.9) {
                        entity.addOrUpdateAttribute(info.attribute, "-1.0");
                    } else {
                        setAngle(entity, info.attribute, direction);
                    }
                    break;
                }
                case RotationType::Euler: {
                    const auto yawPitchRoll = getYawPitchRoll(transformation, rotation);
                    const auto nPitchYawRoll = vm::vec3(-yawPitchRoll.y(), yawPitchRoll.x(), yawPitchRoll.z());
                    entity.addOrUpdateAttribute(info.attribute, kdl::str_to_string(vm::round(nPitchYawRoll)));
                    break;
                }
                case RotationType::Euler_PositivePitchDown: {
                    const auto yawPitchRoll = getYawPitchRoll(transformation, rotation);
                    const auto nPitchYawRoll = vm::vec3(yawPitchRoll.y(), yawPitchRoll.x(), yawPitchRoll.z());
                    entity.addOrUpdateAttribute(info.attribute, kdl::str_to_string(vm::round(nPitchYawRoll)));
                    break;
                }
                case RotationType::Mangle: {
                    const auto yawPitchRoll = getYawPitchRoll(transformation, rotation);
                    const auto yawNPitchRoll = vm::vec3(yawPitchRoll.x(), -yawPitchRoll.y(), yawPitchRoll.z());
                    entity.addOrUpdateAttribute(info.attribute, kdl::str_to_string(vm::round(yawNPitchRoll)));
                    break;
                }
                case RotationType::None:
                    break;
                switchDefault()
            }
        }

        std::string EntityRotationPolicy::getAttribute(const Entity& entity) {
            const auto info = rotationInfo(entity);
            return info.attribute;
        }

        EntityRotationPolicy::RotationInfo EntityRotationPolicy::rotationInfo(const EntityNode* entityNode) {
            return rotationInfo(entityNode->entity()); 
        }

        void EntityRotationPolicy::setAngle(EntityNode* entityNode, const std::string& attribute, const vm::vec3& direction) {
            auto entity = entityNode->entity();
            const auto angle = getAngle(direction);
            entity.addOrUpdateAttribute(attribute, kdl::str_to_string(vm::round(angle)));
            entityNode->setEntity(std::move(entity));
        }


        EntityRotationPolicy::RotationInfo EntityRotationPolicy::rotationInfo(const Entity& entity) {
            auto type = RotationType::None;
            std::string attribute;
            RotationUsage usage = RotationUsage::Allowed;

            const auto* model = entity.model();
            const auto pitchType = model ? model->pitchType() : Assets::PitchType::Normal;
            const RotationType eulerType =
                (pitchType == Assets::PitchType::MdlInverted ? RotationType::Euler : RotationType::Euler_PositivePitchDown);

            // determine the type of rotation to apply to this entity
            const auto classname = entity.classname();
            if (classname != AttributeValues::NoClassname) {
                if (kdl::cs::str_is_prefix(classname, "light")) {
                    if (entity.hasAttribute(AttributeNames::Mangle)) {
                        // spotlight without a target, update mangle
                        type = RotationType::Mangle;
                        attribute = AttributeNames::Mangle;
                    } else if (!entity.hasAttribute(AttributeNames::Target)) {
                        // not a spotlight, but might have a rotatable model, so change angle or angles
                        if (entity.hasAttribute(AttributeNames::Angles)) {
                            type = eulerType;
                            attribute = AttributeNames::Angles;
                        } else {
                            type = RotationType::Angle;
                            attribute = AttributeNames::Angle;
                        }
                    } else {
                        // spotlight with target, don't modify
                    }
                } else {
                    // non-light

                    if (!entity.pointEntity()) {
                        // brush entity
                        if (entity.hasAttribute(AttributeNames::Angles)) {
                            type = eulerType;
                            attribute = AttributeNames::Angles;
                        } else if (entity.hasAttribute(AttributeNames::Mangle)) {
                            type = eulerType;
                            attribute = AttributeNames::Mangle;
                        } else if (entity.hasAttribute(AttributeNames::Angle)) {
                            type = RotationType::AngleUpDown;
                            attribute = AttributeNames::Angle;
                        }
                    } else {
                        // point entity

                        // if the origin of the definition's bounding box is not in its center, don't apply the rotation
                        const auto offset = entity.definitionBounds().center();
                        if (!vm::is_zero(offset.xy(), vm::C::almost_zero())) {
                            // TODO: this only makes sense for Quake
                            usage = RotationUsage::BlockRotation;
                        }

                        if (entity.hasAttribute(AttributeNames::Angles)) {
                            type = eulerType;
                            attribute = AttributeNames::Angles;
                        } else if (entity.hasAttribute(AttributeNames::Mangle)) {
                            type = eulerType;
                            attribute = AttributeNames::Mangle;
                        } else {
                            type = RotationType::AngleUpDown;
                            attribute = AttributeNames::Angle;
                        }
                    }
                }
            }

            return RotationInfo{type, attribute, usage};
        }

        void EntityRotationPolicy::setAngle(Entity& entity, const std::string& attribute, const vm::vec3& direction) {
            const auto angle = getAngle(direction);
            entity.addOrUpdateAttribute(attribute, kdl::str_to_string(vm::round(angle)));
        }
        
        FloatType EntityRotationPolicy::getAngle(vm::vec3 direction) {
            direction[2] = 0.0;
            direction = normalize(direction);

            auto angle = vm::round(vm::to_degrees(std::acos(direction.x())));
            if (direction.y() < FloatType(0.0)) {
                angle = 360.0 - angle;
            }
            angle = vm::normalize_degrees(angle);
            return angle;
        }

        vm::vec3 EntityRotationPolicy::getYawPitchRoll(const vm::mat4x4& transformation, const vm::mat4x4& rotation) {
            const auto M = vm::strip_translation(transformation) * vm::strip_translation(rotation);

            const auto newPosX = vm::normalize(M * vm::vec3::pos_x());
            const auto newPosY = vm::normalize(vm::cross(M * vm::vec3::pos_z(), newPosX));
            const auto newPosZ = vm::normalize(vm::cross(newPosX, newPosY));

            // Build a new rotation matrix from the three transformed unit vectors
            vm::mat4x4d rotMat;
            rotMat[0] = vm::vec4d(newPosX, 0.0);
            rotMat[1] = vm::vec4d(newPosY, 0.0);
            rotMat[2] = vm::vec4d(newPosZ, 0.0);

            const auto rollPitchYaw = vm::rotation_matrix_to_euler_angles(rotMat);

            return vm::vec3(vm::to_degrees(rollPitchYaw[2]), vm::to_degrees(rollPitchYaw[1]), vm::to_degrees(rollPitchYaw[0]));
        }
    }
}
