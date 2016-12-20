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
            ensure(parent != NULL, "parent is null");
            Model::ParentChildrenMap nodes;
            nodes[parent] = children;
            
            return add(nodes);
        }
        
        AddRemoveNodesCommand::Ptr AddRemoveNodesCommand::add(const Model::ParentChildrenMap& nodes) {
            return Ptr(new AddRemoveNodesCommand(Action_Add, nodes));
        }
        
        AddRemoveNodesCommand::Ptr AddRemoveNodesCommand::remove(const Model::ParentChildrenMap& nodes) {
            return Ptr(new AddRemoveNodesCommand(Action_Remove, nodes));
        }
        
        AddRemoveNodesCommand::~AddRemoveNodesCommand() {
            MapUtils::clearAndDelete(m_nodesToAdd);
        }

        AddRemoveNodesCommand::AddRemoveNodesCommand(const Action action, const Model::ParentChildrenMap& nodes) :
        DocumentCommand(Type, makeName(action)),
        m_action(action) {
            switch (m_action) {
                case Action_Add:
                    m_nodesToAdd = nodes;
                    break;
                case Action_Remove:
                    m_nodesToRemove = nodes;
                    break;
                switchDefault()
            }
        }
        
        String AddRemoveNodesCommand::makeName(const Action action) {
            switch (action) {
                case Action_Add:
                    return "Add Objects";
                case Action_Remove:
                    return "Remove Objects";
				switchDefault()
            }
        }
        
        bool AddRemoveNodesCommand::doPerformDo(MapDocumentCommandFacade* document) {
            switch (m_action) {
                case Action_Add:
                    document->performAddNodes(m_nodesToAdd);
                    break;
                case Action_Remove:
                    document->performRemoveNodes(m_nodesToRemove);
                    break;
            }

            using std::swap;
            std::swap(m_nodesToAdd, m_nodesToRemove);
            
            return true;
        }
        
        bool AddRemoveNodesCommand::doPerformUndo(MapDocumentCommandFacade* document) {
            switch (m_action) {
                case Action_Add:
                    document->performRemoveNodes(m_nodesToRemove);
                    break;
                case Action_Remove:
                    document->performAddNodes(m_nodesToAdd);
                    break;
            }

            using std::swap;
            std::swap(m_nodesToAdd, m_nodesToRemove);
            
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
