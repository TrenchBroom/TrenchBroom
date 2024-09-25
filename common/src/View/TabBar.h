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

#include <QWidget>

#include "View/ContainerBar.h"

#include <vector>

class QHBoxLayout;
class QLabel;
class QStackedLayout;

namespace TrenchBroom::View
{
class TabBook;
class TabBookPage;

class TabBarButton : public QWidget
{
  Q_OBJECT
private:
  QLabel* m_label = nullptr;
  QWidget* m_indicator = nullptr;
  bool m_pressed = false;

public:
  explicit TabBarButton(const QString& label = "", QWidget* parent = nullptr);
  /**
   * Update the label color
   */
  void setPressed(bool pressed);

protected:
  void mousePressEvent(QMouseEvent* event) override;

signals:
  void clicked();

private:
  void updateState();
};

class TabBar : public ContainerBar
{
  Q_OBJECT
private:
  TabBook* m_tabBook = nullptr;

  QStackedLayout* m_barBook = nullptr;
  QHBoxLayout* m_controlLayout = nullptr;
  std::vector<TabBarButton*> m_buttons;

public:
  explicit TabBar(TabBook* tabBook);

  void addTab(TabBookPage* bookPage, const QString& title);

private:
  size_t findButtonIndex(QWidget* button) const;
  void setButtonActive(int index);

  void buttonClicked();
  void tabBookPageChanged(int newIndex);
};

} // namespace TrenchBroom::View
