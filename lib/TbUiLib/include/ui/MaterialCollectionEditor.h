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

class QListWidget;
class QAbstractButton;

namespace tb::ui
{
class MapDocument;

class MaterialCollectionEditor : public QWidget
{
  Q_OBJECT
private:
  MapDocument& m_document;

  QListWidget* m_availableCollectionsList = nullptr;
  QListWidget* m_enabledCollectionsList = nullptr;

  QAbstractButton* m_addCollectionsButton = nullptr;
  QAbstractButton* m_removeCollectionsButton = nullptr;
  QAbstractButton* m_reloadCollectionsButton = nullptr;

  NotifierConnection m_notifierConnection;

public:
  explicit MaterialCollectionEditor(MapDocument& document, QWidget* parent = nullptr);

private:
  void addSelectedMaterialCollections();
  void removeSelectedMaterialCollections();
  void reloadMaterialCollections();
  void availableMaterialCollectionSelectionChanged();
  void enabledMaterialCollectionSelectionChanged();

  bool canAddMaterialCollections() const;
  bool canRemoveMaterialCollections() const;
  bool canReloadMaterialCollections() const;

private:
  void createGui();
  void updateButtons();

  void connectObservers();

  void updateUi();

  void updateAllMaterialCollections();
  void updateAvailableMaterialCollections();
  void updateEnabledMaterialCollections();

  std::vector<std::filesystem::path> availableMaterialCollections() const;
  std::vector<std::filesystem::path> enabledMaterialCollections() const;
};

} // namespace tb::ui
