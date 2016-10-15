/*
 Copyright (C) 2010-2016 Kristian Duske
 
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

#include "ReparentNodesCommand.h"

#include "CollectionUtils.h"
#include "Model/ModelUtils.h"
#include "View/MapDocumentCommandFacade.h"

#include <cassert>

namespace TrenchBroom {
    namespace View {
        const Command::CommandType ReparentNodesCommand::Type = Command::freeType();

        ReparentNodesCommand::Ptr ReparentNodesCommand::reparent(const Model::ParentChildrenMap& nodesToAdd, const Model::ParentChildrenMap& nodesToRemove) {
            return Ptr(new ReparentNodesCommand(nodesToAdd, nodesToRemove));
        }

        ReparentNodesCommand::ReparentNodesCommand(const Model::ParentChildrenMap& nodesToAdd, const Model::ParentChildrenMap& nodesToRemove) :
        DocumentCommand(Type, "Reparent Objects"),
        m_nodesToAdd(nodesToAdd),
        m_nodesToRemove(nodesToRemove) {}

        bool ReparentNodesCommand::doPerformDo(MapDocumentCommandFacade* document) {
            document->performRemoveNodes(m_nodesToRemove);
            document->performAddNodes(m_nodesToAdd);
            return true;
        }
        
        bool ReparentNodesCommand::doPerformUndo(MapDocumentCommandFacade* document) {
            document->performRemoveNodes(m_nodesToAdd);
            document->performAddNodes(m_nodesToRemove);
            return true;
        }
        
        bool ReparentNodesCommand::doIsRepeatable(MapDocumentCommandFacade* document) const {
            return false;
        }
        
        bool ReparentNodesCommand::doCollateWith(UndoableCommand::Ptr command) {
            return false;
        }
    }
}
