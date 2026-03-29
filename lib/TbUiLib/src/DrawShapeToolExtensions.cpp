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

#include <QBoxLayout>
#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QSpinBox>
#include <QStackedWidget>
#include <QToolButton>

#include "mdl/BrushBuilder.h"
#include "mdl/GameConfig.h"
#include "mdl/GameInfo.h"
#include "mdl/Map.h"
#include "mdl/WorldNode.h"
#include "ui/BitmapButton.h"
#include "ui/MapDocument.h"
#include "ui/ViewConstants.h"

#include "kd/result_fold.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <ranges>

namespace tb::ui
{

namespace
{

using StairDirection = ShapeParameters::StairDirection;

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

size_t stairDirectionToIndex(const StairDirection direction)
{
  switch (direction)
  {
  case StairDirection::PosX:
    return 0u;
  case StairDirection::NegX:
    return 1u;
  case StairDirection::PosY:
    return 2u;
  case StairDirection::NegY:
    return 3u;
  }

  return 0u;
}

StairDirection indexToStairDirection(const size_t index)
{
  static constexpr auto directions = std::array{
    StairDirection::PosX,
    StairDirection::NegX,
    StairDirection::PosY,
    StairDirection::NegY};
  return directions[std::min(index, directions.size() - 1u)];
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

DrawShapeToolExtensionPage* DrawShapeToolCuboidExtension::createToolPage(
  ShapeParameters&, QWidget* parent)
{
  return new DrawShapeToolExtensionPage{parent};
}

Result<std::vector<mdl::Brush>> DrawShapeToolCuboidExtension::createBrushes(
  const vm::bbox3d& bounds, const ShapeParameters&) const
{
  auto& map = m_document.map();

  const auto builder = mdl::BrushBuilder{
    map.worldNode().mapFormat(),
    map.worldBounds(),
    map.gameInfo().gameConfig.faceAttribsConfig.defaults};

  return builder.createCuboid(bounds, map.currentMaterialName())
    .transform([](auto brush) { return std::vector{std::move(brush)}; });
}

DrawShapeToolAxisAlignedShapeExtensionPage::DrawShapeToolAxisAlignedShapeExtensionPage(
  ShapeParameters& parameters, QWidget* parent)
  : DrawShapeToolExtensionPage{parent}
  , m_parameters{parameters}
{
  auto* axisLabel = new QLabel{tr("Axis: ")};
  auto* axisComboBox = new QComboBox{};
  axisComboBox->addItems({tr("X"), tr("Y"), tr("Z")});

  connect(
    axisComboBox,
    QOverload<int>::of(&QComboBox::currentIndexChanged),
    this,
    [&](const auto index) { m_parameters.setAxis(vm::axis::type(index)); });

  addWidget(axisLabel);
  addWidget(axisComboBox);

  m_notifierConnection += m_parameters.parametersDidChangeNotifier.connect(
    [=, this]() { axisComboBox->setCurrentIndex(int(m_parameters.axis())); });
}

DrawShapeToolCircularShapeExtensionPage::DrawShapeToolCircularShapeExtensionPage(
  ShapeParameters& parameters, QWidget* parent)
  : DrawShapeToolAxisAlignedShapeExtensionPage{parameters, parent}
  , m_parameters{parameters}
{
  auto* numSidesLabel = new QLabel{tr("Number of Sides: ")};
  auto* numSidesBox = new QSpinBox{};
  numSidesBox->setRange(3, 96);

  auto* precisionBox = new QComboBox{};
  precisionBox->addItems({"12", "24", "48", "96"});

  auto* numSidesWidget = new QStackedWidget{};
  numSidesWidget->addWidget(numSidesBox);
  numSidesWidget->addWidget(precisionBox);

  auto* edgeAlignedCircleButton =
    createBitmapToggleButton("CircleEdgeAligned.svg", tr("Align edge to bounding box"));
  edgeAlignedCircleButton->setIconSize({24, 24});
  edgeAlignedCircleButton->setObjectName("toolButton_withBorder");

  auto* vertexAlignedCircleButton = createBitmapToggleButton(
    "CircleVertexAligned.svg", tr("Align vertices to bounding box"));
  vertexAlignedCircleButton->setIconSize({24, 24});
  vertexAlignedCircleButton->setObjectName("toolButton_withBorder");

  auto* scalableCircleButton =
    createBitmapToggleButton("CircleScalable.svg", tr("Scalable circle shape"));
  scalableCircleButton->setIconSize({24, 24});
  scalableCircleButton->setObjectName("toolButton_withBorder");

  auto* radiusModeButtonGroup = new QButtonGroup{};
  radiusModeButtonGroup->addButton(edgeAlignedCircleButton);
  radiusModeButtonGroup->addButton(vertexAlignedCircleButton);
  radiusModeButtonGroup->addButton(scalableCircleButton);

  connect(
    numSidesBox,
    QOverload<int>::of(&QSpinBox::valueChanged),
    this,
    [&](const auto numSides) {
      m_parameters.setCircleShape(std::visit(
        kdl::overload(
          [&](const mdl::EdgeAlignedCircle&) -> mdl::CircleShape {
            return mdl::EdgeAlignedCircle{size_t(numSides)};
          },
          [&](const mdl::VertexAlignedCircle&) -> mdl::CircleShape {
            return mdl::VertexAlignedCircle{size_t(numSides)};
          },
          [&](const mdl::ScalableCircle& circleShape) -> mdl::CircleShape {
            return circleShape;
          }),
        m_parameters.circleShape()));
    });
  connect(
    precisionBox,
    QOverload<int>::of(&QComboBox::currentIndexChanged),
    this,
    [&](const auto precision) {
      m_parameters.setCircleShape(std::visit(
        kdl::overload(
          [&](const mdl::ScalableCircle&) -> mdl::CircleShape {
            return mdl::ScalableCircle{size_t(precision)};
          },
          [](const auto& circleShape) -> mdl::CircleShape { return circleShape; }),
        m_parameters.circleShape()));
    });
  connect(edgeAlignedCircleButton, &QToolButton::clicked, this, [=, this]() {
    m_parameters.setCircleShape(
      mdl::convertCircleShape<mdl::EdgeAlignedCircle>(m_parameters.circleShape()));
  });
  connect(vertexAlignedCircleButton, &QToolButton::clicked, this, [=, this]() {
    m_parameters.setCircleShape(
      mdl::convertCircleShape<mdl::VertexAlignedCircle>(m_parameters.circleShape()));
  });
  connect(scalableCircleButton, &QToolButton::clicked, this, [=, this]() {
    m_parameters.setCircleShape(
      mdl::convertCircleShape<mdl::ScalableCircle>(m_parameters.circleShape()));
  });

  addWidget(numSidesLabel);
  addWidget(numSidesWidget);
  addWidget(edgeAlignedCircleButton);
  addWidget(vertexAlignedCircleButton);
  addWidget(scalableCircleButton);

  m_notifierConnection += m_parameters.parametersDidChangeNotifier.connect([=, this]() {
    std::visit(
      kdl::overload(
        [&](const mdl::EdgeAlignedCircle& circleShape) {
          numSidesBox->setValue(int(circleShape.numSides));
          numSidesWidget->setCurrentWidget(numSidesBox);
        },
        [&](const mdl::VertexAlignedCircle& circleShape) {
          numSidesBox->setValue(int(circleShape.numSides));
          numSidesWidget->setCurrentWidget(numSidesBox);
        },
        [&](const mdl::ScalableCircle& circleShape) {
          precisionBox->setCurrentIndex(int(circleShape.precision));
          numSidesWidget->setCurrentWidget(precisionBox);
        }),
      m_parameters.circleShape());

    edgeAlignedCircleButton->setChecked(
      std::holds_alternative<mdl::EdgeAlignedCircle>(m_parameters.circleShape()));
    vertexAlignedCircleButton->setChecked(
      std::holds_alternative<mdl::VertexAlignedCircle>(m_parameters.circleShape()));
    scalableCircleButton->setChecked(
      std::holds_alternative<mdl::ScalableCircle>(m_parameters.circleShape()));
  });
}

DrawShapeToolCylinderShapeExtensionPage::DrawShapeToolCylinderShapeExtensionPage(
  MapDocument& document, ShapeParameters& parameters, QWidget* parent)
  : DrawShapeToolCircularShapeExtensionPage{parameters, parent}
  , m_parameters{parameters}
{
  auto* hollowCheckBox = new QCheckBox{tr("Hollow")};

  auto* thicknessLabel = new QLabel{tr("Thickness: ")};
  auto* thicknessBox = new QDoubleSpinBox{};
  thicknessBox->setEnabled(parameters.hollow());
  thicknessBox->setRange(1, 128);

  connect(hollowCheckBox, &QCheckBox::toggled, this, [&](const auto hollow) {
    m_parameters.setHollow(hollow);
  });
  connect(
    thicknessBox,
    QOverload<double>::of(&QDoubleSpinBox::valueChanged),
    this,
    [&](const auto thickness) { m_parameters.setThickness(thickness); });

  addWidget(hollowCheckBox);
  addWidget(thicknessLabel);
  addWidget(thicknessBox);
  addApplyButton(document);

  m_notifierConnection += m_parameters.parametersDidChangeNotifier.connect([=, this]() {
    hollowCheckBox->setChecked(m_parameters.hollow());
    thicknessBox->setEnabled(m_parameters.hollow());
    thicknessBox->setValue(m_parameters.thickness());
  });
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

DrawShapeToolExtensionPage* DrawShapeToolCylinderExtension::createToolPage(
  ShapeParameters& parameters, QWidget* parent)
{
  return new DrawShapeToolCylinderShapeExtensionPage{m_document, parameters, parent};
}

Result<std::vector<mdl::Brush>> DrawShapeToolCylinderExtension::createBrushes(
  const vm::bbox3d& bounds, const ShapeParameters& parameters) const
{
  auto& map = m_document.map();

  const auto builder = mdl::BrushBuilder{
    map.worldNode().mapFormat(),
    map.worldBounds(),
    map.gameInfo().gameConfig.faceAttribsConfig.defaults};
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

DrawShapeToolConeShapeExtensionPage::DrawShapeToolConeShapeExtensionPage(
  MapDocument& document, ShapeParameters& parameters, QWidget* parent)
  : DrawShapeToolCircularShapeExtensionPage{parameters, parent}
  , m_parameters{parameters}
{
  addApplyButton(document);
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

DrawShapeToolExtensionPage* DrawShapeToolConeExtension::createToolPage(
  ShapeParameters& parameters, QWidget* parent)
{
  return new DrawShapeToolConeShapeExtensionPage{m_document, parameters, parent};
}

Result<std::vector<mdl::Brush>> DrawShapeToolConeExtension::createBrushes(
  const vm::bbox3d& bounds, const ShapeParameters& parameters) const
{
  auto& map = m_document.map();

  const auto builder = mdl::BrushBuilder{
    map.worldNode().mapFormat(),
    map.worldBounds(),
    map.gameInfo().gameConfig.faceAttribsConfig.defaults};
  return builder
    .createCone(
      bounds, parameters.circleShape(), parameters.axis(), map.currentMaterialName())
    .transform([](auto brush) { return std::vector{std::move(brush)}; });
}

DrawShapeToolIcoSphereShapeExtensionPage::DrawShapeToolIcoSphereShapeExtensionPage(
  MapDocument& document, ShapeParameters& parameters, QWidget* parent)
  : DrawShapeToolExtensionPage{parent}
  , m_parameters{parameters}
{
  auto* accuracyLabel = new QLabel{tr("Accuracy: ")};
  auto* accuracyBox = new QSpinBox{};
  accuracyBox->setRange(0, 4);

  connect(
    accuracyBox,
    QOverload<int>::of(&QSpinBox::valueChanged),
    this,
    [&](const auto accuracy) { m_parameters.setAccuracy(size_t(accuracy)); });

  addWidget(accuracyLabel);
  addWidget(accuracyBox);
  addApplyButton(document);

  m_notifierConnection += m_parameters.parametersDidChangeNotifier.connect(
    [=, this]() { accuracyBox->setValue(int(m_parameters.accuracy())); });
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

DrawShapeToolExtensionPage* DrawShapeToolIcoSphereExtension::createToolPage(
  ShapeParameters& parameters, QWidget* parent)
{
  return new DrawShapeToolIcoSphereShapeExtensionPage{m_document, parameters, parent};
}

Result<std::vector<mdl::Brush>> DrawShapeToolIcoSphereExtension::createBrushes(
  const vm::bbox3d& bounds, const ShapeParameters& parameters) const
{
  auto& map = m_document.map();

  const auto builder = mdl::BrushBuilder{
    map.worldNode().mapFormat(),
    map.worldBounds(),
    map.gameInfo().gameConfig.faceAttribsConfig.defaults};

  return builder.createIcoSphere(bounds, parameters.accuracy(), map.currentMaterialName())
    .transform([](auto brush) { return std::vector{std::move(brush)}; });
}

DrawShapeToolUVSphereShapeExtensionPage::DrawShapeToolUVSphereShapeExtensionPage(
  MapDocument& document, ShapeParameters& parameters, QWidget* parent)
  : DrawShapeToolCircularShapeExtensionPage{parameters, parent}
  , m_parameters{parameters}
{
  auto* numRingsLabel = new QLabel{tr("Number of Rings: ")};
  auto* numRingsBox = new QSpinBox{};
  numRingsBox->setRange(1, 256);

  auto* numRingsLayout = new QHBoxLayout{};
  numRingsLayout->setContentsMargins(QMargins{});
  numRingsLayout->setSpacing(LayoutConstants::MediumHMargin);
  numRingsLayout->addWidget(numRingsLabel);
  numRingsLayout->addWidget(numRingsBox);

  auto* numRingsWidget = new QWidget{};
  numRingsWidget->setLayout(numRingsLayout);

  connect(
    numRingsBox,
    QOverload<int>::of(&QSpinBox::valueChanged),
    this,
    [&](const auto numRings) { m_parameters.setNumRings(size_t(numRings)); });

  addWidget(numRingsWidget);
  addApplyButton(document);

  m_notifierConnection += m_parameters.parametersDidChangeNotifier.connect([=, this]() {
    numRingsWidget->setVisible(
      !std::holds_alternative<mdl::ScalableCircle>(m_parameters.circleShape()));
    numRingsBox->setValue(int(m_parameters.numRings()));
  });
}

DrawShapeToolUVSphereExtension::DrawShapeToolUVSphereExtension(MapDocument& document)
  : DrawShapeToolExtension{document}
{
}

const std::string& DrawShapeToolUVSphereExtension::name() const
{
  static const auto name = std::string{"Spheroid (UV)"};
  return name;
}

const std::filesystem::path& DrawShapeToolUVSphereExtension::iconPath() const
{
  static const auto path = std::filesystem::path{"ShapeTool_UVSphere.svg"};
  return path;
}

DrawShapeToolExtensionPage* DrawShapeToolUVSphereExtension::createToolPage(
  ShapeParameters& parameters, QWidget* parent)
{
  return new DrawShapeToolUVSphereShapeExtensionPage{m_document, parameters, parent};
}

Result<std::vector<mdl::Brush>> DrawShapeToolUVSphereExtension::createBrushes(
  const vm::bbox3d& bounds, const ShapeParameters& parameters) const
{
  auto& map = m_document.map();

  const auto builder = mdl::BrushBuilder{
    map.worldNode().mapFormat(),
    map.worldBounds(),
    map.gameInfo().gameConfig.faceAttribsConfig.defaults};
  return builder
    .createUVSphere(
      bounds,
      parameters.circleShape(),
      parameters.numRings(),
      parameters.axis(),
      map.currentMaterialName())
    .transform([](auto brush) { return std::vector{std::move(brush)}; });
}

DrawShapeToolStairsExtensionPage::DrawShapeToolStairsExtensionPage(
  MapDocument& document, ShapeParameters& parameters, QWidget* parent)
  : DrawShapeToolExtensionPage{parent}
  , m_parameters{parameters}
{
  auto* stepHeightLabel = new QLabel{tr("Step Height: ")};
  auto* stepHeightBox = new QDoubleSpinBox{};
  stepHeightBox->setRange(1.0, 1024.0);
  stepHeightBox->setDecimals(0);

  auto* directionLabel = new QLabel{tr("Direction: ")};
  auto* directionComboBox = new QComboBox{};
  directionComboBox->addItems({tr("+X"), tr("-X"), tr("+Y"), tr("-Y")});

  connect(
    stepHeightBox,
    QOverload<double>::of(&QDoubleSpinBox::valueChanged),
    this,
    [&](const auto stepHeight) { m_parameters.setStepHeight(stepHeight); });

  connect(
    directionComboBox,
    QOverload<int>::of(&QComboBox::currentIndexChanged),
    this,
    [&](const auto index) {
      m_parameters.setStairDirection(indexToStairDirection(size_t(index)));
    });

  addWidget(stepHeightLabel);
  addWidget(stepHeightBox);
  addWidget(directionLabel);
  addWidget(directionComboBox);
  addApplyButton(document);

  m_notifierConnection += m_parameters.parametersDidChangeNotifier.connect([=, this]() {
    const auto direction = m_parameters.stairDirection();
    stepHeightBox->setValue(std::max(1.0, std::abs(m_parameters.stepHeight())));
    directionComboBox->setCurrentIndex(int(stairDirectionToIndex(direction)));
  });
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

DrawShapeToolExtensionPage* DrawShapeToolStairsExtension::createToolPage(
  ShapeParameters& parameters, QWidget* parent)
{
  return new DrawShapeToolStairsExtensionPage{m_document, parameters, parent};
}

Result<std::vector<mdl::Brush>> DrawShapeToolStairsExtension::createBrushes(
  const vm::bbox3d& bounds, const ShapeParameters& parameters) const
{
  auto& map = m_document.map();

  const auto builder = mdl::BrushBuilder{
    map.worldNode().mapFormat(),
    map.worldBounds(),
    map.gameInfo().gameConfig.faceAttribsConfig.defaults};

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

std::vector<std::unique_ptr<DrawShapeToolExtension>> createDrawShapeToolExtensions(
  MapDocument& document)
{
  auto result = std::vector<std::unique_ptr<DrawShapeToolExtension>>{};
  result.push_back(std::make_unique<DrawShapeToolCuboidExtension>(document));
  result.push_back(std::make_unique<DrawShapeToolStairsExtension>(document));
  result.push_back(std::make_unique<DrawShapeToolCylinderExtension>(document));
  result.push_back(std::make_unique<DrawShapeToolConeExtension>(document));
  result.push_back(std::make_unique<DrawShapeToolUVSphereExtension>(document));
  result.push_back(std::make_unique<DrawShapeToolIcoSphereExtension>(document));
  return result;
}

} // namespace tb::ui
