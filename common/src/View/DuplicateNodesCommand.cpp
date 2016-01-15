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
#include "Model/NodeVisitor.h"
#include "View/MapDocumentCommandFacade.h"

namespace TrenchBroom {
    namespace View {
        const Command::CommandType DuplicateNodesCommand::Type = Command::freeType();

        DuplicateNodesCommand::Ptr DuplicateNodesCommand::duplicate() {
            return Ptr(new DuplicateNodesCommand());
        }

        DuplicateNodesCommand::DuplicateNodesCommand() :
        DocumentCommand(Type, "Duplicate objects") {}
        
        bool DuplicateNodesCommand::doPerformDo(MapDocumentCommandFacade* document) {
            typedef std::pair<bool, Model::NodeMap::iterator> NodeMapInsertPos;
            
            Model::NodeMap newParentMap;
            Model::ParentChildrenMap nodesToAdd;
            Model::NodeList nodesToSelect;

            const BBox3& worldBounds = document->worldBounds();
            m_previouslySelectedNodes = document->selectedNodes().nodes();
            
            Model::NodeList::const_iterator it, end;
            for (it = m_previouslySelectedNodes.begin(), end = m_previouslySelectedNodes.end(); it != end; ++it) {
                const Model::Node* original = *it;
                Model::Node* clone = original->cloneRecursively(worldBounds);

                Model::Node* parent = original->parent();
                if (cloneParent(parent)) {
                    NodeMapInsertPos insertPos = MapUtils::findInsertPos(newParentMap, parent);
                    Model::Node* newParent = NULL;
                    if (insertPos.first) {
                        newParent = (insertPos.second)->second;
                    } else {
                        newParent = parent->clone(worldBounds);
                        newParentMap.insert(insertPos.second, std::make_pair(parent, newParent));
                        nodesToAdd[parent->parent()].push_back(newParent);
                    }
                    
                    newParent->addChild(clone);
                } else {
                    nodesToAdd[parent].push_back(clone);
                }
                
                nodesToSelect.push_back(clone);
            }
            
            m_addedNodes = document->performAddNodes(nodesToAdd);
            document->performDeselectAll();
            document->performSelect(nodesToSelect);
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
        
        class DuplicateNodesCommand::CloneParentQuery : public Model::ConstNodeVisitor, public Model::NodeQuery<bool> {
        private:
            void doVisit(const Model::World* world)   { setResult(false); }
            void doVisit(const Model::Layer* layer)   { setResult(false); }
            void doVisit(const Model::Group* group)   { setResult(false);  }
            void doVisit(const Model::Entity* entity) { setResult(true);  }
            void doVisit(const Model::Brush* brush)   { setResult(false); }
        };
        
        bool DuplicateNodesCommand::cloneParent(const Model::Node* node) const {
            CloneParentQuery query;
            node->accept(query);
            return query.result();
        }

        bool DuplicateNodesCommand::doIsRepeatable(MapDocumentCommandFacade* document) const {
            return document->hasSelectedNodes();
        }
        
        UndoableCommand::Ptr DuplicateNodesCommand::doRepeat(MapDocumentCommandFacade* document) const {
            return UndoableCommand::Ptr(new DuplicateNodesCommand());
        }
        
        bool DuplicateNodesCommand::doCollateWith(UndoableCommand::Ptr command) {
            return false;
        }
    }
}
