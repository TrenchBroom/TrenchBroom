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

#ifndef TrenchBroom_UpdateEntitySpawnflagCommand
#define TrenchBroom_UpdateEntitySpawnflagCommand

#include "Macros.h"
#include "View/DocumentCommand.h"

#include <memory>
#include <string>

namespace TrenchBroom {
    namespace View {
        class UpdateEntitySpawnflagCommand : public DocumentCommand {
        public:
            static const CommandType Type;
        private:
            bool m_setFlag;
            std::string m_attributeName;
            size_t m_flagIndex;
        public:
            static std::unique_ptr<UpdateEntitySpawnflagCommand> update(const std::string& attributeName, const size_t flagIndex, const bool setFlag);

            UpdateEntitySpawnflagCommand(const std::string& attributeName, const size_t flagIndex, const bool setFlag);
        private:
            static std::string makeName(const bool setFlag);

            std::unique_ptr<CommandResult> doPerformDo(MapDocumentCommandFacade* document) override;
            std::unique_ptr<CommandResult> doPerformUndo(MapDocumentCommandFacade* document) override;

            bool doCollateWith(UndoableCommand* command) override;

            deleteCopyAndMove(UpdateEntitySpawnflagCommand)
        };
    }
}

#endif /* defined(UpdateEntitySpawnflagCommand) */
