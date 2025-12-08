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

#include "LaunchGameEngineDialog.h"

#include <QCompleter>
#include <QDialogButtonBox>
#include <QDir>
#include <QLabel>
#include <QMessageBox>
#include <QProcess>
#include <QPushButton>

#include "Logger.h"
#include "TrenchBroomApp.h"
#include "mdl/Game.h"
#include "mdl/GameConfig.h"
#include "mdl/GameEngineProfile.h"
#include "mdl/GameInfo.h"
#include "mdl/GameManager.h"
#include "mdl/Map.h"
#include "ui/BorderLine.h"
#include "ui/CompilationVariables.h"
#include "ui/CurrentGameIndicator.h"
#include "ui/GameEngineDialog.h"
#include "ui/GameEngineProfileListBox.h"
#include "ui/LaunchGameEngine.h"
#include "ui/MapDocument.h"
#include "ui/MultiCompletionLineEdit.h"
#include "ui/QtUtils.h"
#include "ui/VariableStoreModel.h"
#include "ui/ViewConstants.h"

#include "kd/contracts.h"
#include "kd/string_utils.h"

#include <string>

namespace tb::ui
{

LaunchGameEngineDialog::LaunchGameEngineDialog(MapDocument& document, QWidget* parent)
  : QDialog{parent}
  , m_document{document}
{
  createGui();
}

void LaunchGameEngineDialog::createGui()
{
  setWindowIconTB(this);
  setWindowTitle("Launch Engine");

  const auto& map = m_document.map();
  const auto& gameConfig = map.game()->config();
  auto* gameIndicator = new CurrentGameIndicator{gameConfig.name};

  auto* midPanel = new QWidget{this};

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
  outerLayout->addWidget(new BorderLine{});
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
  const auto& map = m_document.map();
  const auto& gameConfig = map.game()->config();

  m_config = gameConfig.gameEngineConfig;
  m_gameEngineList->setConfig(m_config);
}

LaunchGameEngineVariables LaunchGameEngineDialog::variables() const
{
  const auto& map = m_document.map();
  return LaunchGameEngineVariables{map};
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

  const auto& map = m_document.map();
  const auto& gameConfig = map.game()->config();

  auto dialog = GameEngineDialog{gameConfig.name, m_document.logger(), this};
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
  const auto* profile = m_gameEngineList->selectedProfile();
  contract_assert(profile != nullptr);

  launchGameEngineProfile(*profile, variables())
    | kdl::transform_error([](const auto& e) {
        const auto message = kdl::str_to_string("Could not launch game engine: ", e.msg);
        QMessageBox::critical(
          nullptr, "TrenchBroom", QString::fromStdString(message), QMessageBox::Ok);
      });
}

void LaunchGameEngineDialog::done(const int r)
{
  saveConfig();

  QDialog::done(r);
}

void LaunchGameEngineDialog::saveConfig()
{
  auto& app = TrenchBroomApp::instance();
  auto& gameManager = app.gameManager();

  const auto& map = m_document.map();
  const auto& gameName = map.game()->config().name;

  gameManager.updateGameEngineConfig(gameName, m_config, m_document.logger())
    | kdl::transform_error([&](const auto& e) { m_document.logger().error() << e.msg; });
}

} // namespace tb::ui
