/*
 Copyright (C) 2024 Kristian Duske

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

#include "ClickableTitleBar.h"

#include <QLabel>
#include <QLayout>

#include "View/BorderLine.h"
#include "View/QtUtils.h"
#include "View/ViewConstants.h"

namespace TrenchBroom::View
{

ClickableTitleBar::ClickableTitleBar(
  const QString& title, const QString& stateText, QWidget* parent)
  : TitleBar{title, parent, LayoutConstants::NarrowHMargin, LayoutConstants::NarrowVMargin, true}
  , m_stateText{new QLabel{stateText}}
{
  m_stateText->setFont(m_titleLabel->font());
  makeInfo(m_stateText);

  layout()->addWidget(m_stateText);
}

void ClickableTitleBar::setStateText(const QString& stateText)
{
  m_stateText->setText(stateText);
}

void ClickableTitleBar::mousePressEvent(QMouseEvent* /* event */)
{
  emit titleBarClicked();
}

} // namespace TrenchBroom::View
