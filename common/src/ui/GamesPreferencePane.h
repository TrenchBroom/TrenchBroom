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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "ui/PreferencePane.h"

class QLineEdit;
class QPushButton;
class QStackedWidget;

namespace tb
{
namespace mdl
{
struct CompilationTool;
struct GameInfo;
} // namespace mdl

namespace ui
{
class AppController;
class GameListBox;
class GamePreferencePane;
class MapDocument;

/**
 * List of all games with the ability to edit game path, compile tools for each game
 */
class GamesPreferencePane : public PreferencePane
{
  Q_OBJECT
private:
  AppController& m_appController;
  MapDocument* m_document = nullptr;
  GameListBox* m_gameListBox = nullptr;
  QStackedWidget* m_stackedWidget = nullptr;
  QWidget* m_defaultPage = nullptr;
  GamePreferencePane* m_currentGamePage = nullptr;

public:
  explicit GamesPreferencePane(
    AppController& appController, MapDocument* document, QWidget* parent = nullptr);

private:
  void createGui();

private:
  void showUserConfigDirClicked();
  bool canResetToDefaults() override;
  void doResetToDefaults() override;
  void updateControls() override;
  bool validate() override;
};

/**
 * Widget for configuring a single game
 */
class GamePreferencePane : public QWidget
{
  Q_OBJECT
private:
  AppController& m_appController;
  MapDocument* m_document = nullptr;
  const mdl::GameInfo& m_gameInfo;
  QLineEdit* m_gamePathText = nullptr;
  QPushButton* m_chooseGamePathButton = nullptr;
  std::vector<std::tuple<const mdl::CompilationTool*, QLineEdit*>> m_toolPathEditors;

public:
  explicit GamePreferencePane(
    AppController& appController,
    MapDocument* document,
    const mdl::GameInfo& gameInfo,
    QWidget* parent = nullptr);

private:
  void createGui();

private:
  void currentGameChanged(const QString& gameName);
  void chooseGamePathClicked();
  void updateGamePath(const QString& str);
  void configureEnginesClicked();

public:
  const mdl::GameInfo& gameInfo() const;
  /**
   * Refresh controls from GameFactory
   */
  void updateControls();
signals:
  /**
   * Emitted by GamePreferencePane after changing a preference
   */
  void requestUpdate();
};

} // namespace ui
} // namespace tb
