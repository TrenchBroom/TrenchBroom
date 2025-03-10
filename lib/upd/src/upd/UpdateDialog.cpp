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

#include "UpdateDialog.h"

#include <QBoxLayout>
#include <QCoreApplication>
#include <QDialogButtonBox>
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>
#include <QTimer>

#include "upd/Overload.h"

#include <variant>

namespace upd
{

CheckingForUpdatesWidget::CheckingForUpdatesWidget(
  const CheckingForUpdatesState&, UpdateDialog* dialog)
  : QWidget{dialog}
{
  using namespace std::chrono_literals;

  auto* header = new QLabel{tr("Checking for updates...")};
  auto font = header->font();
  font.setPointSize(static_cast<int>(1.5f * static_cast<float>(font.pointSize())));
  font.setBold(true);
  header->setFont(font);

  auto* info = new QLabel{tr(R"(Checking for updates. Please wait...)")};
  info->setWordWrap(true);

  auto* progressBar = new QProgressBar{};
  progressBar->setRange(0, 0);
  progressBar->setValue(0);

  auto* progressTimer = new QTimer{this};
  progressTimer->start(500ms);

  auto* buttons = new QDialogButtonBox{};
  buttons->addButton(new QPushButton{tr("Cancel")}, QDialogButtonBox::RejectRole);

  connect(buttons, &QDialogButtonBox::rejected, [dialog]() {
    dialog->updateController().cancelPendingOperation();
    dialog->reject();
  });

  auto* layout = new QVBoxLayout{};
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(header);
  layout->addSpacing(20);
  layout->addWidget(info);
  layout->addSpacing(10);
  layout->addWidget(progressBar);
  layout->addSpacing(10);
  layout->addWidget(buttons);

  setLayout(layout);
}

UpdateAvailableWidget::UpdateAvailableWidget(
  const UpdateAvailableState& updateAvailableState, UpdateDialog* dialog)
  : QWidget{dialog}
{
  auto* header = new QLabel{tr("An update is available!")};
  auto font = header->font();
  font.setPointSize(static_cast<int>(1.5f * static_cast<float>(font.pointSize())));
  font.setBold(true);
  header->setFont(font);

  auto* info1 = new QLabel{
    tr(R"(Version %1 is available to download. You are currently using version %2.)")
      .arg(updateAvailableState.updateInfo.updateVersion)
      .arg(updateAvailableState.updateInfo.currentVersion)};
  info1->setWordWrap(true);

  auto* info2 = new QLabel{tr(
    R"(Start the update by clicking the button below. The application will be restarted after the update is installed.)")};
  info2->setWordWrap(true);

  auto* link = new QLabel{tr(R"(<a href="%3">Click here for the release notes.</a>)")
                            .arg(updateAvailableState.updateInfo.browserUrl.toString())};
  link->setOpenExternalLinks(true);

  auto* buttons = new QDialogButtonBox{};
  buttons->addButton(
    new QPushButton{tr("Download and install")}, QDialogButtonBox::AcceptRole);
  buttons->addButton(new QPushButton{tr("Close")}, QDialogButtonBox::RejectRole);

  connect(buttons, &QDialogButtonBox::accepted, [dialog]() {
    dialog->updateController().downloadAndPrepareUpdate();
  });
  connect(buttons, &QDialogButtonBox::rejected, [dialog]() { dialog->reject(); });

  auto* layout = new QVBoxLayout{};
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(header);
  layout->addSpacing(20);
  layout->addWidget(info1);
  layout->addSpacing(10);
  layout->addWidget(info2);
  layout->addSpacing(10);
  layout->addWidget(link);
  layout->addSpacing(10);
  layout->addWidget(buttons);

  setLayout(layout);
}

UpToDateWidget::UpToDateWidget(const UpToDateState& upToDateState, UpdateDialog* dialog)
  : QWidget{dialog}
{
  auto* header = new QLabel{tr("No update available.")};
  auto font = header->font();
  font.setPointSize(static_cast<int>(1.5f * static_cast<float>(font.pointSize())));
  font.setBold(true);
  header->setFont(font);

  auto* info =
    new QLabel{tr(R"(You are using the latest version %1. There is no update available.)")
                 .arg(upToDateState.currentVersion)};
  info->setWordWrap(true);

  auto* buttons = new QDialogButtonBox{};
  buttons->addButton(new QPushButton{tr("Close")}, QDialogButtonBox::RejectRole);

  connect(buttons, &QDialogButtonBox::rejected, [dialog]() { dialog->reject(); });

  auto* layout = new QVBoxLayout{};
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(header);
  layout->addSpacing(20);
  layout->addWidget(info);
  layout->addSpacing(10);
  layout->addWidget(buttons);

  setLayout(layout);
}

DownloadingUpdateWidget::DownloadingUpdateWidget(
  const DownloadingUpdateState& downloadingUpdateState, UpdateDialog* dialog)
  : QWidget{dialog}
  , m_downloadingUpdateState{downloadingUpdateState}
{
  using namespace std::chrono_literals;

  auto* header = new QLabel{tr("Downloading update...")};
  auto font = header->font();
  font.setPointSize(static_cast<int>(1.5f * static_cast<float>(font.pointSize())));
  font.setBold(true);
  header->setFont(font);

  auto* info = new QLabel{tr(R"(The update is being downloaded. Please wait...)")};
  info->setWordWrap(true);

  auto* progressBar = new QProgressBar{};
  progressBar->setRange(0, 0);
  progressBar->setValue(0);

  auto* progressTimer = new QTimer{this};
  progressTimer->start(500ms);

  connect(progressTimer, &QTimer::timeout, [progressBar, this]() {
    if (const auto progress = m_downloadingUpdateState.pendingOperation->progress())
    {
      progressBar->setRange(0, 100);
      progressBar->setValue(static_cast<int>(*progress * 100.0f));
    }
    else
    {
      progressBar->setRange(0, 0);
      progressBar->setValue(0);
    }
  });

  auto* buttons = new QDialogButtonBox{};
  buttons->addButton(new QPushButton{tr("Cancel")}, QDialogButtonBox::RejectRole);

  connect(buttons, &QDialogButtonBox::rejected, [dialog]() {
    dialog->updateController().cancelPendingOperation();
    dialog->reject();
  });

  auto* layout = new QVBoxLayout{};
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(header);
  layout->addSpacing(20);
  layout->addWidget(info);
  layout->addSpacing(10);
  layout->addWidget(progressBar);
  layout->addSpacing(10);
  layout->addWidget(buttons);

  setLayout(layout);
}

PreparingUpdateWidget::PreparingUpdateWidget(
  const PreparingUpdateState& preparingUpdateState, UpdateDialog* dialog)
  : QWidget{dialog}
  , m_preparingUpdateState{preparingUpdateState}
{
  using namespace std::chrono_literals;

  auto* header = new QLabel{tr("Preparing update...")};
  auto font = header->font();
  font.setPointSize(static_cast<int>(1.5f * static_cast<float>(font.pointSize())));
  font.setBold(true);
  header->setFont(font);

  auto* info = new QLabel{tr(R"(The update is being prepared. Please wait...)")};
  info->setWordWrap(true);

  auto* progressBar = new QProgressBar{};
  progressBar->setRange(0, 0);
  progressBar->setValue(0);

  auto* layout = new QVBoxLayout{};
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(header);
  layout->addSpacing(20);
  layout->addWidget(info);
  layout->addSpacing(10);
  layout->addWidget(progressBar);

  setLayout(layout);
}

UpdatePendingWidget::UpdatePendingWidget(
  const UpdatePendingState& updatePendingState, UpdateDialog* dialog)
  : QWidget{dialog}
  , m_updatePendingState{updatePendingState}
{
  using namespace std::chrono_literals;

  auto* header = new QLabel{tr("Update ready to install!")};
  auto font = header->font();
  font.setPointSize(static_cast<int>(1.5f * static_cast<float>(font.pointSize())));
  font.setBold(true);
  header->setFont(font);

  auto* info = new QLabel{tr(
    R"(The update is now ready to be installed. Alternatively, you can install it later when the application quits.)")};
  info->setWordWrap(true);

  auto* buttons = new QDialogButtonBox{};
  buttons->addButton(new QPushButton{tr("Install now")}, QDialogButtonBox::AcceptRole);
  buttons->addButton(new QPushButton{tr("Install later")}, QDialogButtonBox::RejectRole);

  connect(buttons, &QDialogButtonBox::accepted, [dialog]() {
    dialog->updateController().setRestartApp(true);
    dialog->accept();

    QCoreApplication::quit();
    QTimer::singleShot(0, [&updateController = dialog->updateController()]() {
      // this timer is only executed if the quit event is ignored
      // in that case, we reset the restart flag to false
      updateController.setRestartApp(false);
    });
  });

  connect(buttons, &QDialogButtonBox::rejected, [dialog]() {
    dialog->updateController().setRestartApp(false);
    dialog->reject();
  });

  auto* layout = new QVBoxLayout{};
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(header);
  layout->addSpacing(20);
  layout->addWidget(info);
  layout->addSpacing(10);
  layout->addWidget(buttons);

  setLayout(layout);
}

UpdateErrorWidget::UpdateErrorWidget(
  const UpdateErrorState& updateErrorState, UpdateDialog* dialog)
  : QWidget{dialog}
{
  auto* header = new QLabel{tr("An error occurred!")};
  auto font = header->font();
  font.setPointSize(static_cast<int>(1.5f * static_cast<float>(font.pointSize())));
  font.setBold(true);
  header->setFont(font);

  auto* info = new QLabel{updateErrorState.errorMessage};
  info->setWordWrap(true);

  auto* buttons = new QDialogButtonBox{};
  buttons->addButton(new QPushButton{tr("Retry")}, QDialogButtonBox::AcceptRole);
  buttons->addButton(new QPushButton{tr("Close")}, QDialogButtonBox::RejectRole);

  connect(buttons, &QDialogButtonBox::accepted, [dialog]() {
    dialog->updateController().checkForUpdates();
  });
  connect(buttons, &QDialogButtonBox::rejected, [dialog]() { dialog->reject(); });

  auto* layout = new QVBoxLayout{};
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(header);
  layout->addSpacing(20);
  layout->addWidget(info);
  layout->addSpacing(10);
  layout->addWidget(buttons);

  setLayout(layout);
}

UpdateDialog::UpdateDialog(UpdateController& updateController, QWidget* parent)
  : QDialog{parent}
  , m_updateController{updateController}
{
  auto* layout = new QVBoxLayout{};
  layout->setSizeConstraint(QLayout::SetFixedSize);
  setLayout(layout);

  setWindowTitle(tr("Update"));

  updateUI(m_updateController.state());
  connect(
    &m_updateController, &UpdateController::stateChanged, this, &UpdateDialog::updateUI);
}

UpdateDialog::~UpdateDialog() = default;

UpdateController& UpdateDialog::updateController()
{
  return m_updateController;
}

void UpdateDialog::updateUI(const UpdateControllerState& state)
{
  while (auto* item = layout()->takeAt(0))
  {
    delete item->widget();
    delete item;
  }

  auto* widget = std::visit(
    overload(
      [&](const CheckingForUpdatesState& checkingForUpdatesState) -> QWidget* {
        return new CheckingForUpdatesWidget{checkingForUpdatesState, this};
      },
      [&](const UpdateAvailableState& updateAvailableState) -> QWidget* {
        return new UpdateAvailableWidget{updateAvailableState, this};
      },
      [&](const UpToDateState& upToDateState) -> QWidget* {
        return new UpToDateWidget{upToDateState, this};
      },
      [&](const DownloadingUpdateState& downloadingUpdateState) -> QWidget* {
        return new DownloadingUpdateWidget{downloadingUpdateState, this};
      },
      [&](const PreparingUpdateState& preparingUpdateState) -> QWidget* {
        return new PreparingUpdateWidget{preparingUpdateState, this};
      },
      [&](const UpdatePendingState& updatePendingState) -> QWidget* {
        return new UpdatePendingWidget{updatePendingState, this};
      },
      [&](const UpdateErrorState& updateErrorState) -> QWidget* {
        return new UpdateErrorWidget{updateErrorState, this};
      },
      [](const auto&) -> QWidget* { return new QWidget{}; }),
    state);

  widget->setMinimumWidth(400);
  layout()->addWidget(widget);
}

} // namespace upd
