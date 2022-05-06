/*
 Copyright (C) 2020 Kristian Duske

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

#include "FloatType.h"
#include "Macros.h"

#include "Assets/EntityDefinition.h"
#include "Assets/EntityModel.h"
#include "Assets/PropertyDefinition.h"
#include "Model/Entity.h"
#include "Model/EntityProperties.h"
#include "Model/EntityRotation.h"

#include <vecmath/approx.h>
#include <vecmath/mat.h>
#include <vecmath/mat_ext.h>
#include <vecmath/mat_io.h>
#include <vecmath/scalar.h>
#include <vecmath/vec.h>
#include <vecmath/vec_io.h>

#include "Catch2.h"

namespace TrenchBroom {
namespace Model {
namespace {
struct EntityDefinitionInfo {
  Assets::EntityDefinitionType type;
  std::vector<std::shared_ptr<Assets::PropertyDefinition>> propertyDefinitions;
  vm::bbox3 bounds = vm::bbox3{16.0};
};

std::unique_ptr<Assets::EntityDefinition> createEntityDefinition(
  const std::optional<EntityDefinitionInfo>& info) {
  if (!info.has_value()) {
    return nullptr;
  }

  switch (info->type) {
    case Assets::EntityDefinitionType::PointEntity:
      return std::make_unique<Assets::PointEntityDefinition>(
        "", Color{}, info->bounds, "", info->propertyDefinitions, Assets::ModelDefinition{});
    case Assets::EntityDefinitionType::BrushEntity:
      return std::make_unique<Assets::BrushEntityDefinition>(
        "", Color{}, "", info->propertyDefinitions);
      switchDefault();
  }
}
} // namespace

TEST_CASE("entityRotationInfo") {
  using namespace Assets;

  auto manglePropertyDef = std::make_shared<StringPropertyDefinition>("mangle", "", "", false);
  auto normalPitch = EntityModelLoadedFrame{0, "", {}, PitchType::Normal, Orientation::Oriented};
  auto invertedPitch =
    EntityModelLoadedFrame{0, "", {}, PitchType::MdlInverted, Orientation::Oriented};

  using T = std::tuple<
    std::vector<EntityProperty>, bool, std::optional<EntityDefinitionInfo>, EntityModelFrame*,
    EntityRotationInfo>;

  // clang-format off
  const auto
  [entityProperties,              point, entityDefinitionInfo,            entityModel,    expectedRotationInfo] = GENERATE_REF(values<T>({
  {{},                            false, std::nullopt,                    nullptr,        {EntityRotationType::None, "", EntityRotationUsage::Allowed}},
  // a light with a mangle key
  {{{"classname", "light"},
    {"mangle",    "0 0 0"}},      true,  std::nullopt,                    nullptr,        {EntityRotationType::Mangle, "mangle", EntityRotationUsage::Allowed}},

  // a light without a target key and with an angles key, type is controlled by the model's pitch type (default is normal)
  {{{"classname", "light"},
    {"angles",    "0 0 0"}},      true,  std::nullopt,                    nullptr,        {EntityRotationType::Euler_PositivePitchDown, "angles", EntityRotationUsage::Allowed}},

  // a light without a target key and with an angle key
  {{{"classname", "light"},
    {"angle",    "0"}},           true,  std::nullopt,                    nullptr,        {EntityRotationType::Angle, "angle", EntityRotationUsage::Allowed}},

  // a light without a target key and with an angles key, type is controlled by the model's pitch type (normal)
  {{{"classname", "light"},
    {"angles",    "0 0 0"}},      true,  std::nullopt,                    &normalPitch,   {EntityRotationType::Euler_PositivePitchDown, "angles", EntityRotationUsage::Allowed}},

  // a light without a target key and with an angles key, type is controlled by the model's pitch type (inverted)
  {{{"classname", "light"},
    {"angles",    "0 0 0"}},      true,  std::nullopt,                    &invertedPitch, {EntityRotationType::Euler, "angles", EntityRotationUsage::Allowed}},

  // a light without a target key and without an angles key
  {{{"classname", "light"}},      true,  std::nullopt,                    nullptr,        {EntityRotationType::None, "", EntityRotationUsage::Allowed}},

  // a light with a target key
  {{{"classname", "light"},
    {"target", "xyz"}},           true,  std::nullopt,                    nullptr,        {EntityRotationType::None, "", EntityRotationUsage::Allowed}},

  // non-light brush entity without additional keys
  {{{"classname", "other"}},      false, std::nullopt,                    nullptr,        {EntityRotationType::None, "", EntityRotationUsage::Allowed}},

  // non-light brush entity with angles key
  {{{"classname", "other"},
    {"angles", "0 0 0"}},         false, std::nullopt,                    nullptr,        {EntityRotationType::Euler_PositivePitchDown, "angles", EntityRotationUsage::Allowed}},

  // non-light brush entity with mangle key
  {{{"classname", "other"},
    {"mangle", "0 0 0"}},         false, std::nullopt,                    &invertedPitch, {EntityRotationType::Euler, "mangle", EntityRotationUsage::Allowed}},

  // non-light brush entity with mangle key (model controls the euler type)
  {{{"classname", "other"},
    {"mangle", "0 0 0"}},         false, std::nullopt,                    nullptr,        {EntityRotationType::Euler_PositivePitchDown, "mangle", EntityRotationUsage::Allowed}},

  // non-light brush entity with angle key
  {{{"classname", "other"},
    {"angle", "0"}},              false, std::nullopt,                    nullptr,        {EntityRotationType::AngleUpDown, "angle", EntityRotationUsage::Allowed}},

  // non-light point entity without additional keys
  {{{"classname", "other"}},      true,  std::nullopt,                    nullptr,        {EntityRotationType::AngleUpDown, "angle", EntityRotationUsage::Allowed}},

  // non-light point entity with angles key
  {{{"classname", "other"},
    {"angles", "0 0 0"}},         true,  std::nullopt,                    nullptr,        {EntityRotationType::Euler_PositivePitchDown, "angles", EntityRotationUsage::Allowed}},

  // non-light point entity with angles key (model controls the euler type)
  {{{"classname", "other"},
    {"angles", "0 0 0"}},         true,  std::nullopt,                    &invertedPitch, {EntityRotationType::Euler, "angles", EntityRotationUsage::Allowed}},

  // non-light point entity with mangle key
  {{{"classname", "other"},
    {"mangle", "0 0 0"}},         true,  std::nullopt,                    nullptr,        {EntityRotationType::Euler_PositivePitchDown, "mangle", EntityRotationUsage::Allowed}},

  // non-light point entity with mangle key and off-center definition bounds
  {{{"classname", "other"},
    {"mangle", "0 0 0"}},         true,  {{EntityDefinitionType::PointEntity,
                                           {},
                                           {{0, 0, -16}, {16, 16, 16}}}}, nullptr,        {EntityRotationType::Euler_PositivePitchDown, "mangle", EntityRotationUsage::BlockRotation}},

  // a property definition counts as a property even if the property isn't present
  {{{"classname", "other"}},      true,  {{EntityDefinitionType::PointEntity,
                                           {manglePropertyDef}}},         nullptr,        {EntityRotationType::Euler_PositivePitchDown, "mangle", EntityRotationUsage::Allowed}},

  // but not for light entities
  {{{"classname", "light"}},      true,  {{EntityDefinitionType::PointEntity,
                                           {manglePropertyDef}}},         nullptr,        {EntityRotationType::None, "", EntityRotationUsage::Allowed}},
  }));
  // clang-format on

  CAPTURE(entityProperties, point, entityDefinitionInfo, entityModel);

  auto entityDefinition = createEntityDefinition(entityDefinitionInfo);
  auto entity = Entity{{}, entityProperties};
  entity.setDefinition({}, entityDefinition.get());
  entity.setModel({}, entityModel);
  entity.setPointEntity({}, point);

  CHECK(entityRotationInfo(entity) == expectedRotationInfo);
}

TEST_CASE("entityRotation") {
  using T = std::tuple<std::vector<EntityProperty>, EntityRotationInfo, vm::mat4x4>;
  using ERT = EntityRotationType;
  const auto usage = GENERATE(EntityRotationUsage::Allowed, EntityRotationUsage::BlockRotation);

  // clang-format off
  const auto
  [properties,              info,                          expectedTransformation] = GENERATE_COPY(values<T>({
  {{},                      {ERT::Angle,                   "angle", usage}, vm::mat4x4::identity()},
  {{{"angle", "90"}},       {ERT::Angle,                   "angle", usage}, vm::mat4x4::rot_90_z_ccw()},
  {{},                      {ERT::AngleUpDown,             "angle", usage}, vm::mat4x4::identity()},
  {{{"angle", "90"}},       {ERT::AngleUpDown,             "angle", usage}, vm::mat4x4::rot_90_z_ccw()},
  {{{"angle", "-1"}},       {ERT::AngleUpDown,             "angle", usage}, vm::mat4x4::rot_90_y_cw()},
  {{{"angle", "-2"}},       {ERT::AngleUpDown,             "angle", usage}, vm::mat4x4::rot_90_y_ccw()},
  {{},                      {ERT::Euler,                   "angle", usage}, vm::mat4x4::identity()},
  {{{"angle", "30 60 90"}}, {ERT::Euler,                   "angle", usage}, vm::rotation_matrix(vm::to_radians(90.0), vm::to_radians(-30.0), vm::to_radians(60.0))},
  {{},                      {ERT::Euler_PositivePitchDown, "angle", usage}, vm::mat4x4::identity()},
  {{{"angle", "30 60 90"}}, {ERT::Euler_PositivePitchDown, "angle", usage}, vm::rotation_matrix(vm::to_radians(90.0), vm::to_radians(30.0), vm::to_radians(60.0))},
  {{},                      {ERT::Mangle,                  "angle", usage}, vm::mat4x4::identity()},
  {{{"angle", "30 60 90"}}, {ERT::Mangle,                  "angle", usage}, vm::rotation_matrix(vm::to_radians(90.0), vm::to_radians(-60.0), vm::to_radians(30.0))},
  {{},                      {ERT::None,                    "angle", usage}, vm::mat4x4::identity()},
  {{{"angle", "30 60 90"}}, {ERT::None,                    "angle", usage}, vm::mat4x4::identity()},
  }));
  // clang-format on

  CAPTURE(properties, info);

  CHECK(entityRotation(properties, info) == vm::approx{expectedTransformation});
}

TEST_CASE("entityYawPitchRoll") {
  using T = std::tuple<double, double, double, vm::mat4x4d, vm::vec3d>;

  // clang-format off
  const auto
  [roll, pitch, yaw, transformation, expectedYawPitchRoll] = GENERATE(values<T>({
  {12.0, 13.0, 14.0, vm::mat4x4d::identity(),                 {14, 13, 12}},
  {12.0, 13.0, 14.0, vm::scaling_matrix(vm::vec3d{ 2, 2, 2}), {14, 13, 12}},
  {0.0,  45.0,  0.0, vm::scaling_matrix(vm::vec3d{ 2, 1, 1}), {0, vm::to_degrees(std::atan(0.5)), 0}},
  {10.0, 45.0,  0.0, vm::scaling_matrix(vm::vec3d{-1, 1, 1}), {180, 45, -10}},
  }));
  // clang-format on

  CAPTURE(roll, pitch, yaw, transformation);

  const auto rotation =
    vm::rotation_matrix(vm::to_radians(roll), vm::to_radians(pitch), vm::to_radians(yaw));

  CHECK(entityYawPitchRoll(transformation, rotation) == vm::approx{expectedYawPitchRoll});
}

TEST_CASE("applyEntityRotation") {
  using T = std::tuple<
    std::vector<EntityProperty>, EntityRotationInfo, vm::mat4x4, std::optional<EntityProperty>>;
  using ERT = EntityRotationType;
  using ERU = EntityRotationUsage;

  // clang-format off
  const auto
  [properties,        info,                                      transform, expectedProperty] = GENERATE_COPY(values<T>({
  {{{"angle", "45"}},         {ERT::Angle,                   "angle", ERU::Allowed},       vm::mat4x4::rot_90_z_ccw(),                           {{"angle", "135"}}},
  {{{"angle", "45"}},         {ERT::Angle,                   "angle", ERU::BlockRotation}, vm::mat4x4::rot_90_z_ccw(),                           {}},

  {{{"angle", "45"}},         {ERT::AngleUpDown,             "angle", ERU::Allowed},       vm::mat4x4::rot_90_z_ccw(),                           {{"angle", "135"}}},
  {{{"angle",  "0"}},         {ERT::AngleUpDown,             "angle", ERU::Allowed},       vm::rotation_matrix(0.0, vm::to_radians(-90.0), 0.0), {{"angle", "-1"}}},
  {{{"angle",  "0"}},         {ERT::AngleUpDown,             "angle", ERU::Allowed},       vm::rotation_matrix(0.0, vm::to_radians(90.0), 0.0),  {{"angle", "-2"}}},
  
  {{{"angle",  "30 60 90"}},  {ERT::Euler,                   "angle", ERU::Allowed},       vm::rotation_matrix(vm::to_radians(-90.0), vm::to_radians(-60.0), vm::to_radians(-30.0)), {{"angle", "0 0 0"}}},
  {{{"angle",  "30 60 90"}},  {ERT::Euler,                   "angle", ERU::BlockRotation}, vm::rotation_matrix(vm::to_radians(-90.0), vm::to_radians(-60.0), vm::to_radians(-30.0)), {}},

  {{{"angle",  "-30 60 90"}}, {ERT::Euler_PositivePitchDown, "angle", ERU::Allowed},       vm::rotation_matrix(vm::to_radians(-90.0), vm::to_radians(-60.0), vm::to_radians(-30.0)), {{"angle", "0 0 0"}}},
  {{{"angle",  "-30 60 90"}}, {ERT::Euler_PositivePitchDown, "angle", ERU::BlockRotation}, vm::rotation_matrix(vm::to_radians(-90.0), vm::to_radians(-60.0), vm::to_radians(-30.0)), {}},

  {{{"angle",  "60 30 90"}}, {ERT::Mangle,                  "angle", ERU::Allowed},       vm::rotation_matrix(vm::to_radians(-90.0), vm::to_radians(-60.0), vm::to_radians(-30.0)), {{"angle", "0 0 0"}}},
  {{{"angle",  "60 30 90"}}, {ERT::Mangle,                  "angle", ERU::BlockRotation}, vm::rotation_matrix(vm::to_radians(-90.0), vm::to_radians(-60.0), vm::to_radians(-30.0)), {}},
  }));
  // clang-format on

  CAPTURE(properties, info, transform);

  CHECK(applyEntityRotation(properties, info, transform) == expectedProperty);
}

} // namespace Model
} // namespace TrenchBroom
