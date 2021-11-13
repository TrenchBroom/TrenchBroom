/*
 Copyright (C) 2010-2017 Kristian Duske

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

#include "CrashDialog.h"

#include "IO/PathQt.h"
#include "View/GetVersion.h"
#include "View/QtUtils.h"

#include <QDesktopServices>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QUrl>

namespace TrenchBroom {
namespace View {
CrashDialog::CrashDialog(
  const IO::Path& reportPath, const IO::Path& mapPath, const IO::Path& logPath)
  : QDialog() {
  createGui(reportPath, mapPath, logPath);
}

void CrashDialog::createGui(
  const IO::Path& reportPath, const IO::Path& mapPath, const IO::Path& logPath) {
  setWindowTitle(tr("Crash"));

  auto* header = new QLabel(tr("Crash Report"));
  makeHeader(header);

  auto* text1 =
    new QLabel(tr("TrenchBroom has crashed, but was able to save a crash report,\n"
                  "a log file and the current state of the map to the following locations.\n\n"
                  "Please create an issue report and upload all three files."));

  auto* reportLabel = new QLabel(tr("Report"));
  makeEmphasized(reportLabel);
  auto* reportPathText = new QLabel(IO::pathAsQString(reportPath));

  auto* mapLabel = new QLabel(tr("Map"));
  makeEmphasized(mapLabel);
  auto* mapPathText = new QLabel(IO::pathAsQString(mapPath));

  auto* logLabel = new QLabel(tr("Log"));
  makeEmphasized(logLabel);
  auto* logPathText = new QLabel(IO::pathAsQString(logPath));

  auto* versionLabel = new QLabel(tr("Version"));
  makeEmphasized(versionLabel);
  auto* versionText = new QLabel(getBuildVersion());

  auto* buildLabel = new QLabel(tr("Build"));
  makeEmphasized(buildLabel);
  auto* buildText = new QLabel(getBuildIdStr());

  auto* reportPanelSizer = new QGridLayout();
  reportPanelSizer->addWidget(text1, 0, 0, 1, 2);
  reportPanelSizer->setRowMinimumHeight(1, 20);
  reportPanelSizer->addWidget(reportLabel, 2, 0, 1, 1, Qt::AlignVCenter);
  reportPanelSizer->addWidget(reportPathText, 2, 1, 1, 1, Qt::AlignVCenter);
  reportPanelSizer->addWidget(mapLabel, 3, 0, 1, 1, Qt::AlignVCenter);
  reportPanelSizer->addWidget(mapPathText, 3, 1, 1, 1, Qt::AlignVCenter);
  reportPanelSizer->addWidget(logLabel, 4, 0, 1, 1, Qt::AlignVCenter);
  reportPanelSizer->addWidget(logPathText, 4, 1, 1, 1, Qt::AlignVCenter);
  reportPanelSizer->setRowMinimumHeight(5, 20);
  reportPanelSizer->addWidget(versionLabel, 6, 0, 1, 1, Qt::AlignVCenter);
  reportPanelSizer->addWidget(versionText, 6, 1, 1, 1, Qt::AlignVCenter);
  reportPanelSizer->addWidget(buildLabel, 7, 0, 1, 1, Qt::AlignVCenter);
  reportPanelSizer->addWidget(buildText, 7, 1, 1, 1, Qt::AlignVCenter);

  auto* reportPanel = new QWidget();
  setBaseWindowColor(reportPanel);
  reportPanel->setLayout(reportPanelSizer);

  auto* buttonBox = new QDialogButtonBox();
  buttonBox->addButton(QDialogButtonBox::Close);
  auto* reportButton = buttonBox->addButton(tr("Report"), QDialogButtonBox::AcceptRole);

  connect(reportButton, &QAbstractButton::clicked, this, []() {
    QDesktopServices::openUrl(QUrl("https://github.com/TrenchBroom/TrenchBroom/issues/new"));
  });
  connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

  auto* outerSizer = new QVBoxLayout();
  outerSizer->setSizeConstraint(QLayout::SetFixedSize);
  outerSizer->setContentsMargins(0, 0, 0, 0);
  outerSizer->addWidget(header);
  outerSizer->addWidget(reportPanel, 1);
  outerSizer->addWidget(buttonBox);

  setLayout(outerSizer);

  // TODO: needs spacing tweaks
}
} // namespace View
} // namespace TrenchBroom
