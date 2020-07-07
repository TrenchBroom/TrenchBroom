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

#include "DuplicateNodesCommand.h"

#include "Model/FindLayerVisitor.h"
#include "Model/Node.h"
#include "Model/NodeVisitor.h"
#include "Model/LayerNode.h"
#include "View/MapDocumentCommandFacade.h"

#include <kdl/map_utils.h>

namespace TrenchBroom {
    namespace View {
        const Command::CommandType DuplicateNodesCommand::Type = Command::freeType();

        std::unique_ptr<DuplicateNodesCommand> DuplicateNodesCommand::duplicate() {
            return std::make_unique<DuplicateNodesCommand>();
        }

        DuplicateNodesCommand::DuplicateNodesCommand() :
        DocumentCommand(Type, "Duplicate Objects"),
        m_firstExecution(true) {}

        DuplicateNodesCommand::~DuplicateNodesCommand() {
            if (state() == CommandState::Default) {
                kdl::map_clear_and_delete(m_addedNodes);
            }
        }

        std::unique_ptr<CommandResult> DuplicateNodesCommand::doPerformDo(MapDocumentCommandFacade* document) {
            if (m_firstExecution) {
                std::map<Model::Node*, Model::Node*> newParentMap;

                const vm::bbox3& worldBounds = document->worldBounds();
                m_previouslySelectedNodes = document->selectedNodes().nodes();

                for (Model::Node* original : m_previouslySelectedNodes) {
                    Model::Node* suggestedParent = document->parentForNodes(std::vector<Model::Node*>{original});
                    Model::Node* clone = original->cloneRecursively(worldBounds);

                    Model::Node* parent = original->parent();
                    if (cloneParent(parent)) {
                        // see if the parent was already cloned and if not, clone it and store it
                        Model::Node* newParent = nullptr;
                        const auto it = newParentMap.find(parent);
                        if (it != std::end(newParentMap)) {
                            // parent was already cloned
                            newParent = it->second;
                        } else {
                            // parent was not cloned yet
                            newParent = parent->clone(worldBounds);
                            newParentMap.insert({ parent, newParent });
                            m_addedNodes[suggestedParent].push_back(newParent);
                        }

                        newParent->addChild(clone);
                    } else {
                        m_addedNodes[suggestedParent].push_back(clone);
                    }

                    m_nodesToSelect.push_back(clone);
                }

                m_firstExecution = false;
            }

            document->performAddNodes(m_addedNodes);
            document->performDeselectAll();
            document->performSelect(m_nodesToSelect);
            return std::make_unique<CommandResult>(true);
        }

        std::unique_ptr<CommandResult> DuplicateNodesCommand::doPerformUndo(MapDocumentCommandFacade* document) {
            document->performDeselectAll();
            document->performRemoveNodes(m_addedNodes);
            document->performSelect(m_previouslySelectedNodes);
            return std::make_unique<CommandResult>(true);
        }

        class DuplicateNodesCommand::CloneParentQuery : public Model::ConstNodeVisitor, public Model::NodeQuery<bool> {
        private:
            void doVisit(const Model::WorldNode*) override  { setResult(false); }
            void doVisit(const Model::LayerNode*) override  { setResult(false); }
            void doVisit(const Model::GroupNode*) override  { setResult(false);  }
            void doVisit(const Model::EntityNode*) override { setResult(true);  }
            void doVisit(const Model::BrushNode*) override  { setResult(false); }
        };

        bool DuplicateNodesCommand::cloneParent(const Model::Node* node) const {
            CloneParentQuery query;
            node->accept(query);
            return query.result();
        }

        bool DuplicateNodesCommand::doIsRepeatable(MapDocumentCommandFacade* document) const {
            return document->hasSelectedNodes();
        }

        std::unique_ptr<UndoableCommand> DuplicateNodesCommand::doRepeat(MapDocumentCommandFacade*) const {
            return std::make_unique<DuplicateNodesCommand>();
        }

        bool DuplicateNodesCommand::doCollateWith(UndoableCommand*) {
            return false;
        }
    }
}
