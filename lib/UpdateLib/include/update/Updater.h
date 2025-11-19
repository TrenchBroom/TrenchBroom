/*
 Copyright (C) 2025 Kristian Duske

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

#include "update/UpdateController.h"

#include <optional>

namespace upd
{

class HttpClient;
class UpdateDialog;
struct UpdateInfo;

/**
 * The Updater class is the main entry point for the update functionality.
 */
class Updater : public QObject
{
  Q_OBJECT
private:
  HttpClient& m_httpClient;
  UpdateController* m_updateController;

public:
  /**
   * Create a new Updater instance.
   *
   * @param httpClient The HTTP client to use for update checks and downloads.
   * @param config The update configuration. If not set, the updater is disabled.
   * @param parent The parent object.
   */
  explicit Updater(
    HttpClient& httpClient,
    std::optional<UpdateConfig> config,
    QObject* parent = nullptr);

  /**
   * Show the update dialog.
   */
  void showUpdateDialog();

  /**
   * Perform an update check. Only has an effect while the update controller is in the
   * idle or error state.
   */
  void checkForUpdates();

  /**
   * Reset the update controller to the idle state.
   */
  void reset();

  /**
   * Create an update indicator label. The label shows the current state of the update
   * controller and let the user trigger certain actions.
   */
  QWidget* createUpdateIndicator(QWidget* parent = nullptr);
};

} // namespace upd
