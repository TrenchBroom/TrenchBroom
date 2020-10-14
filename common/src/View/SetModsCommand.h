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

#ifndef TrenchBroom_SetModsCommand
#define TrenchBroom_SetModsCommand

#include "Macros.h"
#include "View/DocumentCommand.h"

#include <memory>
#include <string>
#include <vector>

namespace TrenchBroom {
    namespace View {
        class SetModsCommand : public DocumentCommand {
        public:
            static const CommandType Type;
        private:
            std::vector<std::string> m_oldMods;
            std::vector<std::string> m_newMods;
        public:
            static std::unique_ptr<SetModsCommand> set(const std::vector<std::string>& mods);

            SetModsCommand(const std::string& name, const std::vector<std::string>& mods);
        private:
            std::unique_ptr<CommandResult> doPerformDo(MapDocumentCommandFacade* document) override;
            std::unique_ptr<CommandResult> doPerformUndo(MapDocumentCommandFacade* document) override;

            bool doCollateWith(UndoableCommand* command) override;

            deleteCopyAndMove(SetModsCommand)
        };
    }
}

#endif /* defined(TrenchBroom_SetModsCommand) */
