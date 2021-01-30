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

#include "GameListBox.h"

#include "Ensure.h"
#include "IO/Path.h"
#include "IO/ResourceUtils.h"
#include "Model/GameConfig.h"
#include "Model/GameFactory.h"

#include <string>
#include <vector>

namespace TrenchBroom {
    namespace View {
        GameListBox::GameListBox(QWidget* parent) :
        ImageListBox("No Games Found", true, parent) {
            reloadGameInfos();
        }

        std::string GameListBox::selectedGameName() const {
            const Model::GameFactory& gameFactory = Model::GameFactory::instance();
            const std::vector<std::string>& gameList = gameFactory.gameList();

            const auto index = currentRow();
            if (index >= static_cast<int>(gameList.size())) {
                return "";
            } else {
                return gameList[static_cast<size_t>(index)];
            }
        }

        void GameListBox::selectGame(const size_t index) {
            setCurrentRow(static_cast<int>(index));
        }

        void GameListBox::reloadGameInfos() {
            const auto currentGameName = selectedGameName();

            m_gameInfos.clear();

            const auto& gameFactory = Model::GameFactory::instance();
            for (const std::string& gameName : gameFactory.gameList()) {
                m_gameInfos.push_back(makeGameInfo(gameName));
            }

            reload();

            const std::vector<std::string>& gameList = gameFactory.gameList();
            for (size_t i = 0u; i < gameList.size(); ++i) {
                if (gameList[i] == currentGameName) {
                    selectGame(i);
                    break;
                }
            }
        }

        void GameListBox::updateGameInfos() {
            for (auto& gameInfo : m_gameInfos) {
                gameInfo = makeGameInfo(gameInfo.name);
            }
            updateItems();
        }

        GameListBox::Info GameListBox::makeGameInfo(const std::string& gameName) const {
            const auto& gameFactory = Model::GameFactory::instance();
            const auto gamePath = gameFactory.gamePath(gameName);
            auto iconPath = gameFactory.iconPath(gameName);
            if (iconPath.isEmpty()) {
                iconPath = IO::Path("DefaultGameIcon.svg");
            }
            const auto experimental = gameFactory.gameConfig(gameName).experimental();

            return Info {
                gameName,
                IO::loadPixmapResource(iconPath),
                QString::fromStdString(gameName + (experimental ? " (experimental)" : "")),
                QString::fromStdString(gamePath.isEmpty() ? std::string("Game not found") : gamePath.asString())
            };
        }

        size_t GameListBox::itemCount() const {
            return m_gameInfos.size();
        }

        QPixmap GameListBox::image(size_t index) const {
            ensure(index < m_gameInfos.size(), "index out of range");
            return m_gameInfos[index].image;
        }

        QString GameListBox::title(const size_t n) const {
            ensure(n < m_gameInfos.size(), "index out of range");
            return m_gameInfos[n].title;
        }

        QString GameListBox::subtitle(const size_t n) const {
            ensure(n < m_gameInfos.size(), "index out of range");
            return m_gameInfos[n].subtitle;
        }

        void GameListBox::selectedRowChanged(const int index) {
            if (index >= 0 && index < count()) {
                emit currentGameChanged(QString::fromStdString(m_gameInfos[static_cast<size_t>(index)].name));
            }
        }

        void GameListBox::doubleClicked(const size_t index) {
            if (index < static_cast<size_t>(count())) {
                emit selectCurrentGame(QString::fromStdString(m_gameInfos[index].name));
            }
        }
    }
}
