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

#include "UpdateIndicator.h"

#include <QBoxLayout>
#include <QLabel>

#include "update/Overload.h"
#include "update/UpdateDialog.h"

#include <variant>

namespace upd
{

UpdateIndicator::UpdateIndicator(UpdateController& updateController, QWidget* parent)
  : QLabel{parent}
  , m_updateController{updateController}
{
  updateUI(m_updateController.state());
  connect(
    &m_updateController,
    &UpdateController::stateChanged,
    [this](const UpdateControllerState& state) { updateUI(state); });
  connect(this, &QLabel::linkActivated, [&](const auto& uri) {
    if (uri == "upd://checkForUpdates")
    {
      m_updateController.checkForUpdates();
    }

    auto dialog = UpdateDialog{m_updateController};
    dialog.exec();
  });
}

UpdateIndicator::~UpdateIndicator() = default;

void UpdateIndicator::updateUI(const UpdateControllerState& state)
{
  std::visit(
    overload(
      [&](const IdleState&) {
        setVisible(true);
        setText(tr(R"(<a href="upd://checkForUpdates">Check for updates</a>)"));
      },
      [&](const CheckingForUpdatesState&) {
        setVisible(true);
        setText(tr("Checking for updates..."));
      },
      [&](const UpdateAvailableState&) {
        setVisible(true);
        setText(tr(R"(<a href="upd://showDialog">Update available</a>)"));
      },
      [&](const UpToDateState&) {
        setVisible(true);
        setText(tr(R"(Up to date)"));
      },
      [&](const DownloadingUpdateState&) {
        setVisible(true);
        setText(tr("Downloading update..."));
      },
      [&](const PreparingUpdateState&) {
        setVisible(true);
        setText(tr("Preparing update..."));
      },
      [&](const UpdatePendingState&) {
        setVisible(true);
        setText(tr("Update pending"));
      },
      [&](const UpdateErrorState&) {
        setVisible(true);
        setText(tr(R"(<a href="upd://showDialog">Update error</a>)"));
      },
      [&](const UpdateDisabledState&) {
        setVisible(true);
        setText(tr(R"(Updates disabled)"));
      }),
    state);
}

} // namespace upd
