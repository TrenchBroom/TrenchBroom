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
#include "View/GetVersion.h"

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

            QFont titleFont;
            titleFont.setPointSize(20);
            titleFont.setBold(true);

            QLabel* appName = new QLabel(tr("TrenchBroom"));
            appName->setFont(titleFont);

            QPalette lightText;
            QColor lightColor = lightText.color(QPalette::Disabled, QPalette::WindowText);
            lightText.setColor(QPalette::WindowText, lightColor);

            QFrame* appLine = new QFrame();
            appLine->setFrameShape(QFrame::HLine);
            appLine->setLineWidth(2);
            appLine->setPalette(lightText);
            QLabel* appClaim = new QLabel(tr("Level Editor"));

            ClickableLabel* version = new ClickableLabel(QString(tr("Version ")) % getBuildVersion());
            ClickableLabel* build = new ClickableLabel(QString(tr("Build ")) % getBuildIdStr());

            QFont font;
            font.setPointSize(8);
            version->setFont(font);
            build->setFont(font);

            version->setPalette(lightText);
            build->setPalette(lightText);

            version->setToolTip("Click to copy to clipboard");
            build->setToolTip("Click to copy to clipboard");

            connect(version, &ClickableLabel::clicked, this, &AppInfoPanel::OnClickVersionInfo);
            connect(build, &ClickableLabel::clicked, this, &AppInfoPanel::OnClickVersionInfo);

            QVBoxLayout* sizer = new QVBoxLayout();
            sizer->setSpacing(2);
            sizer->addStretch();
            sizer->addWidget(appIcon, 0, Qt::AlignHCenter);
            sizer->addWidget(appName, 0, Qt::AlignHCenter);
            sizer->addWidget(appLine);
            sizer->addWidget(appClaim, 0, Qt::AlignHCenter);
            sizer->addWidget(version, 0, Qt::AlignHCenter);
            sizer->addWidget(build, 0, Qt::AlignHCenter);
            sizer->addStretch();

            setLayout(sizer);
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
