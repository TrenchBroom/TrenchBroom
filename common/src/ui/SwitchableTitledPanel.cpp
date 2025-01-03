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

#include "SwitchableTitledPanel.h"

#include <QBoxLayout>
#include <QIODevice>
#include <QStackedLayout>

#include "ui/BorderLine.h"
#include "ui/ClickableTitleBar.h"
#include "ui/QtUtils.h"

namespace tb::ui
{

SwitchableTitledPanel::SwitchableTitledPanel(
  const QString& title, const std::array<QString, 2>& stateTexts, QWidget* parent)
  : QWidget{parent}
  , m_titleBar{new ClickableTitleBar{title, stateTexts[1]}}
  , m_divider{new BorderLine{}}
  , m_stackedLayout{new QStackedLayout{}}
  , m_panels{{{new QWidget{}, stateTexts[1]}, {new QWidget{}, stateTexts[0]}}}
{
  m_stackedLayout->addWidget(m_panels[0].panel);
  m_stackedLayout->addWidget(m_panels[1].panel);

  auto* outerLayout = new QVBoxLayout{};
  outerLayout->setContentsMargins(0, 0, 0, 0);
  outerLayout->setSpacing(0);
  outerLayout->addWidget(m_titleBar, 0);
  outerLayout->addWidget(m_divider, 0);
  outerLayout->addLayout(m_stackedLayout, 1);
  setLayout(outerLayout);

  connect(m_titleBar, &ClickableTitleBar::titleBarClicked, this, [&]() {
    setCurrentIndex(1 - currentIndex());
  });
}

QWidget* SwitchableTitledPanel::getPanel(const size_t index) const
{
  assert(index < 2);
  return m_panels[index].panel;
}

size_t SwitchableTitledPanel::currentIndex() const
{
  return size_t(m_stackedLayout->currentIndex());
}

void SwitchableTitledPanel::setCurrentIndex(const size_t index)
{
  m_stackedLayout->setCurrentIndex(int(index));
  m_titleBar->setStateText(m_panels[index].stateText);
}

QByteArray SwitchableTitledPanel::saveState() const
{
  auto result = QByteArray{};
  auto stream = QDataStream{&result, QIODevice::WriteOnly};
  stream << int(currentIndex());
  return result;
}

bool SwitchableTitledPanel::restoreState(const QByteArray& state)
{
  auto stream = QDataStream{state};
  int currentIndex;
  stream >> currentIndex;

  if (stream.status() == QDataStream::Ok && currentIndex >= 0 && currentIndex < 2)
  {
    setCurrentIndex(size_t(currentIndex));
    return true;
  }

  return false;
}

} // namespace tb::ui
