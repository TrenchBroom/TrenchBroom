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

#include "ReparentNodesCommand.h"

#include "CollectionUtils.h"
#include "View/MapDocumentCommandFacade.h"

#include <cassert>

namespace TrenchBroom {
    namespace View {
        const Command::CommandType ReparentNodesCommand::Type = Command::freeType();

        ReparentNodesCommand::Ptr ReparentNodesCommand::reparent(Model::Node* newParent, const Model::NodeList& children) {
            assert(newParent != NULL);
            assert(!children.empty());
            
            Model::ParentChildrenMap map;
            map[newParent] = children;
            return Ptr(new ReparentNodesCommand(map));
        }

        ReparentNodesCommand::Ptr ReparentNodesCommand::reparent(const Model::ParentChildrenMap& nodes) {
            assert(!nodes.empty());
            return Ptr(new ReparentNodesCommand(nodes));
        }

        ReparentNodesCommand::ReparentNodesCommand(const Model::ParentChildrenMap& nodes) :
        DocumentCommand(Type, "Reparent Nodes"),
        m_nodes(nodes) {}
        
        ReparentNodesCommand::~ReparentNodesCommand() {
            MapUtils::clearAndDelete(m_removedNodes);
        }

        bool ReparentNodesCommand::doPerformDo(MapDocumentCommandFacade* document) {
            const MapDocumentCommandFacade::ReparentResult result = document->performReparentNodes(m_nodes, MapDocumentCommandFacade::RemoveEmptyNodes);
            m_nodes = result.movedNodes;
            m_removedNodes = result.removedNodes;
            return true;
        }
        
        bool ReparentNodesCommand::doPerformUndo(MapDocumentCommandFacade* document) {
            document->addNodes(m_removedNodes);
            
            const MapDocumentCommandFacade::ReparentResult result = document->performReparentNodes(m_nodes, MapDocumentCommandFacade::KeepEmptyNodes);
            m_nodes = result.movedNodes;
            m_removedNodes = result.removedNodes;
            assert(m_removedNodes.empty());
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
