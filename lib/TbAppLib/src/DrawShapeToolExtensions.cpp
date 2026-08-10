/*
 Copyright (C) 2023 Kristian Duske

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

#include "ui/DrawShapeToolExtensions.h"

#include "mdl/BrushBuilder.h"
#include "mdl/GameConfig.h"
#include "mdl/GameInfo.h"
#include "mdl/Map.h"
#include "mdl/WorldNode.h"
#include "ui/DrawShapeToolExtensionKind.h"
#include "ui/DrawShapeToolParameters.h"
#include "ui/MapDocument.h"

#include "kd/ranges/to.h"
#include "kd/result_fold.h"

#include <algorithm>
#include <cmath>
#include <ranges>

namespace tb::ui
{

namespace
{

using StairDirection = DrawShapeToolParameters::StairDirection;

vm::axis::type stairDirectionToAxis(const StairDirection direction)
{
  return direction == StairDirection::PosY || direction == StairDirection::NegY
           ? vm::axis::y
           : vm::axis::x;
}

bool isPositiveStairDirection(const StairDirection direction)
{
  return direction == StairDirection::PosX || direction == StairDirection::PosY;
}

std::unique_ptr<DrawShapeToolExtension> createExtension(
  const DrawShapeToolExtensionKind kind, MapDocument& document)
{
  switch (kind)
  {
  case DrawShapeToolExtensionKind::Cuboid:
    return std::make_unique<DrawShapeToolCuboidExtension>(document);
  case DrawShapeToolExtensionKind::Stairs:
    return std::make_unique<DrawShapeToolStairsExtension>(document);
  case DrawShapeToolExtensionKind::Arch:
    return std::make_unique<DrawShapeToolArchExtension>(document);
  case DrawShapeToolExtensionKind::Cylinder:
    return std::make_unique<DrawShapeToolCylinderExtension>(document);
  case DrawShapeToolExtensionKind::Cone:
    return std::make_unique<DrawShapeToolConeExtension>(document);
  case DrawShapeToolExtensionKind::UvSphere:
    return std::make_unique<DrawShapeToolUvSphereExtension>(document);
  case DrawShapeToolExtensionKind::IcoSphere:
    return std::make_unique<DrawShapeToolIcoSphereExtension>(document);
  }

  return nullptr;
}

} // namespace

DrawShapeToolCuboidExtension::DrawShapeToolCuboidExtension(MapDocument& document)
  : DrawShapeToolExtension{document}
{
}

const std::string& DrawShapeToolCuboidExtension::name() const
{
  static const auto name = std::string{"Cuboid"};
  return name;
}

const std::filesystem::path& DrawShapeToolCuboidExtension::iconPath() const
{
  static const auto path = std::filesystem::path{"ShapeTool_Cuboid.svg"};
  return path;
}

Result<std::vector<mdl::Brush>> DrawShapeToolCuboidExtension::createBrushes(
  const vm::bbox3d& bounds, const DrawShapeToolParameters&) const
{
  auto& map = m_document.map();

  const auto builder = mdl::BrushBuilder{
    map.worldNode().mapFormat(),
    map.worldBounds(),
    map.gameInfo().gameConfig.faceAttribsConfig.defaultUvAttributes,
    map.gameInfo().gameConfig.faceAttribsConfig.defaultSurfaceAttributes};

  return builder.createCuboid(bounds, map.currentMaterialName())
    .transform([](auto brush) { return std::vector{std::move(brush)}; });
}

DrawShapeToolCylinderExtension::DrawShapeToolCylinderExtension(MapDocument& document)
  : DrawShapeToolExtension{document}
{
}

const std::string& DrawShapeToolCylinderExtension::name() const
{
  static const auto name = std::string{"Cylinder"};
  return name;
}

const std::filesystem::path& DrawShapeToolCylinderExtension::iconPath() const
{
  static const auto path = std::filesystem::path{"ShapeTool_Cylinder.svg"};
  return path;
}

Result<std::vector<mdl::Brush>> DrawShapeToolCylinderExtension::createBrushes(
  const vm::bbox3d& bounds, const DrawShapeToolParameters& parameters) const
{
  auto& map = m_document.map();

  const auto builder = mdl::BrushBuilder{
    map.worldNode().mapFormat(),
    map.worldBounds(),
    map.gameInfo().gameConfig.faceAttribsConfig.defaultUvAttributes,
    map.gameInfo().gameConfig.faceAttribsConfig.defaultSurfaceAttributes};
  return parameters.hollow()
           ? builder.createHollowCylinder(
               bounds,
               parameters.thickness(),
               parameters.circleShape(),
               parameters.axis(),
               map.currentMaterialName())
           : builder
               .createCylinder(
                 bounds,
                 parameters.circleShape(),
                 parameters.axis(),
                 map.currentMaterialName())
               .transform([](auto brush) { return std::vector{std::move(brush)}; });
}

DrawShapeToolConeExtension::DrawShapeToolConeExtension(MapDocument& document)
  : DrawShapeToolExtension{document}
{
}

const std::string& DrawShapeToolConeExtension::name() const
{
  static const auto name = std::string{"Cone"};
  return name;
}

const std::filesystem::path& DrawShapeToolConeExtension::iconPath() const
{
  static const auto path = std::filesystem::path{"ShapeTool_Cone.svg"};
  return path;
}

Result<std::vector<mdl::Brush>> DrawShapeToolConeExtension::createBrushes(
  const vm::bbox3d& bounds, const DrawShapeToolParameters& parameters) const
{
  auto& map = m_document.map();

  const auto builder = mdl::BrushBuilder{
    map.worldNode().mapFormat(),
    map.worldBounds(),
    map.gameInfo().gameConfig.faceAttribsConfig.defaultUvAttributes,
    map.gameInfo().gameConfig.faceAttribsConfig.defaultSurfaceAttributes};
  return builder
    .createCone(
      bounds, parameters.circleShape(), parameters.axis(), map.currentMaterialName())
    .transform([](auto brush) { return std::vector{std::move(brush)}; });
}

DrawShapeToolIcoSphereExtension::DrawShapeToolIcoSphereExtension(MapDocument& document)
  : DrawShapeToolExtension{document}
{
}

const std::string& DrawShapeToolIcoSphereExtension::name() const
{
  static const auto name = std::string{"Spheroid (Icosahedron)"};
  return name;
}

const std::filesystem::path& DrawShapeToolIcoSphereExtension::iconPath() const
{
  static const auto path = std::filesystem::path{"ShapeTool_IcoSphere.svg"};
  return path;
}

Result<std::vector<mdl::Brush>> DrawShapeToolIcoSphereExtension::createBrushes(
  const vm::bbox3d& bounds, const DrawShapeToolParameters& parameters) const
{
  auto& map = m_document.map();

  const auto builder = mdl::BrushBuilder{
    map.worldNode().mapFormat(),
    map.worldBounds(),
    map.gameInfo().gameConfig.faceAttribsConfig.defaultUvAttributes,
    map.gameInfo().gameConfig.faceAttribsConfig.defaultSurfaceAttributes};

  return builder.createIcoSphere(bounds, parameters.accuracy(), map.currentMaterialName())
    .transform([](auto brush) { return std::vector{std::move(brush)}; });
}

DrawShapeToolUvSphereExtension::DrawShapeToolUvSphereExtension(MapDocument& document)
  : DrawShapeToolExtension{document}
{
}

const std::string& DrawShapeToolUvSphereExtension::name() const
{
  static const auto name = std::string{"Spheroid (UV)"};
  return name;
}

const std::filesystem::path& DrawShapeToolUvSphereExtension::iconPath() const
{
  static const auto path = std::filesystem::path{"ShapeTool_UVSphere.svg"};
  return path;
}

Result<std::vector<mdl::Brush>> DrawShapeToolUvSphereExtension::createBrushes(
  const vm::bbox3d& bounds, const DrawShapeToolParameters& parameters) const
{
  auto& map = m_document.map();

  const auto builder = mdl::BrushBuilder{
    map.worldNode().mapFormat(),
    map.worldBounds(),
    map.gameInfo().gameConfig.faceAttribsConfig.defaultUvAttributes,
    map.gameInfo().gameConfig.faceAttribsConfig.defaultSurfaceAttributes};
  return builder
    .createUvSphere(
      bounds,
      parameters.circleShape(),
      parameters.numRings(),
      parameters.axis(),
      map.currentMaterialName())
    .transform([](auto brush) { return std::vector{std::move(brush)}; });
}

DrawShapeToolStairsExtension::DrawShapeToolStairsExtension(MapDocument& document)
  : DrawShapeToolExtension{document}
{
}

const std::string& DrawShapeToolStairsExtension::name() const
{
  static const auto name = std::string{"Stairs"};
  return name;
}

const std::filesystem::path& DrawShapeToolStairsExtension::iconPath() const
{
  static const auto path = std::filesystem::path{"ShapeTool_Stairs.svg"};
  return path;
}

Result<std::vector<mdl::Brush>> DrawShapeToolStairsExtension::createBrushes(
  const vm::bbox3d& bounds, const DrawShapeToolParameters& parameters) const
{
  auto& map = m_document.map();

  const auto builder = mdl::BrushBuilder{
    map.worldNode().mapFormat(),
    map.worldBounds(),
    map.gameInfo().gameConfig.faceAttribsConfig.defaultUvAttributes,
    map.gameInfo().gameConfig.faceAttribsConfig.defaultSurfaceAttributes};

  const auto materialName = map.currentMaterialName();
  const auto stepHeight = std::max(1.0, std::abs(parameters.stepHeight()));
  const auto stairDirection = parameters.stairDirection();
  const auto axis = stairDirectionToAxis(stairDirection);
  const auto positiveDirection = isPositiveStairDirection(stairDirection);
  const auto size = bounds.size();

  const auto numSteps = std::max(size_t{1}, size_t(std::ceil(size.z() / stepHeight)));
  const auto treadDepth = size[axis] / double(numSteps);

  return std::views::iota(0u, numSteps) | std::views::transform([&](const auto i) {
           auto stepBounds = vm::bbox3d{
             bounds.min,
             {bounds.max.xy(),
              std::min(bounds.min.z() + stepHeight * double(i + 1), bounds.max.z())},
           };

           const auto treadMin = positiveDirection
                                   ? bounds.min[axis] + treadDepth * double(i)
                                   : bounds.max[axis] - treadDepth * double(i + 1);
           const auto treadMax = positiveDirection
                                   ? bounds.min[axis] + treadDepth * double(i + 1)
                                   : bounds.max[axis] - treadDepth * double(i);

           stepBounds.min[axis] = treadMin;
           stepBounds.max[axis] = treadMax;

           return builder.createCuboid(stepBounds, materialName);
         })
         | kdl::fold;
}

DrawShapeToolArchExtension::DrawShapeToolArchExtension(MapDocument& document)
  : DrawShapeToolExtension{document}
{
}

const std::string& DrawShapeToolArchExtension::name() const
{
  static const auto name = std::string{"Arch"};
  return name;
}

const std::filesystem::path& DrawShapeToolArchExtension::iconPath() const
{
  static const auto path = std::filesystem::path{"ShapeTool_Arch.svg"};
  return path;
}

Result<std::vector<mdl::Brush>> DrawShapeToolArchExtension::createBrushes(
  const vm::bbox3d& bounds, const DrawShapeToolParameters& parameters) const
{
  auto& map = m_document.map();

  const auto builder = mdl::BrushBuilder{
    map.worldNode().mapFormat(),
    map.worldBounds(),
    map.gameInfo().gameConfig.faceAttribsConfig.defaultUvAttributes,
    map.gameInfo().gameConfig.faceAttribsConfig.defaultSurfaceAttributes};

  return builder.createArch(
    bounds,
    parameters.thickness(),
    parameters.circleShape(),
    parameters.axis(),
    map.currentMaterialName());
}

std::vector<std::unique_ptr<DrawShapeToolExtension>> createDrawShapeToolExtensions(
  MapDocument& document)
{
  return DrawShapeToolExtensionKinds | std::views::transform([&](const auto kind) {
           return createExtension(kind, document);
         })
         | kdl::ranges::to<std::vector>();
}

} // namespace tb::ui
