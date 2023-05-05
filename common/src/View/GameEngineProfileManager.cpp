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

#include "GameEngineProfileManager.h"

#include "Model/GameEngineConfig.h"
#include "Model/GameEngineProfile.h"
#include "View/BorderLine.h"
#include "View/GameEngineProfileEditor.h"
#include "View/GameEngineProfileListBox.h"
#include "View/QtUtils.h"
#include "View/TitledPanel.h"

#include "kdl/vector_utils.h"

#include <filesystem>

#include <QBoxLayout>
#include <QToolButton>

namespace TrenchBroom
{
namespace View
{
GameEngineProfileManager::GameEngineProfileManager(
  Model::GameEngineConfig config, QWidget* parent)
  : QWidget{parent}
  , m_config{std::move(config)}
{
  auto* listPanel = new TitledPanel{"Profiles"};
  auto* editorPanel = new TitledPanel{"Details"};

  m_profileList = new GameEngineProfileListBox{m_config, listPanel->getPanel()};
  m_profileEditor = new GameEngineProfileEditor{editorPanel->getPanel()};

  auto* addProfileButton = createBitmapButton("Add.svg", "Add profile");
  m_removeProfileButton = createBitmapButton("Remove.svg", "Remove the selected profile");
  m_removeProfileButton->setEnabled(false);

  auto* buttonLayout = createMiniToolBarLayout(addProfileButton, m_removeProfileButton);

  auto* listLayout = new QVBoxLayout{};
  listLayout->setContentsMargins(QMargins{});
  listLayout->setSpacing(0);
  listPanel->getPanel()->setLayout(listLayout);
  listLayout->addWidget(m_profileList, 1);
  listLayout->addWidget(new BorderLine{BorderLine::Direction::Horizontal});
  listLayout->addLayout(buttonLayout);

  auto* editorLayout = new QHBoxLayout{};
  editorLayout->setContentsMargins(QMargins{});
  editorLayout->setSpacing(0);
  editorPanel->getPanel()->setLayout(editorLayout);
  editorLayout->addWidget(m_profileEditor);

  auto* outerLayout = new QHBoxLayout{};
  outerLayout->setContentsMargins(QMargins{});
  outerLayout->setSpacing(0);
  setLayout(outerLayout);
  outerLayout->addWidget(listPanel, 1);
  outerLayout->addWidget(new BorderLine{BorderLine::Direction::Vertical});
  outerLayout->addWidget(editorPanel, 1);

  listPanel->setMaximumWidth(250);

  connect(
    addProfileButton,
    &QAbstractButton::clicked,
    this,
    &GameEngineProfileManager::addProfile);
  connect(
    m_removeProfileButton,
    &QAbstractButton::clicked,
    this,
    &GameEngineProfileManager::removeProfile);
  connect(
    m_profileList,
    &GameEngineProfileListBox::currentProfileChanged,
    this,
    &GameEngineProfileManager::currentProfileChanged);
  connect(m_profileEditor, &GameEngineProfileEditor::profileChanged, this, [&]() {
    // update the names in the list box (but don't refresh() the list) when a profile is
    // edited
    m_profileList->updateProfiles();
  });
}

const Model::GameEngineConfig& GameEngineProfileManager::config() const
{
  return m_config;
}

void GameEngineProfileManager::addProfile()
{
  m_config.profiles.push_back(Model::GameEngineProfile{"", {}, ""});
  m_profileList->reloadProfiles();
  m_profileList->setCurrentRow(int(m_config.profiles.size() - 1));
}

void GameEngineProfileManager::removeProfile()
{
  const auto index = m_profileList->currentRow();
  if (index < 0)
  {
    return;
  }

  kdl::vec_erase_at(m_config.profiles, size_t(index));
  m_profileList->reloadProfiles();
  m_profileList->setCurrentRow(index >= m_profileList->count() ? index - 1 : index);
}

void GameEngineProfileManager::currentProfileChanged(Model::GameEngineProfile* profile)
{
  m_profileEditor->setProfile(profile);
  m_removeProfileButton->setEnabled(profile != nullptr);
}
} // namespace View
} // namespace TrenchBroom
