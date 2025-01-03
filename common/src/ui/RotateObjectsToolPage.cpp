/*
 Copyright (C) 2010 Kristian Duske

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

#include "RotateObjectsToolPage.h"

#include <QCheckBox>
#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QtGlobal>

#include "mdl/EntityProperties.h"
#include "mdl/WorldNode.h"
#include "ui/BorderLine.h"
#include "ui/Grid.h"
#include "ui/MapDocument.h"
#include "ui/RotateObjectsTool.h"
#include "ui/SpinControl.h"
#include "ui/ViewConstants.h"

#include "kdl/memory_utils.h"
#include "kdl/range_to.h"

#include "vm/vec_io.h"

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <ranges>

namespace tb::ui
{

RotateObjectsToolPage::RotateObjectsToolPage(
  std::weak_ptr<MapDocument> document, RotateObjectsTool& tool, QWidget* parent)
  : QWidget{parent}
  , m_document{std::move(document)}
  , m_tool{tool}
{
  createGui();
  connectObservers();
  m_angle->setValue(vm::to_degrees(m_tool.angle()));
}

void RotateObjectsToolPage::connectObservers()
{
  auto document = kdl::mem_lock(m_document);
  m_notifierConnection += document->selectionDidChangeNotifier.connect(
    this, &RotateObjectsToolPage::selectionDidChange);
  m_notifierConnection += document->documentWasNewedNotifier.connect(
    this, &RotateObjectsToolPage::documentWasNewedOrLoaded);
  m_notifierConnection += document->documentWasLoadedNotifier.connect(
    this, &RotateObjectsToolPage::documentWasNewedOrLoaded);

  m_notifierConnection += m_tool.rotationCenterDidChangeNotifier.connect(
    this, &RotateObjectsToolPage::rotationCenterDidChange);
  m_notifierConnection += m_tool.rotationCenterWasUsedNotifier.connect(
    this, &RotateObjectsToolPage::rotationCenterWasUsed);
  m_notifierConnection += m_tool.handleHitAreaDidChangeNotifier.connect(
    this, &RotateObjectsToolPage::handleHitAreaDidChange);
}

void RotateObjectsToolPage::createGui()
{
  auto* centerText = new QLabel{tr("Center")};
  m_recentlyUsedCentersList = new QComboBox{};
  m_recentlyUsedCentersList->setMinimumContentsLength(16);
  m_recentlyUsedCentersList->setEditable(true);

  m_resetCenterButton = new QPushButton{tr("Reset")};
  m_resetCenterButton->setToolTip(tr(
    "Reset the position of the rotate handle to the center of the current selection."));

  auto* text1 = new QLabel{tr("Rotate objects")};
  auto* text2 = new QLabel{tr("degs about")};
  auto* text3 = new QLabel{tr("axis")};
  m_angle = new SpinControl{this};
  m_angle->setRange(-360.0, 360.0);
  m_angle->setValue(vm::to_degrees(m_tool.angle()));

  m_axis = new QComboBox{};
  m_axis->addItem("X");
  m_axis->addItem("Y");
  m_axis->addItem("Z");
  m_axis->setCurrentIndex(2);

  m_rotateButton = new QPushButton{tr("Apply")};

  m_updateAnglePropertyAfterTransformCheckBox =
    new QCheckBox{tr("Update entity properties")};

  connect(
    m_recentlyUsedCentersList,
    &QComboBox::textActivated,
    this,
    &RotateObjectsToolPage::centerChanged);
  connect(
    m_resetCenterButton,
    &QAbstractButton::clicked,
    this,
    &RotateObjectsToolPage::resetCenterClicked);
  connect(
    m_angle,
    QOverload<double>::of(&QDoubleSpinBox::valueChanged),
    this,
    &RotateObjectsToolPage::angleChanged);
  connect(
    m_rotateButton,
    &QAbstractButton::clicked,
    this,
    &RotateObjectsToolPage::rotateClicked);
  connect(
    m_updateAnglePropertyAfterTransformCheckBox,
    &QCheckBox::clicked,
    this,
    &RotateObjectsToolPage::updateAnglePropertyAfterTransformClicked);

  auto* layout = new QHBoxLayout{};
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(0);

  layout->addWidget(centerText, 0, Qt::AlignVCenter);
  layout->addSpacing(LayoutConstants::MediumHMargin);
  layout->addWidget(m_recentlyUsedCentersList, 0, Qt::AlignVCenter);
  layout->addSpacing(LayoutConstants::MediumHMargin);
  layout->addWidget(m_resetCenterButton, 0, Qt::AlignVCenter);
  layout->addSpacing(LayoutConstants::WideHMargin);
  layout->addWidget(new BorderLine{BorderLine::Direction::Vertical}, 0);
  layout->addSpacing(LayoutConstants::WideHMargin);
  layout->addWidget(text1, 0, Qt::AlignVCenter);
  layout->addSpacing(LayoutConstants::NarrowHMargin);
  layout->addWidget(m_angle, 0, Qt::AlignVCenter);
  layout->addSpacing(LayoutConstants::NarrowHMargin);
  layout->addWidget(text2, 0, Qt::AlignVCenter);
  layout->addSpacing(LayoutConstants::NarrowHMargin);
  layout->addWidget(m_axis, 0, Qt::AlignVCenter);
  layout->addSpacing(LayoutConstants::NarrowHMargin);
  layout->addWidget(text3, 0, Qt::AlignVCenter);
  layout->addSpacing(LayoutConstants::NarrowHMargin);
  layout->addWidget(m_rotateButton, 0, Qt::AlignVCenter);
  layout->addSpacing(LayoutConstants::WideHMargin);
  layout->addWidget(new BorderLine{BorderLine::Direction::Vertical}, 0);
  layout->addSpacing(LayoutConstants::WideHMargin);
  layout->addWidget(m_updateAnglePropertyAfterTransformCheckBox);
  layout->addStretch(1);

  setLayout(layout);

  updateGui();
}

void RotateObjectsToolPage::updateGui()
{
  const auto& grid = kdl::mem_lock(m_document)->grid();
  m_angle->setIncrements(vm::to_degrees(grid.angle()), 90.0, 1.0);

  auto document = kdl::mem_lock(m_document);
  m_rotateButton->setEnabled(document->hasSelectedNodes());

  if (const auto* worldNode = document->world())
  {
    m_updateAnglePropertyAfterTransformCheckBox->setChecked(
      worldNode->entityPropertyConfig().updateAnglePropertyAfterTransform);
  }
}

void RotateObjectsToolPage::selectionDidChange(const Selection&)
{
  updateGui();
}

void RotateObjectsToolPage::documentWasNewedOrLoaded(MapDocument*)
{
  updateGui();
}

void RotateObjectsToolPage::rotationCenterDidChange(const vm::vec3d& center)
{
  m_recentlyUsedCentersList->setCurrentText(
    QString::fromStdString(fmt::format("{}", fmt::streamed(center))));
}

void RotateObjectsToolPage::rotationCenterWasUsed(const vm::vec3d& center)
{
  std::erase_if(m_recentlyUsedCenters, [&](const auto& c) { return c == center; });
  m_recentlyUsedCenters.push_back(center);

  m_recentlyUsedCentersList->clear();
  m_recentlyUsedCentersList->addItems(
    m_recentlyUsedCenters | std::views::reverse
    | std::views::transform([](const auto& c) {
        return QString::fromStdString(fmt::format("{}", fmt::streamed(c)));
      })
    | kdl::to<QStringList>());

  if (m_recentlyUsedCentersList->count() > 0)
  {
    m_recentlyUsedCentersList->setCurrentIndex(0);
  }
}

void RotateObjectsToolPage::handleHitAreaDidChange(
  const RotateObjectsHandle::HitArea area)
{
  if (area == RotateObjectsHandle::HitArea::XAxis)
  {
    m_axis->setCurrentIndex(0);
  }
  else if (area == RotateObjectsHandle::HitArea::YAxis)
  {
    m_axis->setCurrentIndex(1);
  }
  else if (area == RotateObjectsHandle::HitArea::ZAxis)
  {
    m_axis->setCurrentIndex(2);
  }
}

void RotateObjectsToolPage::centerChanged()
{
  if (
    const auto center =
      vm::parse<double, 3>(m_recentlyUsedCentersList->currentText().toStdString()))
  {
    m_tool.setRotationCenter(*center);
  }
}

void RotateObjectsToolPage::resetCenterClicked()
{
  m_tool.resetRotationCenter();
}

void RotateObjectsToolPage::angleChanged(double value)
{
  const auto newAngleDegs = vm::correct(value);
  m_angle->setValue(newAngleDegs);
  m_tool.setAngle(vm::to_radians(newAngleDegs));
}

void RotateObjectsToolPage::rotateClicked()
{
  const auto center = m_tool.rotationCenter();
  const auto axis = getAxis();
  const auto angle = vm::to_radians(m_angle->value());

  auto document = kdl::mem_lock(m_document);
  document->rotateObjects(center, axis, angle);
}

void RotateObjectsToolPage::updateAnglePropertyAfterTransformClicked()
{
  auto document = kdl::mem_lock(m_document);
  document->world()->entityPropertyConfig().updateAnglePropertyAfterTransform =
    m_updateAnglePropertyAfterTransformCheckBox->isChecked();
}

vm::vec3d RotateObjectsToolPage::getAxis() const
{
  switch (m_axis->currentIndex())
  {
  case 0:
    return vm::vec3d{1, 0, 0};
  case 1:
    return vm::vec3d{0, 1, 0};
  default:
    return vm::vec3d{0, 0, 1};
  }
}

} // namespace tb::ui
