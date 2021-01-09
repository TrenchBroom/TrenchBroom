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

#include "AddRemoveNodesCommand.h"

#include "Ensure.h"
#include "Macros.h"
#include "Model/Node.h"
#include "Model/UpdateLinkedGroupsError.h"
#include "View/MapDocumentCommandFacade.h"

#include <kdl/map_utils.h>
#include <kdl/result.h>

#include <map>
#include <vector>

namespace TrenchBroom {
    namespace View {
        const Command::CommandType AddRemoveNodesCommand::Type = Command::freeType();

        std::unique_ptr<AddRemoveNodesCommand> AddRemoveNodesCommand::add(Model::Node* parent, const std::vector<Model::Node*>& children, std::vector<Model::GroupNode*> linkedGroupsToUpdate) {
            ensure(parent != nullptr, "parent is null");
            std::map<Model::Node*, std::vector<Model::Node*>> nodes;
            nodes[parent] = children;

            return add(nodes, std::move(linkedGroupsToUpdate));
        }

        std::unique_ptr<AddRemoveNodesCommand> AddRemoveNodesCommand::add(const std::map<Model::Node*, std::vector<Model::Node*>>& nodes, std::vector<Model::GroupNode*> linkedGroupsToUpdate) {
            return std::make_unique<AddRemoveNodesCommand>(Action::Add, nodes, std::move(linkedGroupsToUpdate));
        }

        std::unique_ptr<AddRemoveNodesCommand> AddRemoveNodesCommand::remove(const std::map<Model::Node*, std::vector<Model::Node*>>& nodes, std::vector<Model::GroupNode*> linkedGroupsToUpdate) {
            return std::make_unique<AddRemoveNodesCommand>(Action::Remove, nodes, std::move(linkedGroupsToUpdate));
        }

        AddRemoveNodesCommand::~AddRemoveNodesCommand() {
            kdl::map_clear_and_delete(m_nodesToAdd);
        }

        AddRemoveNodesCommand::AddRemoveNodesCommand(const Action action, const std::map<Model::Node*, std::vector<Model::Node*>>& nodes, std::vector<Model::GroupNode*> linkedGroupsToUpdate) :
        DocumentCommand(Type, makeName(action)),
        m_action(action),
        m_updateLinkedGroupsHelper(std::move(linkedGroupsToUpdate)) {
            switch (m_action) {
                case Action::Add:
                    m_nodesToAdd = nodes;
                    break;
                case Action::Remove:
                    m_nodesToRemove = nodes;
                    break;
                switchDefault()
            }
        }

        std::string AddRemoveNodesCommand::makeName(const Action action) {
            switch (action) {
                case Action::Add:
                    return "Add Objects";
                case Action::Remove:
                    return "Remove Objects";
                switchDefault()
            }
        }

        std::unique_ptr<CommandResult> AddRemoveNodesCommand::doPerformDo(MapDocumentCommandFacade* document) {
            doAction(document);

            const auto success = m_updateLinkedGroupsHelper.applyLinkedGroupUpdates(*document)
                .handle_errors([&](const Model::UpdateLinkedGroupsError& e) {
                    document->error() << e;
                    undoAction(document);
                });

            return std::make_unique<CommandResult>(success);
        }

        std::unique_ptr<CommandResult> AddRemoveNodesCommand::doPerformUndo(MapDocumentCommandFacade* document) {
            undoAction(document);
            m_updateLinkedGroupsHelper.undoLinkedGroupUpdates(*document);
            return std::make_unique<CommandResult>(true);
        }

        void AddRemoveNodesCommand::doAction(MapDocumentCommandFacade* document) {
            switch (m_action) {
                case Action::Add:
                    document->performAddNodes(m_nodesToAdd);
                    break;
                case Action::Remove:
                    document->performRemoveNodes(m_nodesToRemove);
                    break;
            }

            using std::swap;
            std::swap(m_nodesToAdd, m_nodesToRemove);
        }
        
        void AddRemoveNodesCommand::undoAction(MapDocumentCommandFacade* document) {
            switch (m_action) {
                case Action::Add:
                    document->performRemoveNodes(m_nodesToRemove);
                    break;
                case Action::Remove:
                    document->performAddNodes(m_nodesToAdd);
                    break;
            }

            using std::swap;
            std::swap(m_nodesToAdd, m_nodesToRemove);
        }


        bool AddRemoveNodesCommand::doCollateWith(UndoableCommand*) {
            return false;
        }
    }
}
