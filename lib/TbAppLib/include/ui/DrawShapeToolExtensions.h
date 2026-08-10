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

#pragma once

#include "base/Result.h"
#include "mdl/BrushBuilder.h"
#include "ui/DrawShapeToolExtension.h"

#include <vector>

namespace tb::ui
{
class DrawShapeToolParameters;
class MapDocument;

class DrawShapeToolCuboidExtension : public DrawShapeToolExtension
{
public:
  explicit DrawShapeToolCuboidExtension(MapDocument& document);

  const std::string& name() const override;
  const std::filesystem::path& iconPath() const override;
  Result<std::vector<mdl::Brush>> createBrushes(
    const vm::bbox3d& bounds, const DrawShapeToolParameters& parameters) const override;
};

class DrawShapeToolCylinderExtension : public DrawShapeToolExtension
{
public:
  explicit DrawShapeToolCylinderExtension(MapDocument& document);

  const std::string& name() const override;
  const std::filesystem::path& iconPath() const override;
  Result<std::vector<mdl::Brush>> createBrushes(
    const vm::bbox3d& bounds, const DrawShapeToolParameters& parameters) const override;
};

class DrawShapeToolConeExtension : public DrawShapeToolExtension
{
public:
  explicit DrawShapeToolConeExtension(MapDocument& document);

  const std::string& name() const override;
  const std::filesystem::path& iconPath() const override;
  Result<std::vector<mdl::Brush>> createBrushes(
    const vm::bbox3d& bounds, const DrawShapeToolParameters& parameters) const override;
};

class DrawShapeToolIcoSphereExtension : public DrawShapeToolExtension
{
public:
  explicit DrawShapeToolIcoSphereExtension(MapDocument& document);

  const std::string& name() const override;
  const std::filesystem::path& iconPath() const override;
  Result<std::vector<mdl::Brush>> createBrushes(
    const vm::bbox3d& bounds, const DrawShapeToolParameters& parameters) const override;
};

class DrawShapeToolUvSphereExtension : public DrawShapeToolExtension
{
public:
  explicit DrawShapeToolUvSphereExtension(MapDocument& document);

  const std::string& name() const override;
  const std::filesystem::path& iconPath() const override;
  Result<std::vector<mdl::Brush>> createBrushes(
    const vm::bbox3d& bounds, const DrawShapeToolParameters& parameters) const override;
};

class DrawShapeToolStairsExtension : public DrawShapeToolExtension
{
public:
  explicit DrawShapeToolStairsExtension(MapDocument& document);

  const std::string& name() const override;
  const std::filesystem::path& iconPath() const override;
  Result<std::vector<mdl::Brush>> createBrushes(
    const vm::bbox3d& bounds, const DrawShapeToolParameters& parameters) const override;
};

class DrawShapeToolArchExtension : public DrawShapeToolExtension
{
public:
  explicit DrawShapeToolArchExtension(MapDocument& document);

  const std::string& name() const override;
  const std::filesystem::path& iconPath() const override;
  Result<std::vector<mdl::Brush>> createBrushes(
    const vm::bbox3d& bounds, const DrawShapeToolParameters& parameters) const override;
};

std::vector<std::unique_ptr<DrawShapeToolExtension>> createDrawShapeToolExtensions(
  MapDocument& document);

} // namespace tb::ui
