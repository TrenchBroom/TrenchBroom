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

#include "Notifier.h"
#include "Result.h"
#include "mdl/Brush.h"
#include "mdl/BrushBuilder.h"

#include <filesystem>
#include <string>
#include <vector>

namespace tb::mdl
{
class Map;
}

namespace tb::ui
{

class DrawShapeToolExtensionPage : public QWidget
{
  Q_OBJECT
protected:
  NotifierConnection m_notifierConnection;

public:
  Notifier<> applyParametersNotifier;

  explicit DrawShapeToolExtensionPage(QWidget* parent = nullptr);

protected:
  void addWidget(QWidget* widget);
  void addApplyButton(mdl::Map& map);
};

class ShapeParameters
{
private:
  // For axis aligned shapes
  vm::axis::type m_axis = vm::axis::z;

  // For circular shapes
  mdl::CircleShape m_circleShape = mdl::EdgeAlignedCircle{8};

  // For hollow shapes
  bool m_hollow = false;
  double m_thickness = 16.0;

  // For UV sphere
  size_t m_numRings = 8;

  // For ICO sphere
  size_t m_accuracy = 1;

public:
  Notifier<> parametersDidChangeNotifier;

  vm::axis::type axis() const;
  void setAxis(vm::axis::type axis);

  const mdl::CircleShape& circleShape() const;
  void setCircleShape(mdl::CircleShape circleShape);

  bool hollow() const;
  void setHollow(bool hollow);

  double thickness() const;
  void setThickness(double thickness);

  size_t numRings() const;
  void setNumRings(size_t numRings);

  size_t accuracy() const;
  void setAccuracy(size_t accuracy);
};

class DrawShapeToolExtension
{
protected:
  mdl::Map& m_map;

  explicit DrawShapeToolExtension(mdl::Map& map);

public:
  virtual ~DrawShapeToolExtension();
  virtual const std::string& name() const = 0;
  virtual const std::filesystem::path& iconPath() const = 0;
  virtual DrawShapeToolExtensionPage* createToolPage(
    ShapeParameters& parameters, QWidget* parent = nullptr) = 0;
  virtual Result<std::vector<mdl::Brush>> createBrushes(
    const vm::bbox3d& bounds, const ShapeParameters& parameters) const = 0;
};

class DrawShapeToolExtensionManager
{
public:
  Notifier<size_t> currentExtensionDidChangeNotifier;

  explicit DrawShapeToolExtensionManager(mdl::Map& map);

  const std::vector<DrawShapeToolExtension*> extensions() const;

  const DrawShapeToolExtension& currentExtension() const;
  bool setCurrentExtensionIndex(size_t currentExtensionIndex);

  std::vector<DrawShapeToolExtensionPage*> createToolPages(QWidget* parent = nullptr);
  Result<std::vector<mdl::Brush>> createBrushes(const vm::bbox3d& bounds) const;

private:
  ShapeParameters m_parameters;
  std::vector<std::unique_ptr<DrawShapeToolExtension>> m_extensions;
  size_t m_currentExtensionIndex = 0;
};

} // namespace tb::ui
