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

#include "GameEngineDialog.h"

#include <QBoxLayout>
#include <QCloseEvent>
#include <QDialogButtonBox>

#include "Logger.h"
#include "TrenchBroomApp.h"
#include "mdl/GameManager.h"
#include "ui/AppController.h"
#include "ui/BorderLine.h"
#include "ui/CurrentGameIndicator.h"
#include "ui/DialogButtonLayout.h"
#include "ui/GameEngineProfileManager.h"
#include "ui/QStyleUtils.h"

namespace tb::ui
{

GameEngineDialog::GameEngineDialog(
  AppController& appController,
  const mdl::GameInfo& gameInfo,
  Logger& logger,
  QWidget* parent)
  : QDialog{parent}
  , m_appController{appController}
  , m_gameInfo{gameInfo}
  , m_logger{logger}
{
  setWindowTitle("Game Engines");
  setWindowIconTB(this);
  createGui();
}

void GameEngineDialog::createGui()
{
  auto* gameIndicator = new CurrentGameIndicator{m_gameInfo};
  m_profileManager = new GameEngineProfileManager{m_gameInfo.gameEngineConfig};
  auto* buttons = new QDialogButtonBox{QDialogButtonBox::Close};

  auto* layout = new QVBoxLayout{};
  layout->setContentsMargins(QMargins{});
  layout->setSpacing(0);
  setLayout(layout);

  layout->addWidget(gameIndicator);
  layout->addWidget(new BorderLine{});
  layout->addWidget(m_profileManager, 1);
  layout->addLayout(wrapDialogButtonBox(buttons));

  setFixedSize(600, 400);

  connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
  connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::accept);
}

void GameEngineDialog::done(const int r)
{
  saveConfig();

  QDialog::done(r);
}

void GameEngineDialog::saveConfig()
{
  auto& gameManager = m_appController.gameManager();

  gameManager.updateGameEngineConfig(
    m_gameInfo.gameConfig.name, m_profileManager->config(), m_logger)
    | kdl::transform_error([&](const auto& e) { m_logger.error() << e.msg; });
}

} // namespace tb::ui
