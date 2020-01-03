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

#include "SetModsCommand.h"

#include "View/MapDocumentCommandFacade.h"

#include <cassert>
#include <string>

namespace TrenchBroom {
    namespace View {
        const Command::CommandType SetModsCommand::Type = Command::freeType();

        std::unique_ptr<SetModsCommand> SetModsCommand::set(const std::vector<std::string>& mods) {
            return std::make_unique<SetModsCommand>("Set Mods", mods);
        }

        SetModsCommand::SetModsCommand(const std::string& name, const std::vector<std::string>& mods) :
        DocumentCommand(Type, name),
        m_newMods(mods) {}

        std::unique_ptr<CommandResult> SetModsCommand::doPerformDo(MapDocumentCommandFacade* document) {
            m_oldMods = document->mods();
            document->performSetMods(m_newMods);
            return std::make_unique<CommandResult>(true);
        }

        std::unique_ptr<CommandResult> SetModsCommand::doPerformUndo(MapDocumentCommandFacade* document) {
            document->performSetMods(m_oldMods);
            return std::make_unique<CommandResult>(true);
        }

        bool SetModsCommand::doIsRepeatable(MapDocumentCommandFacade*) const {
            return false;
        }

        bool SetModsCommand::doCollateWith(UndoableCommand*) {
            return false;
        }
    }
}
