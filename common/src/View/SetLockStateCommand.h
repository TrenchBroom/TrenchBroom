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

#ifndef TrenchBroom_SetLockStateCommand
#define TrenchBroom_SetLockStateCommand

#include "Model/ModelTypes.h"
#include "View/UndoableCommand.h"

#include <map>
#include <vector>

namespace TrenchBroom {
    namespace View {
        class SetLockStateCommand : public UndoableCommand {
        public:
            static const CommandType Type;
            using Ptr = std::shared_ptr<SetLockStateCommand>;
        private:
            std::vector<Model::Node*> m_nodes;
            Model::LockState m_lockState;
            std::map<Model::Node*, Model::LockState> m_oldLockState;
        public:
            static Ptr lock(const std::vector<Model::Node*>& nodes);
            static Ptr unlock(const std::vector<Model::Node*>& nodes);
            static Ptr reset(const std::vector<Model::Node*>& nodes);
        private:
            SetLockStateCommand(const std::vector<Model::Node*>& nodes, Model::LockState lockState);
            static String makeName(Model::LockState lockState);
        private:
            bool doPerformDo(MapDocumentCommandFacade* document) override;
            bool doPerformUndo(MapDocumentCommandFacade* document) override;

            bool doCollateWith(UndoableCommand::Ptr command) override;
            bool doIsRepeatable(MapDocumentCommandFacade* document) const override;
        };
    }
}

#endif /* defined(TrenchBroom_SetLockStateCommand) */
