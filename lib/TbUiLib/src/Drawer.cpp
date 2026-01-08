/*
 Copyright (C) 2026 Kristian Duske

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

#include "ui/Drawer.h"

#include <QHBoxLayout>
#include <QPaintEvent>
#include <QPainter>
#include <QPropertyAnimation>

namespace tb::ui
{

DrawerEventFilter::DrawerEventFilter(Drawer* drawer)
  : QObject{drawer}
  , m_drawer{drawer}
{
  m_drawer->parent()->installEventFilter(this);
}

bool DrawerEventFilter::eventFilter(QObject* watched, QEvent* event)
{
  if (watched == m_drawer->parent())
  {
    if (event->type() == QEvent::Resize)
    {
      m_drawer->updateGeometry();
    }
    else if (event->type() == QEvent::Leave)
    {
      m_drawer->close();
    }
  }
  return QObject::eventFilter(watched, event);
}

DrawerHandle::DrawerHandle(QWidget* parent)
  : QWidget{parent}
{
}

QSize DrawerHandle::sizeHint() const
{
  return {6, 6};
}

void DrawerHandle::paintEvent(QPaintEvent* event)
{
  const auto rect = event->rect();

  auto painter = QPainter{this};
  painter.setPen(Qt::NoPen);
  painter.fillRect(rect, QBrush{palette().color(QPalette::Mid)});

  // const auto knob = QRect{rect.center() - QPoint{20, 20}, QSize{40, 40}};
  // painter.fillRect(
  //   knob.intersected(rect.adjusted(+1, +1, -1, -1)),
  //   QBrush{palette().color(QPalette::Midlight)});
}

Drawer::Drawer(QWidget* child, QWidget* parent)
  : QWidget{parent}
  , m_handle{new DrawerHandle{this}}
  , m_child{child}
{
  new DrawerEventFilter{this};

  auto* layout = new QHBoxLayout{};
  layout->setContentsMargins(QMargins{});
  layout->setSpacing(0);
  layout->addWidget(m_handle);
  layout->addWidget(m_child, 1);
  setLayout(layout);

  updateGeometry();
}

void Drawer::updateGeometry()
{
  const auto hw = 6;
  const auto cw = m_child->width();
  const auto w = hw + cw;

  if (m_open)
  {
    const auto x = parentWidget()->width() - w;
    const auto h = parentWidget()->height();

    setGeometry(QRect{QPoint{x, 0}, QSize{w, h}});
  }
  else
  {
    const auto x = parentWidget()->width() - hw;
    const auto h = parentWidget()->height();

    setGeometry(QRect{QPoint{x, 0}, QSize{w, h}});
  }
}

void Drawer::open()
{
  m_open = true;
  updateGeometry();
}

void Drawer::close()
{
  m_open = false;
  updateGeometry();
}

void Drawer::enterEvent(QEnterEvent*)
{
  open();
}

void Drawer::leaveEvent(QEvent*) {}

} // namespace tb::ui
