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

#include <string>

namespace TrenchBroom {
    namespace View {
        UndoableCommand::UndoableCommand(const CommandType type, const std::string& name) :
        Command(type, name) {}

        UndoableCommand::~UndoableCommand() {}

        std::unique_ptr<CommandResult> UndoableCommand::performUndo(MapDocumentCommandFacade* document) {
            m_state = CommandState::Undoing;
            auto result = doPerformUndo(document);
            if (result->success()) {
                m_state = CommandState::Default;
            } else {
                m_state = CommandState::Done;
            }
            return result;
        }

        bool UndoableCommand::isRepeatDelimiter() const {
            return doIsRepeatDelimiter();
        }

        bool UndoableCommand::isRepeatable(MapDocumentCommandFacade* document) const {
            return doIsRepeatable(document);
        }

        std::unique_ptr<UndoableCommand> UndoableCommand::repeat(MapDocumentCommandFacade* document) const {
            return doRepeat(document);
        }

        bool UndoableCommand::collateWith(UndoableCommand* command) {
            assert(command != this);
            if (command->type() != m_type)
                return false;
            return doCollateWith(command);
        }

        bool UndoableCommand::doIsRepeatDelimiter() const {
            return false;
        }

        std::unique_ptr<UndoableCommand> UndoableCommand::doRepeat(MapDocumentCommandFacade*) const {
            throw CommandProcessorException("Command is not repeatable");
        }

        size_t UndoableCommand::documentModificationCount() const {
            throw CommandProcessorException("Command does not modify the document");
        }
    }
}
