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

#ifndef TrenchBroom_MoveBrushVerticesCommand
#define TrenchBroom_MoveBrushVerticesCommand

#include "SharedPointer.h"
#include "Model/ModelTypes.h"
#include "View/VertexCommand.h"

namespace TrenchBroom {
    namespace Model {
        class Snapshot;
    }
    
    namespace View {
        class MoveBrushVerticesCommand : public VertexCommand {
        public:
            static const CommandType Type;
            typedef std::shared_ptr<MoveBrushVerticesCommand> Ptr;
        private:
            Model::BrushVerticesMap m_vertices;
            std::vector<vm::vec3> m_oldVertexPositions;
            std::vector<vm::vec3> m_newVertexPositions;
            vm::vec3 m_delta;
        public:
            static Ptr move(const Model::VertexToBrushesMap& vertices, const vm::vec3& delta);
            bool hasRemainingVertices() const;
        private:
            MoveBrushVerticesCommand(const Model::BrushList& brushes, const Model::BrushVerticesMap& vertices, const std::vector<vm::vec3>& vertexPositions, const vm::vec3& delta);
            
            bool doCanDoVertexOperation(const MapDocument* document) const override;
            bool doVertexOperation(MapDocumentCommandFacade* document) override;
            
            bool doCollateWith(UndoableCommand::Ptr command) override;

            void doSelectNewHandlePositions(VertexHandleManagerBaseT<vm::vec3>& manager) const override;
            void doSelectOldHandlePositions(VertexHandleManagerBaseT<vm::vec3>& manager) const override;
        };
    }
}

#endif /* defined(TrenchBroom_MoveBrushVerticesCommand) */
