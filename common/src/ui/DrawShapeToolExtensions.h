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

#include <QWidget>

#include "Result.h"
#include "ui/DrawShapeToolExtension.h"

#include <memory>
#include <vector>

namespace tb::mdl
{
enum class RadiusMode;
}

namespace tb::ui
{
class MapDocument;

class DrawShapeToolExtensionPage : public QWidget
{
public:
  explicit DrawShapeToolExtensionPage(QWidget* parent = nullptr);

protected:
  void addWidget(QWidget* widget);

  Q_OBJECT
};


class DrawShapeToolCuboidExtension : public DrawShapeToolExtension
{
public:
  const std::string& name() const override;
  const std::filesystem::path& iconPath() const override;
  QWidget* createToolPage(QWidget* parent) override;
  Result<std::vector<mdl::Brush>> createBrushes(
    const vm::bbox3d& bounds, const MapDocument& document) const override;
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

  Q_OBJECT
};

struct CircularShapeParameters : AxisAlignedShapeParameters
{
  size_t numSides;
  mdl::RadiusMode radiusMode;
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
    CylinderShapeParameters& parameters, QWidget* parent = nullptr);

private:
  CylinderShapeParameters& m_parameters;

  Q_OBJECT
};

class DrawShapeToolCylinderExtension : public DrawShapeToolExtension
{
public:
  DrawShapeToolCylinderExtension();

  const std::string& name() const override;
  const std::filesystem::path& iconPath() const override;
  QWidget* createToolPage(QWidget* parent) override;
  Result<std::vector<mdl::Brush>> createBrushes(
    const vm::bbox3d& bounds, const MapDocument& document) const override;

private:
  CylinderShapeParameters m_parameters;
};

class DrawShapeToolConeExtension : public DrawShapeToolExtension
{
public:
  DrawShapeToolConeExtension();

  const std::string& name() const override;
  const std::filesystem::path& iconPath() const override;
  QWidget* createToolPage(QWidget* parent) override;
  Result<std::vector<mdl::Brush>> createBrushes(
    const vm::bbox3d& bounds, const MapDocument& document) const override;

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
    IcoSphereShapeParameters& parameters, QWidget* parent = nullptr);

private:
  IcoSphereShapeParameters& m_parameters;

  Q_OBJECT
};

class DrawShapeToolIcoSphereExtension : public DrawShapeToolExtension
{
public:
  DrawShapeToolIcoSphereExtension();

  const std::string& name() const override;
  const std::filesystem::path& iconPath() const override;
  QWidget* createToolPage(QWidget* parent) override;
  Result<std::vector<mdl::Brush>> createBrushes(
    const vm::bbox3d& bounds, const MapDocument& document) const override;

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
    UVSphereShapeParameters& parameters, QWidget* parent = nullptr);

private:
  UVSphereShapeParameters& m_parameters;

  Q_OBJECT
};

class DrawShapeToolUVSphereExtension : public DrawShapeToolExtension
{
public:
  DrawShapeToolUVSphereExtension();

  const std::string& name() const override;
  const std::filesystem::path& iconPath() const override;
  QWidget* createToolPage(QWidget* parent) override;
  Result<std::vector<mdl::Brush>> createBrushes(
    const vm::bbox3d& bounds, const MapDocument& document) const override;

private:
  UVSphereShapeParameters m_parameters;
};

std::vector<std::unique_ptr<DrawShapeToolExtension>> createDrawShapeToolExtensions();

} // namespace tb::ui
