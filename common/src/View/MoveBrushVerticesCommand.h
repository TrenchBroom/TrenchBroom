/*
 Copyright (C) 2010-2014 Kristian Duske
 
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

#ifndef __TrenchBroom__MoveBrushVerticesCommand__
#define __TrenchBroom__MoveBrushVerticesCommand__

#include "Model/ModelTypes.h"
#include "View/DocumentCommand.h"

namespace TrenchBroom {
    namespace Model {
        class Snapshot;
    }
    
    namespace View {
        class VertexHandleManager;
        
        class MoveBrushVerticesCommand : public DocumentCommand {
        public:
            static const CommandType Type;
        private:
            struct VertexInfo {
                Model::BrushList brushes;
                Model::BrushVerticesMap vertices;
                Vec3::List vertexPositions;
            };
        private:
            VertexHandleManager& m_handleManager;
            Vec3 m_delta;
            
            Model::Snapshot* m_snapshot;
            
            Model::BrushList m_brushes;
            Vec3::List m_oldVertexPositions;
            Vec3::List m_newVertexPositions;
        public:
            static MoveBrushVerticesCommand* move(VertexHandleManager& handleManager, const Vec3& delta);
            ~MoveBrushVerticesCommand();
            
            bool hasRemainingVertices() const;
        private:
            MoveBrushVerticesCommand(VertexHandleManager& handleManager, const Vec3& delta);
            
            bool doPerformDo(MapDocumentCommandFacade* document);
            bool doPerformUndo(MapDocumentCommandFacade* document);
            
            bool canMoveVertices(const BBox3& worldBounds, const Model::BrushVerticesMap& brushVertices) const;
            
            VertexInfo buildInfo() const;
            void buildInfo(const Model::VertexToBrushesMap& vertices, VertexInfo& info) const;
            
            void takeSnapshot(const Model::BrushList& brushes);
            void deleteSnapshot();
            
            bool doIsRepeatable(MapDocumentCommandFacade* document) const;
            UndoableCommand* doRepeat(MapDocumentCommandFacade* document) const;

            bool doCollateWith(UndoableCommand* command);
        };
    }
}
#endif /* defined(__TrenchBroom__MoveBrushVerticesCommand__) */
