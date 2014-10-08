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

#include "DuplicateNodesCommand.h"

#include "Model/Node.h"
#include "View/MapDocumentCommandFacade.h"

namespace TrenchBroom {
    namespace View {
        const Command::CommandType DuplicateNodesCommand::Type = Command::freeType();

        DuplicateNodesCommand* DuplicateNodesCommand::duplicate() {
            return new DuplicateNodesCommand();
        }

        DuplicateNodesCommand::DuplicateNodesCommand() :
        DocumentCommand(Type, "Duplicate objects") {}
        
        bool DuplicateNodesCommand::doPerformDo(MapDocumentCommandFacade* document) {
            const BBox3& worldBounds = document->worldBounds();
            m_previouslySelectedNodes = document->selectedNodes();
            Model::ParentChildrenMap nodesToAdd;
            
            Model::NodeList::const_iterator it, end;
            for (it = m_previouslySelectedNodes.begin(), end = m_previouslySelectedNodes.end(); it != end; ++it) {
                const Model::Node* original = *it;
                Model::Node* parent = original->parent();
                Model::Node* clone = original->clone(worldBounds);
                nodesToAdd[parent].push_back(clone);
            }
            
            m_addedNodes = document->performAddNodes(nodesToAdd);
            document->performDeselectAll();
            document->performSelect(m_addedNodes);
            return true;
        }
        
        bool DuplicateNodesCommand::doPerformUndo(MapDocumentCommandFacade* document) {
            document->performDeselectAll();
            document->performRemoveNodes(m_addedNodes);
            document->performSelect(m_previouslySelectedNodes);
            
            m_previouslySelectedNodes.clear();
            VectorUtils::clearAndDelete(m_addedNodes);
            return true;
        }
        
        bool DuplicateNodesCommand::doIsRepeatable(MapDocumentCommandFacade* document) const {
            return document->hasSelectedNodes();
        }
        
        UndoableCommand* DuplicateNodesCommand::doRepeat(MapDocumentCommandFacade* document) const {
            return new DuplicateNodesCommand();
        }
        
        bool DuplicateNodesCommand::doCollateWith(UndoableCommand* command) {
            return false;
        }
    }
}
