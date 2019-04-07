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

#include "IO/Path.h"
#include "Model/GameEngineConfig.h"
#include "Model/GameEngineProfile.h"
#include "View/BorderLine.h"
#include "View/GameEngineProfileListBox.h"
#include "View/GameEngineProfileEditor.h"
#include "View/TitledPanel.h"
#include "View/ViewConstants.h"
#include "View/wxUtils.h"

#include <QAbstractButton>
#include <QBoxLayout>

namespace TrenchBroom {
    namespace View {
        GameEngineProfileManager::GameEngineProfileManager(Model::GameEngineConfig& config, QWidget* parent) :
        QWidget(parent),
        m_config(config),
        m_profileList(nullptr),
        m_profileEditor(nullptr),
        m_removeProfileButton(nullptr) {
            auto* listPanel = new TitledPanel(this, "Profiles");
            auto* editorPanel = new TitledPanel(this, "Details");

            m_profileList = new GameEngineProfileListBox(m_config, listPanel->getPanel());
            m_profileEditor = new GameEngineProfileEditor(editorPanel->getPanel());

            auto* addProfileButton = createBitmapButton("Add.png", "Add profile");
            m_removeProfileButton = createBitmapButton("Remove.png", "Remove the selected profile");

            auto* buttonLayout = new QHBoxLayout();
            buttonLayout->addWidget(addProfileButton);
            buttonLayout->addWidget(m_removeProfileButton);
            buttonLayout->addStretch(1);

            auto* listLayout = new QVBoxLayout();
            listPanel->getPanel()->setLayout(listLayout);
            listLayout->addWidget(m_profileList, 1);
            listLayout->addWidget(new BorderLine(BorderLine::Direction_Horizontal));
            listLayout->addLayout(buttonLayout);

            auto* outerLayout = new QHBoxLayout();
            setLayout(outerLayout);
            outerLayout->addWidget(listPanel, 1);
            outerLayout->addWidget(new BorderLine(BorderLine::Direction_Vertical));
            outerLayout->addWidget(editorPanel, 1);

            connect(addProfileButton, &QAbstractButton::clicked, this, &GameEngineProfileManager::addProfile);
            connect(m_removeProfileButton, &QAbstractButton::clicked, this, &GameEngineProfileManager::removeProfile);
            connect(m_profileList, &GameEngineProfileListBox::currentProfileChanged, this, &GameEngineProfileManager::currentProfileChanged);
        }

        void GameEngineProfileManager::addProfile() {
            m_config.addProfile(new Model::GameEngineProfile("", IO::Path(), ""));
            m_profileList->setCurrentRow(static_cast<int>(m_config.profileCount() - 1));
        }

        void GameEngineProfileManager::removeProfile() {
            const int index = m_profileList->currentRow();

            if (m_config.profileCount() == 1) {
                m_profileList->setCurrentRow(-1);
                m_config.removeProfile(static_cast<size_t>(index));
            } else if (index > 0) {
                m_profileList->setCurrentRow(index - 1);
                m_config.removeProfile(static_cast<size_t>(index));
            } else {
                m_profileList->setCurrentRow(1);
                m_config.removeProfile(static_cast<size_t>(index));
                m_profileList->setCurrentRow(0);
            }
        }

        void GameEngineProfileManager::currentProfileChanged(Model::GameEngineProfile* profile) {
            m_profileEditor->setProfile(profile);
            m_removeProfileButton->setEnabled(profile != nullptr);
        }
    }
}
