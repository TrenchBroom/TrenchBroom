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

namespace tb::mdl
{
class Map;
}

namespace tb::ui
{
class FaceInspector;
class EntityInspector;
class GLContextManager;
class MapInspector;
class MapViewBar;
class SyncHeightEventFilter;
class TabBook;

enum class InspectorPage
{
  Map = 0,
  Entity = 1,
  Face = 2
};

class Inspector : public QWidget
{
  Q_OBJECT
private:
  TabBook* m_tabBook = nullptr;
  MapInspector* m_mapInspector = nullptr;
  EntityInspector* m_entityInspector = nullptr;
  FaceInspector* m_faceInspector = nullptr;

  SyncHeightEventFilter* m_syncTabBarEventFilter = nullptr;

public:
  Inspector(mdl::Map& map, GLContextManager& contextManager, QWidget* parent = nullptr);
  ~Inspector() override;

  void connectTopWidgets(MapViewBar* mapViewBar);
  void switchToPage(InspectorPage page);
  bool cancelMouseDrag();

  FaceInspector* faceInspector();

  QByteArray saveState() const;
  bool restoreState(const QByteArray& state);
};

} // namespace tb::ui
