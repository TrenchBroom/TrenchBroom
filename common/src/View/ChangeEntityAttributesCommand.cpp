/*
 Copyright (C) 2010-2014 Kristian Duske
 
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
#include "View/MapDocument.h"
#include "View/MapDocumentCommandFacade.h"

namespace TrenchBroom {
    namespace View {
        const Command::CommandType ChangeEntityAttributesCommand::Type = Command::freeType();

        ChangeEntityAttributesCommand::Ptr ChangeEntityAttributesCommand::set(const Model::AttributeName& name, const Model::AttributeValue& value) {
            Ptr command(new ChangeEntityAttributesCommand(Action_Set));
            command->setName(name);
            command->setNewValue(value);
            return command;
        }
        
        ChangeEntityAttributesCommand::Ptr ChangeEntityAttributesCommand::remove(const Model::AttributeName& name) {
            Ptr command(new ChangeEntityAttributesCommand(Action_Remove));
            command->setName(name);
            return command;
        }
        
        ChangeEntityAttributesCommand::Ptr ChangeEntityAttributesCommand::rename(const Model::AttributeName& oldName, const Model::AttributeName& newName) {
            Ptr command(new ChangeEntityAttributesCommand(Action_Rename));
            command->setName(oldName);
            command->setNewName(newName);
            return command;
        }

        void ChangeEntityAttributesCommand::setName(const Model::AttributeName& name) {
            m_oldName = name;
        }
        
        void ChangeEntityAttributesCommand::setNewName(const Model::AttributeName& newName) {
            assert(m_action == Action_Rename);
            m_newName = newName;
        }
        
        void ChangeEntityAttributesCommand::setNewValue(const Model::AttributeValue& newValue) {
            assert(m_action == Action_Set);
            m_newValue = newValue;
        }

        ChangeEntityAttributesCommand::ChangeEntityAttributesCommand(const Action action) :
        DocumentCommand(Type, makeName(action)),
        m_action(action) {}
        
        String ChangeEntityAttributesCommand::makeName(const Action action) {
            switch (action) {
                case Action_Set:
                    return "Set Attribute";
                case Action_Remove:
                    return "Remove Attribute";
                case Action_Rename:
                    return "Rename Attribute";
				switchDefault()
            }
        }
        
        bool ChangeEntityAttributesCommand::doPerformDo(MapDocumentCommandFacade* document) {
            switch (m_action) {
                case Action_Set:
                    m_snapshots = document->performSetAttribute(m_oldName, m_newValue);
                    break;
                case Action_Remove:
                    m_snapshots = document->performRemoveAttribute(m_oldName);
                    break;
                case Action_Rename:
                    document->performRenameAttribute(m_oldName, m_newName);
                    break;
            };
            return true;
        }
        
        bool ChangeEntityAttributesCommand::doPerformUndo(MapDocumentCommandFacade* document) {
            switch (m_action) {
                case Action_Set:
                case Action_Remove:
                    document->restoreAttributes(m_snapshots);
                    m_snapshots.clear();
                    break;
                case Action_Rename:
                    document->performRenameAttribute(m_newName, m_oldName);
                    break;
            };
            return true;
        }
        
        bool ChangeEntityAttributesCommand::doIsRepeatable(MapDocumentCommandFacade* document) const {
            return false;
        }
        
        bool ChangeEntityAttributesCommand::doCollateWith(UndoableCommand::Ptr command) {
            ChangeEntityAttributesCommand* other = static_cast<ChangeEntityAttributesCommand*>(command.get());
            if (other->m_action != m_action)
                return false;
            if (other->m_oldName != m_oldName)
                return false;
            m_newName = other->m_newName;
            m_newValue = other->m_newValue;
            return true;
        }
    }
}
