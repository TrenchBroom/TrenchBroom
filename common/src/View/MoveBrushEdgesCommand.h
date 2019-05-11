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

#ifndef TrenchBroom_MoveBrushEdgesCommand
#define TrenchBroom_MoveBrushEdgesCommand

#include "SharedPointer.h"
#include "Model/ModelTypes.h"
#include "View/VertexCommand.h"

namespace TrenchBroom {
    namespace Model {
        class Snapshot;
    }

    namespace View {
        class MoveBrushEdgesCommand : public VertexCommand {
        public:
            static const CommandType Type;
            using Ptr = std::shared_ptr<MoveBrushEdgesCommand>;
        private:
            Model::BrushEdgesMap m_edges;
            std::vector<vm::segment3> m_oldEdgePositions;
            std::vector<vm::segment3> m_newEdgePositions;
            vm::vec3 m_delta;
        public:
            static Ptr move(const Model::EdgeToBrushesMap& edges, const vm::vec3& delta);
        private:
        private:
            MoveBrushEdgesCommand(const Model::BrushList& brushes, const Model::BrushEdgesMap& edges, const std::vector<vm::segment3>& edgePositions, const vm::vec3& delta);

            bool doCanDoVertexOperation(const MapDocument* document) const override;
            bool doVertexOperation(MapDocumentCommandFacade* document) override;

            bool doCollateWith(UndoableCommand::Ptr command) override;

            void doSelectNewHandlePositions(VertexHandleManagerBaseT<vm::segment3>& manager) const override;
            void doSelectOldHandlePositions(VertexHandleManagerBaseT<vm::segment3>& manager) const override;
        };
    }
}

#endif /* defined(TrenchBroom_MoveBrushEdgesCommand) */
