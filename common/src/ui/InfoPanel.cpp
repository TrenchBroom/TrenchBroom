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

#include "InfoPanel.h"

#include <QByteArray>
#include <QDataStream>
#include <QIODevice>
#include <QVBoxLayout>

#include "ui/Console.h"
#include "ui/IssueBrowser.h"
#include "ui/QtUtils.h"
#include "ui/TabBook.h"

namespace tb::ui
{

InfoPanel::InfoPanel(MapDocument& document, QWidget* parent)
  : QWidget{parent}
{
  m_tabBook = new TabBook{};
  m_tabBook->setObjectName("InfoPanel_TabBook");

  m_console = new Console{};
  m_issueBrowser = new IssueBrowser{document};

  m_tabBook->addPage(m_console, tr("Console"));
  m_tabBook->addPage(m_issueBrowser, tr("Issues"));

  auto* sizer = new QVBoxLayout{};
  sizer->setContentsMargins(0, 0, 0, 0);
  sizer->addWidget(m_tabBook);
  setLayout(sizer);

  restoreWindowState(m_tabBook);
}

InfoPanel::~InfoPanel()
{
  saveWindowState(m_tabBook);
}

Console* InfoPanel::console() const
{
  return m_console;
}

QByteArray InfoPanel::saveState() const
{
  auto result = QByteArray{};
  auto stream = QDataStream{&result, QIODevice::WriteOnly};
  stream << isVisible();
  return result;
}

bool InfoPanel::restoreState(const QByteArray& state)
{
  auto stream = QDataStream{state};
  bool visible;
  stream >> visible;

  if (stream.status() == QDataStream::Ok)
  {
    setVisible(visible);
    return true;
  }

  return false;
}

} // namespace tb::ui
