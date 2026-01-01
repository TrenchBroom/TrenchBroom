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

#include "ui/TabBar.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QStackedLayout>

#include "ui/QStyleUtils.h"
#include "ui/TabBook.h"
#include "ui/ViewConstants.h"

#include "kd/contracts.h"

namespace tb::ui
{
// TabBarButton

TabBarButton::TabBarButton(const QString& label, QWidget* parent)
  : QWidget{parent}
  , m_label{new QLabel{label}}
  , m_indicator{new QWidget{}}
{
  auto* labelLayout = new QHBoxLayout{};
  labelLayout->setContentsMargins(
    LayoutConstants::WideHMargin, 0, LayoutConstants::WideHMargin, 0);
  labelLayout->addWidget(m_label);

  auto* outerLayout = new QVBoxLayout{};
  outerLayout->setContentsMargins(0, 1, 0, 1); // needs extra vertical space!
  outerLayout->setSpacing(0);

  outerLayout->addSpacing(LayoutConstants::MediumVMargin);
  outerLayout->addSpacing(LayoutConstants::NarrowVMargin);
  outerLayout->addLayout(labelLayout);
  outerLayout->addSpacing(LayoutConstants::NarrowVMargin);
  outerLayout->addWidget(m_indicator);

  setEmphasizedStyle(m_label);
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
  m_indicator->setBackgroundRole(m_pressed ? QPalette::Highlight : QPalette::NoRole);
}

// TabBar

TabBar::TabBar(TabBook* tabBook)
  : ContainerBar{BorderPanel::BottomSide, tabBook}
  , m_tabBook{tabBook}
  , m_barBook{new QStackedLayout{}}
{
  contract_pre(m_tabBook != nullptr);

  connect(m_tabBook, &TabBook::pageChanged, this, &TabBar::tabBookPageChanged);

  m_controlLayout = new QHBoxLayout{};
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
  contract_pre(bookPage != nullptr);

  auto* button = new TabBarButton{title};
  connect(button, &TabBarButton::clicked, this, &TabBar::buttonClicked);
  button->setPressed(m_buttons.empty());
  m_buttons.push_back(button);

  const auto sizerIndex = int(m_buttons.size());
  m_controlLayout->insertWidget(sizerIndex, button);

  auto* barPage = bookPage->createTabBarPage();
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
  m_buttons.at(size_t(index))->setPressed(true);
}

void TabBar::buttonClicked()
{
  auto* button = dynamic_cast<QWidget*>(QObject::sender());
  const auto index = findButtonIndex(button);
  contract_assert(index < m_buttons.size());

  m_tabBook->switchToPage(int(index));
}

void TabBar::tabBookPageChanged(const int newIndex)
{
  for (auto* button : m_buttons)
  {
    button->setPressed(false);
  }

  setButtonActive(newIndex);
  m_barBook->setCurrentIndex(newIndex);
}

} // namespace tb::ui
