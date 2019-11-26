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

#include <vecmath/vec.h>

#include <map>
#include <memory>
#include <set>
#include <vector>

namespace TrenchBroom {
    namespace Model {
        class Snapshot;
    }

    namespace View {
        class MapDocument;
        class VertexTool;

        class VertexCommand : public DocumentCommand {
        protected:
            using VertexToBrushesMap = std::map<vm::vec3, std::set<Model::Brush*>>;
            using EdgeToBrushesMap = std::map<vm::segment3, std::set<Model::Brush*>>;
            using FaceToBrushesMap = std::map<vm::polygon3, std::set<Model::Brush*>>;
            using VertexToFacesMap = std::map<vm::vec3, std::set<Model::BrushFace*>>;
            using BrushVerticesMap = std::map<Model::Brush*, std::vector<vm::vec3>>;
            using BrushEdgesMap = std::map<Model::Brush*, std::vector<vm::segment3>>;
            using BrushFacesMap = std::map<Model::Brush*, std::vector<vm::polygon3>>;
        private:
            std::vector<Model::Brush*> m_brushes;
            std::unique_ptr<Model::Snapshot> m_snapshot;
        protected:
            VertexCommand(CommandType type, const String& name, const std::vector<Model::Brush*>& brushes);
        public:
            ~VertexCommand() override;
        protected:
            template <typename H, typename C>
            static void extract(const std::map<H, std::set<Model::Brush*>, C>& handleToBrushes, std::vector<Model::Brush*>& brushes, std::map<Model::Brush*, std::vector<H>>& brushToHandles, std::vector<H>& handles) {

                for (const auto& entry : handleToBrushes) {
                    const H& handle = entry.first;
                    const std::set<Model::Brush*>& mappedBrushes = entry.second;
                    for (Model::Brush* brush : mappedBrushes) {
                        const auto result = brushToHandles.insert(std::make_pair(brush, std::vector<H>()));
                        if (result.second)
                            brushes.push_back(brush);
                        result.first->second.push_back(handle);
                    }
                    handles.push_back(handle);
                }
            }

            static void extractVertexMap(const VertexToBrushesMap& vertices, std::vector<Model::Brush*>& brushes, BrushVerticesMap& brushVertices, std::vector<vm::vec3>& vertexPositions);
            static void extractEdgeMap(const EdgeToBrushesMap& edges, std::vector<Model::Brush*>& brushes, BrushEdgesMap& brushEdges, std::vector<vm::segment3>& edgePositions);
            static void extractFaceMap(const FaceToBrushesMap& faces, std::vector<Model::Brush*>& brushes, BrushFacesMap& brushFaces, std::vector<vm::polygon3>& facePositions);

            using BrushEdgeSet = std::set<Model::BrushEdge*>;
            using VertexToEdgesMap = std::map<vm::vec3, BrushEdgeSet>;
            static void extractEdgeMap(const VertexToEdgesMap& edges, std::vector<Model::Brush*>& brushes, BrushEdgesMap& brushEdges, std::vector<vm::segment3>& edgePositions);
            static void extractFaceMap(const VertexToFacesMap& faces, std::vector<Model::Brush*>& brushes, BrushFacesMap& brushFaces, std::vector<vm::polygon3>& facePositions);

            static BrushVerticesMap brushVertexMap(const BrushEdgesMap& edges);
            static BrushVerticesMap brushVertexMap(const BrushFacesMap& faces);
        private:
            bool doPerformDo(MapDocumentCommandFacade* document) override;
            bool doPerformUndo(MapDocumentCommandFacade* document) override;
            void restoreAndTakeNewSnapshot(MapDocumentCommandFacade* document);
            bool doIsRepeatable(MapDocumentCommandFacade* document) const override;
        private:
            void takeSnapshot();
            void deleteSnapshot();
        protected:
            bool canCollateWith(const VertexCommand& other) const;
        private:
            virtual bool doCanDoVertexOperation(const MapDocument* document) const = 0;
            virtual bool doVertexOperation(MapDocumentCommandFacade* document) = 0;
        public:
            void removeHandles(VertexHandleManagerBase& manager);
            void addHandles(VertexHandleManagerBase& manager);
        public:
            void selectNewHandlePositions(VertexHandleManagerBaseT<vm::vec3>& manager) const;
            void selectOldHandlePositions(VertexHandleManagerBaseT<vm::vec3>& manager) const;
            void selectNewHandlePositions(VertexHandleManagerBaseT<vm::segment3>& manager) const;
            void selectOldHandlePositions(VertexHandleManagerBaseT<vm::segment3>& manager) const;
            void selectNewHandlePositions(VertexHandleManagerBaseT<vm::polygon3>& manager) const;
            void selectOldHandlePositions(VertexHandleManagerBaseT<vm::polygon3>& manager) const;
        private:
            virtual void doSelectNewHandlePositions(VertexHandleManagerBaseT<vm::vec3>& manager) const;
            virtual void doSelectOldHandlePositions(VertexHandleManagerBaseT<vm::vec3>& manager) const;
            virtual void doSelectNewHandlePositions(VertexHandleManagerBaseT<vm::segment3>& manager) const;
            virtual void doSelectOldHandlePositions(VertexHandleManagerBaseT<vm::segment3>& manager) const;
            virtual void doSelectNewHandlePositions(VertexHandleManagerBaseT<vm::polygon3>& manager) const;
            virtual void doSelectOldHandlePositions(VertexHandleManagerBaseT<vm::polygon3>& manager) const;
        };
    }
}

#endif /* defined(TrenchBroom_VertexCommand) */
