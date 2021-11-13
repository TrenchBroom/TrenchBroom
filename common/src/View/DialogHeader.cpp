/*
 Copyright (C) 2021 Kristian Duske

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

#include "DialogHeader.h"

#include "QtUtils.h"

#include <QBoxLayout>
#include <QLabel>

namespace TrenchBroom {
namespace View {
DialogHeader::DialogHeader(QWidget* parent)
  : QWidget{parent}
  , m_iconLabel{nullptr}
  , m_textLabel{nullptr} {
  createGui();
}

DialogHeader::DialogHeader(const QString& text, QWidget* parent)
  : DialogHeader{parent} {
  set(text);
}

DialogHeader::DialogHeader(const QString& text, QPixmap icon, QWidget* parent)
  : DialogHeader{parent} {
  set(text, icon);
}

void DialogHeader::set(const QString& text) {
  m_textLabel->setText(text);
  m_iconLabel->setVisible(false);
}

void DialogHeader::set(const QString& text, QPixmap icon) {
  m_textLabel->setText(text);
  m_iconLabel->setPixmap(icon);
  m_iconLabel->setVisible(true);
}

void DialogHeader::createGui() {
  // Use white background (or whatever color a text widget uses)
  setBaseWindowColor(this);

  m_iconLabel = new QLabel{};
  m_textLabel = new QLabel{};
  makeHeader(m_textLabel);

  auto* layout = new QHBoxLayout{};
  layout->setContentsMargins(
    LayoutConstants::WideHMargin, LayoutConstants::MediumVMargin, LayoutConstants::WideHMargin,
    LayoutConstants::MediumVMargin);
  layout->setSpacing(LayoutConstants::MediumHMargin);
  layout->addWidget(m_iconLabel, 0, Qt::AlignLeft | Qt::AlignVCenter);
  layout->addWidget(m_textLabel, 1, Qt::AlignLeft | Qt::AlignVCenter);
  setLayout(layout);
}

} // namespace View
} // namespace TrenchBroom
