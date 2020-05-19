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

#include "VertexCommand.h"

#include "Model/BrushNode.h"
#include "Model/BrushFace.h"
#include "Model/BrushGeometry.h"
#include "Model/Snapshot.h"
#include "View/MapDocumentCommandFacade.h"
#include "View/VertexTool.h"

#include <kdl/vector_utils.h>

#include <vecmath/polygon.h>

#include <iterator>
#include <set>

namespace TrenchBroom {
    namespace View {
        VertexCommand::VertexCommand(const CommandType type, const std::string& name, const std::vector<Model::BrushNode*>& brushes) :
        DocumentCommand(type, name),
        m_brushes(brushes) {}

        VertexCommand::~VertexCommand() = default;

        void VertexCommand::extractVertexMap(const VertexToBrushesMap& vertices, std::vector<Model::BrushNode*>& brushes, BrushVerticesMap& brushVertices, std::vector<vm::vec3>& vertexPositions) {
            extract(vertices, brushes, brushVertices, vertexPositions);
        }

        void VertexCommand::extractEdgeMap(const EdgeToBrushesMap& edges, std::vector<Model::BrushNode*>& brushes, BrushEdgesMap& brushEdges, std::vector<vm::segment3>& edgePositions) {
            extract(edges, brushes, brushEdges, edgePositions);
        }

        void VertexCommand::extractFaceMap(const FaceToBrushesMap& faces, std::vector<Model::BrushNode*>& brushes, BrushFacesMap& brushFaces, std::vector<vm::polygon3>& facePositions) {
            extract(faces, brushes, brushFaces, facePositions);
        }

        void VertexCommand::extractEdgeMap(const VertexToEdgesMap& edges, std::vector<Model::BrushNode*>& brushes, BrushEdgesMap& brushEdges, std::vector<vm::segment3>& edgePositions) {

            for (const auto& entry : edges) {
                const BrushEdgeSet& mappedEdges = entry.second;
                for (Model::BrushEdge* edge : mappedEdges) {
                    Model::BrushNode* brush = edge->firstFace()->payload()->brush();
                    const vm::segment3 edgePosition(edge->firstVertex()->position(), edge->secondVertex()->position());

                    const auto result = brushEdges.insert(std::make_pair(brush, std::vector<vm::segment3>()));
                    if (result.second) {
                        brushes.push_back(brush);
                    }
                    result.first->second.push_back(edgePosition);
                    edgePositions.push_back(edgePosition);
                }
            }

            assert(!brushes.empty());
            assert(brushes.size() == brushEdges.size());
        }

        void VertexCommand::extractFaceMap(const VertexToFacesMap& faces, std::vector<Model::BrushNode*>& brushes, BrushFacesMap& brushFaces, std::vector<vm::polygon3>& facePositions) {
            for (const auto& entry : faces) {
                const std::set<Model::BrushFace*>& mappedFaces = entry.second;
                for (Model::BrushFace* face : mappedFaces) {
                    Model::BrushNode* brush = face->brush();
                    const auto result = brushFaces.insert(std::make_pair(brush, std::vector<vm::polygon3>()));
                    if (result.second) {
                        brushes.push_back(brush);
                    }

                    const vm::polygon3 facePosition = face->polygon();
                    result.first->second.push_back(facePosition);
                    facePositions.push_back(facePosition);
                }
            }

            kdl::vec_sort(facePositions);

            assert(!brushes.empty());
            assert(brushes.size() == brushFaces.size());
        }

        VertexCommand::BrushVerticesMap VertexCommand::brushVertexMap(const BrushEdgesMap& edges) {
            BrushVerticesMap result;
            for (const auto& entry : edges) {
                Model::BrushNode* brush = entry.first;
                const std::vector<vm::segment3>& edgeList = entry.second;

                std::vector<vm::vec3> vertices;
                vertices.reserve(2 * edgeList.size());
                vm::segment3::get_vertices(std::begin(edgeList), std::end(edgeList), std::back_inserter(vertices));
                kdl::vec_sort_and_remove_duplicates(vertices);
                result.insert(std::make_pair(brush, vertices));
            }
            return result;
        }

        VertexCommand::BrushVerticesMap VertexCommand::brushVertexMap(const BrushFacesMap& faces) {
            BrushVerticesMap result;
            for (const auto& entry : faces) {
                Model::BrushNode* brush = entry.first;
                const std::vector<vm::polygon3>& faceList = entry.second;

                std::vector<vm::vec3> vertices;
                vm::polygon3::get_vertices(std::begin(faceList), std::end(faceList), std::back_inserter(vertices));
                kdl::vec_sort_and_remove_duplicates(vertices);
                result.insert(std::make_pair(brush, vertices));
            }
            return result;
        }

        std::unique_ptr<CommandResult> VertexCommand::doPerformDo(MapDocumentCommandFacade* document) {
            if (m_snapshot != nullptr) {
                restoreAndTakeNewSnapshot(document);
                return doCreateCommandResult(true);
            } else {
                if (!doCanDoVertexOperation(document)) {
                    return doCreateCommandResult(false);
                }

                takeSnapshot();
                const auto success = doVertexOperation(document);
                return doCreateCommandResult(success);
            }
        }

        std::unique_ptr<CommandResult> VertexCommand::doPerformUndo(MapDocumentCommandFacade* document) {
            restoreAndTakeNewSnapshot(document);
            return std::make_unique<CommandResult>(true);
        }

        void VertexCommand::restoreAndTakeNewSnapshot(MapDocumentCommandFacade* document) {
            ensure(m_snapshot != nullptr, "snapshot is null");

            auto snapshot = std::move(m_snapshot);
            takeSnapshot();
            document->restoreSnapshot(snapshot.get());
        }

        bool VertexCommand::doIsRepeatable(MapDocumentCommandFacade*) const {
            return false;
        }

        void VertexCommand::takeSnapshot() {
            assert(m_snapshot == nullptr);
            m_snapshot = std::make_unique<Model::Snapshot>(std::begin(m_brushes), std::end(m_brushes));
        }

        void VertexCommand::deleteSnapshot() {
            ensure(m_snapshot != nullptr, "snapshot is null");
            m_snapshot.reset();
        }

        bool VertexCommand::canCollateWith(const VertexCommand& other) const {
            return m_brushes == other.m_brushes;
        }

        std::unique_ptr<CommandResult> VertexCommand::doCreateCommandResult(const bool success) {
            return std::make_unique<CommandResult>(success);
        }

        void VertexCommand::removeHandles(VertexHandleManagerBase& manager) {
            manager.removeHandles(std::begin(m_brushes), std::end(m_brushes));
        }

        void VertexCommand::addHandles(VertexHandleManagerBase& manager) {
            manager.addHandles(std::begin(m_brushes), std::end(m_brushes));
        }

        void VertexCommand::selectNewHandlePositions(VertexHandleManagerBaseT<vm::vec3>& manager) const {
            doSelectNewHandlePositions(manager);
        }

        void VertexCommand::selectOldHandlePositions(VertexHandleManagerBaseT<vm::vec3>& manager) const {
            doSelectOldHandlePositions(manager);
        }

        void VertexCommand::selectNewHandlePositions(VertexHandleManagerBaseT<vm::segment3>& manager) const {
            doSelectNewHandlePositions(manager);
        }

        void VertexCommand::selectOldHandlePositions(VertexHandleManagerBaseT<vm::segment3>& manager) const {
            doSelectOldHandlePositions(manager);
        }

        void VertexCommand::selectNewHandlePositions(VertexHandleManagerBaseT<vm::polygon3>& manager) const {
            doSelectNewHandlePositions(manager);
        }

        void VertexCommand::selectOldHandlePositions(VertexHandleManagerBaseT<vm::polygon3>& manager) const {
            doSelectOldHandlePositions(manager);
        }

        void VertexCommand::doSelectNewHandlePositions(VertexHandleManagerBaseT<vm::vec3>&) const {}
        void VertexCommand::doSelectOldHandlePositions(VertexHandleManagerBaseT<vm::vec3>&) const {}
        void VertexCommand::doSelectNewHandlePositions(VertexHandleManagerBaseT<vm::segment3>&) const {}
        void VertexCommand::doSelectOldHandlePositions(VertexHandleManagerBaseT<vm::segment3>&) const {}
        void VertexCommand::doSelectNewHandlePositions(VertexHandleManagerBaseT<vm::polygon3>&) const {}
        void VertexCommand::doSelectOldHandlePositions(VertexHandleManagerBaseT<vm::polygon3>&) const {}
    }
}
