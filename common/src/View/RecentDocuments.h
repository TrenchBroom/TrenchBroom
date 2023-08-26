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

#pragma once

#include <QObject>

#include "Notifier.h"

#include <filesystem>
#include <vector>

class QMenu;

namespace TrenchBroom::View
{
std::vector<std::filesystem::path> loadRecentDocuments(size_t max);
void saveRecentDocuments(const std::vector<std::filesystem::path>& paths);

class RecentDocuments : public QObject
{
  Q_OBJECT
private:
  using MenuList = std::vector<QMenu*>;
  MenuList m_menus;

  size_t m_maxSize;
  std::vector<std::filesystem::path> m_recentDocuments;

public:
  explicit RecentDocuments(size_t maxSize, QObject* parent = nullptr);

  const std::vector<std::filesystem::path>& recentDocuments() const;

  void reload();

  void addMenu(QMenu& menu);
  void removeMenu(QMenu& menu);

  void updatePath(const std::filesystem::path& path);
  void removePath(const std::filesystem::path& path);

private:
  std::vector<std::filesystem::path> loadFromConfig();
  void saveToConfig();

  void insertPath(const std::filesystem::path& path);

  void updateMenus();
  void clearMenu(QMenu& menu);
  void createMenuItems(QMenu& menu);
signals:
  void loadDocument(const std::filesystem::path& path) const;
  void didChange();
};
} // namespace TrenchBroom::View
