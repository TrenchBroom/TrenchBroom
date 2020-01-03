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

#include "ChangeEntityAttributesCommand.h"

#include "Macros.h"
#include "Model/EntityAttributeSnapshot.h"
#include "View/MapDocument.h"
#include "View/MapDocumentCommandFacade.h"

namespace TrenchBroom {
    namespace View {
        const Command::CommandType ChangeEntityAttributesCommand::Type = Command::freeType();

        std::unique_ptr<ChangeEntityAttributesCommand> ChangeEntityAttributesCommand::set(const std::string& name, const std::string& value) {
            auto command = std::make_unique<ChangeEntityAttributesCommand>(Action::Set);
            command->setName(name);
            command->setNewValue(value);
            return command;
        }

        std::unique_ptr<ChangeEntityAttributesCommand> ChangeEntityAttributesCommand::remove(const std::string& name) {
            auto command = std::make_unique<ChangeEntityAttributesCommand>(Action::Remove);
            command->setName(name);
            return command;
        }

        std::unique_ptr<ChangeEntityAttributesCommand> ChangeEntityAttributesCommand::rename(const std::string& oldName, const std::string& newName) {
            auto command = std::make_unique<ChangeEntityAttributesCommand>(Action::Rename);
            command->setName(oldName);
            command->setNewName(newName);
            return command;
        }

        void ChangeEntityAttributesCommand::setName(const std::string& name) {
            m_oldName = name;
        }

        void ChangeEntityAttributesCommand::setNewName(const std::string& newName) {
            assert(m_action == Action::Rename);
            m_newName = newName;
        }

        void ChangeEntityAttributesCommand::setNewValue(const std::string& newValue) {
            assert(m_action == Action::Set);
            m_newValue = newValue;
        }

        ChangeEntityAttributesCommand::ChangeEntityAttributesCommand(const Action action) :
        DocumentCommand(Type, makeName(action)),
        m_action(action) {}

        ChangeEntityAttributesCommand::~ChangeEntityAttributesCommand() = default;

        std::string ChangeEntityAttributesCommand::makeName(const Action action) {
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

        std::unique_ptr<CommandResult> ChangeEntityAttributesCommand::doPerformDo(MapDocumentCommandFacade* document) {
            switch (m_action) {
                case Action::Set:
                    m_snapshots = document->performSetAttribute(m_oldName, m_newValue);
                    break;
                case Action::Remove:
                    m_snapshots = document->performRemoveAttribute(m_oldName);
                    break;
                case Action::Rename:
                    m_snapshots = document->performRenameAttribute(m_oldName, m_newName);
                    break;
            };
            return std::make_unique<CommandResult>(true);
        }

        std::unique_ptr<CommandResult> ChangeEntityAttributesCommand::doPerformUndo(MapDocumentCommandFacade* document) {
            document->restoreAttributes(m_snapshots);
            m_snapshots.clear();
            return std::make_unique<CommandResult>(true);
        }

        bool ChangeEntityAttributesCommand::doIsRepeatable(MapDocumentCommandFacade*) const {
            return false;
        }

        bool ChangeEntityAttributesCommand::doCollateWith(UndoableCommand* command) {
            ChangeEntityAttributesCommand* other = static_cast<ChangeEntityAttributesCommand*>(command);
            if (other->m_action != m_action) {
                return false;
            }
            if (other->m_oldName != m_oldName) {
                return false;
            }

            m_newName = other->m_newName;
            m_newValue = other->m_newValue;
            return true;
        }
    }
}
