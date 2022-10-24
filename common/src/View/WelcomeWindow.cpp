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

#include "WelcomeWindow.h"

#include "IO/PathQt.h"
#include "TrenchBroomApp.h"
#include "View/AppInfoPanel.h"
#include "View/BorderLine.h"
#include "View/QtUtils.h"
#include "View/RecentDocumentListBox.h"
#include "View/ViewConstants.h"

#include <QCloseEvent>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QPushButton>

namespace TrenchBroom::View
{
WelcomeWindow::WelcomeWindow()
  : QMainWindow{nullptr, Qt::Dialog} // Qt::Dialog flag centers window on Ubuntu
  , m_recentDocumentListBox{nullptr}
  , m_createNewDocumentButton{nullptr}
  , m_openOtherDocumentButton{nullptr}
{
  createGui();
}

void WelcomeWindow::createGui()
{
  setWindowIconTB(this);
  setWindowTitle("Welcome to TrenchBroom");

  m_recentDocumentListBox = new RecentDocumentListBox{};
  m_recentDocumentListBox->setToolTip("Double click on a file to open it");
  m_recentDocumentListBox->setFixedWidth(300);
  m_recentDocumentListBox->setSizePolicy(
    QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

  connect(
    m_recentDocumentListBox,
    &RecentDocumentListBox::loadRecentDocument,
    this,
    &WelcomeWindow::openDocument);

  auto* innerLayout = new QHBoxLayout{};
  innerLayout->setContentsMargins(QMargins{});
  innerLayout->setSpacing(0);

  auto* appPanel = createAppPanel();

  innerLayout->addWidget(appPanel, 0, Qt::AlignTop);
  innerLayout->addWidget(new BorderLine{BorderLine::Direction::Vertical}, 0);
  innerLayout->addWidget(m_recentDocumentListBox, 1);

  auto* container = new QWidget{};
  auto* outerLayout = new QVBoxLayout{};
  outerLayout->setContentsMargins(QMargins{});
  outerLayout->setSpacing(0);

  outerLayout->addLayout(innerLayout);
  insertTitleBarSeparator(outerLayout);

  container->setLayout(outerLayout);

  setCentralWidget(container);
  setFixedSize(700, 500);
}

QWidget* WelcomeWindow::createAppPanel()
{
  auto* appPanel = new QWidget{};
  auto* infoPanel = new AppInfoPanel{appPanel};

  m_createNewDocumentButton = new QPushButton{"New map..."};
  m_createNewDocumentButton->setToolTip("Create a new map document");
  m_openOtherDocumentButton = new QPushButton{"Browse..."};
  m_openOtherDocumentButton->setToolTip("Open an existing map document");

  connect(
    m_createNewDocumentButton,
    &QPushButton::clicked,
    this,
    &WelcomeWindow::createNewDocument);
  connect(
    m_openOtherDocumentButton,
    &QPushButton::clicked,
    this,
    &WelcomeWindow::openOtherDocument);

  auto* buttonLayout = new QHBoxLayout{};
  buttonLayout->setContentsMargins(0, 0, 0, 0);
  buttonLayout->setSpacing(LayoutConstants::WideHMargin);
  buttonLayout->addStretch();
  buttonLayout->addWidget(m_createNewDocumentButton);
  buttonLayout->addWidget(m_openOtherDocumentButton);
  buttonLayout->addStretch();

  auto* outerLayout = new QVBoxLayout{};
  outerLayout->setContentsMargins(0, 0, 0, 0);
  outerLayout->setSpacing(0);
  outerLayout->addWidget(infoPanel, 0, Qt::AlignHCenter);
  outerLayout->addSpacing(20);
  outerLayout->addLayout(buttonLayout);
  outerLayout->addSpacing(20);
  appPanel->setLayout(outerLayout);

  return appPanel;
}

void WelcomeWindow::createNewDocument()
{
  auto& app = TrenchBroomApp::instance();
  if (!app.newDocument())
  {
    show();
  }
}

void WelcomeWindow::openOtherDocument()
{
  const auto pathStr = QFileDialog::getOpenFileName(
    nullptr,
    tr("Open Map"),
    fileDialogDefaultDirectory(FileDialogDir::Map),
    "Map files (*.map);;Any files (*.*)");
  const auto path = IO::pathFromQString(pathStr);

  if (!path.isEmpty())
  {
    updateFileDialogDefaultDirectoryWithFilename(FileDialogDir::Map, pathStr);
    openDocument(path);
  }
}

void WelcomeWindow::openDocument(const IO::Path& path)
{
  auto& app = TrenchBroomApp::instance();
  if (!app.openDocument(path))
  {
    show();
  }
}
} // namespace TrenchBroom::View
