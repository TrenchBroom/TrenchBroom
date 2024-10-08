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

#pragma once

#include <QSplitter>

namespace tb::ui
{

enum class DrawKnob
{
  Yes,
  No,
};

class SplitterHandle : public QSplitterHandle
{
  Q_OBJECT
private:
  bool m_drawKnob;

public:
  explicit SplitterHandle(
    Qt::Orientation orientation, DrawKnob drawKnob, QSplitter* parent = nullptr);

  QSize sizeHint() const override;

protected:
  void paintEvent(QPaintEvent* event) override;
};

class Splitter : public QSplitter
{
  Q_OBJECT
private:
  DrawKnob m_drawKnob = DrawKnob::Yes;

public:
  Splitter(Qt::Orientation orientation, DrawKnob drawKnob, QWidget* parent = nullptr);
  explicit Splitter(Qt::Orientation orientation, QWidget* parent = nullptr);
  explicit Splitter(DrawKnob drawKnob, QWidget* parent = nullptr);
  explicit Splitter(QWidget* parent = nullptr);

protected:
  QSplitterHandle* createHandle() override;

#ifdef __APPLE__
  // on macOS, the widgets are not repainted properly when the splitter moves, so we force
  // them to repaint
private slots:
  void doSplitterMoved();
#endif
};

} // namespace tb::ui
