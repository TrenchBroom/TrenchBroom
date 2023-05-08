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

#include "TabBook.h"

#include <QStackedLayout>
#include <QVBoxLayout>

#include "Ensure.h"
#include "Macros.h"
#include "View/TabBar.h"

namespace TrenchBroom
{
namespace View
{
TabBookPage::TabBookPage(QWidget* parent)
  : QWidget(parent)
{
}
TabBookPage::~TabBookPage() {}

QWidget* TabBookPage::createTabBarPage(QWidget* parent)
{
  return new QWidget(parent);
}

TabBook::TabBook(QWidget* parent)
  : QWidget(parent)
  , m_tabBar(new TabBar(this))
{
  m_tabBook = new QStackedLayout();
  m_tabBook->setContentsMargins(0, 0, 0, 0);

  QVBoxLayout* sizer = new QVBoxLayout();
  sizer->setSpacing(0);
  sizer->setContentsMargins(0, 0, 0, 0);
  sizer->addWidget(m_tabBar, 0);
  sizer->addLayout(m_tabBook, 1);
  setLayout(sizer);

  // Forward the signal, so we don't have to expose the QStackedLayout
  connect(m_tabBook, &QStackedLayout::currentChanged, this, &TabBook::pageChanged);
}

TabBar* TabBook::tabBar()
{
  return m_tabBar;
}

void TabBook::addPage(TabBookPage* page, const QString& title)
{
  ensure(page != nullptr, "page is null");

  m_tabBar->addTab(page, title);
  m_tabBook->addWidget(page);
}

void TabBook::switchToPage(const int index)
{
  assert(index < m_tabBook->count());
  m_tabBook->setCurrentIndex(index);
}
} // namespace View
} // namespace TrenchBroom
