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

#include "TabBar.h"

#include "Ensure.h"
#include "View/QtUtils.h"
#include "View/TabBook.h"
#include "View/ViewConstants.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QStackedLayout>

namespace TrenchBroom
{
namespace View
{
// TabBarButton

TabBarButton::TabBarButton(const QString& label, QWidget* parent)
  : QWidget(parent)
  , m_label(new QLabel(label, this))
  , m_indicator(new QWidget(this))
  , m_pressed(false)
{
  auto* labelLayout = new QHBoxLayout();
  labelLayout->setContentsMargins(
    LayoutConstants::WideHMargin, 0, LayoutConstants::WideHMargin, 0);
  labelLayout->addWidget(m_label);

  auto* outerLayout = new QVBoxLayout();
  outerLayout->setContentsMargins(0, 1, 0, 1); // needs extra vertical space!
  outerLayout->setSpacing(0);

  outerLayout->addSpacing(LayoutConstants::MediumVMargin);
  outerLayout->addSpacing(LayoutConstants::NarrowVMargin);
  outerLayout->addLayout(labelLayout);
  outerLayout->addSpacing(LayoutConstants::NarrowVMargin);
  outerLayout->addWidget(m_indicator);

  makeEmphasized(m_label);
  m_indicator->setFixedHeight(LayoutConstants::MediumVMargin);
  m_indicator->setAutoFillBackground(true);

  setLayout(outerLayout);
}

void TabBarButton::setPressed(const bool pressed)
{
  m_pressed = pressed;
  updateState();
}

void TabBarButton::mousePressEvent(QMouseEvent*)
{
  emit clicked();
}

void TabBarButton::updateState()
{
  QPalette pal;
  if (m_pressed)
  {
    m_indicator->setBackgroundRole(QPalette::Highlight);
  }
  else
  {
    m_indicator->setBackgroundRole(QPalette::NoRole);
  }
}

// TabBar

TabBar::TabBar(TabBook* tabBook)
  : ContainerBar(BorderPanel::BottomSide, tabBook)
  , m_tabBook(tabBook)
  , m_barBook(new QStackedLayout())
{
  ensure(m_tabBook != nullptr, "tabBook is null");
  connect(m_tabBook, &TabBook::pageChanged, this, &TabBar::tabBookPageChanged);

  m_controlLayout = new QHBoxLayout();
  m_controlLayout->setContentsMargins(0, 0, 0, 0);
  m_controlLayout->setSpacing(0);
  m_controlLayout->addSpacing(LayoutConstants::TabBarBarLeftMargin);
  m_controlLayout->addStretch(1);
  m_controlLayout->addLayout(m_barBook, 0);
  m_controlLayout->setAlignment(m_barBook, Qt::AlignVCenter);
  m_controlLayout->addSpacing(LayoutConstants::NarrowHMargin);

  setLayout(m_controlLayout);
}

void TabBar::addTab(TabBookPage* bookPage, const QString& title)
{
  ensure(bookPage != nullptr, "bookPage is null");

  auto* button = new TabBarButton(title);
  connect(button, &TabBarButton::clicked, this, &TabBar::buttonClicked);
  button->setPressed(m_buttons.empty());
  m_buttons.push_back(button);

  const auto sizerIndex = static_cast<int>(m_buttons.size());
  m_controlLayout->insertWidget(sizerIndex, button);

  QWidget* barPage = bookPage->createTabBarPage(nullptr);
  m_barBook->addWidget(barPage);
}

size_t TabBar::findButtonIndex(QWidget* button) const
{

  for (size_t i = 0; i < m_buttons.size(); ++i)
  {
    if (m_buttons[i] == button)
    {
      return i;
    }
  }
  return m_buttons.size();
}

void TabBar::setButtonActive(const int index)
{
  m_buttons.at(static_cast<size_t>(index))->setPressed(true);
}

void TabBar::buttonClicked()
{
  auto* button = dynamic_cast<QWidget*>(QObject::sender());
  const size_t index = findButtonIndex(button);
  ensure(index < m_buttons.size(), "index out of range");
  m_tabBook->switchToPage(static_cast<int>(index));
}

void TabBar::tabBookPageChanged(const int newIndex)
{
  for (TabBarButton* button : m_buttons)
  {
    button->setPressed(false);
  }

  setButtonActive(newIndex);
  m_barBook->setCurrentIndex(newIndex);
}
} // namespace View
} // namespace TrenchBroom
