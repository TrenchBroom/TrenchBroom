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

#include <QDialog>

#include "upd/UpdateController.h"

namespace upd
{

class UpdateDialog;

class CheckingForUpdatesWidget : public QWidget
{
  Q_OBJECT
public:
  explicit CheckingForUpdatesWidget(
    const CheckingForUpdatesState& checkingForUpdatesState, UpdateDialog* dialog);
};

class UpdateAvailableWidget : public QWidget
{
  Q_OBJECT
public:
  explicit UpdateAvailableWidget(
    const UpdateAvailableState& updateAvailableState, UpdateDialog* dialog);
};

class UpToDateWidget : public QWidget
{
  Q_OBJECT
public:
  explicit UpToDateWidget(const UpToDateState& upToDateState, UpdateDialog* dialog);
};

class DownloadingUpdateWidget : public QWidget
{
  Q_OBJECT
private:
  const DownloadingUpdateState& m_downloadingUpdateState;

public:
  explicit DownloadingUpdateWidget(
    const DownloadingUpdateState& downloadingUpdateState, UpdateDialog* dialog);
};

class PreparingUpdateWidget : public QWidget
{
  Q_OBJECT
private:
  const PreparingUpdateState& m_preparingUpdateState;

public:
  explicit PreparingUpdateWidget(
    const PreparingUpdateState& preparingUpdateState, UpdateDialog* dialog);
};

class UpdatePendingWidget : public QWidget
{
  Q_OBJECT
private:
  const UpdatePendingState& m_updatePendingState;

public:
  explicit UpdatePendingWidget(
    const UpdatePendingState& updatePendingState, UpdateDialog* dialog);

private:
  QWidget* createRequiresAdminPrivilegesWidget(
    const UpdatePendingState& updatePendingState) const;
};

class UpdateErrorWidget : public QWidget
{
  Q_OBJECT
public:
  explicit UpdateErrorWidget(
    const UpdateErrorState& updateErrorState, UpdateDialog* dialog);
};

/**
 * A dialog that shows the current update state and lets users interact with it. Also
 * shows the progress of long running actions such as downloading an update.
 */
class UpdateDialog : public QDialog
{
  Q_OBJECT
private:
  UpdateController& m_updateController;

public:
  /**
   * Creates a new update dialog using the given update controller.
   */
  explicit UpdateDialog(UpdateController& updateController, QWidget* parent = nullptr);

  ~UpdateDialog() override;

  UpdateController& updateController();

private:
  void updateUI(const UpdateControllerState& state);
};

} // namespace upd
