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

class QKeyEvent;
class QCloseEvent;

namespace tb
{
class Logger;

namespace mdl
{
struct GameInfo;
}

namespace ui
{
class AppController;
class GameEngineProfileManager;

/**
 * Dialog for editing game engine profiles (name/path, not parameters).
 */
class GameEngineDialog : public QDialog
{
  Q_OBJECT
private:
  AppController& m_appController;
  const mdl::GameInfo& m_gameInfo;
  Logger& m_logger;
  GameEngineProfileManager* m_profileManager = nullptr;

public:
  explicit GameEngineDialog(
    AppController& appController,
    const mdl::GameInfo& gameInfo,
    Logger& logger,
    QWidget* parent = nullptr);
public slots: // QDialog overrides
  void done(int r) override;

private:
  void createGui();
  void saveConfig();
};

} // namespace ui
} // namespace tb
