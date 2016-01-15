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

#include "CurrentGroupCommand.h"

#include "View/MapDocumentCommandFacade.h"

namespace TrenchBroom {
    namespace View {
        const Command::CommandType CurrentGroupCommand::Type = Command::freeType();
        
        CurrentGroupCommand::Ptr CurrentGroupCommand::push(Model::Group* group) {
            return Ptr(new CurrentGroupCommand(group));
        }
        
        CurrentGroupCommand::Ptr CurrentGroupCommand::pop() {
            return Ptr(new CurrentGroupCommand(NULL));
        }
        
        CurrentGroupCommand::CurrentGroupCommand(Model::Group* group) :
        UndoableCommand(Type, group != NULL ? "Push Group" : "Pop Group"),
        m_group(group) {}
        
        bool CurrentGroupCommand::doPerformDo(MapDocumentCommandFacade* document) {
            if (m_group != NULL) {
                document->performPushGroup(m_group);
                m_group = NULL;
            } else {
                m_group = document->currentGroup();
                document->performPopGroup();
            }
            return true;
        }
        
        bool CurrentGroupCommand::doPerformUndo(MapDocumentCommandFacade* document) {
            if (m_group == NULL) {
                m_group = document->currentGroup();
                document->performPopGroup();
            } else {
                document->performPushGroup(m_group);
                m_group = NULL;
            }
            return true;
        }
        
        bool CurrentGroupCommand::doCollateWith(UndoableCommand::Ptr command) {
            return false;
        }
        
        bool CurrentGroupCommand::doIsRepeatable(MapDocumentCommandFacade* document) const {
            return false;
        }
    }
}
