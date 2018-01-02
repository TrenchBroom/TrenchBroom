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

#ifndef TrenchBroom_VertexCommand
#define TrenchBroom_VertexCommand

#include "Model/ModelTypes.h"
#include "View/DocumentCommand.h"
#include "View/VertexHandleManager.h"

namespace TrenchBroom {
    namespace Model {
        class Snapshot;
    }
    
    namespace View {
        class VertexTool;
        
        class VertexCommand : public DocumentCommand {
        private:
            Model::BrushList m_brushes;
            Model::Snapshot* m_snapshot;
        protected:
            VertexCommand(CommandType type, const String& name, const Model::BrushList& brushes);
        public:
            virtual ~VertexCommand();
        protected:
            template <typename H, typename C>
            static void extract(const std::map<H, Model::BrushSet, C>& handleToBrushes, Model::BrushList& brushes, std::map<Model::Brush*, std::vector<H>>& brushToHandles, std::vector<H>& handles) {
                
                for (const auto& entry : handleToBrushes) {
                    const H& handle = entry.first;
                    const Model::BrushSet& mappedBrushes = entry.second;
                    for (Model::Brush* brush : mappedBrushes) {
                        const auto result = brushToHandles.insert(std::make_pair(brush, std::vector<H>()));
                        if (result.second)
                            brushes.push_back(brush);
                        result.first->second.push_back(handle);
                    }
                    handles.push_back(handle);
                }
            }
            
            static void extractVertexMap(const Model::VertexToBrushesMap& vertices, Model::BrushList& brushes, Model::BrushVerticesMap& brushVertices, Vec3::List& vertexPositions);
            static void extractEdgeMap(const Model::EdgeToBrushesMap& edges, Model::BrushList& brushes, Model::BrushEdgesMap& brushEdges, Edge3::List& edgePositions);
            static void extractFaceMap(const Model::FaceToBrushesMap& faces, Model::BrushList& brushes, Model::BrushFacesMap& brushFaces, Polygon3::List& facePositions);

            // TODO 1720: Remove these methods if possible.
            static void extractEdgeMap(const Model::VertexToEdgesMap& edges, Model::BrushList& brushes, Model::BrushEdgesMap& brushEdges, Edge3::List& edgePositions);
            static void extractFaceMap(const Model::VertexToFacesMap& faces, Model::BrushList& brushes, Model::BrushFacesMap& brushFaces, Polygon3::List& facePositions);
            
            static Model::BrushVerticesMap brushVertexMap(const Model::BrushEdgesMap& edges);
            static Model::BrushVerticesMap brushVertexMap(const Model::BrushFacesMap& faces);
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
            void removeHandles(VertexHandleManagerBase& manager);
            void addHandles(VertexHandleManagerBase& manager);
        public:
            void selectNewHandlePositions(VertexHandleManagerBaseT<Vec3>& manager) const;
            void selectOldHandlePositions(VertexHandleManagerBaseT<Vec3>& manager) const;
            void selectNewHandlePositions(VertexHandleManagerBaseT<Edge3>& manager) const;
            void selectOldHandlePositions(VertexHandleManagerBaseT<Edge3>& manager) const;
            void selectNewHandlePositions(VertexHandleManagerBaseT<Polygon3>& manager) const;
            void selectOldHandlePositions(VertexHandleManagerBaseT<Polygon3>& manager) const;
        private:
            virtual void doSelectNewHandlePositions(VertexHandleManagerBaseT<Vec3>& manager) const;
            virtual void doSelectOldHandlePositions(VertexHandleManagerBaseT<Vec3>& manager) const;
            virtual void doSelectNewHandlePositions(VertexHandleManagerBaseT<Edge3>& manager) const;
            virtual void doSelectOldHandlePositions(VertexHandleManagerBaseT<Edge3>& manager) const;
            virtual void doSelectNewHandlePositions(VertexHandleManagerBaseT<Polygon3>& manager) const;
            virtual void doSelectOldHandlePositions(VertexHandleManagerBaseT<Polygon3>& manager) const;
        };
    }
}

#endif /* defined(TrenchBroom_VertexCommand) */
