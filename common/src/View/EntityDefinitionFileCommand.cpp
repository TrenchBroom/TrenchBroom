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

#include "EntityDefinitionFileCommand.h"
#include "View/MapDocumentCommandFacade.h"

#include <cassert>

namespace TrenchBroom {
    namespace View {
        const Command::CommandType EntityDefinitionFileCommand::Type = Command::freeType();

        EntityDefinitionFileCommand::Ptr EntityDefinitionFileCommand::set(const Assets::EntityDefinitionFileSpec& spec) {
            return Ptr(new EntityDefinitionFileCommand("Set Entity Definitions", spec));
        }

        EntityDefinitionFileCommand::EntityDefinitionFileCommand(const String& name, const Assets::EntityDefinitionFileSpec& spec) :
        DocumentCommand(Type, name),
        m_newSpec(spec) {}
        
        bool EntityDefinitionFileCommand::doPerformDo(MapDocumentCommandFacade* document) {
            m_oldSpec = document->entityDefinitionFile();
            document->performSetEntityDefinitionFile(m_newSpec);
            return true;
        }
        
        bool EntityDefinitionFileCommand::doPerformUndo(MapDocumentCommandFacade* document) {
            document->performSetEntityDefinitionFile(m_oldSpec);
            return true;
        }
        
        bool EntityDefinitionFileCommand::doIsRepeatable(MapDocumentCommandFacade* document) const {
            return false;
        }
        
        bool EntityDefinitionFileCommand::doCollateWith(UndoableCommand::Ptr command) {
            return false;
        }
    }
}
