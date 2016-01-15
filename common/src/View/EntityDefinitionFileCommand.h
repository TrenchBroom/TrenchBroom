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

#ifndef TrenchBroom_EntityDefinitionFileCommand
#define TrenchBroom_EntityDefinitionFileCommand

#include "SharedPointer.h"
#include "View/DocumentCommand.h"
#include "Assets/EntityDefinitionFileSpec.h"

namespace TrenchBroom {
    namespace View {
        class EntityDefinitionFileCommand : public DocumentCommand {
        public:
            static const CommandType Type;
            typedef std::tr1::shared_ptr<EntityDefinitionFileCommand> Ptr;
        private:
            Assets::EntityDefinitionFileSpec m_oldSpec;
            Assets::EntityDefinitionFileSpec m_newSpec;
        public:
            static Ptr set(const Assets::EntityDefinitionFileSpec& spec);
        private:
            EntityDefinitionFileCommand(const String& name, const Assets::EntityDefinitionFileSpec& spec);
            
            bool doPerformDo(MapDocumentCommandFacade* document);
            bool doPerformUndo(MapDocumentCommandFacade* document);
            
            bool doIsRepeatable(MapDocumentCommandFacade* document) const;
            bool doCollateWith(UndoableCommand::Ptr command);
        };
    }
}

#endif /* defined(TrenchBroom_EntityDefinitionFileCommand) */
