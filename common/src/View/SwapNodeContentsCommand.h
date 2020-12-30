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

#pragma once

#include "Macros.h"
#include "Model/NodeContents.h"
#include "View/DocumentCommand.h"

#include <memory>
#include <string>
#include <tuple>
#include <vector>

namespace TrenchBroom {
    namespace Model {
        class Brush;
        class Entity;
        class Node;
    }

    namespace View {
        class SwapNodeContentsCommand : public DocumentCommand {
        public:
            static const CommandType Type;
        protected:
            std::vector<std::pair<Model::Node*, Model::NodeContents>> m_nodes;
        public:
            SwapNodeContentsCommand(const std::string& name, std::vector<std::pair<Model::Node*, Model::NodeContents>> nodes);
            ~SwapNodeContentsCommand();

            std::unique_ptr<CommandResult> doPerformDo(MapDocumentCommandFacade* document) override;
            std::unique_ptr<CommandResult> doPerformUndo(MapDocumentCommandFacade* document) override;

            bool doCollateWith(UndoableCommand* command) override;

            deleteCopyAndMove(SwapNodeContentsCommand)
        };
    }
}
