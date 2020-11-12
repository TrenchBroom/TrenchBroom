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

#include "UpdateEntitySpawnflagCommand.h"

#include "Model/EntityAttributeSnapshot.h"
#include "View/MapDocumentCommandFacade.h"

#include <string>

namespace TrenchBroom {
    namespace View {
        const Command::CommandType UpdateEntitySpawnflagCommand::Type = Command::freeType();

        std::unique_ptr<UpdateEntitySpawnflagCommand> UpdateEntitySpawnflagCommand::update(const std::string& attributeName, const size_t flagIndex, const bool setFlag) {
            return std::make_unique<UpdateEntitySpawnflagCommand>(attributeName, flagIndex, setFlag);
        }

        UpdateEntitySpawnflagCommand::UpdateEntitySpawnflagCommand(const std::string& attributeName, const size_t flagIndex, const bool setFlag) :
        DocumentCommand(Type, makeName(setFlag)),
        m_setFlag(setFlag),
        m_attributeName(attributeName),
        m_flagIndex(flagIndex) {}

        std::string UpdateEntitySpawnflagCommand::makeName(const bool setFlag) {
            return setFlag ? "Set Spawnflag" : "Unset Spawnflag";
        }

        std::unique_ptr<CommandResult> UpdateEntitySpawnflagCommand::doPerformDo(MapDocumentCommandFacade* document) {
            document->performUpdateSpawnflag(m_attributeName, m_flagIndex, m_setFlag);
            return std::make_unique<CommandResult>(true);
        }

        std::unique_ptr<CommandResult> UpdateEntitySpawnflagCommand::doPerformUndo(MapDocumentCommandFacade* document) {
            document->performUpdateSpawnflag(m_attributeName, m_flagIndex, !m_setFlag);
            return std::make_unique<CommandResult>(true);
        }

        bool UpdateEntitySpawnflagCommand::doCollateWith(UndoableCommand*) {
            return false;
        }
    }
}
