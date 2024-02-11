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

#include <QWidget>

#include "NotifierConnection.h"

#include <filesystem>
#include <memory>
#include <vector>

class QListWidget;
class QAbstractButton;

namespace TrenchBroom::Model
{
class Node;
}

namespace TrenchBroom::View
{
class MapDocument;

class TextureCollectionEditor : public QWidget
{
  Q_OBJECT
private:
  std::weak_ptr<MapDocument> m_document;

  QListWidget* m_availableCollectionsList = nullptr;
  QListWidget* m_enabledCollectionsList = nullptr;

  QAbstractButton* m_addCollectionsButton = nullptr;
  QAbstractButton* m_removeCollectionsButton = nullptr;
  QAbstractButton* m_reloadCollectionsButton = nullptr;

  NotifierConnection m_notifierConnection;

public:
  explicit TextureCollectionEditor(
    std::weak_ptr<MapDocument> document, QWidget* parent = nullptr);

private:
  void addSelectedTextureCollections();
  void removeSelectedTextureCollections();
  void reloadTextureCollections();
  void availableTextureCollectionSelectionChanged();
  void enabledTextureCollectionSelectionChanged();

  bool canAddTextureCollections() const;
  bool canRemoveTextureCollections() const;
  bool canReloadTextureCollections() const;

private:
  void createGui();
  void updateButtons();

  void connectObservers();

  void documentWasNewedOrLoaded(MapDocument*);
  void nodesDidChange(const std::vector<Model::Node*>& nodes);
  void textureCollectionsDidChange();
  void modsDidChange();
  void preferenceDidChange(const std::filesystem::path& path);

  void updateAllTextureCollections();
  void updateAvailableTextureCollections();
  void updateEnabledTextureCollections();
  void updateListBox(QListWidget* box, const std::vector<std::filesystem::path>& paths);

  std::vector<std::filesystem::path> availableTextureCollections() const;
  std::vector<std::filesystem::path> enabledTextureCollections() const;
};

} // namespace TrenchBroom::View
