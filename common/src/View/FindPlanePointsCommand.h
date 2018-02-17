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

#ifndef TrenchBroom_FindPlanePointsCommand
#define TrenchBroom_FindPlanePointsCommand

#include "SharedPointer.h"
#include "View/DocumentCommand.h"

namespace TrenchBroom {
    namespace Model {
        class Snapshot;
    }
    
    namespace View {
        class FindPlanePointsCommand : public DocumentCommand {
        public:
            static const CommandType Type;
            typedef std::shared_ptr<FindPlanePointsCommand> Ptr;
        private:
            Model::Snapshot* m_snapshot;
        public:
            static Ptr findPlanePoints();
        private:
            FindPlanePointsCommand();

            bool doPerformDo(MapDocumentCommandFacade* document) override;
            bool doPerformUndo(MapDocumentCommandFacade* document) override;
            
            void deleteSnapshot();
            
            bool doIsRepeatable(MapDocumentCommandFacade* document) const override;
            
            bool doCollateWith(UndoableCommand::Ptr command) override;
        };
    }
}

#endif /* defined(TrenchBroom_FindPlanePointsCommand) */
