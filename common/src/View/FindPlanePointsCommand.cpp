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

#include "FindPlanePointsCommand.h"

#include "Model/Snapshot.h"
#include "View/MapDocumentCommandFacade.h"

namespace TrenchBroom {
    namespace View {
        const Command::CommandType FindPlanePointsCommand::Type = Command::freeType();

        FindPlanePointsCommand::Ptr FindPlanePointsCommand::findPlanePoints() {
            return Ptr(new FindPlanePointsCommand());
        }

        FindPlanePointsCommand::FindPlanePointsCommand() :
        DocumentCommand(Type, "Find Plane Points"),
        m_snapshot(NULL) {}
        
        bool FindPlanePointsCommand::doPerformDo(MapDocumentCommandFacade* document) {
            assert(m_snapshot == NULL);
            m_snapshot = document->performFindPlanePoints();
            return m_snapshot != NULL;
        }
        
        bool FindPlanePointsCommand::doPerformUndo(MapDocumentCommandFacade* document) {
            assert(m_snapshot != NULL);
            document->restoreSnapshot(m_snapshot);
            deleteSnapshot();
            return true;
        }
        
        void FindPlanePointsCommand::deleteSnapshot() {
            delete m_snapshot;
            m_snapshot = NULL;
        }

        bool FindPlanePointsCommand::doIsRepeatable(MapDocumentCommandFacade* document) const {
            return false;
        }
        
        bool FindPlanePointsCommand::doCollateWith(UndoableCommand::Ptr command) {
            return false;
        }
    }
}
