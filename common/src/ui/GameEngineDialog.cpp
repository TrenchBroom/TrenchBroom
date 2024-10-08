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

#include "FileLogger.h"
#include "mdl/GameConfig.h"
#include "mdl/GameFactory.h"
#include "ui/BorderLine.h"
#include "ui/CurrentGameIndicator.h"
#include "ui/GameEngineProfileManager.h"
#include "ui/QtUtils.h"

#include <string>

namespace tb::ui
{

GameEngineDialog::GameEngineDialog(std::string gameName, QWidget* parent)
  : QDialog{parent}
  , m_gameName{std::move(gameName)}
{
  setWindowTitle("Game Engines");
  setWindowIconTB(this);
  createGui();
}

void GameEngineDialog::createGui()
{
  auto* gameIndicator = new CurrentGameIndicator{m_gameName};

  auto& gameFactory = mdl::GameFactory::instance();
  auto& gameConfig = gameFactory.gameConfig(m_gameName);
  m_profileManager = new GameEngineProfileManager{gameConfig.gameEngineConfig};

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
  auto& logger = FileLogger::instance();
  auto& gameFactory = mdl::GameFactory::instance();
  gameFactory.saveGameEngineConfig(m_gameName, m_profileManager->config(), logger);
}

} // namespace tb::ui
