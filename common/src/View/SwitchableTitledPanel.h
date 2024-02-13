/*
 Copyright (C) 2010-2017 Kristian Duske

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

#include <array>

class QStackedLayout;

namespace TrenchBroom::View
{
class BorderLine;
class ClickableTitleBar;

class SwitchableTitledPanel : public QWidget
{
  Q_OBJECT
private:
  struct SwitchablePanel
  {
    QWidget* panel = nullptr;
    QString stateText;
  };

  ClickableTitleBar* m_titleBar = nullptr;
  BorderLine* m_divider = nullptr;
  QStackedLayout* m_stackedLayout = nullptr;
  std::array<SwitchablePanel, 2> m_panels;

public:
  explicit SwitchableTitledPanel(
    const QString& title,
    const std::array<QString, 2>& stateTexts,
    QWidget* parent = nullptr);

  QWidget* getPanel(size_t index) const;

  size_t currentIndex() const;
  void setCurrentIndex(size_t index);

  QByteArray saveState() const;
  bool restoreState(const QByteArray& state);
};

} // namespace TrenchBroom::View
