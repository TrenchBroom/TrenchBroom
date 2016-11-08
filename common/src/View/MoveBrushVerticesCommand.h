/*
 Copyright (C) 2010-2016 Kristian Duske
 
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
        class VertexHandleManager;
        
        class MoveBrushVerticesCommand : public VertexCommand {
        public:
            static const CommandType Type;
            typedef std::shared_ptr<MoveBrushVerticesCommand> Ptr;
        private:
            Model::BrushVerticesMap m_vertices;
            Vec3::List m_oldVertexPositions;
            Vec3::List m_newVertexPositions;
            Vec3 m_delta;
        public:
            static Ptr move(const Model::VertexToBrushesMap& vertices, const Vec3& delta);
            bool hasRemainingVertices() const;
        private:
            MoveBrushVerticesCommand(const Model::BrushList& brushes, const Model::BrushVerticesMap& vertices, const Vec3::List& vertexPositions, const Vec3& delta);
            
            bool doCanDoVertexOperation(const MapDocument* document) const;
            bool doVertexOperation(MapDocumentCommandFacade* document);
            
            void doSelectNewHandlePositions(VertexHandleManager& manager, const Model::BrushList& brushes);
            void doSelectOldHandlePositions(VertexHandleManager& manager, const Model::BrushList& brushes);

            bool doCollateWith(UndoableCommand::Ptr command);
        };
    }
}

#endif /* defined(TrenchBroom_MoveBrushVerticesCommand) */
