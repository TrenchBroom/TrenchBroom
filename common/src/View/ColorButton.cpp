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

#include "ColorButton.h"

#include "View/ViewConstants.h"

#include <QBoxLayout>
#include <QColorDialog>
#include <QPushButton>

namespace TrenchBroom
{
namespace View
{
ColorButton::ColorButton(QWidget* parent)
  : QWidget(parent)
  , m_colorIndicator(nullptr)
  , m_button(nullptr)
{
  m_colorIndicator = new QWidget();
  m_button = new QPushButton("...");

  m_colorIndicator->setSizePolicy(
    QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred));
  m_colorIndicator->setMinimumSize(20, 15);

  auto* layout = new QHBoxLayout();
  layout->setContentsMargins(QMargins());
  layout->setSpacing(LayoutConstants::MediumHMargin);
  layout->addWidget(m_colorIndicator);
  layout->addWidget(m_button);
  layout->addStretch();
  setLayout(layout);

  connect(m_button, &QPushButton::clicked, this, [this]() {
    const QColor color = QColorDialog::getColor(m_color, this);
    if (color.isValid())
    {
      setColor(color);
      emit colorChangedByUser(m_color);
    }
  });
}

void ColorButton::setColor(const QColor& color)
{
  const auto borderColor = palette().color(QPalette::Active, QPalette::Mid);
  if (color != m_color)
  {
    m_color = color;
    m_colorIndicator->setStyleSheet(
      "QWidget { background-color: " + m_color.name()
      + "; border-radius: 3px; border: 1px solid " + borderColor.name() + ";}");

    update();
    emit colorChanged(m_color);
  }
}
} // namespace View
} // namespace TrenchBroom
