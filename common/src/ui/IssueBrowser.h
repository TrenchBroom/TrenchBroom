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

class QCheckBox;
class QStackedLayout;
class QWidget;

namespace tb
{
namespace mdl
{
class Issue;
class Map;
} // namespace mdl

namespace ui
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

  MapDocument& m_document;
  IssueBrowserView* m_view = nullptr;
  QCheckBox* m_showHiddenIssuesCheckBox = nullptr;
  FlagsPopupEditor* m_filterEditor = nullptr;

  NotifierConnection m_notifierConnection;

public:
  explicit IssueBrowser(MapDocument& document, QWidget* parent = nullptr);

  QWidget* createTabBarPage(QWidget* parent) override;

private:
  void connectObservers();
  void mapWasSaved(mdl::Map& map);
  void documentDidChange();
  void issueIgnoreChanged(mdl::Issue* issue);

  void reload();

  void updateFilterFlags();

  void showHiddenIssuesChanged();
  void filterChanged(size_t index, int value, int setFlag, int mixedFlag);
};

} // namespace ui
} // namespace tb
