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

namespace TrenchBroom
{
namespace View
{
SplitterHandle::SplitterHandle(const Qt::Orientation orientation, QSplitter* parent)
  : QSplitterHandle(orientation, parent)
{
}

QSize SplitterHandle::sizeHint() const
{
  return QSize(3, 3);
}

void SplitterHandle::paintEvent(QPaintEvent* event)
{
  QPainter painter(this);
  painter.setPen(Qt::NoPen);
  painter.fillRect(event->rect(), QBrush(palette().color(QPalette::Mid)));
}

Splitter::Splitter(const Qt::Orientation orientation, QWidget* parent)
  : QSplitter(orientation, parent)
{
#ifdef __APPLE__
  connect(this, &QSplitter::splitterMoved, this, &Splitter::doSplitterMoved);
#endif
}

Splitter::Splitter(QWidget* parent)
  : QSplitter(parent)
{
#ifdef __APPLE__
  connect(this, &QSplitter::splitterMoved, this, &Splitter::doSplitterMoved);
#endif
}

QSplitterHandle* Splitter::createHandle()
{
  return new SplitterHandle(orientation(), this);
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
} // namespace View
} // namespace TrenchBroom
