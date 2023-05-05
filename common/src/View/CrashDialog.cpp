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
#include "View/DialogHeader.h"
#include "View/FormWithSectionsLayout.h"
#include "View/GetVersion.h"
#include "View/QtUtils.h"

#include <QDesktopServices>
#include <QDialogButtonBox>
#include <QLabel>
#include <QPushButton>
#include <QUrl>

namespace TrenchBroom::View
{
CrashDialog::CrashDialog(
  const std::string& reason,
  const std::filesystem::path& reportPath,
  const std::filesystem::path& mapPath,
  const std::filesystem::path& logPath)
  : QDialog{}
{
  createGui(reason, reportPath, mapPath, logPath);
}

void CrashDialog::createGui(
  const std::string& reason,
  const std::filesystem::path& reportPath,
  const std::filesystem::path& mapPath,
  const std::filesystem::path& logPath)
{
  setWindowTitle(tr("Crash"));

  auto* header = new DialogHeader{"Crash Report"};

  auto* text1 = new QLabel{
    tr("TrenchBroom has crashed, but was able to save a crash report,"
       "a log file and the current state of the map to the following locations.\n\n"
       "Please create an issue report and upload all three files.")};
  text1->setWordWrap(true);

  auto* reasonText = new QLabel{QString::fromStdString(reason)};
  auto* reportPathText = new QLabel{IO::pathAsQString(reportPath)};
  auto* mapPathText = new QLabel{IO::pathAsQString(mapPath)};
  auto* logPathText = new QLabel{IO::pathAsQString(logPath)};
  auto* versionText = new QLabel{getBuildVersion()};
  auto* buildText = new QLabel{getBuildIdStr()};

  auto* reportLayout = new FormWithSectionsLayout{};
  reportLayout->setContentsMargins(
    0, LayoutConstants::MediumVMargin, 0, LayoutConstants::MediumVMargin);
  reportLayout->setVerticalSpacing(2);

  reportLayout->addRow(text1);

  reportLayout->addSection("Info");
  reportLayout->addRow("Reason", reasonText);
  reportLayout->addRow("Version", versionText);
  reportLayout->addRow("Build", buildText);

  reportLayout->addSection("Files");
  reportLayout->addRow("Crash Report", reportPathText);
  reportLayout->addRow("Map File", mapPathText);
  reportLayout->addRow("Log File", logPathText);

  auto* buttonBox = new QDialogButtonBox{};
  buttonBox->addButton(QDialogButtonBox::Close);
  auto* reportButton = buttonBox->addButton(tr("Report"), QDialogButtonBox::AcceptRole);

  connect(reportButton, &QAbstractButton::clicked, this, []() {
    QDesktopServices::openUrl(
      QUrl{"https://github.com/TrenchBroom/TrenchBroom/issues/new"});
  });
  connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

  auto* outerLayout = new QVBoxLayout{};
  outerLayout->setSizeConstraint(QLayout::SetFixedSize);
  outerLayout->setContentsMargins(0, 0, 0, 0);
  outerLayout->addWidget(header);
  outerLayout->addLayout(reportLayout, 1);
  outerLayout->addLayout(wrapDialogButtonBox(buttonBox));

  setLayout(outerLayout);

  // TODO: needs spacing tweaks
}
} // namespace TrenchBroom::View
