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

#pragma once

#include <QWidget>

class QEnterEvent;
class QEvent;
class QObject;
class QPaintEvent;

namespace tb::ui
{

class Drawer;

class DrawerEventFilter : public QObject
{
  Q_OBJECT
private:
  Drawer* m_drawer;

public:
  explicit DrawerEventFilter(Drawer* drawer);

  bool eventFilter(QObject* watched, QEvent* event) override;
};

class DrawerHandle : public QWidget
{
  Q_OBJECT
public:
  explicit DrawerHandle(QWidget* parent = nullptr);

  QSize sizeHint() const override;

protected:
  void paintEvent(QPaintEvent* event) override;
};

class Drawer : public QWidget
{
  Q_OBJECT
private:
  DrawerHandle* m_handle = nullptr;
  QWidget* m_child = nullptr;
  bool m_open = false;

public:
  explicit Drawer(QWidget* child, QWidget* parent = nullptr);

  void updateGeometry();

  void open();
  void close();

protected:
  void enterEvent(QEnterEvent* event) override;
  void leaveEvent(QEvent* event) override;
};

} // namespace tb::ui
