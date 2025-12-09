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
#include <string>
#include <vector>

class QLineEdit;
class QListWidget;
class QWidget;
class QAbstractButton;

namespace tb::ui
{
class MapDocument;

class ModEditor : public QWidget
{
  Q_OBJECT
private:
  MapDocument& m_document;

  QListWidget* m_availableModList = nullptr;
  QListWidget* m_enabledModList = nullptr;
  QLineEdit* m_filterBox = nullptr;
  QAbstractButton* m_addModsButton = nullptr;
  QAbstractButton* m_removeModsButton = nullptr;
  QAbstractButton* m_moveModUpButton = nullptr;
  QAbstractButton* m_moveModDownButton = nullptr;

  std::vector<std::string> m_availableMods;

  NotifierConnection m_notifierConnection;

public:
  explicit ModEditor(MapDocument& document, QWidget* parent = nullptr);

private:
  void createGui();
private slots:
  void updateButtons();

private:
  void connectObservers();

  void documentWasCreated();
  void documentWasLoaded();
  void modsDidChange();
  void preferenceDidChange(const std::filesystem::path& path);

  void updateAvailableMods();
  void updateMods();

  void addModClicked();
  void removeModClicked();
  void moveModUpClicked();
  void moveModDownClicked();
  bool canEnableAddButton() const;
  bool canEnableRemoveButton() const;
  bool canEnableMoveUpButton() const;
  bool canEnableMoveDownButton() const;
  void filterBoxChanged();
};

} // namespace tb::ui
