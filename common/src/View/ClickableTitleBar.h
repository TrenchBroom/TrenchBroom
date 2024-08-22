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

#pragma once

#include <QWidget>

#include "View/TitleBar.h"

class QLabel;

namespace TrenchBroom::View
{
class BorderLine;

class ClickableTitleBar : public TitleBar
{
  Q_OBJECT
private:
  QLabel* m_stateText = nullptr;

public:
  ClickableTitleBar(
    const QString& title, const QString& stateText, QWidget* parent = nullptr);

  void setStateText(const QString& stateText);
signals:
  void titleBarClicked();

protected:
  void mousePressEvent(QMouseEvent* event) override;
};

} // namespace TrenchBroom::View
