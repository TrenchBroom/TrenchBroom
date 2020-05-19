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

#ifndef TrenchBroom_RenameGroupsCommand
#define TrenchBroom_RenameGroupsCommand

#include "Macros.h"
#include "View/DocumentCommand.h"

#include <map>
#include <memory>
#include <string>

namespace TrenchBroom {
    namespace Model {
        class GroupNode;
    }

    namespace View {
        class MapDocumentCommandFacade;

        class RenameGroupsCommand : public DocumentCommand {
        public:
            static const CommandType Type;
        private:
            const std::string m_newName;
            std::map<Model::GroupNode*, std::string> m_oldNames;
        public:
            static std::unique_ptr<RenameGroupsCommand> rename(const std::string& newName);

            explicit RenameGroupsCommand(const std::string& newName);
        private:
            std::unique_ptr<CommandResult> doPerformDo(MapDocumentCommandFacade* document) override;
            std::unique_ptr<CommandResult> doPerformUndo(MapDocumentCommandFacade* document) override;

            bool doIsRepeatable(MapDocumentCommandFacade* document) const override;

            bool doCollateWith(UndoableCommand* command) override;

            deleteCopyAndMove(RenameGroupsCommand)
        };
    }
}

#endif /* defined(TrenchBroom_RenameGroupsCommand) */
