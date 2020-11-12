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

#include "EntityDefinitionFileCommand.h"
#include "View/MapDocumentCommandFacade.h"

#include <string>

namespace TrenchBroom {
    namespace View {
        const Command::CommandType EntityDefinitionFileCommand::Type = Command::freeType();

        std::unique_ptr<EntityDefinitionFileCommand> EntityDefinitionFileCommand::set(const Assets::EntityDefinitionFileSpec& spec) {
            return std::make_unique<EntityDefinitionFileCommand>("Set Entity Definitions", spec);
        }

        EntityDefinitionFileCommand::EntityDefinitionFileCommand(const std::string& name, const Assets::EntityDefinitionFileSpec& spec) :
        DocumentCommand(Type, name),
        m_newSpec(spec) {}

        std::unique_ptr<CommandResult> EntityDefinitionFileCommand::doPerformDo(MapDocumentCommandFacade* document) {
            m_oldSpec = document->entityDefinitionFile();
            document->performSetEntityDefinitionFile(m_newSpec);
            return std::make_unique<CommandResult>(true);
        }

        std::unique_ptr<CommandResult> EntityDefinitionFileCommand::doPerformUndo(MapDocumentCommandFacade* document) {
            document->performSetEntityDefinitionFile(m_oldSpec);
            return std::make_unique<CommandResult>(true);
        }

        bool EntityDefinitionFileCommand::doCollateWith(UndoableCommand*) {
            return false;
        }
    }
}
