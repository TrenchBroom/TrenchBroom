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

#include "LaunchGameEngineDialog.h"

#include <QCompleter>
#include <QDialogButtonBox>
#include <QDir>
#include <QLabel>
#include <QMessageBox>
#include <QProcess>
#include <QPushButton>

#include "EL/EvaluationContext.h"
#include "EL/Interpolator.h"
#include "IO/PathQt.h"
#include "Model/Game.h"
#include "Model/GameConfig.h"
#include "Model/GameEngineProfile.h"
#include "Model/GameFactory.h"
#include "View/BorderLine.h"
#include "View/CompilationVariables.h"
#include "View/CurrentGameIndicator.h"
#include "View/GameEngineDialog.h"
#include "View/GameEngineProfileListBox.h"
#include "View/MapDocument.h"
#include "View/MultiCompletionLineEdit.h"
#include "View/QtUtils.h"
#include "View/VariableStoreModel.h"
#include "View/ViewConstants.h"

#include "kdl/memory_utils.h"
#include "kdl/string_utils.h"

#include <string>

namespace TrenchBroom
{
namespace View
{
LaunchGameEngineDialog::LaunchGameEngineDialog(
  std::weak_ptr<MapDocument> document, QWidget* parent)
  : QDialog{parent}
  , m_document{std::move(document)}
{
  createGui();
}

void LaunchGameEngineDialog::createGui()
{
  setWindowIconTB(this);
  setWindowTitle("Launch Engine");

  auto document = kdl::mem_lock(m_document);
  const auto& gameName = document->game()->config().name;
  auto* gameIndicator = new CurrentGameIndicator{gameName};

  auto* midPanel = new QWidget{this};

  auto& gameFactory = Model::GameFactory::instance();
  const auto& gameConfig = gameFactory.gameConfig(gameName);
  m_config = gameConfig.gameEngineConfig;
  m_gameEngineList = new GameEngineProfileListBox{m_config};
  m_gameEngineList->setEmptyText(
    R"(Click the 'Configure engines...' button to create a game engine profile.)");
  m_gameEngineList->setMinimumSize(250, 280);

  auto* header = new QLabel{"Launch Engine"};
  makeHeader(header);

  auto* message = new QLabel{
    R"(Select a game engine from the list on the right and edit the commandline parameters in the text box below. You can use variables to refer to the map name and other values.)"};
  message->setWordWrap(true);

  auto* openPreferencesButton = new QPushButton{"Configure engines..."};

  auto* parameterLabel = new QLabel{"Parameters"};
  makeEmphasized(parameterLabel);

  m_parameterText = new MultiCompletionLineEdit{};
  m_parameterText->setFont(Fonts::fixedWidthFont());
  m_parameterText->setMultiCompleter(new QCompleter{new VariableStoreModel{variables()}});
  m_parameterText->setWordDelimiters(
    QRegularExpression{"\\$"}, QRegularExpression{"\\}"});

  auto* midLeftLayout = new QVBoxLayout{};
  midLeftLayout->setContentsMargins(0, 0, 0, 0);
  midLeftLayout->setSpacing(0);
  midLeftLayout->addSpacing(20);
  midLeftLayout->addWidget(header);
  midLeftLayout->addSpacing(20);
  midLeftLayout->addWidget(message);
  midLeftLayout->addSpacing(10);
  midLeftLayout->addWidget(openPreferencesButton, 0, Qt::AlignHCenter);
  midLeftLayout->addStretch(1);
  midLeftLayout->addWidget(parameterLabel);
  midLeftLayout->addSpacing(LayoutConstants::NarrowVMargin);
  midLeftLayout->addWidget(m_parameterText);
  midLeftLayout->addSpacing(20);

  auto* midLayout = new QHBoxLayout{};
  midLayout->setContentsMargins(0, 0, 0, 0);
  midLayout->setSpacing(0);
  midLayout->addSpacing(20);
  midLayout->addLayout(midLeftLayout, 1);
  midLayout->addSpacing(20);
  midLayout->addWidget(new BorderLine{BorderLine::Direction::Vertical});
  midLayout->addWidget(m_gameEngineList);
  midPanel->setLayout(midLayout);

  auto* buttonBox = new QDialogButtonBox{};
  m_launchButton = buttonBox->addButton("Launch", QDialogButtonBox::AcceptRole);
  auto* closeButton = buttonBox->addButton("Close", QDialogButtonBox::RejectRole);

  auto* outerLayout = new QVBoxLayout{};
  outerLayout->setContentsMargins(0, 0, 0, 0);
  outerLayout->setSpacing(0);
  outerLayout->addWidget(gameIndicator);
  outerLayout->addWidget(new BorderLine{BorderLine::Direction::Horizontal});
  outerLayout->addWidget(midPanel, 1);
  outerLayout->addLayout(wrapDialogButtonBox(buttonBox));

  setLayout(outerLayout);

  m_parameterText->setEnabled(false);
  m_launchButton->setEnabled(false);

  connect(
    openPreferencesButton,
    &QPushButton::clicked,
    this,
    &LaunchGameEngineDialog::editGameEngines);

  connect(
    m_parameterText,
    &QLineEdit::textChanged,
    this,
    &LaunchGameEngineDialog::parametersChanged);
  connect(
    m_parameterText,
    &QLineEdit::returnPressed,
    this,
    &LaunchGameEngineDialog::launchEngine);

  connect(
    m_launchButton, &QPushButton::clicked, this, &LaunchGameEngineDialog::launchEngine);
  connect(closeButton, &QPushButton::clicked, this, &LaunchGameEngineDialog::close);

  connect(
    m_gameEngineList,
    &GameEngineProfileListBox::currentProfileChanged,
    this,
    &LaunchGameEngineDialog::gameEngineProfileChanged);
  connect(
    m_gameEngineList,
    &GameEngineProfileListBox::profileSelected,
    this,
    &LaunchGameEngineDialog::launchEngine);

  if (m_gameEngineList->count() > 0)
  {
    m_gameEngineList->setCurrentRow(0);
  }
}

void LaunchGameEngineDialog::reloadConfig()
{
  auto document = kdl::mem_lock(m_document);
  const auto& gameName = document->game()->config().name;

  auto& gameFactory = Model::GameFactory::instance();
  const auto& gameConfig = gameFactory.gameConfig(gameName);
  m_config = gameConfig.gameEngineConfig;

  m_gameEngineList->setConfig(m_config);
}

LaunchGameEngineVariables LaunchGameEngineDialog::variables() const
{
  return LaunchGameEngineVariables{kdl::mem_lock(m_document)};
}

void LaunchGameEngineDialog::gameEngineProfileChanged()
{
  m_lastProfile = m_gameEngineList->selectedProfile();
  m_parameterText->setText(
    m_lastProfile ? QString::fromStdString(m_lastProfile->parameterSpec) : "");
  m_parameterText->setEnabled(m_lastProfile != nullptr);
  m_launchButton->setEnabled(m_lastProfile != nullptr);
}

void LaunchGameEngineDialog::parametersChanged(const QString& text)
{
  if (auto* profile = m_gameEngineList->selectedProfile())
  {
    profile->parameterSpec = text.toStdString();
  }
}

void LaunchGameEngineDialog::editGameEngines()
{
  saveConfig();

  auto dialog = GameEngineDialog{kdl::mem_lock(m_document)->game()->config().name, this};
  dialog.exec();

  const auto previousRow = m_gameEngineList->currentRow();

  // reload m_config as it may have been changed by the GameEngineDialog
  reloadConfig();

  if (m_gameEngineList->count() > 0)
  {
    if (previousRow >= 0)
    {
      m_gameEngineList->setCurrentRow(
        std::min(previousRow, m_gameEngineList->count() - 1));
    }
    else
    {
      m_gameEngineList->setCurrentRow(0);
    }
  }
}

void LaunchGameEngineDialog::launchEngine()
{
  try
  {
    const auto* profile = m_gameEngineList->selectedProfile();
    ensure(profile != nullptr, "profile is null");

    const auto parameters =
      EL::interpolate(profile->parameterSpec, EL::EvaluationContext{variables()});

    const auto workDir = IO::pathAsQString(profile->path.parent_path());

#ifdef __APPLE__
    // We have to launch apps via the 'open' command so that we can properly pass
    // parameters.
    const auto arguments = QStringList{
      "-a",
      IO::pathAsQString(profile->path),
      "--args",
      QString::fromStdString(parameters)};

    if (!QProcess::startDetached("/usr/bin/open", arguments, workDir))
    {
      throw Exception("Unknown error");
    }
#else
    const auto commandAndArgs = QString::fromLatin1("\"%1\" %2")
                                  .arg(IO::pathAsQString(profile->path))
                                  .arg(QString::fromStdString(parameters));

    const auto oldWorkDir = QDir::currentPath();
    QDir::setCurrent(workDir);
    const auto success = QProcess::startDetached(commandAndArgs);
    QDir::setCurrent(oldWorkDir);

    if (!success)
    {
      throw Exception("Unknown error");
    }
#endif

    accept();
  }
  catch (const Exception& e)
  {
    const auto message = kdl::str_to_string("Could not launch game engine: ", e.what());
    QMessageBox::critical(
      this, "TrenchBroom", QString::fromStdString(message), QMessageBox::Ok);
  }
}

void LaunchGameEngineDialog::done(const int r)
{
  saveConfig();

  QDialog::done(r);
}

void LaunchGameEngineDialog::saveConfig()
{
  auto document = kdl::mem_lock(m_document);
  const auto& gameName = document->game()->config().name;
  auto& gameFactory = Model::GameFactory::instance();
  gameFactory.saveGameEngineConfig(gameName, m_config, document->logger());
}
} // namespace View
} // namespace TrenchBroom
