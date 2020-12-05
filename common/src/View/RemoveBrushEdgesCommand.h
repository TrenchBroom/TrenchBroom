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

#pragma once

#include "Macros.h"
#include "View/RemoveBrushElementsCommand.h"

#include <vecmath/forward.h>

#include <memory>
#include <vector>

namespace TrenchBroom {
    namespace View {
        class RemoveBrushEdgesCommand : public RemoveBrushElementsCommand {
        public:
            static const CommandType Type;
        private:
            std::vector<vm::segment3> m_oldEdgePositions;
        public:
            static std::unique_ptr<RemoveBrushEdgesCommand> remove(const EdgeToBrushesMap& edges);

            RemoveBrushEdgesCommand(const std::vector<Model::BrushNode*>& brushes, const BrushVerticesMap& vertices, const std::vector<vm::segment3>& edgePositions);
            ~RemoveBrushEdgesCommand() override;
        private:
            void doSelectOldHandlePositions(VertexHandleManagerBaseT<vm::segment3>& manager) const override;

            deleteCopyAndMove(RemoveBrushEdgesCommand)
        };
    }
}

