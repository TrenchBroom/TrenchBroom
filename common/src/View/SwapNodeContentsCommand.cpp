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

#include "SwapNodeContentsCommand.h"

#include "Model/Brush.h"
#include "Model/Entity.h"
#include "Model/Node.h"
#include "View/MapDocumentCommandFacade.h"

#include <kdl/vector_utils.h>

namespace TrenchBroom {
    namespace View {
        const Command::CommandType SwapNodeContentsCommand::Type = Command::freeType();

        SwapNodeContentsCommand::SwapNodeContentsCommand(const std::string& name, std::vector<std::pair<Model::Node*, Model::NodeContents>> nodes) :
        DocumentCommand(Type, name),
        m_nodes(std::move(nodes)) {}

        SwapNodeContentsCommand::~SwapNodeContentsCommand() = default;

        std::unique_ptr<CommandResult> SwapNodeContentsCommand::doPerformDo(MapDocumentCommandFacade* document) {
            document->performSwapNodeContents(m_nodes);
            return std::make_unique<CommandResult>(true);
        }

        std::unique_ptr<CommandResult> SwapNodeContentsCommand::doPerformUndo(MapDocumentCommandFacade* document) {
            document->performSwapNodeContents(m_nodes);
            return std::make_unique<CommandResult>(true);
        }

        bool SwapNodeContentsCommand::doCollateWith(UndoableCommand* command) {
            auto* other = static_cast<SwapNodeContentsCommand*>(command);

            auto myNodes = kdl::vec_transform(m_nodes, [](const auto& pair) { return pair.first; });
            auto theirNodes = kdl::vec_transform(other->m_nodes, [](const auto& pair) { return pair.first; });

            kdl::vec_sort(myNodes);
            kdl::vec_sort(theirNodes);
            return myNodes == theirNodes;
        }
    }
}
