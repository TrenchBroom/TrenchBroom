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

#include "Assets/EntityDefinition.h"
#include "Assets/EntityModel.h"
#include "Macros.h"
#include "Model/Entity.h"
#include "Model/EntityNode.h"

#include <kdl/reflection_impl.h>
#include <kdl/string_compare.h>
#include <kdl/string_utils.h>

#include <vecmath/forward.h>
#include <vecmath/mat.h>
#include <vecmath/mat_ext.h>
#include <vecmath/vec.h>
#include <vecmath/vec_io.h>

#include <ostream>

namespace TrenchBroom {
namespace Model {

std::ostream& operator<<(std::ostream& lhs, const EntityRotationType& rhs) {
  switch (rhs) {
    case EntityRotationType::None:
      lhs << "None";
      break;
    case EntityRotationType::Angle:
      lhs << "Angle";
      break;
    case EntityRotationType::AngleUpDown:
      lhs << "AngleUpDown";
      break;
    case EntityRotationType::Euler:
      lhs << "Euler";
      break;
    case EntityRotationType::Euler_PositivePitchDown:
      lhs << "Euler_PositivePitchDown";
      break;
    case EntityRotationType::Mangle:
      lhs << "Mangle";
      break;
      switchDefault();
  }
  return lhs;
}

std::ostream& operator<<(std::ostream& lhs, const EntityRotationUsage& rhs) {
  switch (rhs) {
    case EntityRotationUsage::Allowed:
      lhs << "Allowed";
      break;
    case EntityRotationUsage::BlockRotation:
      lhs << "BlockRotation";
      break;
      switchDefault();
  }
  return lhs;
}

kdl_reflect_impl(EntityRotationInfo);

namespace {
bool hasPropertyOrDefinition(const Entity& entity, const std::string& propertyKey) {
  if (entity.hasProperty(propertyKey)) {
    return true;
  }

  const auto* definition = entity.definition();
  return definition != nullptr && definition->propertyDefinition(propertyKey) != nullptr;
}
} // namespace

EntityRotationInfo entityRotationInfo(const Entity& entity) {
  auto type = EntityRotationType::None;
  std::string propertyKey;
  EntityRotationUsage usage = EntityRotationUsage::Allowed;

  const auto* model = entity.model();
  const auto pitchType = model ? model->pitchType() : Assets::PitchType::Normal;
  const EntityRotationType eulerType =
    (pitchType == Assets::PitchType::MdlInverted ? EntityRotationType::Euler
                                                 : EntityRotationType::Euler_PositivePitchDown);

  // determine the type of rotation to apply to this entity
  const auto classname = entity.classname();
  if (classname != EntityPropertyValues::NoClassname) {
    if (kdl::cs::str_is_prefix(classname, "light")) {
      if (entity.hasProperty(EntityPropertyKeys::Mangle)) {
        // spotlight without a target, update mangle
        type = EntityRotationType::Mangle;
        propertyKey = EntityPropertyKeys::Mangle;
      } else if (!entity.hasProperty(EntityPropertyKeys::Target)) {
        // not a spotlight, but might have a rotatable model, so change angle or angles
        if (entity.hasProperty(EntityPropertyKeys::Angles)) {
          type = eulerType;
          propertyKey = EntityPropertyKeys::Angles;
        } else {
          type = EntityRotationType::Angle;
          propertyKey = EntityPropertyKeys::Angle;
        }
      } else {
        // spotlight with target, don't modify
      }
    } else {
      // non-light

      if (!entity.pointEntity()) {
        // brush entity
        if (hasPropertyOrDefinition(entity, EntityPropertyKeys::Angles)) {
          type = eulerType;
          propertyKey = EntityPropertyKeys::Angles;
        } else if (hasPropertyOrDefinition(entity, EntityPropertyKeys::Mangle)) {
          type = eulerType;
          propertyKey = EntityPropertyKeys::Mangle;
        } else if (hasPropertyOrDefinition(entity, EntityPropertyKeys::Angle)) {
          type = EntityRotationType::AngleUpDown;
          propertyKey = EntityPropertyKeys::Angle;
        }
      } else {
        // point entity

        // if the origin of the definition's bounding box is not in its center, don't apply the
        // rotation
        const auto offset = entity.definitionBounds().center();
        if (!vm::is_zero(offset.xy(), vm::C::almost_zero())) {
          // TODO: this only makes sense for Quake
          usage = EntityRotationUsage::BlockRotation;
        }

        if (hasPropertyOrDefinition(entity, EntityPropertyKeys::Angles)) {
          type = eulerType;
          propertyKey = EntityPropertyKeys::Angles;
        } else if (hasPropertyOrDefinition(entity, EntityPropertyKeys::Mangle)) {
          type = eulerType;
          propertyKey = EntityPropertyKeys::Mangle;
        } else {
          type = EntityRotationType::AngleUpDown;
          propertyKey = EntityPropertyKeys::Angle;
        }
      }
    }
  }

  return EntityRotationInfo{type, propertyKey, usage};
}

vm::mat4x4 EntityRotationPolicy::getRotation(const Entity& entity) {
  const EntityRotationInfo info = entityRotationInfo(entity);
  switch (info.type) {
    case EntityRotationType::Angle: {
      const auto* angleValue = entity.property(info.propertyKey);
      if (!angleValue || angleValue->empty()) {
        return vm::mat4x4::identity();
      } else {
        const auto angle = static_cast<FloatType>(std::atof(angleValue->c_str()));
        return vm::rotation_matrix(vm::vec3::pos_z(), vm::to_radians(angle));
      }
    }
    case EntityRotationType::AngleUpDown: {
      const auto* angleValue = entity.property(info.propertyKey);
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
    case EntityRotationType::Euler: {
      const auto* angleValue = entity.property(info.propertyKey);
      const auto angles = angleValue
                            ? vm::parse<FloatType, 3>(*angleValue).value_or(vm::vec3::zero())
                            : vm::vec3::zero();

      // x = -pitch
      // y =  yaw
      // z =  roll
      // pitch is applied with an inverted sign
      // see QuakeSpasm sources gl_rmain R_RotateForEntity function
      const auto roll = +vm::to_radians(angles.z());
      const auto pitch = -vm::to_radians(angles.x());
      const auto yaw = +vm::to_radians(angles.y());
      return vm::rotation_matrix(roll, pitch, yaw);
    }
    case EntityRotationType::Euler_PositivePitchDown: {
      const auto* angleValue = entity.property(info.propertyKey);
      const auto angles = angleValue
                            ? vm::parse<FloatType, 3>(*angleValue).value_or(vm::vec3::zero())
                            : vm::vec3::zero();

      // x = pitch
      // y = yaw
      // z = roll
      const auto roll = +vm::to_radians(angles.z());
      const auto pitch = +vm::to_radians(angles.x());
      const auto yaw = +vm::to_radians(angles.y());
      return vm::rotation_matrix(roll, pitch, yaw);
    }
    case EntityRotationType::Mangle: {
      const auto* angleValue = entity.property(info.propertyKey);
      const auto angles = angleValue
                            ? vm::parse<FloatType, 3>(*angleValue).value_or(vm::vec3::zero())
                            : vm::vec3::zero();

      // x = yaw
      // y = -pitch
      // z = roll
      const auto roll = +vm::to_radians(angles.z());
      const auto pitch = -vm::to_radians(angles.y());
      const auto yaw = +vm::to_radians(angles.x());
      return vm::rotation_matrix(roll, pitch, yaw);
    }
    case EntityRotationType::None:
      return vm::mat4x4::identity();
      switchDefault();
  }
}

namespace {
FloatType getEntityRotationAngle(vm::vec3 direction) {
  direction[2] = 0.0;
  direction = normalize(direction);

  auto angle = vm::round(vm::to_degrees(std::acos(direction.x())));
  if (direction.y() < FloatType(0.0)) {
    angle = 360.0 - angle;
  }
  angle = vm::normalize_degrees(angle);
  return angle;
}

void setEntityRotationAngle(
  Entity& entity, const EntityPropertyConfig& propertyConfig, const std::string& propertyKey,
  const vm::vec3& direction) {
  const auto angle = getEntityRotationAngle(direction);
  entity.addOrUpdateProperty(propertyConfig, propertyKey, kdl::str_to_string(vm::round(angle)));
}
} // namespace

void EntityRotationPolicy::applyRotation(
  Entity& entity, const EntityPropertyConfig& propertyConfig, const vm::mat4x4& transformation) {
  const auto info = entityRotationInfo(entity);

  if (info.usage == EntityRotationUsage::BlockRotation) {
    return;
  }

  const auto rotation = getRotation(entity);

  switch (info.type) {
    case EntityRotationType::Angle: {
      const auto direction = normalize(transformation * rotation * vm::vec3::pos_x());
      setEntityRotationAngle(entity, propertyConfig, info.propertyKey, direction);
      break;
    }
    case EntityRotationType::AngleUpDown: {
      const auto direction = normalize(transformation * rotation * vm::vec3::pos_x());
      if (direction.z() > 0.9) {
        entity.addOrUpdateProperty(propertyConfig, info.propertyKey, "1.0");
      } else if (direction.z() < -0.9) {
        entity.addOrUpdateProperty(propertyConfig, info.propertyKey, "-1.0");
      } else {
        setEntityRotationAngle(entity, propertyConfig, info.propertyKey, direction);
      }
      break;
    }
    case EntityRotationType::Euler: {
      const auto yawPitchRoll = getYawPitchRoll(transformation, rotation);
      const auto nPitchYawRoll = vm::vec3(-yawPitchRoll.y(), yawPitchRoll.x(), yawPitchRoll.z());
      entity.addOrUpdateProperty(
        propertyConfig, info.propertyKey, kdl::str_to_string(vm::round(nPitchYawRoll)));
      break;
    }
    case EntityRotationType::Euler_PositivePitchDown: {
      const auto yawPitchRoll = getYawPitchRoll(transformation, rotation);
      const auto nPitchYawRoll = vm::vec3(yawPitchRoll.y(), yawPitchRoll.x(), yawPitchRoll.z());
      entity.addOrUpdateProperty(
        propertyConfig, info.propertyKey, kdl::str_to_string(vm::round(nPitchYawRoll)));
      break;
    }
    case EntityRotationType::Mangle: {
      const auto yawPitchRoll = getYawPitchRoll(transformation, rotation);
      const auto yawNPitchRoll = vm::vec3(yawPitchRoll.x(), -yawPitchRoll.y(), yawPitchRoll.z());
      entity.addOrUpdateProperty(
        propertyConfig, info.propertyKey, kdl::str_to_string(vm::round(yawNPitchRoll)));
      break;
    }
    case EntityRotationType::None:
      break;
      switchDefault();
  }
}

std::string EntityRotationPolicy::getPropertyKey(const Entity& entity) {
  const auto info = entityRotationInfo(entity);
  return info.propertyKey;
}

vm::vec3 EntityRotationPolicy::getYawPitchRoll(
  const vm::mat4x4& transformation, const vm::mat4x4& rotation) {
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

  return vm::vec3(
    vm::to_degrees(rollPitchYaw[2]), vm::to_degrees(rollPitchYaw[1]),
    vm::to_degrees(rollPitchYaw[0]));
}
} // namespace Model
} // namespace TrenchBroom
