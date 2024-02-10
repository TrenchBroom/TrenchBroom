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

#include "View/DrawBrushToolExtension.h"

#include <memory>
#include <vector>

namespace TrenchBroom::Model
{
enum class RadiusMode;
}

namespace TrenchBroom::View
{
class MapDocument;

class DrawBrushToolCuboidExtension : public DrawBrushToolExtension
{
public:
  const std::string& name() const override;
  QWidget* createToolPage(QWidget* parent) override;
  std::vector<Result<Model::Brush>> createBrushes(
    const vm::bbox3& bounds,
    vm::axis::type axis,
    const MapDocument& document) const override;
};

struct CircularShapeParameters
{
  size_t numSides;
  Model::RadiusMode radiusMode;
  double thickness = 16.0;
  bool hollow;
};

class DrawBrushToolCircularShapeExtensionPage : public QWidget
{
public:
  explicit DrawBrushToolCircularShapeExtensionPage(
    CircularShapeParameters& parameters, QWidget* parent = nullptr);

private:
  CircularShapeParameters& m_parameters;

  Q_OBJECT
};

class DrawBrushToolCylinderExtension : public DrawBrushToolExtension
{
public:
  DrawBrushToolCylinderExtension();

  const std::string& name() const override;
  QWidget* createToolPage(QWidget* parent) override;
  std::vector<Result<Model::Brush>> createBrushes(
    const vm::bbox3& bounds,
    vm::axis::type axis,
    const MapDocument& document) const override;

private:
  CircularShapeParameters m_parameters;
};

class DrawBrushToolConeExtension : public DrawBrushToolExtension
{
public:
  DrawBrushToolConeExtension();

  const std::string& name() const override;
  QWidget* createToolPage(QWidget* parent) override;
  std::vector<Result<Model::Brush>> createBrushes(
    const vm::bbox3& bounds,
    vm::axis::type axis,
    const MapDocument& document) const override;

private:
  CircularShapeParameters m_parameters;
};

std::vector<std::unique_ptr<DrawBrushToolExtension>> createDrawBrushToolExtensions();

} // namespace TrenchBroom::View
