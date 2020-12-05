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

#pragma once

#include "Macros.h"
#include "View/UndoableCommand.h"

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace TrenchBroom {
    namespace Model {
        enum class LockState;
        class Node;
    }

    namespace View {
        class SetLockStateCommand : public UndoableCommand {
        public:
            static const CommandType Type;
        private:
            std::vector<Model::Node*> m_nodes;
            Model::LockState m_lockState;
            std::map<Model::Node*, Model::LockState> m_oldLockState;
        public:
            static std::unique_ptr<SetLockStateCommand> lock(const std::vector<Model::Node*>& nodes);
            static std::unique_ptr<SetLockStateCommand> unlock(const std::vector<Model::Node*>& nodes);
            static std::unique_ptr<SetLockStateCommand> reset(const std::vector<Model::Node*>& nodes);

            SetLockStateCommand(const std::vector<Model::Node*>& nodes, Model::LockState lockState);
        private:
            static std::string makeName(Model::LockState lockState);

            std::unique_ptr<CommandResult> doPerformDo(MapDocumentCommandFacade* document) override;
            std::unique_ptr<CommandResult> doPerformUndo(MapDocumentCommandFacade* document) override;

            bool doCollateWith(UndoableCommand* command) override;

            deleteCopyAndMove(SetLockStateCommand)
        };
    }
}

