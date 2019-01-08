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

#include "StringUtils.h"
#include "IO/Path.h"
#include "IO/ResourceUtils.h"
#include "Model/GameConfig.h"
#include "Model/GameFactory.h"
#include "View/GameSelectionCommand.h"

#include <cassert>
#include <wx/log.h>

namespace TrenchBroom {
    namespace View {
        GameListBox::GameListBox(wxWindow* parent) :
        ImageListBox(parent, "No Games Found") {
            reloadGameInfos();
            Bind(wxEVT_LISTBOX, &GameListBox::OnListBoxChange, this);
            Bind(wxEVT_LISTBOX_DCLICK, &GameListBox::OnListBoxDoubleClick, this);
        }

        String GameListBox::selectedGameName() const {
            const Model::GameFactory& gameFactory = Model::GameFactory::instance();
            const StringList& gameList = gameFactory.gameList();
            
            const int index = GetSelection();
            if (index < 0 || index >= static_cast<int>(gameList.size()))
                return "";
            return gameList[static_cast<size_t>(index)];
        }

        void GameListBox::selectGame(const int index) {
            const Model::GameFactory& gameFactory = Model::GameFactory::instance();
            const StringList& gameList = gameFactory.gameList();

            if (index < 0 || index >= static_cast<int>(gameList.size()))
                return;
            
            SetSelection(index);
        }

        void GameListBox::OnListBoxChange(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            submitChangeEvent(GAME_SELECTION_CHANGE_EVENT);
        }

        void GameListBox::OnListBoxDoubleClick(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            submitChangeEvent(GAME_SELECTION_DBLCLICK_EVENT);
        }

        void GameListBox::reloadGameInfos() {
            m_gameInfos.clear();
            
            const Model::GameFactory& gameFactory = Model::GameFactory::instance();
            for (const String& gameName : gameFactory.gameList()) {
                const IO::Path gamePath = gameFactory.gamePath(gameName);
                IO::Path iconPath = gameFactory.iconPath(gameName);
                if (iconPath.isEmpty()) {
                    iconPath = IO::Path("DefaultGameIcon.png");
                }
                const bool experimental = gameFactory.gameConfig(gameName).experimental();

                Info gameInfo;
                gameInfo.image = IO::loadImageResource(iconPath);
                gameInfo.title = gameName + (experimental ? " (experimental)" : "");
                gameInfo.subtitle = gamePath.isEmpty() ? String("Game not found") : gamePath.asString();
                
                m_gameInfos.push_back(gameInfo);
            }

            SetItemCount(m_gameInfos.size());
            Refresh();
        }

        bool GameListBox::image(const size_t n, wxBitmap& result) const {
            ensure(n < m_gameInfos.size(), "index out of range");
            result = m_gameInfos[n].image;
            return true;
        }
        
        wxString GameListBox::title(const size_t n) const {
            ensure(n < m_gameInfos.size(), "index out of range");
            return m_gameInfos[n].title;
        }
        
        wxString GameListBox::subtitle(const size_t n) const {
            ensure(n < m_gameInfos.size(), "index out of range");
            return m_gameInfos[n].subtitle;
        }

        void GameListBox::submitChangeEvent(const wxEventType type) {
            GameSelectionCommand command(type, selectedGameName());
            command.SetEventObject(this);
            command.SetId(GetId());
            ProcessEvent(command);
        }
    }
}
