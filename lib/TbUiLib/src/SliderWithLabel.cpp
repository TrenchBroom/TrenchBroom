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

#include "ui/SliderWithLabel.h"

#include <QBoxLayout>
#include <QLabel>
#include <QSlider>

#include "ui/ViewConstants.h"

#include <cmath>

namespace tb::ui
{

SliderWithLabel::SliderWithLabel(const int minimum, const int maximum, QWidget* parent)
  : QWidget{parent}
  , m_slider{new QSlider{}}
  , m_label{new QLabel{}}
{
  m_slider->setMinimum(minimum);
  m_slider->setMaximum(maximum);
  m_slider->setTickPosition(QSlider::TicksBelow);
  m_slider->setTracking(true);
  m_slider->setOrientation(Qt::Horizontal);

  const auto maxDigits = int(std::log10(m_slider->maximum())) + 1;
  const auto str = QString{""}.fill('9', maxDigits);
  const auto rect = m_label->fontMetrics().boundingRect(str);
  const auto width = rect.width() + 1;
  m_label->setMinimumWidth(width);
  m_label->setAlignment(Qt::AlignRight);
  m_label->setText(QString::number(m_slider->value()));

  auto* layout = new QHBoxLayout{};
  layout->setContentsMargins(QMargins{});
  layout->setSpacing(LayoutConstants::MediumHMargin);
  layout->addWidget(m_slider, 1);
  layout->addWidget(m_label);
  setLayout(layout);

  connect(m_slider, &QSlider::valueChanged, this, &SliderWithLabel::valueChangedInternal);
}

int SliderWithLabel::value() const
{
  return m_slider->value();
}

float SliderWithLabel::ratio() const
{
  return float(m_slider->value() - m_slider->minimum())
         / float(m_slider->maximum() - m_slider->minimum());
}

void SliderWithLabel::setValue(const int value)
{
  m_slider->setValue(value);
}

void SliderWithLabel::setRatio(const float ratio)
{
  const auto value =
    ratio * float(m_slider->maximum() - m_slider->minimum()) + float(m_slider->minimum());
  m_slider->setValue(int(value));
}

void SliderWithLabel::valueChangedInternal(const int value)
{
  m_label->setText(QString::number(value));
  emit valueChanged(value);
}

} // namespace tb::ui
