/*
 Copyright (C) 2010-2016 Kristian Duske
 
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

#include "Model/GameFactory.h"
#include "View/BorderLine.h"
#include "View/CurrentGameIndicator.h"

namespace TrenchBroom {
    namespace View {
        LaunchGameEngineDialog::LaunchGameEngineDialog(wxWindow* parent, const String& gameName, const VariableTable& variables) :
        wxDialog(parent, wxID_ANY, "Launch Game Engine", wxDefaultPosition, wxDefaultSize, wxCAPTION | wxCLOSE_BOX),
        m_gameName(gameName),
        m_variables(variables) {
            createGui();
        }
        
        void LaunchGameEngineDialog::createGui() {
            CurrentGameIndicator* gameIndicator = new CurrentGameIndicator(this, m_gameName);
            
            Model::GameFactory& gameFactory = Model::GameFactory::instance();
            Model::GameConfig& gameConfig = gameFactory.gameConfig(m_gameName);
        }
    }
}
