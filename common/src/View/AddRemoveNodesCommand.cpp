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

#include "AddRemoveNodesCommand.h"

#include "CollectionUtils.h"
#include "Macros.h"
#include "Model/Node.h"
#include "View/MapDocumentCommandFacade.h"

#include <cassert>

namespace TrenchBroom {
    namespace View {
        const Command::CommandType AddRemoveNodesCommand::Type = Command::freeType();
        
        AddRemoveNodesCommand::Ptr AddRemoveNodesCommand::add(Model::Node* parent, const Model::NodeList& children) {
            assert(parent != NULL);
            Model::ParentChildrenMap nodes;
            nodes[parent] = children;
            
            return add(nodes);
        }
        
        AddRemoveNodesCommand::Ptr AddRemoveNodesCommand::add(const Model::ParentChildrenMap& nodes) {
            return Ptr(new AddRemoveNodesCommand(nodes));
        }
        
        AddRemoveNodesCommand::Ptr AddRemoveNodesCommand::remove(const Model::NodeList& nodes) {
            return Ptr(new AddRemoveNodesCommand(nodes));
        }
        
        AddRemoveNodesCommand::~AddRemoveNodesCommand() {
            MapUtils::clearAndDelete(m_nodesToAdd);
        }

        AddRemoveNodesCommand::AddRemoveNodesCommand(const Model::ParentChildrenMap& nodesToAdd) :
        DocumentCommand(Type, makeName(Action_Add)),
        m_action(Action_Add),
        m_nodesToAdd(nodesToAdd) {}
        
        AddRemoveNodesCommand::AddRemoveNodesCommand(const Model::NodeList& nodesToRemove) :
        DocumentCommand(Type, makeName(Action_Remove)),
        m_action(Action_Remove),
        m_nodesToRemove(nodesToRemove) {}
        
        const Model::NodeList& AddRemoveNodesCommand::addedNodes() const {
            return m_nodesToRemove;
        }

        String AddRemoveNodesCommand::makeName(const Action action) {
            switch (action) {
                case Action_Add:
                    return "Add objects";
                case Action_Remove:
                    return "Remove objects";
				switchDefault()
            }
        }
        
        bool AddRemoveNodesCommand::doPerformDo(MapDocumentCommandFacade* document) {
            switch (m_action) {
                case Action_Add:
                    m_nodesToRemove = document->performAddNodes(m_nodesToAdd);
                    m_nodesToAdd.clear();
                    break;
                case Action_Remove:
                    m_nodesToAdd = document->performRemoveNodes(m_nodesToRemove);
                    m_nodesToRemove.clear();
                    break;
            }
            return true;
        }
        
        bool AddRemoveNodesCommand::doPerformUndo(MapDocumentCommandFacade* document) {
            switch (m_action) {
                case Action_Add:
                    m_nodesToAdd = document->performRemoveNodes(m_nodesToRemove);
                    m_nodesToRemove.clear();
                    break;
                case Action_Remove:
                    m_nodesToRemove = document->performAddNodes(m_nodesToAdd);
                    m_nodesToAdd.clear();
                    break;
            }
            return true;
        }

        bool AddRemoveNodesCommand::doIsRepeatable(MapDocumentCommandFacade* document) const {
            return false;
        }
        
        bool AddRemoveNodesCommand::doCollateWith(UndoableCommand::Ptr command) {
            return false;
        }
    }
}
