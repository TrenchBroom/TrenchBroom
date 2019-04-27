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

#include "WelcomeFrame.h"

#include "TrenchBroomApp.h"
#include "View/AppInfoPanel.h"
#include "View/BorderLine.h"
#include "View/ViewConstants.h"
#include "View/RecentDocumentListBox.h"
// FIXME:
//#include "View/RecentDocumentSelectedCommand.h"
#include "View/wxUtils.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QFileDialog>

namespace TrenchBroom {
    namespace View {
        WelcomeFrame::WelcomeFrame() :
        QMainWindow() {
            //nullptr, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxCAPTION | wxCLOSE_BOX | wxCLIP_CHILDREN) {
            createGui();
            bindEvents();
            centerOnScreen(this);
        }

        void WelcomeFrame::createGui() {
            setAttribute(Qt::WA_DeleteOnClose);
            setWindowIconTB(this);
            setWindowTitle("Welcome to TrenchBroom");

            // FIXME:
            m_recentDocumentListBox = new RecentDocumentListBox();
            m_recentDocumentListBox->setToolTip("Double click on a file to open it");
            m_recentDocumentListBox->setFixedWidth(300);
            m_recentDocumentListBox->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
            connect(m_recentDocumentListBox, &RecentDocumentListBox::loadRecentDocument, this, &WelcomeFrame::onRecentDocumentSelected);

            auto* innerLayout = new QHBoxLayout();
            innerLayout->setContentsMargins(QMargins());
            innerLayout->setSpacing(0);

            auto* appPanel = createAppPanel();

            innerLayout->addWidget(appPanel, 0, Qt::AlignTop);
            innerLayout->addWidget(new BorderLine(BorderLine::Direction_Vertical), 0);
            innerLayout->addWidget(m_recentDocumentListBox, 1);

            auto* container = new QWidget();
            auto* outerLayout = new QVBoxLayout();
            outerLayout->setContentsMargins(QMargins());
            outerLayout->setSpacing(0);

            // outerLayout->addWidget(new BorderLine());
            outerLayout->addLayout(innerLayout);
            container->setLayout(outerLayout);

            setCentralWidget(container);
            setFixedSize(700, 500);
        }

        void WelcomeFrame::onCreateNewDocumentClicked() {
            hide();
            TrenchBroomApp& app = TrenchBroomApp::instance();
            if (app.newDocument()) {
                close();
            } else {
                show();
            }
        }

        void WelcomeFrame::onOpenOtherDocumentClicked() {
            const auto pathStr = QFileDialog::getOpenFileName(nullptr, "Open Map", "", "Map files (*.map);;Any files (*.*)");
            const auto path = IO::Path(pathStr.toStdString());

            if (!path.isEmpty()) {
                hide();
                TrenchBroomApp& app = TrenchBroomApp::instance();
                if (app.openDocument(path)) {
                    close();
                } else {
                    show();
                }
            }
        }

        void WelcomeFrame::onRecentDocumentSelected(const IO::Path& path) {
            hide();
            TrenchBroomApp& app = TrenchBroomApp::instance();
            if (app.openDocument(path)) {
                close();
            } else {
                show();
            }
        }

        QWidget* WelcomeFrame::createAppPanel() {
            auto* appPanel = new QWidget();
            auto* infoPanel = new AppInfoPanel(appPanel);

            m_createNewDocumentButton = new QPushButton("New map...");
            m_createNewDocumentButton->setToolTip("Create a new map document");
            m_openOtherDocumentButton = new QPushButton("Browse...");
            m_openOtherDocumentButton->setToolTip("Open an existing map document");

            auto* buttonSizer = new QHBoxLayout();
            buttonSizer->addStretch();
            buttonSizer->addWidget(m_createNewDocumentButton);
            buttonSizer->addSpacing(LayoutConstants::WideHMargin);
            buttonSizer->addWidget(m_openOtherDocumentButton);
            buttonSizer->addStretch();

            auto* outerSizer = new QVBoxLayout();
            outerSizer->addWidget(infoPanel, 0, Qt::AlignHCenter);
            outerSizer->addSpacing(20);
            outerSizer->addLayout(buttonSizer);
            outerSizer->addSpacing(20);
            appPanel->setLayout(outerSizer);

            return appPanel;
        }

        void WelcomeFrame::bindEvents() {
            connect(m_createNewDocumentButton, &QPushButton::clicked, this, &WelcomeFrame::onCreateNewDocumentClicked);
            connect(m_openOtherDocumentButton, &QPushButton::clicked, this, &WelcomeFrame::onOpenOtherDocumentClicked);

            // FIXME: implement
//            m_recentDocumentListBox->Bind(RECENT_DOCUMENT_SELECTED_EVENT, &WelcomeFrame::OnRecentDocumentSelected, this);
        }
    }
}
