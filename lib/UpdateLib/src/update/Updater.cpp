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

#include "Updater.h"

#include "update/HttpClient.h"
#include "update/UpdateDialog.h"
#include "update/UpdateIndicator.h"

namespace upd
{

Updater::Updater(
  HttpClient& httpClient, std::optional<UpdateConfig> config, QObject* parent)
  : QObject(parent)
  , m_httpClient{httpClient}
  , m_updateController{new UpdateController{m_httpClient, std::move(config), this}}
{
}

void Updater::showUpdateDialog()
{
  auto dialog = new UpdateDialog(*m_updateController);
  dialog->exec();
}

void Updater::checkForUpdates()
{
  m_updateController->checkForUpdates();
}

void Updater::reset()
{
  m_updateController->reset();
}

QWidget* Updater::createUpdateIndicator(QWidget* parent)
{
  return new UpdateIndicator(*m_updateController, parent);
}

} // namespace upd
