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

#include "ui/DrawShapeToolExtensionPages.h"

#include <QBoxLayout>
#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QSpinBox>
#include <QStackedWidget>
#include <QToolButton>

#include "ui/BitmapButton.h"
#include "ui/DrawShapeToolExtension.h"
#include "ui/DrawShapeToolExtensionKind.h"
#include "ui/DrawShapeToolParameters.h"
#include "ui/ViewConstants.h"

#include "kd/ranges/to.h"

#include <algorithm>
#include <array>
#include <ranges>

namespace tb::ui
{

namespace
{

using StairDirection = DrawShapeToolParameters::StairDirection;

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

DrawShapeToolExtensionPage* createExtensionPage(
  const DrawShapeToolExtensionKind kind,
  MapDocument& document,
  DrawShapeToolParameters& parameters,
  QWidget* parent)
{
  switch (kind)
  {
  case DrawShapeToolExtensionKind::Cuboid:
    return new DrawShapeToolExtensionPage{parent};
  case DrawShapeToolExtensionKind::Stairs:
    return new DrawShapeToolStairsExtensionPage{document, parameters, parent};
  case DrawShapeToolExtensionKind::Arch:
    return new DrawShapeToolArchShapeExtensionPage{document, parameters, parent};
  case DrawShapeToolExtensionKind::Cylinder:
    return new DrawShapeToolCylinderShapeExtensionPage{document, parameters, parent};
  case DrawShapeToolExtensionKind::Cone:
    return new DrawShapeToolConeShapeExtensionPage{document, parameters, parent};
  case DrawShapeToolExtensionKind::UvSphere:
    return new DrawShapeToolUvSphereShapeExtensionPage{document, parameters, parent};
  case DrawShapeToolExtensionKind::IcoSphere:
    return new DrawShapeToolIcoSphereShapeExtensionPage{document, parameters, parent};
  }

  return nullptr;
}

} // namespace

DrawShapeToolAxisAlignedShapeExtensionPage::DrawShapeToolAxisAlignedShapeExtensionPage(
  DrawShapeToolParameters& parameters, QWidget* parent)
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
  DrawShapeToolParameters& parameters, QWidget* parent)
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
  MapDocument& document, DrawShapeToolParameters& parameters, QWidget* parent)
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

DrawShapeToolConeShapeExtensionPage::DrawShapeToolConeShapeExtensionPage(
  MapDocument& document, DrawShapeToolParameters& parameters, QWidget* parent)
  : DrawShapeToolCircularShapeExtensionPage{parameters, parent}
  , m_parameters{parameters}
{
  addApplyButton(document);
}

DrawShapeToolIcoSphereShapeExtensionPage::DrawShapeToolIcoSphereShapeExtensionPage(
  MapDocument& document, DrawShapeToolParameters& parameters, QWidget* parent)
  : DrawShapeToolExtensionPage{parent}
  , m_parameters{parameters}
{
  auto* accuracyLabel = new QLabel{tr("Accuracy: ")};
  auto* accuracyBox = new QSpinBox{};
  accuracyBox->setRange(1, 4);

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

DrawShapeToolUvSphereShapeExtensionPage::DrawShapeToolUvSphereShapeExtensionPage(
  MapDocument& document, DrawShapeToolParameters& parameters, QWidget* parent)
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

DrawShapeToolStairsExtensionPage::DrawShapeToolStairsExtensionPage(
  MapDocument& document, DrawShapeToolParameters& parameters, QWidget* parent)
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

DrawShapeToolArchShapeExtensionPage::DrawShapeToolArchShapeExtensionPage(
  MapDocument& document, DrawShapeToolParameters& parameters, QWidget* parent)
  : DrawShapeToolCircularShapeExtensionPage{parameters, parent}
  , m_parameters{parameters}
{
  auto* thicknessLabel = new QLabel{tr("Thickness: ")};
  auto* thicknessBox = new QDoubleSpinBox{};
  thicknessBox->setRange(1, 1024);

  connect(
    thicknessBox,
    QOverload<double>::of(&QDoubleSpinBox::valueChanged),
    this,
    [&](const auto thickness) { m_parameters.setThickness(thickness); });

  addWidget(thicknessLabel);
  addWidget(thicknessBox);
  addApplyButton(document);

  m_notifierConnection += m_parameters.parametersDidChangeNotifier.connect(
    [=, this]() { thicknessBox->setValue(m_parameters.thickness()); });
}

std::vector<DrawShapeToolExtensionPage*> createDrawShapeToolExtensionPages(
  MapDocument& document, DrawShapeToolParameters& parameters, QWidget* parent)
{
  auto result = DrawShapeToolExtensionKinds | std::views::transform([&](const auto kind) {
                  return createExtensionPage(kind, document, parameters, parent);
                })
                | kdl::ranges::to<std::vector>();

  // update all pages to reflect the current parameter values
  parameters.parametersDidChangeNotifier();

  return result;
}

} // namespace tb::ui
