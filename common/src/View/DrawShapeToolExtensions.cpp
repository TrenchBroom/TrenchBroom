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
#include <QCheckBox>
#include <QLabel>
#include <QSpinBox>
#include <QToolButton>

#include "Model/BrushBuilder.h"
#include "Model/WorldNode.h"
#include "View/MapDocument.h"
#include "View/QtUtils.h"

namespace TrenchBroom::View
{

DrawShapeToolExtensionPage::DrawShapeToolExtensionPage(QWidget* parent)
  : QWidget{parent}
{

  auto* layout = new QHBoxLayout{};
  layout->setContentsMargins(QMargins{});
  layout->setSpacing(LayoutConstants::MediumHMargin);
  layout->addStretch(1);
  setLayout(layout);
}

void DrawShapeToolExtensionPage::addWidget(QWidget* widget)
{
  auto* boxLayout = qobject_cast<QHBoxLayout*>(layout());
  boxLayout->insertWidget(boxLayout->count() - 1, widget, 0, Qt::AlignVCenter);
}

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
    document.world()->mapFormat(),
    document.worldBounds(),
    game->config().faceAttribsConfig.defaults};

  return builder.createCuboid(bounds, document.currentMaterialName())
    .transform([](auto brush) { return std::vector{std::move(brush)}; });
}

DrawShapeToolCircularShapeExtensionPage::DrawShapeToolCircularShapeExtensionPage(
  CircularShapeParameters& parameters, QWidget* parent)
  : DrawShapeToolExtensionPage{parent}
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
  connect(radiusModeEdgeButton, &QToolButton::clicked, this, [&]() {
    m_parameters.radiusMode = Model::RadiusMode::ToEdge;
  });
  connect(radiusModeVertexButton, &QToolButton::clicked, this, [&]() {
    m_parameters.radiusMode = Model::RadiusMode::ToVertex;
  });

  addWidget(numSidesLabel);
  addWidget(numSidesBox);
  addWidget(radiusModeEdgeButton);
  addWidget(radiusModeVertexButton);
}

DrawShapeToolCylinderShapeExtensionPage::DrawShapeToolCylinderShapeExtensionPage(
  CylinderShapeParameters& parameters, QWidget* parent)
  : DrawShapeToolCircularShapeExtensionPage{parameters, parent}
  , m_parameters{parameters}
{
  auto* hollowCheckBox = new QCheckBox{tr("Hollow")};
  hollowCheckBox->setChecked(m_parameters.hollow);

  auto* thicknessLabel = new QLabel{tr("Thickness: ")};
  auto* thicknessBox = new QDoubleSpinBox{};
  thicknessBox->setEnabled(parameters.hollow);
  thicknessBox->setRange(1, 128);
  thicknessBox->setValue(parameters.thickness);

  connect(
    hollowCheckBox, &QCheckBox::toggled, this, [&, thicknessBox](const auto hollow) {
      m_parameters.hollow = hollow;
      thicknessBox->setEnabled(hollow);
    });
  connect(
    thicknessBox,
    QOverload<double>::of(&QDoubleSpinBox::valueChanged),
    this,
    [&](const auto thickness) { m_parameters.thickness = thickness; });

  addWidget(hollowCheckBox);
  addWidget(thicknessLabel);
  addWidget(thicknessBox);
}

DrawShapeToolCylinderExtension::DrawShapeToolCylinderExtension()
  : m_parameters{8, Model::RadiusMode::ToEdge, false, 16.0}
{
}

const std::string& DrawShapeToolCylinderExtension::name() const
{
  static const auto name = std::string{"Cylinder"};
  return name;
}

QWidget* DrawShapeToolCylinderExtension::createToolPage(QWidget* parent)
{
  return new DrawShapeToolCylinderShapeExtensionPage{m_parameters, parent};
}

Result<std::vector<Model::Brush>> DrawShapeToolCylinderExtension::createBrushes(
  const vm::bbox3& bounds, vm::axis::type axis, const MapDocument& document) const
{
  const auto game = document.game();
  const auto builder = Model::BrushBuilder{
    document.world()->mapFormat(),
    document.worldBounds(),
    game->config().faceAttribsConfig.defaults};
  return m_parameters.hollow
           ? builder.createHollowCylinder(
             bounds,
             m_parameters.thickness,
             m_parameters.numSides,
             m_parameters.radiusMode,
             axis,
             document.currentMaterialName())
           : builder
               .createCylinder(
                 bounds,
                 m_parameters.numSides,
                 m_parameters.radiusMode,
                 axis,
                 document.currentMaterialName())
               .transform([](auto brush) { return std::vector{std::move(brush)}; });
}

DrawShapeToolConeExtension::DrawShapeToolConeExtension()
  : m_parameters{8, Model::RadiusMode::ToEdge}
{
}

const std::string& DrawShapeToolConeExtension::name() const
{
  static const auto name = std::string{"Cone"};
  return name;
}

QWidget* DrawShapeToolConeExtension::createToolPage(QWidget* parent)
{
  return new DrawShapeToolCircularShapeExtensionPage{m_parameters, parent};
}

Result<std::vector<Model::Brush>> DrawShapeToolConeExtension::createBrushes(
  const vm::bbox3& bounds, vm::axis::type axis, const MapDocument& document) const
{
  const auto game = document.game();
  const auto builder = Model::BrushBuilder{
    document.world()->mapFormat(),
    document.worldBounds(),
    game->config().faceAttribsConfig.defaults};
  return builder
    .createCone(
      bounds,
      m_parameters.numSides,
      m_parameters.radiusMode,
      axis,
      document.currentMaterialName())
    .transform([](auto brush) { return std::vector{std::move(brush)}; });
}

DrawShapeToolIcoSphereShapeExtensionPage::DrawShapeToolIcoSphereShapeExtensionPage(
  IcoSphereShapeParameters& parameters, QWidget* parent)
  : DrawShapeToolExtensionPage{parent}
  , m_parameters{parameters}
{
  auto* accuracyLabel = new QLabel{tr("Accuracy: ")};
  auto* accuracyBox = new QSpinBox{};
  accuracyBox->setRange(0, 4);
  accuracyBox->setValue(int(m_parameters.accuracy));

  connect(
    accuracyBox,
    QOverload<int>::of(&QSpinBox::valueChanged),
    this,
    [&](const auto accuracy) { m_parameters.accuracy = size_t(accuracy); });

  addWidget(accuracyLabel);
  addWidget(accuracyBox);
}

DrawShapeToolIcoSphereExtension::DrawShapeToolIcoSphereExtension()
  : m_parameters{1}
{
}

const std::string& DrawShapeToolIcoSphereExtension::name() const
{
  static const auto name = std::string{"Spheroid (Icosahedron)"};
  return name;
}

QWidget* DrawShapeToolIcoSphereExtension::createToolPage(QWidget* parent)
{
  return new DrawShapeToolIcoSphereShapeExtensionPage{m_parameters, parent};
}

Result<std::vector<Model::Brush>> DrawShapeToolIcoSphereExtension::createBrushes(
  const vm::bbox3& bounds, const vm::axis::type, const MapDocument& document) const
{
  const auto game = document.game();
  const auto builder = Model::BrushBuilder{
    document.world()->mapFormat(),
    document.worldBounds(),
    game->config().faceAttribsConfig.defaults};

  return builder
    .createIcoSphere(bounds, m_parameters.accuracy, document.currentMaterialName())
    .transform([](auto brush) { return std::vector{std::move(brush)}; });
}

DrawShapeToolUVSphereShapeExtensionPage::DrawShapeToolUVSphereShapeExtensionPage(
  UVSphereShapeParameters& parameters, QWidget* parent)
  : DrawShapeToolCircularShapeExtensionPage{parameters, parent}
  , m_parameters{parameters}
{
  auto* numRingsLabel = new QLabel{tr("Number of Rings: ")};
  auto* numRingsBox = new QSpinBox{};
  numRingsBox->setRange(1, 256);
  numRingsBox->setValue(int(m_parameters.numRings));

  connect(
    numRingsBox,
    QOverload<int>::of(&QSpinBox::valueChanged),
    this,
    [&](const auto numRings) { m_parameters.numRings = size_t(numRings); });

  addWidget(numRingsLabel);
  addWidget(numRingsBox);
}

DrawShapeToolUVSphereExtension::DrawShapeToolUVSphereExtension()
  : m_parameters{8, Model::RadiusMode::ToEdge, 8}
{
}

const std::string& DrawShapeToolUVSphereExtension::name() const
{
  static const auto name = std::string{"Spheroid (UV)"};
  return name;
}

QWidget* DrawShapeToolUVSphereExtension::createToolPage(QWidget* parent)
{
  return new DrawShapeToolUVSphereShapeExtensionPage{m_parameters, parent};
}

Result<std::vector<Model::Brush>> DrawShapeToolUVSphereExtension::createBrushes(
  const vm::bbox3& bounds, vm::axis::type axis, const MapDocument& document) const
{
  const auto game = document.game();
  const auto builder = Model::BrushBuilder{
    document.world()->mapFormat(),
    document.worldBounds(),
    game->config().faceAttribsConfig.defaults};
  return builder
    .createUVSphere(
      bounds,
      m_parameters.numSides,
      m_parameters.numRings,
      m_parameters.radiusMode,
      axis,
      document.currentMaterialName())
    .transform([](auto brush) { return std::vector{std::move(brush)}; });
}

std::vector<std::unique_ptr<DrawShapeToolExtension>> createDrawShapeToolExtensions()
{
  auto result = std::vector<std::unique_ptr<DrawShapeToolExtension>>{};
  result.push_back(std::make_unique<DrawShapeToolCuboidExtension>());
  result.push_back(std::make_unique<DrawShapeToolCylinderExtension>());
  result.push_back(std::make_unique<DrawShapeToolConeExtension>());
  result.push_back(std::make_unique<DrawShapeToolUVSphereExtension>());
  result.push_back(std::make_unique<DrawShapeToolIcoSphereExtension>());
  return result;
}


} // namespace TrenchBroom::View
