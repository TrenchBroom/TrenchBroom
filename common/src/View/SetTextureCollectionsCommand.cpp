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

#include "SetTextureCollectionsCommand.h"
#include "View/MapDocumentCommandFacade.h"

namespace TrenchBroom {
    namespace View {
        const Command::CommandType SetTextureCollectionsCommand::Type = Command::freeType();

        std::unique_ptr<SetTextureCollectionsCommand> SetTextureCollectionsCommand::set(const std::vector<IO::Path>& paths) {
            return std::make_unique<SetTextureCollectionsCommand>(paths);
        }

        SetTextureCollectionsCommand::SetTextureCollectionsCommand(const std::vector<IO::Path>& paths) :
        DocumentCommand(Type, "Set Texture Collections"),
        m_paths(paths) {}

        std::unique_ptr<CommandResult> SetTextureCollectionsCommand::doPerformDo(MapDocumentCommandFacade* document) {
            m_oldPaths = document->enabledTextureCollections();
            document->performSetTextureCollections(m_paths);
            return std::make_unique<CommandResult>(true);
        }

        std::unique_ptr<CommandResult> SetTextureCollectionsCommand::doPerformUndo(MapDocumentCommandFacade* document) {
            document->performSetTextureCollections(m_oldPaths);
            m_oldPaths.clear();
            return std::make_unique<CommandResult>(true);
        }

        bool SetTextureCollectionsCommand::doCollateWith(UndoableCommand*) {
            return false;
        }
    }
}
