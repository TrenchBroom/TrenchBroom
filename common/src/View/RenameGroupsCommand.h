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

#ifndef TrenchBroom_RenameGroupsCommand
#define TrenchBroom_RenameGroupsCommand

#include "SharedPointer.h"
#include "Model/ModelTypes.h"
#include "View/DocumentCommand.h"

namespace TrenchBroom {
    namespace View {
        class MapDocumentCommandFacade;
        
        class RenameGroupsCommand : public DocumentCommand {
        public:
            static const CommandType Type;
            typedef std::tr1::shared_ptr<RenameGroupsCommand> Ptr;
        private:
            const String m_newName;
            Model::GroupNameMap m_oldNames;
        public:
            static Ptr rename(const String& newName);
        private:
            RenameGroupsCommand(const String& newName);
            
            bool doPerformDo(MapDocumentCommandFacade* document);
            bool doPerformUndo(MapDocumentCommandFacade* document);
            
            bool doIsRepeatable(MapDocumentCommandFacade* document) const;
            
            bool doCollateWith(UndoableCommand::Ptr command);
        };
    }
}

#endif /* defined(TrenchBroom_RenameGroupsCommand) */
