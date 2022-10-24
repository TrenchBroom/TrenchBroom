/*
 Copyright (C) 2010-2017 Kristian Duske

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

#include "FloatType.h"
#include "View/BorderLine.h"
#include "View/Grid.h"
#include "View/MapDocument.h"
#include "View/RotateObjectsTool.h"
#include "View/SpinControl.h"
#include "View/ViewConstants.h"

#include <kdl/memory_utils.h>
#include <kdl/string_utils.h>

#include <vecmath/util.h>
#include <vecmath/vec.h>
#include <vecmath/vec_io.h>

#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QtGlobal>

namespace TrenchBroom
{
namespace View
{
RotateObjectsToolPage::RotateObjectsToolPage(
  std::weak_ptr<MapDocument> document, RotateObjectsTool& tool, QWidget* parent)
  : QWidget(parent)
  , m_document(std::move(document))
  , m_tool(tool)
  , m_recentlyUsedCentersList(nullptr)
  , m_resetCenterButton(nullptr)
  , m_angle(nullptr)
  , m_axis(nullptr)
  , m_rotateButton(nullptr)
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
}

void RotateObjectsToolPage::setAxis(const vm::axis::type axis)
{
  m_axis->setCurrentIndex(static_cast<int>(axis));
}

void RotateObjectsToolPage::setRecentlyUsedCenters(const std::vector<vm::vec3>& centers)
{
  m_recentlyUsedCentersList->clear();

  for (auto it = centers.rbegin(), end = centers.rend(); it != end; ++it)
  {
    const auto& center = *it;
    m_recentlyUsedCentersList->addItem(
      QString::fromStdString(kdl::str_to_string(center)));
  }

  if (m_recentlyUsedCentersList->count() > 0)
    m_recentlyUsedCentersList->setCurrentIndex(0);
}

void RotateObjectsToolPage::setCurrentCenter(const vm::vec3& center)
{
  m_recentlyUsedCentersList->setCurrentText(
    QString::fromStdString(kdl::str_to_string(center)));
}

void RotateObjectsToolPage::createGui()
{
  auto* centerText = new QLabel(tr("Center"));
  m_recentlyUsedCentersList = new QComboBox();
  m_recentlyUsedCentersList->setMinimumContentsLength(16);
  m_recentlyUsedCentersList->setEditable(true);

  m_resetCenterButton = new QPushButton(tr("Reset"));
  m_resetCenterButton->setToolTip(tr(
    "Reset the position of the rotate handle to the center of the current selection."));

  auto* text1 = new QLabel(tr("Rotate objects"));
  auto* text2 = new QLabel(tr("degs about"));
  auto* text3 = new QLabel(tr("axis"));
  m_angle = new SpinControl(this);
  m_angle->setRange(-360.0, 360.0);
  m_angle->setValue(vm::to_degrees(m_tool.angle()));

  m_axis = new QComboBox();
  m_axis->addItem("X");
  m_axis->addItem("Y");
  m_axis->addItem("Z");
  m_axis->setCurrentIndex(2);

  m_rotateButton = new QPushButton(tr("Apply"));

  connect(
    m_recentlyUsedCentersList,
    QOverload<const QString&>::of(&QComboBox::activated),
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

  auto* layout = new QHBoxLayout();
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(0);

  layout->addWidget(centerText, 0, Qt::AlignVCenter);
  layout->addSpacing(LayoutConstants::MediumHMargin);
  layout->addWidget(m_recentlyUsedCentersList, 0, Qt::AlignVCenter);
  layout->addSpacing(LayoutConstants::MediumHMargin);
  layout->addWidget(m_resetCenterButton, 0, Qt::AlignVCenter);
  layout->addSpacing(LayoutConstants::WideHMargin);
  layout->addWidget(new BorderLine(BorderLine::Direction::Vertical), 0);
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
}

void RotateObjectsToolPage::selectionDidChange(const Selection&)
{
  updateGui();
}

void RotateObjectsToolPage::centerChanged()
{
  if (
    const auto center =
      vm::parse<FloatType, 3>(m_recentlyUsedCentersList->currentText().toStdString()))
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
  const double newAngleDegs = vm::correct(value);
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

vm::vec3 RotateObjectsToolPage::getAxis() const
{
  switch (m_axis->currentIndex())
  {
  case 0:
    return vm::vec3::pos_x();
  case 1:
    return vm::vec3::pos_y();
  default:
    return vm::vec3::pos_z();
  }
}
} // namespace View
} // namespace TrenchBroom
