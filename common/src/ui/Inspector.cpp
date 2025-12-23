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

#include "Inspector.h"

#include <QByteArray>
#include <QDataStream>
#include <QIODevice>
#include <QVBoxLayout>

#include "ui/EntityInspector.h"
#include "ui/FaceInspector.h"
#include "ui/MapInspector.h"
#include "ui/MapViewBar.h"
#include "ui/QtUtils.h"
#include "ui/TabBar.h"
#include "ui/TabBook.h"

namespace tb::ui
{
Inspector::Inspector(
  MapDocument& document, gl::ContextManager& contextManager, QWidget* parent)
  : QWidget{parent}
{
  m_tabBook = new TabBook{};
  m_tabBook->setObjectName("Inspector_TabBook");

  m_mapInspector = new MapInspector{document};
  m_entityInspector = new EntityInspector{document, contextManager};
  m_faceInspector = new FaceInspector{document, contextManager};

  m_tabBook->addPage(m_mapInspector, "Map");
  m_tabBook->addPage(m_entityInspector, "Entity");
  m_tabBook->addPage(m_faceInspector, "Face");

  auto* layout = new QVBoxLayout{};
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(m_tabBook);
  setLayout(layout);

  restoreWindowState(m_tabBook);
}

Inspector::~Inspector()
{
  saveWindowState(m_tabBook);
}

void Inspector::connectTopWidgets(MapViewBar* mapViewBar)
{
  if (m_syncTabBarEventFilter)
  {
    delete std::exchange(m_syncTabBarEventFilter, nullptr);
  }

  m_syncTabBarEventFilter =
    new SyncHeightEventFilter{mapViewBar, m_tabBook->tabBar(), this};
}

void Inspector::switchToPage(const InspectorPage page)
{
  m_tabBook->switchToPage(static_cast<int>(page));
}

bool Inspector::cancelMouseDrag()
{
  return m_faceInspector->cancelMouseDrag();
}

FaceInspector* Inspector::faceInspector()
{
  return m_faceInspector;
}

QByteArray Inspector::saveState() const
{
  auto result = QByteArray{};
  auto stream = QDataStream{&result, QIODevice::WriteOnly};
  stream << isVisible();
  return result;
}

bool Inspector::restoreState(const QByteArray& state)
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
