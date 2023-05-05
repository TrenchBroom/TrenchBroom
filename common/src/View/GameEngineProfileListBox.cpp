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

#include "GameEngineProfileListBox.h"

#include <QBoxLayout>

#include "IO/PathQt.h"
#include "Model/GameEngineConfig.h"
#include "Model/GameEngineProfile.h"
#include "View/ElidedLabel.h"
#include "View/QtUtils.h"

namespace TrenchBroom
{
namespace View
{
// GameEngineProfileItemRenderer

GameEngineProfileItemRenderer::GameEngineProfileItemRenderer(
  Model::GameEngineProfile& profile, QWidget* parent)
  : ControlListBoxItemRenderer{parent}
  , m_profile{&profile}
{
  createGui();
  refresh();
}

void GameEngineProfileItemRenderer::updateItem()
{
  refresh();
}

void GameEngineProfileItemRenderer::createGui()
{
  m_nameLabel = new ElidedLabel{"not set", Qt::ElideRight};
  m_pathLabel = new ElidedLabel{"not set", Qt::ElideMiddle};

  makeEmphasized(m_nameLabel);
  makeInfo(m_pathLabel);

  auto* layout = new QVBoxLayout{};
  layout->setContentsMargins(QMargins{});
  layout->setSpacing(LayoutConstants::NarrowVMargin);
  layout->addWidget(m_nameLabel);
  layout->addWidget(m_pathLabel);
  setLayout(layout);
}

void GameEngineProfileItemRenderer::refresh()
{
  m_nameLabel->setText(m_profile ? QString::fromStdString(m_profile->name) : "");
  m_pathLabel->setText(m_profile ? IO::pathAsQString(m_profile->path) : "");
}

void GameEngineProfileItemRenderer::profileWillBeRemoved()
{
  m_profile = nullptr;
}

void GameEngineProfileItemRenderer::profileDidChange()
{
  refresh();
}

// GameEngineProfileListBox

GameEngineProfileListBox::GameEngineProfileListBox(
  Model::GameEngineConfig& config, QWidget* parent)
  : ControlListBox{"Click the '+' button to create a game engine profile.", true, parent}
  , m_config{&config}
{
  reload();
}

Model::GameEngineProfile* GameEngineProfileListBox::selectedProfile()
{
  return (currentRow() >= 0 && size_t(currentRow()) < m_config->profiles.size())
           ? &m_config->profiles[size_t(currentRow())]
           : nullptr;
}

void GameEngineProfileListBox::setConfig(Model::GameEngineConfig& config)
{
  m_config = &config;
  reload();
}

void GameEngineProfileListBox::reloadProfiles()
{
  reload();
}

void GameEngineProfileListBox::updateProfiles()
{
  updateItems();
}

size_t GameEngineProfileListBox::itemCount() const
{
  return m_config->profiles.size();
}

ControlListBoxItemRenderer* GameEngineProfileListBox::createItemRenderer(
  QWidget* parent, const size_t index)
{
  return new GameEngineProfileItemRenderer{m_config->profiles[index], parent};
}

void GameEngineProfileListBox::selectedRowChanged(const int index)
{
  emit currentProfileChanged(
    (index >= 0 && index < count()) ? &m_config->profiles[size_t(index)] : nullptr);
}

void GameEngineProfileListBox::doubleClicked(const size_t index)
{
  if (index < size_t(count()))
  {
    emit profileSelected(m_config->profiles[index]);
  }
}
} // namespace View
} // namespace TrenchBroom
