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
#include "View/VertexCommand.h"

#include <vecmath/forward.h>

#include <memory>
#include <vector>

namespace TrenchBroom {
    namespace View {
        class MoveBrushEdgesCommand : public VertexCommand {
        public:
            static const CommandType Type;
        private:
            BrushEdgesMap m_edges;
            std::vector<vm::segment3> m_oldEdgePositions;
            std::vector<vm::segment3> m_newEdgePositions;
            vm::vec3 m_delta;
        public:
            static std::unique_ptr<MoveBrushEdgesCommand> move(const EdgeToBrushesMap& edges, const vm::vec3& delta);

            MoveBrushEdgesCommand(const std::vector<Model::BrushNode*>& brushes, const BrushEdgesMap& edges, const std::vector<vm::segment3>& edgePositions, const vm::vec3& delta);
            ~MoveBrushEdgesCommand() override;
        private:
            bool doCanDoVertexOperation(const MapDocument* document) const override;
            bool doVertexOperation(MapDocumentCommandFacade* document) override;

            bool doCollateWith(UndoableCommand* command) override;

            void doSelectNewHandlePositions(VertexHandleManagerBaseT<vm::segment3>& manager) const override;
            void doSelectOldHandlePositions(VertexHandleManagerBaseT<vm::segment3>& manager) const override;

            deleteCopyAndMove(MoveBrushEdgesCommand)
        };
    }
}

