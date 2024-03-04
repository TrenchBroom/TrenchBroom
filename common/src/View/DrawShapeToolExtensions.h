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

#include "View/DrawShapeToolExtension.h"

#include <memory>
#include <vector>

namespace TrenchBroom::Model
{
enum class RadiusMode;
}

namespace TrenchBroom::View
{
class MapDocument;

class DrawShapeToolCuboidExtension : public DrawShapeToolExtension
{
public:
  const std::string& name() const override;
  QWidget* createToolPage(QWidget* parent) override;
  Result<std::vector<Model::Brush>> createBrushes(
    const vm::bbox3& bounds,
    vm::axis::type axis,
    const MapDocument& document) const override;
};

struct CircularShapeParameters
{
  size_t numSides;
  Model::RadiusMode radiusMode;
};

class DrawShapeToolCircularShapeExtensionPage : public QWidget
{
public:
  explicit DrawShapeToolCircularShapeExtensionPage(
    CircularShapeParameters& parameters, QWidget* parent = nullptr);

private:
  CircularShapeParameters& m_parameters;

  Q_OBJECT
};

class DrawShapeToolCylinderExtension : public DrawShapeToolExtension
{
public:
  DrawShapeToolCylinderExtension();

  const std::string& name() const override;
  QWidget* createToolPage(QWidget* parent) override;
  Result<std::vector<Model::Brush>> createBrushes(
    const vm::bbox3& bounds,
    vm::axis::type axis,
    const MapDocument& document) const override;

private:
  CircularShapeParameters m_parameters;
};

class DrawShapeToolConeExtension : public DrawShapeToolExtension
{
public:
  DrawShapeToolConeExtension();

  const std::string& name() const override;
  QWidget* createToolPage(QWidget* parent) override;
  Result<std::vector<Model::Brush>> createBrushes(
    const vm::bbox3& bounds,
    vm::axis::type axis,
    const MapDocument& document) const override;

private:
  CircularShapeParameters m_parameters;
};

std::vector<std::unique_ptr<DrawShapeToolExtension>> createDrawShapeToolExtensions();

} // namespace TrenchBroom::View
