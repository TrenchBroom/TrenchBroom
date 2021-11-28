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

namespace TrenchBroom::View {
CrashDialog::CrashDialog(
  const IO::Path& reportPath, const IO::Path& mapPath, const IO::Path& logPath)
  : QDialog{} {
  createGui(reportPath, mapPath, logPath);
}

void CrashDialog::createGui(
  const IO::Path& reportPath, const IO::Path& mapPath, const IO::Path& logPath) {
  setWindowTitle(tr("Crash"));

  auto* header = makeHeader(new QLabel{tr("Crash Report")});

  auto* text1 =
    new QLabel{tr("TrenchBroom has crashed, but was able to save a crash report,\n"
                  "a log file and the current state of the map to the following locations.\n\n"
                  "Please create an issue report and upload all three files.")};

  auto* reportLabel = makeEmphasized(new QLabel{tr("Report")});
  auto* reportPathText = new QLabel{IO::pathAsQString(reportPath)};

  auto* mapLabel = makeEmphasized(new QLabel{tr("Map")});
  auto* mapPathText = new QLabel{IO::pathAsQString(mapPath)};

  auto* logLabel = makeEmphasized(new QLabel{tr("Log")});
  auto* logPathText = new QLabel{IO::pathAsQString(logPath)};

  auto* versionLabel = makeEmphasized(new QLabel{tr("Version")});
  auto* versionText = new QLabel{getBuildVersion()};

  auto* buildLabel = makeEmphasized(new QLabel{tr("Build")});
  auto* buildText = new QLabel{getBuildIdStr()};

  auto* reportLayout = new QGridLayout{};
  reportLayout->addWidget(text1, 0, 0, 1, 2);
  reportLayout->setRowMinimumHeight(1, 20);
  reportLayout->addWidget(reportLabel, 2, 0, 1, 1, Qt::AlignVCenter);
  reportLayout->addWidget(reportPathText, 2, 1, 1, 1, Qt::AlignVCenter);
  reportLayout->addWidget(mapLabel, 3, 0, 1, 1, Qt::AlignVCenter);
  reportLayout->addWidget(mapPathText, 3, 1, 1, 1, Qt::AlignVCenter);
  reportLayout->addWidget(logLabel, 4, 0, 1, 1, Qt::AlignVCenter);
  reportLayout->addWidget(logPathText, 4, 1, 1, 1, Qt::AlignVCenter);
  reportLayout->setRowMinimumHeight(5, 20);
  reportLayout->addWidget(versionLabel, 6, 0, 1, 1, Qt::AlignVCenter);
  reportLayout->addWidget(versionText, 6, 1, 1, 1, Qt::AlignVCenter);
  reportLayout->addWidget(buildLabel, 7, 0, 1, 1, Qt::AlignVCenter);
  reportLayout->addWidget(buildText, 7, 1, 1, 1, Qt::AlignVCenter);

  auto* reportPanel = new QWidget{};
  setBaseWindowColor(reportPanel);
  reportPanel->setLayout(reportLayout);

  auto* buttonBox = new QDialogButtonBox{};
  buttonBox->addButton(QDialogButtonBox::Close);
  auto* reportButton = buttonBox->addButton(tr("Report"), QDialogButtonBox::AcceptRole);

  connect(reportButton, &QAbstractButton::clicked, this, []() {
    QDesktopServices::openUrl(QUrl{"https://github.com/TrenchBroom/TrenchBroom/issues/new"});
  });
  connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

  auto* outerLayout = new QVBoxLayout{};
  outerLayout->setSizeConstraint(QLayout::SetFixedSize);
  outerLayout->setContentsMargins(0, 0, 0, 0);
  outerLayout->addWidget(header);
  outerLayout->addWidget(reportPanel, 1);
  outerLayout->addWidget(buttonBox);

  setLayout(outerLayout);

  // TODO: needs spacing tweaks
}
} // namespace TrenchBroom::View
