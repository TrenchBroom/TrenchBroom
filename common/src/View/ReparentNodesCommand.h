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

#ifndef TrenchBroom_ReparentNodesCommand
#define TrenchBroom_ReparentNodesCommand

#include "Macros.h"
#include "View/DocumentCommand.h"

#include <map>
#include <memory>
#include <vector>

namespace TrenchBroom {
    namespace Model {
        class Node;
    }

    namespace View {
        class ReparentNodesCommand : public DocumentCommand {
        public:
            static const CommandType Type;
        private:
            std::map<Model::Node*, std::vector<Model::Node*>> m_nodesToAdd;
            std::map<Model::Node*, std::vector<Model::Node*>> m_nodesToRemove;
        public:
            static std::unique_ptr<ReparentNodesCommand> reparent(const std::map<Model::Node*, std::vector<Model::Node*>>& nodesToAdd, const std::map<Model::Node*, std::vector<Model::Node*>>& nodesToRemove);

            ReparentNodesCommand(const std::map<Model::Node*, std::vector<Model::Node*>>& nodesToAdd, const std::map<Model::Node*, std::vector<Model::Node*>>& nodesToRemove);
        private:
            std::unique_ptr<CommandResult> doPerformDo(MapDocumentCommandFacade* document) override;
            std::unique_ptr<CommandResult> doPerformUndo(MapDocumentCommandFacade* document) override;

            bool doCollateWith(UndoableCommand* command) override;

            deleteCopyAndMove(ReparentNodesCommand)
        };
    }
}

#endif /* defined(TrenchBroom_ReparentNodesCommand) */
