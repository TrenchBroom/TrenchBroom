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

class QStackedLayout;

namespace tb::ui
{
class TabBar;

class TabBookPage : public QWidget
{
  Q_OBJECT
public:
  explicit TabBookPage(QWidget* parent = nullptr);
  ~TabBookPage() override;
  virtual QWidget* createTabBarPage(QWidget* parent = nullptr);
};

class TabBook : public QWidget
{
  Q_OBJECT
private:
  TabBar* m_tabBar = nullptr;
  QStackedLayout* m_tabBook = nullptr;

public:
  explicit TabBook(QWidget* parent = nullptr);

  TabBar* tabBar();

  void addPage(TabBookPage* page, const QString& title);
  void switchToPage(int index);

  QByteArray saveState() const;
  bool restoreState(const QByteArray& state);
signals:
  void pageChanged(int page);
};

} // namespace tb::ui
