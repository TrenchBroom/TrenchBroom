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

#include "ui/DialogButtonLayout.h"

#include <QBoxLayout>

#include "ui/BorderLine.h"
#include "ui/ViewConstants.h"

namespace tb::ui
{

QLayout* wrapDialogButtonBox(QWidget* buttonBox)
{
  auto* innerLayout = new QHBoxLayout{};
  innerLayout->setContentsMargins(
    LayoutConstants::DialogButtonLeftMargin,
    LayoutConstants::DialogButtonTopMargin,
    LayoutConstants::DialogButtonRightMargin,
    LayoutConstants::DialogButtonBottomMargin);
  innerLayout->setSpacing(0);
  innerLayout->addWidget(buttonBox);

  auto* outerLayout = new QVBoxLayout{};
  outerLayout->setContentsMargins(QMargins{});
  outerLayout->setSpacing(0);
  outerLayout->addWidget(new BorderLine{});
  outerLayout->addLayout(innerLayout);

  return outerLayout;
}

QLayout* wrapDialogButtonBox(QLayout* buttonBox)
{
  auto* innerLayout = new QHBoxLayout{};
  innerLayout->setContentsMargins(
    LayoutConstants::DialogButtonLeftMargin,
    LayoutConstants::DialogButtonTopMargin,
    LayoutConstants::DialogButtonRightMargin,
    LayoutConstants::DialogButtonBottomMargin);
  innerLayout->setSpacing(0);
  innerLayout->addLayout(buttonBox);

  auto* outerLayout = new QVBoxLayout{};
  outerLayout->setContentsMargins(QMargins{});
  outerLayout->setSpacing(0);
  outerLayout->addWidget(new BorderLine{});
  outerLayout->addLayout(innerLayout);

  return outerLayout;
}

} // namespace tb::ui
