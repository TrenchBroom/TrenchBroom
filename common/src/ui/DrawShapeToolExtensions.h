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

#include "Result.h"
#include "mdl/BrushBuilder.h"
#include "ui/DrawShapeToolExtension.h"

#include <memory>
#include <vector>

class QWidget;

namespace tb::ui
{
class MapDocument;

class DrawShapeToolCuboidExtension : public DrawShapeToolExtension
{
public:
  explicit DrawShapeToolCuboidExtension(std::weak_ptr<MapDocument> document);

  const std::string& name() const override;
  const std::filesystem::path& iconPath() const override;
  DrawShapeToolExtensionPage* createToolPage(QWidget* parent) override;
  Result<std::vector<mdl::Brush>> createBrushes(const vm::bbox3d& bounds) const override;
};

struct AxisAlignedShapeParameters
{
  vm::axis::type axis;
};

class DrawShapeToolAxisAlignedShapeExtensionPage : public DrawShapeToolExtensionPage
{
public:
  explicit DrawShapeToolAxisAlignedShapeExtensionPage(
    AxisAlignedShapeParameters& parameters, QWidget* parent = nullptr);

private:
  AxisAlignedShapeParameters& m_parameters;
};

struct CircularShapeParameters : AxisAlignedShapeParameters
{
  mdl::CircleShape circleShape;
};

class DrawShapeToolCircularShapeExtensionPage
  : public DrawShapeToolAxisAlignedShapeExtensionPage
{
public:
  explicit DrawShapeToolCircularShapeExtensionPage(
    CircularShapeParameters& parameters, QWidget* parent = nullptr);

private:
  CircularShapeParameters& m_parameters;

  Q_OBJECT
};

struct CylinderShapeParameters : public CircularShapeParameters
{
  bool hollow;
  double thickness;
};

class DrawShapeToolCylinderShapeExtensionPage
  : public DrawShapeToolCircularShapeExtensionPage
{
public:
  explicit DrawShapeToolCylinderShapeExtensionPage(
    std::weak_ptr<MapDocument> document,
    CylinderShapeParameters& parameters,
    QWidget* parent = nullptr);

private:
  CylinderShapeParameters& m_parameters;

  Q_OBJECT
};

class DrawShapeToolCylinderExtension : public DrawShapeToolExtension
{
public:
  explicit DrawShapeToolCylinderExtension(std::weak_ptr<MapDocument> document);

  const std::string& name() const override;
  const std::filesystem::path& iconPath() const override;
  DrawShapeToolExtensionPage* createToolPage(QWidget* parent) override;
  Result<std::vector<mdl::Brush>> createBrushes(const vm::bbox3d& bounds) const override;

private:
  CylinderShapeParameters m_parameters;
};

class DrawShapeToolConeShapeExtensionPage : public DrawShapeToolCircularShapeExtensionPage
{
public:
  explicit DrawShapeToolConeShapeExtensionPage(
    std::weak_ptr<MapDocument> document,
    CircularShapeParameters& parameters,
    QWidget* parent = nullptr);

  Q_OBJECT
};

class DrawShapeToolConeExtension : public DrawShapeToolExtension
{
public:
  explicit DrawShapeToolConeExtension(std::weak_ptr<MapDocument> document);

  const std::string& name() const override;
  const std::filesystem::path& iconPath() const override;
  DrawShapeToolExtensionPage* createToolPage(QWidget* parent) override;
  Result<std::vector<mdl::Brush>> createBrushes(const vm::bbox3d& bounds) const override;

private:
  CircularShapeParameters m_parameters;
};

struct IcoSphereShapeParameters
{
  size_t accuracy;
};

class DrawShapeToolIcoSphereShapeExtensionPage : public DrawShapeToolExtensionPage
{
public:
  explicit DrawShapeToolIcoSphereShapeExtensionPage(
    std::weak_ptr<MapDocument> document,
    IcoSphereShapeParameters& parameters,
    QWidget* parent = nullptr);

private:
  IcoSphereShapeParameters& m_parameters;

  Q_OBJECT
};

class DrawShapeToolIcoSphereExtension : public DrawShapeToolExtension
{
public:
  explicit DrawShapeToolIcoSphereExtension(std::weak_ptr<MapDocument> document);

  const std::string& name() const override;
  const std::filesystem::path& iconPath() const override;
  DrawShapeToolExtensionPage* createToolPage(QWidget* parent) override;
  Result<std::vector<mdl::Brush>> createBrushes(const vm::bbox3d& bounds) const override;

private:
  IcoSphereShapeParameters m_parameters;
};

struct UVSphereShapeParameters : public CircularShapeParameters
{
  size_t numRings;
};

class DrawShapeToolUVSphereShapeExtensionPage
  : public DrawShapeToolCircularShapeExtensionPage
{
public:
  explicit DrawShapeToolUVSphereShapeExtensionPage(
    std::weak_ptr<MapDocument> document,
    UVSphereShapeParameters& parameters,
    QWidget* parent = nullptr);

private:
  UVSphereShapeParameters& m_parameters;

  Q_OBJECT
};

class DrawShapeToolUVSphereExtension : public DrawShapeToolExtension
{
public:
  explicit DrawShapeToolUVSphereExtension(std::weak_ptr<MapDocument> document);

  const std::string& name() const override;
  const std::filesystem::path& iconPath() const override;
  DrawShapeToolExtensionPage* createToolPage(QWidget* parent) override;
  Result<std::vector<mdl::Brush>> createBrushes(const vm::bbox3d& bounds) const override;

private:
  UVSphereShapeParameters m_parameters;
};

std::vector<std::unique_ptr<DrawShapeToolExtension>> createDrawShapeToolExtensions(
  std::weak_ptr<MapDocument> document);

} // namespace tb::ui
