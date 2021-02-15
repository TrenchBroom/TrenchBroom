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
#include "View/UpdateLinkedGroupsHelper.h"

#include <map>
#include <memory>
#include <vector>

namespace TrenchBroom {
    namespace Model {
        class GroupNode;
        class Node;
    }

    namespace View {
        class AddRemoveNodesCommand : public UndoableCommand {
        public:
            static const CommandType Type;
        private:
            enum class Action {
                Add,
                Remove
            };

            Action m_action;
            std::map<Model::Node*, std::vector<Model::Node*>> m_nodesToAdd;
            std::map<Model::Node*, std::vector<Model::Node*>> m_nodesToRemove;
            UpdateLinkedGroupsHelper m_updateLinkedGroupsHelper;
        public:
            static std::unique_ptr<AddRemoveNodesCommand> add(Model::Node* parent, const std::vector<Model::Node*>& children, std::vector<std::pair<const Model::GroupNode*, std::vector<Model::GroupNode*>>> linkedGroupsToUpdate);
            static std::unique_ptr<AddRemoveNodesCommand> add(const std::map<Model::Node*, std::vector<Model::Node*>>& nodes, std::vector<std::pair<const Model::GroupNode*, std::vector<Model::GroupNode*>>> linkedGroupsToUpdate);
            static std::unique_ptr<AddRemoveNodesCommand> remove(const std::map<Model::Node*, std::vector<Model::Node*>>& nodes, std::vector<std::pair<const Model::GroupNode*, std::vector<Model::GroupNode*>>> linkedGroupsToUpdate);

            AddRemoveNodesCommand(Action action, const std::map<Model::Node*, std::vector<Model::Node*>>& nodes, std::vector<std::pair<const Model::GroupNode*, std::vector<Model::GroupNode*>>> linkedGroupsToUpdate);
            ~AddRemoveNodesCommand() override;
        private:
            static std::string makeName(Action action);

            std::unique_ptr<CommandResult> doPerformDo(MapDocumentCommandFacade* document) override;
            std::unique_ptr<CommandResult> doPerformUndo(MapDocumentCommandFacade* document) override;

            void doAction(MapDocumentCommandFacade* document);
            void undoAction(MapDocumentCommandFacade* document);

            bool doCollateWith(UndoableCommand* command) override;

            deleteCopyAndMove(AddRemoveNodesCommand)
        };
    }
}

