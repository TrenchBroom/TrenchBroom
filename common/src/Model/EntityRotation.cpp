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

#include "EntityRotation.h"

#include "Assets/EntityDefinition.h"
#include "Assets/EntityModel.h"
#include "Macros.h"
#include "Model/Entity.h"
#include "Model/EntityNode.h"

#include "kdl/reflection_impl.h"
#include "kdl/string_compare.h"
#include "kdl/string_utils.h"

#include "vm/forward.h"
#include "vm/mat.h"
#include "vm/mat_ext.h"
#include "vm/vec.h"
#include "vm/vec_io.h"

#include <ostream>

namespace TrenchBroom::Model
{

std::ostream& operator<<(std::ostream& lhs, const EntityRotationType& rhs)
{
  switch (rhs)
  {
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

std::ostream& operator<<(std::ostream& lhs, const EntityRotationUsage& rhs)
{
  switch (rhs)
  {
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

namespace
{
std::optional<std::tuple<std::string, EntityRotationType>> selectEntityRotationType(
  const Entity& entity,
  const std::vector<std::tuple<std::string, EntityRotationType>>&
    propertyToEntityRotationTypeMapping)
{
  for (const auto& [propertyKey, entityRotationType] :
       propertyToEntityRotationTypeMapping)
  {
    if (entity.hasProperty(propertyKey))
    {
      return {{propertyKey, entityRotationType}};
    }
  }

  for (const auto& [propertyKey, entityRotationType] :
       propertyToEntityRotationTypeMapping)
  {
    const auto* definition = entity.definition();
    if (definition != nullptr && definition->propertyDefinition(propertyKey) != nullptr)
    {
      return {{propertyKey, entityRotationType}};
    }
  }

  return std::nullopt;
}
} // namespace

EntityRotationInfo entityRotationInfo(const Entity& entity)
{
  auto type = EntityRotationType::None;
  std::string propertyKey;
  EntityRotationUsage usage = EntityRotationUsage::Allowed;

  const auto* model = entity.model();
  const auto* modelData = model ? model->data() : nullptr;
  const auto pitchType = modelData ? modelData->pitchType() : Assets::PitchType::Normal;
  const EntityRotationType eulerType =
    (pitchType == Assets::PitchType::MdlInverted
       ? EntityRotationType::Euler
       : EntityRotationType::Euler_PositivePitchDown);

  // determine the type of rotation to apply to this entity
  const auto classname = entity.classname();
  if (classname != EntityPropertyValues::NoClassname)
  {
    if (kdl::cs::str_is_prefix(classname, "light"))
    {
      if (entity.hasProperty(EntityPropertyKeys::Mangle))
      {
        // spotlight without a target, update mangle
        type = EntityRotationType::Mangle;
        propertyKey = EntityPropertyKeys::Mangle;
      }
      else if (!entity.hasProperty(EntityPropertyKeys::Target))
      {
        // not a spotlight, but might have a rotatable model, so change angle or angles
        if (entity.hasProperty(EntityPropertyKeys::Angles))
        {
          type = eulerType;
          propertyKey = EntityPropertyKeys::Angles;
        }
        else if (entity.hasProperty(EntityPropertyKeys::Angle))
        {
          type = EntityRotationType::Angle;
          propertyKey = EntityPropertyKeys::Angle;
        }
        else
        {
          // not a spotlight, don't modify
        }
      }
      else
      {
        // spotlight with target, don't modify
      }
    }
    else
    {
      // non-light

      if (!entity.pointEntity())
      {
        // brush entity
        std::tie(propertyKey, type) =
          selectEntityRotationType(
            entity,
            {{EntityPropertyKeys::Angles, eulerType},
             {EntityPropertyKeys::Mangle, eulerType},
             {EntityPropertyKeys::Angle, EntityRotationType::AngleUpDown}})
            .value_or(std::make_tuple(propertyKey, type));
      }
      else
      {
        // point entity

        // if the origin of the definition's bounding box is not in its center, don't
        // apply the rotation
        const auto offset = entity.definitionBounds().center();
        if (!vm::is_zero(offset.xy(), vm::C::almost_zero()))
        {
          // TODO: this only makes sense for Quake
          usage = EntityRotationUsage::BlockRotation;
        }

        std::tie(propertyKey, type) =
          selectEntityRotationType(
            entity,
            {{EntityPropertyKeys::Angles, eulerType},
             {EntityPropertyKeys::Mangle, eulerType},
             {EntityPropertyKeys::Angle, EntityRotationType::AngleUpDown}})
            .value_or(std::make_tuple(
              EntityPropertyKeys::Angle, EntityRotationType::AngleUpDown));
      }
    }
  }

  return EntityRotationInfo{type, propertyKey, usage};
}

vm::mat4x4 entityRotation(
  const std::vector<EntityProperty>& properties, const EntityRotationInfo& info)
{
  switch (info.type)
  {
  case EntityRotationType::Angle: {
    const auto it = findEntityProperty(properties, info.propertyKey);
    if (it == std::end(properties) || it->value().empty())
    {
      return vm::mat4x4::identity();
    }
    else
    {
      const auto angle = static_cast<FloatType>(std::atof(it->value().c_str()));
      return vm::rotation_matrix(vm::vec3::pos_z(), vm::to_radians(angle));
    }
  }
  case EntityRotationType::AngleUpDown: {
    const auto it = findEntityProperty(properties, info.propertyKey);
    if (it == std::end(properties) || it->value().empty())
    {
      return vm::mat4x4::identity();
    }
    const auto angle = static_cast<FloatType>(std::atof(it->value().c_str()));
    if (angle == -1.0)
    {
      return vm::mat4x4::rot_90_y_cw();
    }
    else if (angle == -2.0)
    {
      return vm::mat4x4::rot_90_y_ccw();
    }
    else
    {
      return vm::rotation_matrix(vm::vec3::pos_z(), vm::to_radians(angle));
    }
  }
  case EntityRotationType::Euler: {
    const auto it = findEntityProperty(properties, info.propertyKey);
    const auto angles =
      it != std::end(properties)
        ? vm::parse<FloatType, 3>(it->value()).value_or(vm::vec3::zero())
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
    const auto it = findEntityProperty(properties, info.propertyKey);
    const auto angles =
      it != std::end(properties)
        ? vm::parse<FloatType, 3>(it->value()).value_or(vm::vec3::zero())
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
    const auto it = findEntityProperty(properties, info.propertyKey);
    const auto angles =
      it != std::end(properties)
        ? vm::parse<FloatType, 3>(it->value()).value_or(vm::vec3::zero())
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

vm::mat4x4 entityRotation(const Entity& entity)
{
  return entityRotation(entity.properties(), entityRotationInfo(entity));
}

vm::vec3 entityYawPitchRoll(const vm::mat4x4& transformation, const vm::mat4x4& rotation)
{
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
    vm::to_degrees(rollPitchYaw[2]),
    vm::to_degrees(rollPitchYaw[1]),
    vm::to_degrees(rollPitchYaw[0]));
}

namespace
{
FloatType getEntityRotationAngle(vm::vec3 direction)
{
  direction[2] = 0.0;
  direction = normalize(direction);

  auto angle = vm::round(vm::to_degrees(std::acos(direction.x())));
  if (direction.y() < FloatType(0.0))
  {
    angle = 360.0 - angle;
  }
  angle = vm::normalize_degrees(angle);
  return angle;
}

EntityProperty setEntityRotationAngle(
  const std::string& propertyKey, const vm::vec3& direction)
{
  const auto angle = getEntityRotationAngle(direction);
  return {propertyKey, kdl::str_to_string(vm::round(angle))};
}
} // namespace

std::optional<EntityProperty> applyEntityRotation(
  const std::vector<EntityProperty>& properties,
  const EntityRotationInfo& info,
  const vm::mat4x4& transformation)
{
  if (info.usage == EntityRotationUsage::BlockRotation)
  {
    return std::nullopt;
  }

  const auto rotation = entityRotation(properties, info);
  switch (info.type)
  {
  case EntityRotationType::Angle: {
    const auto direction = normalize(transformation * rotation * vm::vec3::pos_x());
    return setEntityRotationAngle(info.propertyKey, direction);
  }
  case EntityRotationType::AngleUpDown: {
    const auto direction = normalize(transformation * rotation * vm::vec3::pos_x());
    if (direction.z() > 0.9)
    {
      return {{info.propertyKey, "-1"}};
    }
    if (direction.z() < -0.9)
    {
      return {{info.propertyKey, "-2"}};
    }
    return setEntityRotationAngle(info.propertyKey, direction);
  }
  case EntityRotationType::Euler: {
    const auto yawPitchRoll = entityYawPitchRoll(transformation, rotation);
    const auto nPitchYawRoll =
      vm::vec3(-yawPitchRoll.y(), yawPitchRoll.x(), yawPitchRoll.z());
    return {{info.propertyKey, kdl::str_to_string(vm::round(nPitchYawRoll))}};
  }
  case EntityRotationType::Euler_PositivePitchDown: {
    const auto yawPitchRoll = entityYawPitchRoll(transformation, rotation);
    const auto nPitchYawRoll =
      vm::vec3(yawPitchRoll.y(), yawPitchRoll.x(), yawPitchRoll.z());
    return {{info.propertyKey, kdl::str_to_string(vm::round(nPitchYawRoll))}};
  }
  case EntityRotationType::Mangle: {
    const auto yawPitchRoll = entityYawPitchRoll(transformation, rotation);
    const auto yawNPitchRoll =
      vm::vec3(yawPitchRoll.x(), -yawPitchRoll.y(), yawPitchRoll.z());
    return {{info.propertyKey, kdl::str_to_string(vm::round(yawNPitchRoll))}};
  }
  case EntityRotationType::None:
    return std::nullopt;
    switchDefault();
  }
}

void applyEntityRotation(Entity& entity, const vm::mat4x4& transformation)
{
  const auto info = entityRotationInfo(entity);

  if (
    const auto entityProperty =
      applyEntityRotation(entity.properties(), info, transformation))
  {
    entity.addOrUpdateProperty(entityProperty->key(), entityProperty->value());
  }
}

} // namespace TrenchBroom::Model
