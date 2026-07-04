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

#include "ui/PopupButton.h"

#include <QHBoxLayout>
#include <QToolButton>
#include <QWindow>

#include "ui/PopupWindow.h"

namespace tb::ui
{

PopupButton::PopupButton(const QString& caption, QWidget* parent)
  : QWidget{parent}
  , m_button{new QToolButton()}
  , m_window{new PopupWindow{this}}
{
  m_button->setText(caption);
  m_button->setCheckable(true);

  auto* sizer = new QHBoxLayout();
  sizer->setContentsMargins(0, 0, 0, 0);
  sizer->addWidget(m_button);
  setLayout(sizer);

  connect(m_button, &QAbstractButton::clicked, this, &PopupButton::buttonClicked);
  connect(
    m_window,
    &PopupWindow::visibilityChanged,
    this,
    &PopupButton::popupVisibilityChanged);
}

QWidget* PopupButton::GetPopupWindow() const
{
  return m_window;
}

void PopupButton::buttonClicked(bool checked)
{
  if (checked)
  {
    // Compute the popup's size and position it before showing it. Under Wayland,
    // top-level (and popup) windows cannot be moved to an arbitrary screen position
    // once shown, so showing first and repositioning afterwards is unreliable there.
    m_window->adjustSize();
    m_window->positionTouchingWidget(this);
    m_window->show();
    m_window->raise();
    m_window->activateWindow();
  }
  else
  {
    m_window->close();
  }
}

void PopupButton::popupVisibilityChanged(bool visible)
{
  m_button->setChecked(visible);
}

} // namespace tb::ui
