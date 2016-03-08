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

#ifndef TrenchBroom_VertexCommand
#define TrenchBroom_VertexCommand

#include "Model/ModelTypes.h"
#include "View/DocumentCommand.h"

namespace TrenchBroom {
    namespace Model {
        class Snapshot;
    }
    
    namespace View {
        class VertexHandleManager;
        
        class VertexCommand : public DocumentCommand {
        private:
            Model::BrushList m_brushes;
            Model::Snapshot* m_snapshot;
        protected:
            VertexCommand(CommandType type, const String& name, const Model::BrushList& brushes);
        public:
            virtual ~VertexCommand();
        protected:
            static void extractVertexMap(const Model::VertexToBrushesMap& vertices, Model::BrushList& brushes, Model::BrushVerticesMap& brushVertices, Vec3::List& vertexPositions);
            static void extractEdgeMap(const Model::VertexToEdgesMap& edges, Model::BrushList& brushes, Model::BrushEdgesMap& brushEdges, Edge3::List& edgePositions);
            static void extractFaceMap(const Model::VertexToFacesMap& faces, Model::BrushList& brushes, Model::BrushFacesMap& brushFaces, Polygon3::List& facePositions);
        private:
            bool doPerformDo(MapDocumentCommandFacade* document);
            bool doPerformUndo(MapDocumentCommandFacade* document);
            bool doIsRepeatable(MapDocumentCommandFacade* document) const;
        private:
            void takeSnapshot();
            void deleteSnapshot();
        private:
            virtual bool doCanDoVertexOperation(const MapDocument* document) const = 0;
            virtual bool doVertexOperation(MapDocumentCommandFacade* document) = 0;
        public:
            void removeBrushes(VertexHandleManager& manager);
            void addBrushes(VertexHandleManager& manager);
            void selectNewHandlePositions(VertexHandleManager& manager);
            void selectOldHandlePositions(VertexHandleManager& manager);
        private:
            virtual void doSelectNewHandlePositions(VertexHandleManager& manager, const Model::BrushList& brushes) = 0;
            virtual void doSelectOldHandlePositions(VertexHandleManager& manager, const Model::BrushList& brushes) = 0;
        };
    }
}

#endif /* defined(TrenchBroom_VertexCommand) */
