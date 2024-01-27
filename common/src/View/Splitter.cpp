/*
 Copyright (C) 2010-2014 Kristian Duske

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

#include "Splitter.h"

#include <QDebug>
#include <QPaintEvent>
#include <QPainter>

namespace TrenchBroom::View
{

SplitterHandle::SplitterHandle(
  const Qt::Orientation orientation, const DrawKnob drawKnob, QSplitter* parent)
  : QSplitterHandle{orientation, parent}
  , m_drawKnob{drawKnob == DrawKnob::Yes}
{
}

QSize SplitterHandle::sizeHint() const
{
  return {6, 6};
}

void SplitterHandle::paintEvent(QPaintEvent* event)
{
  const auto rect = event->rect();

  auto painter = QPainter{this};
  painter.setPen(Qt::NoPen);
  painter.fillRect(rect, QBrush{palette().color(QPalette::Mid)});

  if (m_drawKnob)
  {
    const auto knob = QRect{rect.center() - QPoint{20, 20}, QSize{40, 40}};
    painter.fillRect(
      knob.intersected(rect.adjusted(+1, +1, -1, -1)),
      QBrush{palette().color(QPalette::Midlight)});
  }
}

Splitter::Splitter(
  const Qt::Orientation orientation, const DrawKnob drawKnob, QWidget* parent)
  : QSplitter{orientation, parent}
  , m_drawKnob{drawKnob}
{
#ifdef __APPLE__
  connect(this, &QSplitter::splitterMoved, this, &Splitter::doSplitterMoved);
#endif
}

Splitter::Splitter(const Qt::Orientation orientation, QWidget* parent)
  : Splitter{orientation, DrawKnob::Yes, parent}
{
}

Splitter::Splitter(const DrawKnob drawKnob, QWidget* parent)
  : QSplitter{parent}
  , m_drawKnob{drawKnob}
{
#ifdef __APPLE__
  connect(this, &QSplitter::splitterMoved, this, &Splitter::doSplitterMoved);
#endif
}

Splitter::Splitter(QWidget* parent)
  : Splitter{DrawKnob::Yes, parent}
{
}

QSplitterHandle* Splitter::createHandle()
{
  return new SplitterHandle{orientation(), m_drawKnob, this};
}

#ifdef __APPLE__
void Splitter::doSplitterMoved()
{
  for (int i = 0; i < count(); ++i)
  {
    auto* widget = this->widget(i);
    widget->repaint();
  }
}
#endif

} // namespace TrenchBroom::View
