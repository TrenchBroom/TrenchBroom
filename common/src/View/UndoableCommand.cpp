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

#include "UndoableCommand.h"

#include "Exceptions.h"
#include "View/MapDocumentCommandFacade.h"

#include <string>

namespace TrenchBroom {
    namespace View {
        UndoableCommand::UndoableCommand(const CommandType type, const std::string& name, const bool updateModificationCount) :
        Command(type, name),
        m_modificationCount(updateModificationCount ? 1u : 0u) {}

        UndoableCommand::~UndoableCommand() {}

        std::unique_ptr<CommandResult> UndoableCommand::performDo(MapDocumentCommandFacade* document) {
            auto result = Command::performDo(document);
            if (result->success() && m_modificationCount) {
                if (document) {
                    document->incModificationCount(m_modificationCount);
                }
            }
            return result;
        }

        std::unique_ptr<CommandResult> UndoableCommand::performUndo(MapDocumentCommandFacade* document) {
            m_state = CommandState::Undoing;
            auto result = doPerformUndo(document);
            if (result->success()) {
                if (document) {
                    document->decModificationCount(m_modificationCount);
                }
                m_state = CommandState::Default;
            } else {
                m_state = CommandState::Done;
            }
            return result;
        }

        bool UndoableCommand::collateWith(UndoableCommand* command) {
            assert(command != this);
            if (command->type() == m_type && doCollateWith(command)) {
                m_modificationCount += command->m_modificationCount;
                return true;
            }
            return false;
        }
    }
}
