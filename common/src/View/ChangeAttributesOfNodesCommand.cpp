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

#include "ChangeAttributesOfNodesCommand.h"

#include "Macros.h"
#include "Model/EntityAttributeSnapshot.h"
#include "View/MapDocument.h"
#include "View/MapDocumentCommandFacade.h"

namespace TrenchBroom {
    namespace View {
        const Command::CommandType ChangeAttributesOfNodesCommand::Type = Command::freeType();

        std::unique_ptr<ChangeAttributesOfNodesCommand> ChangeAttributesOfNodesCommand::set(const std::vector<Model::AttributableNode*>& attributableNodes, const std::string& name, const std::string& value) {
            auto command = std::make_unique<ChangeAttributesOfNodesCommand>(attributableNodes, Action::Set);
            command->setName(name);
            command->setNewValue(value);
            return command;
        }

        std::unique_ptr<ChangeAttributesOfNodesCommand> ChangeAttributesOfNodesCommand::remove(const std::vector<Model::AttributableNode*>& attributableNodes, const std::string& name) {
            auto command = std::make_unique<ChangeAttributesOfNodesCommand>(attributableNodes, Action::Remove);
            command->setName(name);
            return command;
        }

        std::unique_ptr<ChangeAttributesOfNodesCommand> ChangeAttributesOfNodesCommand::rename(const std::vector<Model::AttributableNode*>& attributableNodes, const std::string& oldName, const std::string& newName) {
            auto command = std::make_unique<ChangeAttributesOfNodesCommand>(attributableNodes, Action::Rename);
            command->setName(oldName);
            command->setNewName(newName);
            return command;
        }

        void ChangeAttributesOfNodesCommand::setName(const std::string& name) {
            m_oldName = name;
        }

        void ChangeAttributesOfNodesCommand::setNewName(const std::string& newName) {
            assert(m_action == Action::Rename);
            m_newName = newName;
        }

        void ChangeAttributesOfNodesCommand::setNewValue(const std::string& newValue) {
            assert(m_action == Action::Set);
            m_newValue = newValue;
        }

        ChangeAttributesOfNodesCommand::ChangeAttributesOfNodesCommand(const std::vector<Model::AttributableNode*>& attributableNodes, const Action action) :
        DocumentCommand(Type, makeName(action)),
        m_attributableNodes(attributableNodes),
        m_action(action) {}

        ChangeAttributesOfNodesCommand::~ChangeAttributesOfNodesCommand() = default;

        std::string ChangeAttributesOfNodesCommand::makeName(const Action action) {
            switch (action) {
                case Action::Set:
                    return "Set Property";
                case Action::Remove:
                    return "Remove Property";
                case Action::Rename:
                    return "Rename Property";
				switchDefault()
            }
        }

        std::unique_ptr<CommandResult> ChangeAttributesOfNodesCommand::doPerformDo(MapDocumentCommandFacade* document) {
            switch (m_action) {
                case Action::Set:
                    m_snapshots = document->performSetAttribute(m_attributableNodes, m_oldName, m_newValue);
                    break;
                case Action::Remove:
                    m_snapshots = document->performRemoveAttribute(m_attributableNodes, m_oldName);
                    break;
                case Action::Rename:
                    m_snapshots = document->performRenameAttribute(m_attributableNodes, m_oldName, m_newName);
                    break;
            };
            return std::make_unique<CommandResult>(true);
        }

        std::unique_ptr<CommandResult> ChangeAttributesOfNodesCommand::doPerformUndo(MapDocumentCommandFacade* document) {
            document->restoreAttributes(m_snapshots);
            m_snapshots.clear();
            return std::make_unique<CommandResult>(true);
        }

        bool ChangeAttributesOfNodesCommand::doIsRepeatable(MapDocumentCommandFacade*) const {
            return false;
        }

        bool ChangeAttributesOfNodesCommand::doCollateWith(UndoableCommand*) {
            return false;
        }
    }
}
