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

#include "upd/UpdateController.h"

#include <optional>

namespace upd
{

class HttpClient;
class UpdateDialog;
struct UpdateInfo;

class Updater : public QObject
{
  Q_OBJECT
private:
  HttpClient& m_httpClient;
  UpdateController* m_updateController;

public:
  explicit Updater(
    HttpClient& httpClient,
    std::optional<UpdateConfig> config,
    QObject* parent = nullptr);

  void setAutoCheckEnabled(bool autoCheckEnabled);

  void showUpdateDialog();

  QWidget* createUpdateIndicator(QWidget* parent = nullptr);
};

} // namespace upd
