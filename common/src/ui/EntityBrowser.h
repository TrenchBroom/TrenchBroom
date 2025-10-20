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

#include "NotifierConnection.h"

#include <filesystem>
#include <vector>

class QPushButton;
class QComboBox;
class QLineEdit;
class QScrollBar;

namespace tb::mdl
{
class Map;
class Node;
class ResourceId;
} // namespace tb::mdl

namespace tb::ui
{
class EntityBrowserView;
class GLContextManager;
class MapDocument;

class EntityBrowser : public QWidget
{
  Q_OBJECT
private:
  mdl::Map& m_map;
  QComboBox* m_sortOrderChoice = nullptr;
  QPushButton* m_groupButton = nullptr;
  QPushButton* m_usedButton = nullptr;
  QLineEdit* m_filterBox = nullptr;
  QScrollBar* m_scrollBar = nullptr;
  EntityBrowserView* m_view = nullptr;

  NotifierConnection m_notifierConnection;

public:
  EntityBrowser(
    mdl::Map& map, GLContextManager& contextManager, QWidget* parent = nullptr);

  void reload();

private:
  void createGui(GLContextManager& contextManager);

  void connectObservers();

  void mapWasCreated(mdl::Map& map);
  void mapWasLoaded(mdl::Map& map);

  void modsDidChange();
  void nodesDidChange(const std::vector<mdl::Node*>& nodes);
  void entityDefinitionsDidChange();
  void preferenceDidChange(const std::filesystem::path& path);
  void resourcesWereProcessed(const std::vector<mdl::ResourceId>& resources);
};

} // namespace tb::ui
