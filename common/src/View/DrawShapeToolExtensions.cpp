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

#include "DrawShapeToolExtensions.h"

#include <QButtonGroup>
#include <QLabel>
#include <QSpinBox>
#include <QToolButton>

#include "Error.h" // IWYU pragma: keep
#include "Model/BrushBuilder.h"
#include "Model/WorldNode.h"
#include "View/MapDocument.h"
#include "View/QtUtils.h"

#include "kdl/result.h"

namespace TrenchBroom::View
{

const std::string& DrawShapeToolCuboidExtension::name() const
{
  static const auto name = std::string{"Cuboid"};
  return name;
}

QWidget* DrawShapeToolCuboidExtension::createToolPage(QWidget* parent)
{
  return new QWidget{parent};
}

Result<std::vector<Model::Brush>> DrawShapeToolCuboidExtension::createBrushes(
  const vm::bbox3& bounds, const vm::axis::type, const MapDocument& document) const
{
  const auto game = document.game();
  const auto builder = Model::BrushBuilder{
    document.world()->mapFormat(), document.worldBounds(), game->defaultFaceAttribs()};

  return builder.createCuboid(bounds, document.currentTextureName())
    .transform([](auto brush) { return std::vector{std::move(brush)}; });
}

DrawShapeToolCylinderExtensionPage::DrawShapeToolCylinderExtensionPage(
  CylinderParameters& parameters, QWidget* parent)
  : QWidget{parent}
  , m_parameters{parameters}
{
  auto* numSidesLabel = new QLabel{tr("Number of Sides: ")};
  auto* numSidesBox = new QSpinBox{};
  numSidesBox->setRange(3, 256);
  numSidesBox->setValue(int(m_parameters.numSides));

  auto* radiusModeEdgeButton =
    createBitmapToggleButton("RadiusModeEdge.svg", tr("Radius is to edge"));
  radiusModeEdgeButton->setIconSize({24, 24});
  radiusModeEdgeButton->setObjectName("backgroundChecked");
  radiusModeEdgeButton->setChecked(m_parameters.radiusMode == Model::RadiusMode::ToEdge);

  auto* radiusModeVertexButton =
    createBitmapToggleButton("RadiusModeVertex.svg", tr("Radius is to vertex"));
  radiusModeVertexButton->setIconSize({24, 24});
  radiusModeVertexButton->setObjectName("backgroundChecked");
  radiusModeVertexButton->setChecked(
    m_parameters.radiusMode == Model::RadiusMode::ToVertex);

  auto* radiusModeButtonGroup = new QButtonGroup{};
  radiusModeButtonGroup->addButton(radiusModeEdgeButton);
  radiusModeButtonGroup->addButton(radiusModeVertexButton);

  connect(
    numSidesBox,
    QOverload<int>::of(&QSpinBox::valueChanged),
    this,
    [&](const auto numSides) { m_parameters.numSides = size_t(numSides); });
  connect(radiusModeEdgeButton, &QToolButton::clicked, this, [=]() {
    m_parameters.radiusMode = Model::RadiusMode::ToEdge;
  });
  connect(radiusModeVertexButton, &QToolButton::clicked, this, [=]() {
    m_parameters.radiusMode = Model::RadiusMode::ToVertex;
  });

  auto* layout = new QHBoxLayout{};
  layout->setContentsMargins(QMargins{});
  layout->setSpacing(LayoutConstants::MediumHMargin);

  layout->addWidget(numSidesLabel, 0, Qt::AlignVCenter);
  layout->addWidget(numSidesBox, 0, Qt::AlignVCenter);
  layout->addWidget(radiusModeEdgeButton, 0, Qt::AlignVCenter);
  layout->addWidget(radiusModeVertexButton, 0, Qt::AlignVCenter);
  layout->addStretch(1);

  setLayout(layout);
}

DrawShapeToolCylinderExtension::DrawShapeToolCylinderExtension()
  : m_parameters{8, Model::RadiusMode::ToEdge}
{
}

const std::string& DrawShapeToolCylinderExtension::name() const
{
  static const auto name = std::string{"Cylinder"};
  return name;
}

QWidget* DrawShapeToolCylinderExtension::createToolPage(QWidget* parent)
{
  return new DrawShapeToolCylinderExtensionPage{m_parameters, parent};
}

Result<std::vector<Model::Brush>> DrawShapeToolCylinderExtension::createBrushes(
  const vm::bbox3& bounds, vm::axis::type axis, const MapDocument& document) const
{
  const auto game = document.game();
  const auto builder = Model::BrushBuilder{
    document.world()->mapFormat(), document.worldBounds(), game->defaultFaceAttribs()};
  return builder
    .createCylinder(
      bounds,
      m_parameters.numSides,
      m_parameters.radiusMode,
      axis,
      document.currentTextureName())
    .transform([](auto brush) { return std::vector{std::move(brush)}; });
}

std::vector<std::unique_ptr<DrawShapeToolExtension>> createDrawShapeToolExtensions()
{
  auto result = std::vector<std::unique_ptr<DrawShapeToolExtension>>{};
  result.push_back(std::make_unique<DrawShapeToolCuboidExtension>());
  result.push_back(std::make_unique<DrawShapeToolCylinderExtension>());
  return result;
}

} // namespace TrenchBroom::View
