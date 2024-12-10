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

#include <QDialog>

#include "NotifierConnection.h"
#include "mdl/MapFormat.h"

#include <filesystem>
#include <string>

class QComboBox;
class QPushButton;
class QWidget;

namespace tb::ui
{
class GameListBox;

class GameDialog : public QDialog
{
  Q_OBJECT
private:
  enum class DialogType
  {
    Open,
    New
  };

protected:
  DialogType m_dialogType;
  GameListBox* m_gameListBox = nullptr;
  QComboBox* m_mapFormatComboBox = nullptr;
  QPushButton* m_openPreferencesButton = nullptr;
  QPushButton* m_okButton = nullptr;

  NotifierConnection m_notifierConnection;

public:
  static std::optional<std::tuple<std::string, mdl::MapFormat>> showNewDocumentDialog(
    QWidget* parent = nullptr);
  static std::optional<std::tuple<std::string, mdl::MapFormat>> showOpenDocumentDialog(
    QWidget* parent = nullptr);

  std::string currentGameName() const;
  mdl::MapFormat currentMapFormat() const;
private slots:
  void currentGameChanged(const QString& gameName);
  void gameSelected(const QString& gameName);
  void openPreferencesClicked();

protected:
  GameDialog(
    const QString& title,
    const QString& infoText,
    DialogType type,
    QWidget* parent = nullptr);

  void createGui(const QString& title, const QString& infoText);
  QWidget* createInfoPanel(const QString& title, const QString& infoText);
  QWidget* createSelectionPanel();

private:
  void updateMapFormats(const std::string& gameName);

  void connectObservers();
  void preferenceDidChange(const std::filesystem::path& path);
};

} // namespace tb::ui
