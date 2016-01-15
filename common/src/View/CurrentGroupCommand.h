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

#ifndef TrenchBroom_CurrentGroupCommand
#define TrenchBroom_CurrentGroupCommand

#include "SharedPointer.h"
#include "Model/ModelTypes.h"
#include "View/UndoableCommand.h"

#include <map>

namespace TrenchBroom {
    namespace View {
        class CurrentGroupCommand : public UndoableCommand {
        public:
            static const CommandType Type;
            typedef std::tr1::shared_ptr<CurrentGroupCommand> Ptr;
        private:
            Model::Group* m_group;
        public:
            static Ptr push(Model::Group* group);
            static Ptr pop();
        private:
            CurrentGroupCommand(Model::Group* group);
        private:
            bool doPerformDo(MapDocumentCommandFacade* document);
            bool doPerformUndo(MapDocumentCommandFacade* document);
            
            bool doCollateWith(UndoableCommand::Ptr command);
            bool doIsRepeatable(MapDocumentCommandFacade* document) const;
        };
    }
}

#endif /* defined(TrenchBroom_CurrentGroupCommand) */
