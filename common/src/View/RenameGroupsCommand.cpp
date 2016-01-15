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

#include "RenameGroupsCommand.h"

#include "View/MapDocumentCommandFacade.h"

namespace TrenchBroom {
    namespace View {
        const Command::CommandType RenameGroupsCommand::Type = Command::freeType();
        
        RenameGroupsCommand::Ptr RenameGroupsCommand::rename(const String& newName) {
            return Ptr(new RenameGroupsCommand(newName));
        }

        RenameGroupsCommand::RenameGroupsCommand(const String& newName) :
        DocumentCommand(Type, "Rename Groups"),
        m_newName(newName) {}
        
        bool RenameGroupsCommand::doPerformDo(MapDocumentCommandFacade* document) {
            m_oldNames = document->performRenameGroups(m_newName);
            return true;
        }
        
        bool RenameGroupsCommand::doPerformUndo(MapDocumentCommandFacade* document) {
            document->performUndoRenameGroups(m_oldNames);
            return true;
        }
        
        bool RenameGroupsCommand::doIsRepeatable(MapDocumentCommandFacade* document) const {
            return false;
        }
        
        bool RenameGroupsCommand::doCollateWith(UndoableCommand::Ptr command) {
            return false;
        }
    }
}
