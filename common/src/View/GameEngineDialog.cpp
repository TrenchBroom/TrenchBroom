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

#include "GameEngineDialog.h"

#include "Model/GameConfig.h"
#include "Model/GameFactory.h"
#include "View/BorderLine.h"
#include "View/CurrentGameIndicator.h"
#include "View/GameEngineProfileManager.h"
#include "View/QtUtils.h"

#include <string>

#include <QBoxLayout>
#include <QCloseEvent>
#include <QDialogButtonBox>

namespace TrenchBroom {
namespace View {
GameEngineDialog::GameEngineDialog(const std::string& gameName, QWidget* parent)
  : QDialog(parent)
  , m_gameName(gameName)
  , m_profileManager(nullptr) {
  setWindowTitle("Game Engines");
  setWindowIconTB(this);
  createGui();
}

void GameEngineDialog::createGui() {
  auto* gameIndicator = new CurrentGameIndicator(m_gameName);

  auto& gameFactory = Model::GameFactory::instance();
  auto& gameConfig = gameFactory.gameConfig(m_gameName);
  m_profileManager = new GameEngineProfileManager(gameConfig.gameEngineConfig);

  auto* buttons = new QDialogButtonBox(QDialogButtonBox::Close);

  auto* layout = new QVBoxLayout();
  layout->setContentsMargins(QMargins());
  layout->setSpacing(0);
  setLayout(layout);

  layout->addWidget(gameIndicator);
  layout->addWidget(new BorderLine(BorderLine::Direction::Horizontal));
  layout->addWidget(m_profileManager, 1);
  layout->addLayout(wrapDialogButtonBox(buttons));

  setFixedSize(600, 400);

  connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
  connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::accept);
}

void GameEngineDialog::done(const int r) {
  saveConfig();

  QDialog::done(r);
}

void GameEngineDialog::saveConfig() {
  auto& gameFactory = Model::GameFactory::instance();
  gameFactory.saveGameEngineConfig(m_gameName, m_profileManager->config());
}
} // namespace View
} // namespace TrenchBroom
