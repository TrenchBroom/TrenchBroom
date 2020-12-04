/*
 Copyright (C) 2020 Kristian Duske

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

#include "BrushVertexCommands.h"
#include "View/SwapNodeContentsCommand.h"

#include "View/VertexTool.h"

#include <kdl/vector_utils.h>

namespace TrenchBroom {
    namespace View {
        BrushVertexCommandBase::BrushVertexCommandBase(const std::string& name, std::vector<std::pair<Model::Node*, Model::NodeContents>> nodes) :
        SwapNodeContentsCommand(name, std::move(nodes)) {}

        std::unique_ptr<CommandResult> BrushVertexCommandBase::doPerformDo(MapDocumentCommandFacade* document) {
            return createCommandResult(SwapNodeContentsCommand::doPerformDo(document));
        }

        std::unique_ptr<CommandResult> BrushVertexCommandBase::createCommandResult(std::unique_ptr<CommandResult> swapResult) {
            return swapResult;
        }

        static auto collectBrushNodes(const std::vector<std::pair<Model::Node*, Model::NodeContents>> nodes) {
            auto result = std::vector<Model::BrushNode*>{};
            for (const auto& pair : nodes) {
                if (auto* brushNode = dynamic_cast<Model::BrushNode*>(pair.first)) {
                    result.push_back(brushNode);
                }
            }
            return result;
        }

        void BrushVertexCommandBase::removeHandles(VertexHandleManagerBase& manager) {
            const auto nodes = collectBrushNodes(m_nodes);
            manager.removeHandles(std::begin(nodes), std::end(nodes));
        }

        void BrushVertexCommandBase::addHandles(VertexHandleManagerBase& manager) {
            const auto nodes = collectBrushNodes(m_nodes);
            manager.addHandles(std::begin(nodes), std::end(nodes));
        }

        void BrushVertexCommandBase::selectNewHandlePositions(VertexHandleManagerBaseT<vm::vec3>&) const {}
        void BrushVertexCommandBase::selectOldHandlePositions(VertexHandleManagerBaseT<vm::vec3>&) const {}
        void BrushVertexCommandBase::selectNewHandlePositions(VertexHandleManagerBaseT<vm::segment3>&) const {}
        void BrushVertexCommandBase::selectOldHandlePositions(VertexHandleManagerBaseT<vm::segment3>&) const {}
        void BrushVertexCommandBase::selectNewHandlePositions(VertexHandleManagerBaseT<vm::polygon3>&) const {}
        void BrushVertexCommandBase::selectOldHandlePositions(VertexHandleManagerBaseT<vm::polygon3>&) const {}

        BrushVertexCommandResult::BrushVertexCommandResult(const bool success, const bool hasRemainingVertices) :
        CommandResult(success),
        m_hasRemainingVertices(hasRemainingVertices) {}

        bool BrushVertexCommandResult::hasRemainingVertices() const {
            return m_hasRemainingVertices;
        }

        const Command::CommandType BrushVertexCommand::Type = Command::freeType();

        BrushVertexCommand::BrushVertexCommand(const std::string& name, std::vector<std::pair<Model::Node*, Model::NodeContents>> nodes, std::vector<vm::vec3> oldVertexPositions, std::vector<vm::vec3> newVertexPositions) :
        BrushVertexCommandBase(name, std::move(nodes)),
        m_oldVertexPositions(std::move(oldVertexPositions)),
        m_newVertexPositions(std::move(newVertexPositions)) {}
        
        std::unique_ptr<CommandResult> BrushVertexCommand::createCommandResult(std::unique_ptr<CommandResult> swapResult) {
            return std::make_unique<BrushVertexCommandResult>(swapResult->success(), !m_newVertexPositions.empty());
        }

        bool BrushVertexCommand::doCollateWith(UndoableCommand* command) {
            BrushVertexCommand* other = static_cast<BrushVertexCommand*>(command);
            
            if (m_newVertexPositions != other->m_oldVertexPositions) {
                return false;
            }

            if (!SwapNodeContentsCommand::doCollateWith(command)) {
                return false;
            }
            
            m_newVertexPositions = std::move(other->m_newVertexPositions);

            return true;
        }

        void BrushVertexCommand::selectNewHandlePositions(VertexHandleManagerBaseT<vm::vec3>& manager) const {
            manager.select(std::begin(m_newVertexPositions), std::end(m_newVertexPositions));
        }

        void BrushVertexCommand::selectOldHandlePositions(VertexHandleManagerBaseT<vm::vec3>& manager) const {
            manager.select(std::begin(m_oldVertexPositions), std::end(m_oldVertexPositions));
        }
    }
}
