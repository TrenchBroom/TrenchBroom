/*
 Copyright (C) 2026 Jackson Palmer

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

#include "ui/SweepToolPage.h"

#include <QCheckBox>
#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSignalBlocker>
#include <QSpinBox>

#include "ui/BorderLine.h"
#include "ui/SweepTool.h"
#include "ui/ViewConstants.h"

namespace tb::ui
{

SweepToolPage::SweepToolPage(SweepTool& tool, QWidget* parent)
  : QWidget{parent}
  , m_tool{tool}
{
  createGui();

  const auto& parameters = m_tool.parameters();
  m_segments->setValue(int(parameters.segments));
  m_iterations->setValue(int(parameters.iterations));
  m_pathMode->setCurrentIndex(m_pathMode->findData(int(parameters.pathMode)));
  m_snapToInteger->setChecked(parameters.alignment == SweepAlignment::Integer);
}

void SweepToolPage::createGui()
{
  auto* segmentsText = new QLabel{tr("Segments")};
  m_segments = new QSpinBox{this};
  m_segments->setRange(1, 64);
  m_segments->setToolTip(
    tr("Number of brushes between the source faces and the destination cap"));

  auto* pathText = new QLabel{tr("Path")};
  m_pathMode = new QComboBox{};
  m_pathMode->addItem(tr("Arc"), int(SweepPathMode::Arc));
  m_pathMode->addItem(tr("Straight"), int(SweepPathMode::Straight));
  m_pathMode->addItem(tr("S-bend"), int(SweepPathMode::SBend));
  m_pathMode->setToolTip(
    tr("Arc revolves around an axis, Straight is a linear loft, S-bend follows an "
       "S-curve; the destination cap ends up in the same place in each mode"));

  auto* iterationsText = new QLabel{tr("Iterations")};
  m_iterations = new QSpinBox{this};
  m_iterations->setRange(1, 8);
  m_iterations->setToolTip(
    tr("Repeats the sweep, continuing from the previous destination cap"));

  m_snapToInteger = new QCheckBox{tr("Snap to integer grid")};
  m_snapToInteger->setToolTip(tr("Round generated vertices to integer coordinates"));

  m_resetButton = new QPushButton{tr("Reset")};
  m_resetButton->setToolTip(tr("Move the destination cap back onto the selected faces"));

  connect(
    m_segments,
    QOverload<int>::of(&QSpinBox::valueChanged),
    this,
    &SweepToolPage::segmentsChanged);
  connect(
    m_pathMode,
    QOverload<int>::of(&QComboBox::currentIndexChanged),
    this,
    &SweepToolPage::pathModeChanged);
  connect(
    m_iterations,
    QOverload<int>::of(&QSpinBox::valueChanged),
    this,
    &SweepToolPage::iterationsChanged);
  connect(
    m_snapToInteger, &QCheckBox::toggled, this, &SweepToolPage::snapToIntegerChanged);
  connect(m_resetButton, &QAbstractButton::clicked, this, &SweepToolPage::resetClicked);

  auto* layout = new QHBoxLayout{};
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(0);

  layout->addWidget(segmentsText, 0, Qt::AlignVCenter);
  layout->addSpacing(LayoutConstants::NarrowHMargin);
  layout->addWidget(m_segments, 0, Qt::AlignVCenter);
  layout->addSpacing(LayoutConstants::WideHMargin);
  layout->addWidget(pathText, 0, Qt::AlignVCenter);
  layout->addSpacing(LayoutConstants::NarrowHMargin);
  layout->addWidget(m_pathMode, 0, Qt::AlignVCenter);
  layout->addSpacing(LayoutConstants::WideHMargin);
  layout->addWidget(iterationsText, 0, Qt::AlignVCenter);
  layout->addSpacing(LayoutConstants::NarrowHMargin);
  layout->addWidget(m_iterations, 0, Qt::AlignVCenter);
  layout->addSpacing(LayoutConstants::WideHMargin);
  layout->addWidget(new BorderLine{BorderLine::Direction::Vertical}, 0);
  layout->addSpacing(LayoutConstants::WideHMargin);
  layout->addWidget(m_snapToInteger, 0, Qt::AlignVCenter);
  layout->addSpacing(LayoutConstants::WideHMargin);
  layout->addWidget(m_resetButton, 0, Qt::AlignVCenter);
  layout->addStretch(1);

  setLayout(layout);
}

void SweepToolPage::segmentsChanged(const int value)
{
  if (value >= 0)
  {
    auto parameters = m_tool.parameters();
    parameters.segments = size_t(value);
    m_tool.setParameters(parameters);
  }
}

void SweepToolPage::pathModeChanged(const int index)
{
  if (index >= 0)
  {
    auto parameters = m_tool.parameters();
    parameters.pathMode = SweepPathMode(m_pathMode->itemData(index).toInt());
    m_tool.setParameters(parameters);
  }
}

void SweepToolPage::iterationsChanged(const int value)
{
  if (value >= 0)
  {
    auto parameters = m_tool.parameters();
    parameters.iterations = size_t(value);
    m_tool.setParameters(parameters);
  }
}

void SweepToolPage::snapToIntegerChanged(const bool checked)
{
  auto parameters = m_tool.parameters();
  parameters.alignment = checked ? SweepAlignment::Integer : SweepAlignment::Free;
  m_tool.setParameters(parameters);
}

void SweepToolPage::resetClicked()
{
  m_tool.reset();
}

} // namespace tb::ui
