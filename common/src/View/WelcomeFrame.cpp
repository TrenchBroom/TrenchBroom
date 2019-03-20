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
// FIXME:
//#include "View/RecentDocumentListBox.h"
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
            // FIXME:
            //Centre();
        }

        void WelcomeFrame::createGui() {
            setAttribute(Qt::WA_DeleteOnClose);
            setWindowIconTB(this);
            setWindowTitle("Welcome to TrenchBroom");


            // FIXME:
//            m_recentDocumentListBox = new RecentDocumentListBox(container);
//            m_recentDocumentListBox->setToolTip("Double click on a file to open it");
//            m_recentDocumentListBox->setMaxSize(wxSize(350, wxDefaultCoord));


            QHBoxLayout* innerSizer = new QHBoxLayout();
            innerSizer->addWidget(createAppPanel(), 1, Qt::AlignVCenter);
            innerSizer->addWidget(new BorderLine(nullptr, BorderLine::Direction_Vertical), 0);
            innerSizer->addWidget(new QPushButton("TODO: Recent list"), 1);

            QWidget* innerContent = new QWidget();
            innerContent->setLayout(innerSizer);
            innerSizer->setContentsMargins(0,0,0,0);

//            // FIXME:
//            //innerSizer->addWidget(m_recentDocumentListBox, wxSizerFlags().Expand().Proportion(1));
//            //innerSizer->SetItemMinSize(m_recentDocumentListBox, wxSize(350, 400));


            QVBoxLayout* outerSizer = new QVBoxLayout();
            outerSizer->setContentsMargins(0,0,0,0);
            outerSizer->addWidget(new BorderLine(nullptr));
            outerSizer->addWidget(innerContent);

            QWidget* outerContainer = new QWidget();
            outerContainer->setLayout(outerSizer);
            setCentralWidget(outerContainer);
        }

        void WelcomeFrame::OnCreateNewDocumentClicked() {
            hide();
            TrenchBroomApp& app = TrenchBroomApp::instance();
            if (app.newDocument())
                close();
            else
                show();
        }

        void WelcomeFrame::OnOpenOtherDocumentClicked() {
            const QString fileName = QFileDialog::getOpenFileName(nullptr, "Open Map", "", "Map files (*.map);;Any files (*.*)");

            if (!fileName.isEmpty()) {
                hide();
                TrenchBroomApp& app = TrenchBroomApp::instance();
                if (app.openDocument(fileName.toStdString()))
                    close();
                else
                    show();
            }
        }

        void WelcomeFrame::OnRecentDocumentSelected(RecentDocumentSelectedCommand& event) {
            // FIXME:
#if 0
            hide();
            TrenchBroomApp& app = TrenchBroomApp::instance();
            if (app.openDocument(event.documentPath().asString()))
                close();
            else
                show();
#endif
        }

        QWidget* WelcomeFrame::createAppPanel() {
            QWidget* appPanel = new QWidget();
            AppInfoPanel* infoPanel = new AppInfoPanel(appPanel);

            m_createNewDocumentButton = new QPushButton("New map...");
            m_createNewDocumentButton->setToolTip("Create a new map document");
            m_openOtherDocumentButton = new QPushButton("Browse...");
            m_openOtherDocumentButton->setToolTip("Open an existing map document");

            QHBoxLayout* buttonSizer = new QHBoxLayout();
            buttonSizer->addStretch();
            buttonSizer->addWidget(m_createNewDocumentButton);
            buttonSizer->addSpacing(LayoutConstants::WideHMargin);
            buttonSizer->addWidget(m_openOtherDocumentButton);
            buttonSizer->addStretch();

            QVBoxLayout* outerSizer = new QVBoxLayout();
            outerSizer->addWidget(infoPanel, 0, Qt::AlignHCenter);
            outerSizer->addSpacing(20);
            outerSizer->addLayout(buttonSizer);
            outerSizer->addSpacing(20);

            appPanel->setLayout(outerSizer);

            return appPanel;
        }

        void WelcomeFrame::bindEvents() {
            connect(m_createNewDocumentButton, &QPushButton::clicked, this, &WelcomeFrame::OnCreateNewDocumentClicked);
            connect(m_openOtherDocumentButton, &QPushButton::clicked, this, &WelcomeFrame::OnOpenOtherDocumentClicked);

            // FIXME: implement
//            m_recentDocumentListBox->Bind(RECENT_DOCUMENT_SELECTED_EVENT, &WelcomeFrame::OnRecentDocumentSelected, this);
        }
    }
}
