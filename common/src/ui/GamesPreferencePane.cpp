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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "GamesPreferencePane.h"

#include <QAction>
#include <QBoxLayout>
#include <QDesktopServices>
#include <QDir>
#include <QFileDialog>
#include <QFormLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QStackedWidget>
#include <QToolButton>
#include <QWidget>

#include "FileLogger.h"
#include "PreferenceManager.h"
#include "TrenchBroomApp.h"
#include "fs/DiskIO.h"
#include "mdl/GameConfig.h"
#include "mdl/GameManager.h"
#include "ui/BitmapButton.h"
#include "ui/BorderLine.h"
#include "ui/FileDialogDefaultDir.h"
#include "ui/FormWithSectionsLayout.h"
#include "ui/GameEngineDialog.h"
#include "ui/GameListBox.h"
#include "ui/ImageUtils.h"
#include "ui/MapDocument.h"
#include "ui/MiniToolBarLayout.h"
#include "ui/QPathUtils.h"
#include "ui/QtUtils.h"
#include "ui/SystemPaths.h"
#include "ui/ViewConstants.h"

namespace tb::ui
{

GamesPreferencePane::GamesPreferencePane(MapDocument* document, QWidget* parent)
  : PreferencePane{parent}
  , m_document{document}
{
  createGui();
  updateControls();
  m_gameListBox->setFocus();
}

void GamesPreferencePane::createGui()
{
  m_gameListBox = new GameListBox{};
  m_gameListBox->selectGame(0);
  m_gameListBox->setMaximumWidth(220);
  m_gameListBox->setMinimumHeight(300);

  m_defaultPage = createDefaultPage(tr("Select a game."));

  m_stackedWidget = new QStackedWidget{};
  m_stackedWidget->addWidget(m_defaultPage);

  auto* showUserConfigDirButton =
    createBitmapButton("Folder.svg", tr("Open custom game configurations folder"));
  connect(
    showUserConfigDirButton,
    &QAbstractButton::clicked,
    this,
    &GamesPreferencePane::showUserConfigDirClicked);

  auto* buttonLayout = createMiniToolBarLayout(showUserConfigDirButton);

  auto* glbLayout = new QVBoxLayout{};
  glbLayout->addWidget(m_gameListBox);
  glbLayout->addWidget(new BorderLine(BorderLine::Direction::Horizontal));
  glbLayout->addLayout(buttonLayout);

  auto* stwLayout = new QVBoxLayout{};
  stwLayout->setContentsMargins(
    LayoutConstants::DialogOuterMargin,
    LayoutConstants::DialogOuterMargin,
    LayoutConstants::DialogOuterMargin,
    LayoutConstants::DialogOuterMargin);
  stwLayout->setSpacing(LayoutConstants::WideVMargin);
  stwLayout->addWidget(m_stackedWidget, 1, Qt::AlignTop);

  auto* layout = new QHBoxLayout{};
  layout->setContentsMargins(QMargins{});
  layout->setSpacing(0);
  setLayout(layout);

  layout->addLayout(glbLayout);
  layout->addWidget(new BorderLine(BorderLine::Direction::Vertical));
  layout->addSpacing(LayoutConstants::MediumVMargin);
  layout->addLayout(stwLayout, 1);

  setMinimumWidth(600);

  connect(
    m_gameListBox, &GameListBox::currentGameChanged, this, [&]() { updateControls(); });
}

void GamesPreferencePane::showUserConfigDirClicked()
{
  const auto path = SystemPaths::userGamesDirectory().lexically_normal();

  fs::Disk::createDirectory(path) | kdl::transform([&](auto) {
    const auto url = QUrl::fromLocalFile(pathAsQPath(path));
    QDesktopServices::openUrl(url);
  }) | kdl::transform_error([&](auto e) {
    if (m_document)
    {
      m_document->logger().error() << e.msg;
    }
    else
    {
      FileLogger::instance().error() << e.msg;
    }
  });
}

bool GamesPreferencePane::canResetToDefaults()
{
  return false;
}

void GamesPreferencePane::doResetToDefaults() {}

void GamesPreferencePane::updateControls()
{
  m_gameListBox->updateGameInfos();

  const auto desiredGame = m_gameListBox->selectedGameName();
  if (desiredGame.empty())
  {
    m_stackedWidget->setCurrentWidget(m_defaultPage);
  }
  else if (m_currentGamePage && m_currentGamePage->gameName() == desiredGame)
  {
    // refresh the current page
    m_currentGamePage->updateControls();
  }
  else
  {
    // build a new current page
    delete m_currentGamePage;
    m_currentGamePage = new GamePreferencePane{m_document, desiredGame};

    m_stackedWidget->addWidget(m_currentGamePage);
    m_stackedWidget->setCurrentWidget(m_currentGamePage);

    connect(
      m_currentGamePage,
      &GamePreferencePane::requestUpdate,
      this,
      &GamesPreferencePane::updateControls);
  }
}

bool GamesPreferencePane::validate()
{
  return true;
}

// GamePreferencePane

GamePreferencePane::GamePreferencePane(
  MapDocument* document, std::string gameName, QWidget* parent)
  : QWidget{parent}
  , m_document{document}
  , m_gameName{std::move(gameName)}
{
  createGui();
}

void GamePreferencePane::createGui()
{
  m_gamePathText = new QLineEdit{};
  m_gamePathText->setPlaceholderText(tr("Click on the button to change..."));
  connect(m_gamePathText, &QLineEdit::editingFinished, this, [this]() {
    updateGamePath(m_gamePathText->text());
  });

  auto* validDirectoryIcon = new QAction{m_gamePathText};
  m_gamePathText->addAction(validDirectoryIcon, QLineEdit::TrailingPosition);
  connect(
    m_gamePathText,
    &QLineEdit::textChanged,
    this,
    [validDirectoryIcon](const QString& text) {
      if (text.isEmpty() || QDir{text}.exists())
      {
        validDirectoryIcon->setToolTip("");
        validDirectoryIcon->setIcon(QIcon{});
      }
      else
      {
        validDirectoryIcon->setToolTip(tr("Directory not found"));
        validDirectoryIcon->setIcon(loadSVGIcon("IssueBrowser.svg"));
      }
    });

  auto* chooseGamePathButton = new QPushButton{tr("...")};
  connect(
    chooseGamePathButton,
    &QPushButton::clicked,
    this,
    &GamePreferencePane::chooseGamePathClicked);

  auto* configureEnginesButton = new QPushButton{tr("Configure engines...")};
  connect(
    configureEnginesButton,
    &QPushButton::clicked,
    this,
    &GamePreferencePane::configureEnginesClicked);

  auto* gamePathLayout = new QHBoxLayout{};
  gamePathLayout->setContentsMargins(QMargins{});
  gamePathLayout->setSpacing(LayoutConstants::MediumHMargin);
  gamePathLayout->addWidget(m_gamePathText, 1);
  gamePathLayout->addWidget(chooseGamePathButton);

  auto* layout = new FormWithSectionsLayout{};
  layout->setContentsMargins(0, LayoutConstants::MediumVMargin, 0, 0);
  layout->setVerticalSpacing(2);
  layout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);

  layout->addSection(QString::fromStdString(m_gameName));
  layout->addRow(tr("Game Path"), gamePathLayout);
  layout->addRow("", configureEnginesButton);

  layout->addSection(tr("Compilation Tools"));

  auto& app = TrenchBroomApp::instance();
  auto& gameManager = app.gameManager();
  auto* gameInfo = gameManager.gameInfo(m_gameName);
  contract_assert(gameInfo);

  for (auto& tool : gameInfo->gameConfig.compilationTools)
  {
    const auto toolName = tool.name;
    auto& toolPathPref = tool.pathPreference;

    auto* edit = new QLineEdit{};
    edit->setText(pathAsQString(pref(tool.pathPreference)));
    if (tool.description)
    {
      edit->setToolTip(QString::fromStdString(*tool.description));
    }
    connect(edit, &QLineEdit::editingFinished, this, [&toolPathPref, edit]() {
      auto& prefs = PreferenceManager::instance();
      prefs.set(toolPathPref, pathFromQString(edit->text()));
    });

    m_toolPathEditors.emplace_back(&tool, edit);

    auto* browseButton = new QPushButton{"..."};
    connect(
      browseButton, &QPushButton::clicked, this, [this, &toolPathPref, toolName, edit]() {
        const auto pathStr = QFileDialog::getOpenFileName(
          this,
          tr("%1 Path").arg(QString::fromStdString(toolName)),
          fileDialogDefaultDirectory(FileDialogDir::CompileTool));
        if (!pathStr.isEmpty())
        {
          edit->setText(pathStr);
          auto& prefs = PreferenceManager::instance();
          prefs.set(toolPathPref, pathFromQString(edit->text()));
          emit requestUpdate();
        }
      });

    auto* rowLayout = new QHBoxLayout{};
    rowLayout->setContentsMargins(QMargins{});
    rowLayout->setSpacing(LayoutConstants::MediumHMargin);
    rowLayout->addWidget(edit, 1);
    rowLayout->addWidget(browseButton);

    layout->addRow(QString::fromStdString(tool.name), rowLayout);
  }

  setLayout(layout);
}

void GamePreferencePane::chooseGamePathClicked()
{
  const auto pathStr = QFileDialog::getExistingDirectory(
    this, tr("Game Path"), fileDialogDefaultDirectory(FileDialogDir::GamePath));
  if (!pathStr.isEmpty())
  {
    updateGamePath(pathStr);
  }
}

void GamePreferencePane::updateGamePath(const QString& str)
{
  auto& app = TrenchBroomApp::instance();
  auto& gameManager = app.gameManager();
  auto* gameInfo = gameManager.gameInfo(m_gameName);
  contract_assert(gameInfo);

  updateFileDialogDefaultDirectoryWithDirectory(FileDialogDir::GamePath, str);

  auto& prefs = PreferenceManager::instance();
  prefs.set(gameInfo->gamePathPreference, pathFromQString(str));
  emit requestUpdate();
}

void GamePreferencePane::configureEnginesClicked()
{
  auto& logger = m_document ? m_document->logger() : FileLogger::instance();
  auto dialog = GameEngineDialog{m_gameName, logger, this};
  dialog.exec();
}

const std::string& GamePreferencePane::gameName() const
{
  return m_gameName;
}

void GamePreferencePane::updateControls()
{
  auto& app = TrenchBroomApp::instance();
  const auto& gameManager = app.gameManager();
  const auto* gameInfo = gameManager.gameInfo(m_gameName);
  contract_assert(gameInfo);

  // Refresh tool paths from preferences
  for (const auto& [tool, toolPathEditor] : m_toolPathEditors)
  {
    const auto& toolPath = pref(tool->pathPreference);
    toolPathEditor->setText(pathAsQString(toolPath));
  }

  // Refresh game path
  const auto gamePath = pref(gameInfo->gamePathPreference);
  m_gamePathText->setText(pathAsQString(gamePath));
}

} // namespace tb::ui
