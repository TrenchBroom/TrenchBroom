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

#include "RenameGroupsCommand.h"

#include "View/MapDocumentCommandFacade.h"

#include <string>

namespace TrenchBroom {
    namespace View {
        const Command::CommandType RenameGroupsCommand::Type = Command::freeType();

        std::unique_ptr<RenameGroupsCommand> RenameGroupsCommand::rename(const std::string& newName) {
            return std::make_unique<RenameGroupsCommand>(newName);
        }

        RenameGroupsCommand::RenameGroupsCommand(const std::string& newName) :
        DocumentCommand(Type, "Rename Groups"),
        m_newName(newName) {}

        std::unique_ptr<CommandResult> RenameGroupsCommand::doPerformDo(MapDocumentCommandFacade* document) {
            m_oldNames = document->performRenameGroups(m_newName);
            return std::make_unique<CommandResult>(true);
        }

        std::unique_ptr<CommandResult> RenameGroupsCommand::doPerformUndo(MapDocumentCommandFacade* document) {
            document->performUndoRenameGroups(m_oldNames);
            return std::make_unique<CommandResult>(true);
        }

        bool RenameGroupsCommand::doIsRepeatable(MapDocumentCommandFacade*) const {
            return false;
        }

        bool RenameGroupsCommand::doCollateWith(UndoableCommand*) {
            return false;
        }
    }
}
