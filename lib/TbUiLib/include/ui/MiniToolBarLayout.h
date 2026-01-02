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

#include <QBoxLayout>

#include "ui/ViewConstants.h"

class QWidget;

namespace tb::ui
{

void addToMiniToolBarLayout(QBoxLayout* layout);

template <typename... Rest>
void addToMiniToolBarLayout(QBoxLayout* layout, int first, Rest... rest);

template <typename... Rest>
void addToMiniToolBarLayout(QBoxLayout* layout, QWidget* first, Rest... rest)
{
  layout->addWidget(first);
  addToMiniToolBarLayout(layout, rest...);
}

template <typename... Rest>
void addToMiniToolBarLayout(QBoxLayout* layout, int first, Rest... rest)
{
  layout->addSpacing(first - LayoutConstants::NarrowHMargin);
  addToMiniToolBarLayout(layout, rest...);
}

template <typename... Rest>
QLayout* createMiniToolBarLayout(QWidget* first, Rest... rest)
{
  auto* layout = new QHBoxLayout{};
  layout->setContentsMargins(
    LayoutConstants::NarrowHMargin, 0, LayoutConstants::NarrowHMargin, 0);
  layout->setSpacing(LayoutConstants::NarrowHMargin);
  addToMiniToolBarLayout(layout, first, rest...);
  layout->addStretch(1);
  return layout;
}

} // namespace tb::ui
