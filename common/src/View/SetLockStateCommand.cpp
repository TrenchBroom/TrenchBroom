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

#include "SetLockStateCommand.h"
#include "Macros.h"
#include "View/MapDocumentCommandFacade.h"

namespace TrenchBroom {
    namespace View {
        const Command::CommandType SetLockStateCommand::Type = Command::freeType();

        SetLockStateCommand::Ptr SetLockStateCommand::lock(const Model::NodeList& nodes) {
            return Ptr(new SetLockStateCommand(nodes, Model::Lock_Locked));
        }
        
        SetLockStateCommand::Ptr SetLockStateCommand::unlock(const Model::NodeList& nodes) {
            return Ptr(new SetLockStateCommand(nodes, Model::Lock_Unlocked));
        }
        
        SetLockStateCommand::Ptr SetLockStateCommand::reset(const Model::NodeList& nodes) {
            return Ptr(new SetLockStateCommand(nodes, Model::Lock_Inherited));
        }

        SetLockStateCommand::SetLockStateCommand(const Model::NodeList& nodes, const Model::LockState state) :
        UndoableCommand(Type, makeName(state)),
        m_nodes(nodes),
        m_state(state) {}

        String SetLockStateCommand::makeName(const Model::LockState state) {
            switch (state) {
                case Model::Lock_Inherited:
                    return "Reset Locking";
                case Model::Lock_Locked:
                    return "Lock Objects";
                case Model::Lock_Unlocked:
                    return "Unlock Objects";
		switchDefault()
            }
        }
        
        bool SetLockStateCommand::doPerformDo(MapDocumentCommandFacade* document) {
            m_oldState = document->setLockState(m_nodes, m_state);
            return true;
        }

        bool SetLockStateCommand::doPerformUndo(MapDocumentCommandFacade* document) {
            document->restoreLockState(m_oldState);
            return true;
        }

        bool SetLockStateCommand::doCollateWith(UndoableCommand::Ptr command) {
            return false;
        }

        bool SetLockStateCommand::doIsRepeatable(MapDocumentCommandFacade* document) const {
            return false;
        }
    }
}
