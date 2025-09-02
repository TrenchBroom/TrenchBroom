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

#include "NotifierConnection.h"
#include "ui/TabBook.h"

#include <memory>
#include <vector>

class QCheckBox;
class QStackedLayout;
class QWidget;

namespace tb::mdl
{
class BrushFaceHandle;
class Issue;
class Map;
class Node;
} // namespace tb::mdl

namespace tb::ui
{
class FlagsPopupEditor;
class IssueBrowserView;
class MapDocument;

class IssueBrowser : public TabBookPage
{
  Q_OBJECT
private:
  static const int SelectObjectsCommandId = 1;
  static const int ShowIssuesCommandId = 2;
  static const int HideIssuesCommandId = 3;
  static const int FixObjectsBaseId = 4;

  std::weak_ptr<MapDocument> m_document;
  IssueBrowserView* m_view = nullptr;
  QCheckBox* m_showHiddenIssuesCheckBox = nullptr;
  FlagsPopupEditor* m_filterEditor = nullptr;

  NotifierConnection m_notifierConnection;

public:
  explicit IssueBrowser(std::weak_ptr<MapDocument> document, QWidget* parent = nullptr);

  QWidget* createTabBarPage(QWidget* parent) override;

private:
  void connectObservers();
  void mapWasCreated(mdl::Map& map);
  void mapWasLoaded(mdl::Map& map);
  void mapWasSaved(mdl::Map& map);
  void nodesWereAdded(const std::vector<mdl::Node*>& nodes);
  void nodesWereRemoved(const std::vector<mdl::Node*>& nodes);
  void nodesDidChange(const std::vector<mdl::Node*>& nodes);
  void brushFacesDidChange(const std::vector<mdl::BrushFaceHandle>& faces);
  void issueIgnoreChanged(mdl::Issue* issue);

  void reload();

  void updateFilterFlags();

  void showHiddenIssuesChanged();
  void filterChanged(size_t index, int value, int setFlag, int mixedFlag);
};

} // namespace tb::ui
