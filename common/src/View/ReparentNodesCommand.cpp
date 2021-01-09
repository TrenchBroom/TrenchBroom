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

#include "ReparentNodesCommand.h"

#include "Model/UpdateLinkedGroupsError.h"
#include "View/MapDocumentCommandFacade.h"

#include <kdl/result.h>

namespace TrenchBroom {
    namespace View {
        const Command::CommandType ReparentNodesCommand::Type = Command::freeType();

        std::unique_ptr<ReparentNodesCommand> ReparentNodesCommand::reparent(std::map<Model::Node*, std::vector<Model::Node*>> nodesToAdd, std::map<Model::Node*, std::vector<Model::Node*>> nodesToRemove, std::vector<Model::GroupNode*> linkedGroupsToUpdate) {
            return std::make_unique<ReparentNodesCommand>(std::move(nodesToAdd), std::move(nodesToRemove), std::move(linkedGroupsToUpdate));
        }

        ReparentNodesCommand::ReparentNodesCommand(std::map<Model::Node*, std::vector<Model::Node*>> nodesToAdd, std::map<Model::Node*, std::vector<Model::Node*>> nodesToRemove, std::vector<Model::GroupNode*> linkedGroupsToUpdate) :
        DocumentCommand(Type, "Reparent Objects"),
        m_nodesToAdd(std::move(nodesToAdd)),
        m_nodesToRemove(std::move(nodesToRemove)),
        m_updateLinkedGroupsHelper(std::move(linkedGroupsToUpdate)) {}

        std::unique_ptr<CommandResult> ReparentNodesCommand::doPerformDo(MapDocumentCommandFacade* document) {
            doAction(document);

            const auto success = m_updateLinkedGroupsHelper.applyLinkedGroupUpdates(*document)
                .handle_errors([&](const Model::UpdateLinkedGroupsError& e) {
                    document->error() << e;
                    undoAction(document);
                });

            return std::make_unique<CommandResult>(success);
        }

        std::unique_ptr<CommandResult> ReparentNodesCommand::doPerformUndo(MapDocumentCommandFacade* document) {
            undoAction(document);
            m_updateLinkedGroupsHelper.undoLinkedGroupUpdates(*document);
            return std::make_unique<CommandResult>(true);
        }

        void ReparentNodesCommand::doAction(MapDocumentCommandFacade* document) {
            document->performRemoveNodes(m_nodesToRemove);
            document->performAddNodes(m_nodesToAdd);
        }

        void ReparentNodesCommand::undoAction(MapDocumentCommandFacade* document) {
            document->performRemoveNodes(m_nodesToAdd);
            document->performAddNodes(m_nodesToRemove);
        }

        bool ReparentNodesCommand::doCollateWith(UndoableCommand*) {
            return false;
        }
    }
}
