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

#include "AppInfoPanel.h"

#include "IO/Path.h"
#include "IO/ResourceUtils.h"
#include "View/BorderLine.h"
#include "View/GetVersion.h"
#include "View/wxUtils.h"

#include <QString>
#include <QFont>
#include <QLabel>
#include <QClipboard>
#include <QStringBuilder>
#include <QApplication>
#include <QFrame>
#include <QVBoxLayout>
#include <QPushButton>

namespace TrenchBroom {
    namespace View {
        AppInfoPanel::AppInfoPanel(QWidget* parent) :
        QWidget(parent) {
            createGui();
        }

        void AppInfoPanel::createGui() {
            QPixmap appIconImage = IO::loadPixmapResource("AppIcon.png");
            QLabel* appIcon = new QLabel();
            appIcon->setPixmap(appIconImage);

            QLabel* appName = new QLabel(tr("TrenchBroom"));
            makeHeader(appName);

            BorderLine* appLine = new BorderLine(BorderLine::Direction_Horizontal);
            QLabel* appClaim = new QLabel(tr("Level Editor"));

            ClickableLabel* version = new ClickableLabel(QString(tr("Version ")) % getBuildVersion());
            ClickableLabel* build = new ClickableLabel(QString(tr("Build ")) % getBuildIdStr());

            makeInfo(version);
            makeInfo(build);

            version->setToolTip("Click to copy to clipboard");
            build->setToolTip("Click to copy to clipboard");

            connect(version, &ClickableLabel::clicked, this, &AppInfoPanel::OnClickVersionInfo);
            connect(build, &ClickableLabel::clicked, this, &AppInfoPanel::OnClickVersionInfo);

            auto* layout = new QVBoxLayout();
            layout->setContentsMargins(20, 20, 20, 20);
            layout->setSpacing(2);
            layout->addStretch();
            layout->addWidget(appIcon, 0, Qt::AlignHCenter);
            layout->addWidget(appName, 0, Qt::AlignHCenter);
            layout->addWidget(appLine);
            layout->addWidget(appClaim, 0, Qt::AlignHCenter);
            layout->addWidget(version, 0, Qt::AlignHCenter);
            layout->addWidget(build, 0, Qt::AlignHCenter);
            layout->addStretch();

            setLayout(layout);
        }

        void AppInfoPanel::OnClickVersionInfo() {
            QClipboard *clipboard = QApplication::clipboard();
            const QString str = QString("TrenchBroom ") % getBuildVersion() % QString(" Build ") % getBuildIdStr();
            clipboard->setText(str);
        }

        ClickableLabel::ClickableLabel(const QString& text, QWidget* parent)
        : QLabel(text, parent) {}

        void ClickableLabel::mousePressEvent(QMouseEvent*) {
            emit clicked();
        }
    }
}
