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

#include <memory>
#include <vector>

namespace TrenchBroom {
    namespace View {
        class MoveBrushVerticesCommandResult : public CommandResult {
        private:
            bool m_hasRemainingVertices;
        public:
            MoveBrushVerticesCommandResult(bool success, bool hasRemainingVertices);

            bool hasRemainingVertices() const;
        };

        class MoveBrushVerticesCommand : public VertexCommand {
        public:
            static const CommandType Type;
        private:
            BrushVerticesMap m_vertices;
            std::vector<vm::vec3> m_oldVertexPositions;
            std::vector<vm::vec3> m_newVertexPositions;
            vm::vec3 m_delta;
        public:
            static std::unique_ptr<MoveBrushVerticesCommand> move(const VertexToBrushesMap& vertices, const vm::vec3& delta);

            MoveBrushVerticesCommand(const std::vector<Model::BrushNode*>& brushes, const BrushVerticesMap& vertices, const std::vector<vm::vec3>& vertexPositions, const vm::vec3& delta);
        private:
            bool doCanDoVertexOperation(const MapDocument* document) const override;
            bool doVertexOperation(MapDocumentCommandFacade* document) override;
            std::unique_ptr<CommandResult> doCreateCommandResult(bool success) override;

            bool doCollateWith(UndoableCommand* command) override;

            void doSelectNewHandlePositions(VertexHandleManagerBaseT<vm::vec3>& manager) const override;
            void doSelectOldHandlePositions(VertexHandleManagerBaseT<vm::vec3>& manager) const override;

            deleteCopyAndMove(MoveBrushVerticesCommand)
        };
    }
}


