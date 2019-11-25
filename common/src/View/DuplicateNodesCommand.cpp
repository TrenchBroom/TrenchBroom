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

#include "CollectionUtils.h"
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
        DocumentCommand(Type, "Duplicate Objects"),
        m_firstExecution(true) {}

        DuplicateNodesCommand::~DuplicateNodesCommand() {
            if (state() == CommandState_Default)
                MapUtils::clearAndDelete(m_addedNodes);
        }

        bool DuplicateNodesCommand::doPerformDo(MapDocumentCommandFacade* document) {
            if (m_firstExecution) {
                std::map<Model::Node*, Model::Node*> newParentMap;

                const vm::bbox3& worldBounds = document->worldBounds();
                m_previouslySelectedNodes = document->selectedNodes().nodes();

                for (const Model::Node* original : m_previouslySelectedNodes) {
                    Model::Node* clone = original->cloneRecursively(worldBounds);

                    Model::Node* parent = original->parent();
                    if (cloneParent(parent)) {
                        auto insertPos = MapUtils::findInsertPos(newParentMap, parent);
                        Model::Node* newParent = nullptr;
                        if (insertPos.first) {
                            assert(insertPos.second != std::begin(newParentMap));
                            newParent = std::prev(insertPos.second)->second;
                        } else {
                            newParent = parent->clone(worldBounds);
                            newParentMap.insert(insertPos.second, std::make_pair(parent, newParent));
                            m_addedNodes[document->currentParent()].push_back(newParent);
                        }

                        newParent->addChild(clone);
                    } else {
                        m_addedNodes[document->currentParent()].push_back(clone);
                    }

                    m_nodesToSelect.push_back(clone);
                }

                m_firstExecution = false;
            }

            document->performAddNodes(m_addedNodes);
            document->performDeselectAll();
            document->performSelect(m_nodesToSelect);
            return true;
        }

        bool DuplicateNodesCommand::doPerformUndo(MapDocumentCommandFacade* document) {
            document->performDeselectAll();
            document->performRemoveNodes(m_addedNodes);
            document->performSelect(m_previouslySelectedNodes);
            return true;
        }

        class DuplicateNodesCommand::CloneParentQuery : public Model::ConstNodeVisitor, public Model::NodeQuery<bool> {
        private:
            void doVisit(const Model::World*) override  { setResult(false); }
            void doVisit(const Model::Layer*) override  { setResult(false); }
            void doVisit(const Model::Group*) override  { setResult(false);  }
            void doVisit(const Model::Entity*) override { setResult(true);  }
            void doVisit(const Model::Brush*) override  { setResult(false); }
        };

        bool DuplicateNodesCommand::cloneParent(const Model::Node* node) const {
            CloneParentQuery query;
            node->accept(query);
            return query.result();
        }

        bool DuplicateNodesCommand::doIsRepeatable(MapDocumentCommandFacade* document) const {
            return document->hasSelectedNodes();
        }

        UndoableCommand::Ptr DuplicateNodesCommand::doRepeat(MapDocumentCommandFacade*) const {
            return UndoableCommand::Ptr(new DuplicateNodesCommand());
        }

        bool DuplicateNodesCommand::doCollateWith(UndoableCommand::Ptr) {
            return false;
        }
    }
}
