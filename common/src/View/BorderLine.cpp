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

#include "BorderLine.h"

#include "View/ViewConstants.h"

namespace tb::View
{

BorderLine::BorderLine(const Direction direction, const int thickness, QWidget* parent)
  : QFrame{parent}
{
  setObjectName("borderLine");
  setContentsMargins(0, 0, 0, 0);
  setFrameShadow(QFrame::Plain);
  setForegroundRole(QPalette::Mid);
  setLineWidth(thickness - 1);
  if (direction == Direction::Horizontal)
  {
    setFrameShape(QFrame::HLine);
#if !defined __APPLE__
    // necessary to remove extra space around the horizontal line
    setFixedHeight(thickness);
#endif
  }
  else
  {
    setFrameShape(QFrame::VLine);
#if !defined __APPLE__
    // this makes the vertical line disappear on macOS
    setFixedWidth(thickness);
#endif
  }
}

} // namespace tb::View
