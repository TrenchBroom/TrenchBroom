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

#include "CompilationProfileManager.h"

#include "Model/CompilationConfig.h"
#include "Model/CompilationProfile.h"
#include "View/BorderLine.h"
#include "View/CompilationProfileListBox.h"
#include "View/CompilationProfileEditor.h"
#include "View/TitledPanel.h"
#include "View/QtUtils.h"

#include <QAbstractButton>
#include <QMenu>

namespace TrenchBroom {
    namespace View {
        CompilationProfileManager::CompilationProfileManager(std::weak_ptr<MapDocument> document, Model::CompilationConfig& config, QWidget* parent) :
        QWidget(parent),
        m_config(config),
        m_profileList(nullptr),
        m_profileEditor() {
            setBaseWindowColor(this);

            auto* listPanel = new TitledPanel("Profiles");
            auto* editorPanel = new TitledPanel("Details");

            m_profileList = new CompilationProfileListBox(m_config, listPanel->getPanel());
            m_profileEditor = new CompilationProfileEditor(std::move(document), editorPanel->getPanel());

            auto* addProfileButton = createBitmapButton("Add.svg", "Add profile");
            m_removeProfileButton = createBitmapButton("Remove.svg", "Remove the selected profile");
            auto* buttonLayout = createMiniToolBarLayout(addProfileButton, m_removeProfileButton);

            auto* listLayout = new QVBoxLayout();
            listLayout->setContentsMargins(0, 0, 0, 0);
            listLayout->setSpacing(0);
            listLayout->addWidget(m_profileList, 1);
            listLayout->addWidget(new BorderLine(BorderLine::Direction::Horizontal));
            listLayout->addLayout(buttonLayout);
            listPanel->getPanel()->setLayout(listLayout);

            auto* editorLayout = new QVBoxLayout();
            editorLayout->setContentsMargins(0, 0, 0, 0);
            editorLayout->setSpacing(0);
            editorLayout->addWidget(m_profileEditor);
            editorPanel->getPanel()->setLayout(editorLayout);

            auto* outerLayout = new QHBoxLayout();
            outerLayout->setContentsMargins(0, 0, 0, 0);
            outerLayout->setSpacing(0);
            outerLayout->addWidget(listPanel);
            outerLayout->addWidget(new BorderLine(BorderLine::Direction::Vertical));
            outerLayout->addWidget(editorPanel, 1);
            setLayout(outerLayout);

            listPanel->setMinimumSize(200, 200);

            connect(m_profileList, &ControlListBox::itemSelectionChanged, this, &CompilationProfileManager::profileSelectionChanged);
            connect(m_profileList, &CompilationProfileListBox::profileContextMenuRequested, this, &CompilationProfileManager::profileContextMenuRequested);
            connect(addProfileButton, &QAbstractButton::clicked, this, &CompilationProfileManager::addProfile);
            connect(m_removeProfileButton, &QAbstractButton::clicked, this, qOverload<>(&CompilationProfileManager::removeProfile));

            if (m_profileList->count() > 0) {
                m_profileList->setCurrentRow(0);
            }
        }

        const Model::CompilationProfile* CompilationProfileManager::selectedProfile() const {
            const auto index = m_profileList->currentRow();
            if (index < 0) {
                return nullptr;
            } else {
                return m_config.profile(static_cast<size_t>(index));
            }
        }

        void CompilationProfileManager::addProfile() {
            m_config.addProfile(std::make_unique<Model::CompilationProfile>("unnamed", "${MAP_DIR_PATH}"));
            m_profileList->setCurrentRow(static_cast<int>(m_config.profileCount() - 1));
        }

        void CompilationProfileManager::removeProfile() {
            const auto index = m_profileList->currentRow();
            assert(index >= 0);
            removeProfile(static_cast<size_t>(index));
        }

        void CompilationProfileManager::removeProfile(const size_t index) {
            if (m_config.profileCount() == 1) {
                m_profileList->setCurrentRow(-1);
                m_config.removeProfile(index);
            } else if (index > 0) {
                m_profileList->setCurrentRow(static_cast<int>(index - 1));
                m_config.removeProfile(index);
            } else {
                m_profileList->setCurrentRow(1);
                m_config.removeProfile(index);
                m_profileList->setCurrentRow(0);
            }
        }

        void CompilationProfileManager::removeProfile(Model::CompilationProfile* profile) {
            removeProfile(m_config.indexOfProfile(profile));
        }

        void CompilationProfileManager::duplicateProfile(Model::CompilationProfile* profile) {
            m_config.addProfile(profile->clone());
            m_profileList->setCurrentRow(static_cast<int>(m_config.profileCount() - 1));
        }

        void CompilationProfileManager::profileContextMenuRequested(const QPoint& globalPos, Model::CompilationProfile* profile) {
            QMenu menu(this);
            menu.addAction(tr("Duplicate"), this, [=](){ duplicateProfile(profile); });
            menu.addAction(tr("Remove"), this, [=](){ removeProfile(profile); });
            menu.exec(globalPos);
        }

        void CompilationProfileManager::profileSelectionChanged() {
            const auto selection = m_profileList->currentRow();
            if (selection >= 0) {
                Model::CompilationProfile* profile = m_config.profile(static_cast<size_t>(selection));
                m_profileEditor->setProfile(profile);
                m_removeProfileButton->setEnabled(true);
            } else {
                m_profileEditor->setProfile(nullptr);
                m_removeProfileButton->setEnabled(false);
            }

            emit selectedProfileChanged();
        }
    }
}
