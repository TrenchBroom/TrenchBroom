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

#include <memory>

namespace TrenchBroom {
    namespace Model {
        class Snapshot;
    }

    namespace View {
        class VertexTool;

        class VertexCommand : public DocumentCommand {
        private:
            Model::BrushList m_brushes;
            std::unique_ptr<Model::Snapshot> m_snapshot;
        protected:
            VertexCommand(CommandType type, const String& name, const Model::BrushList& brushes);
        public:
            ~VertexCommand() override;
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

            static void extractVertexMap(const Model::VertexToBrushesMap& vertices, Model::BrushList& brushes, Model::BrushVerticesMap& brushVertices, std::vector<vm::vec3>& vertexPositions);
            static void extractEdgeMap(const Model::EdgeToBrushesMap& edges, Model::BrushList& brushes, Model::BrushEdgesMap& brushEdges, std::vector<vm::segment3>& edgePositions);
            static void extractFaceMap(const Model::FaceToBrushesMap& faces, Model::BrushList& brushes, Model::BrushFacesMap& brushFaces, std::vector<vm::polygon3>& facePositions);

            // TODO 1720: Remove these methods if possible.
            static void extractEdgeMap(const Model::VertexToEdgesMap& edges, Model::BrushList& brushes, Model::BrushEdgesMap& brushEdges, std::vector<vm::segment3>& edgePositions);
            static void extractFaceMap(const Model::VertexToFacesMap& faces, Model::BrushList& brushes, Model::BrushFacesMap& brushFaces, std::vector<vm::polygon3>& facePositions);

            static Model::BrushVerticesMap brushVertexMap(const Model::BrushEdgesMap& edges);
            static Model::BrushVerticesMap brushVertexMap(const Model::BrushFacesMap& faces);
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
