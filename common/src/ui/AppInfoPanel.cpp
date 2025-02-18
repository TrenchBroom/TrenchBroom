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

#include "AppInfoPanel.h"

#include <QApplication>
#include <QClipboard>
#include <QLabel>
#include <QString>
#include <QStringBuilder>
#include <QVBoxLayout>

#include "TrenchBroomApp.h"
#include "io/ResourceUtils.h"
#include "ui/BorderLine.h"
#include "ui/ClickableLabel.h"
#include "ui/GetVersion.h"
#include "ui/QtUtils.h"
#include "upd/Updater.h"

namespace tb::ui
{

AppInfoPanel::AppInfoPanel(QWidget* parent)
  : QWidget{parent}
{
  auto appIconImage = io::loadPixmapResource("AppIcon.png");
  auto* appIcon = new QLabel{};
  appIcon->setPixmap(appIconImage);

  auto* appName = new QLabel{tr("TrenchBroom")};
  makeHeader(appName);

  auto* appLine = new BorderLine{};
  auto* appClaim = new QLabel{tr("Level Editor")};

  auto version = new ClickableLabel{tr("Version %1").arg(getBuildVersion())};
  auto build = new ClickableLabel{tr("Build %1").arg(getBuildIdStr())};
  auto qtVersion =
    new ClickableLabel{tr("Qt %1").arg(QString::fromLocal8Bit(qVersion()))};

  makeInfo(version);
  makeInfo(build);
  makeInfo(qtVersion);
  build->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

  const auto tooltip = tr("Click to copy to clipboard");
  version->setToolTip(tooltip);
  build->setToolTip(tooltip);
  qtVersion->setToolTip(tooltip);

  connect(version, &ClickableLabel::clicked, this, &AppInfoPanel::versionInfoClicked);
  connect(build, &ClickableLabel::clicked, this, &AppInfoPanel::versionInfoClicked);
  connect(qtVersion, &ClickableLabel::clicked, this, &AppInfoPanel::versionInfoClicked);

  auto& app = TrenchBroomApp::instance();
  auto* updateIndicator = app.updater().createUpdateIndicator();
  makeInfo(updateIndicator);

  auto* versionLayout = new QHBoxLayout{};
  versionLayout->setContentsMargins(0, 0, 0, 0);
  versionLayout->setSpacing(LayoutConstants::MediumHMargin);
  versionLayout->addWidget(version);
  versionLayout->addWidget(updateIndicator);

  auto* versionWidget = new QWidget{};
  versionWidget->setLayout(versionLayout);

  auto* layout = new QVBoxLayout{};
  layout->setContentsMargins(20, 20, 20, 20);
  layout->setSpacing(2);
  layout->addStretch();
  layout->addWidget(appIcon, 0, Qt::AlignHCenter);
  layout->addWidget(appName, 0, Qt::AlignHCenter);
  layout->addWidget(appLine);
  layout->addWidget(appClaim, 0, Qt::AlignHCenter);
  layout->addWidget(versionWidget, 0, Qt::AlignHCenter);
  layout->addWidget(build, 0, Qt::AlignHCenter);
  layout->addWidget(qtVersion, 0, Qt::AlignHCenter);
  layout->addStretch();

  setLayout(layout);
}

void AppInfoPanel::versionInfoClicked()
{
  const auto str =
    tr("TrenchBroom %1 Build %2").arg(getBuildVersion()).arg(getBuildIdStr());

  QApplication::clipboard()->setText(str);
}

} // namespace tb::ui
