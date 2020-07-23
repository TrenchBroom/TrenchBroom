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

#include "SnapBrushVerticesCommand.h"

#include "Model/BrushNode.h"
#include "View/MapDocumentCommandFacade.h"
#include "View/VertexHandleManager.h"

#include <vecmath/segment.h> // do not remove
#include <vecmath/polygon.h> // do not remove

namespace TrenchBroom {
    namespace View {
        // SnapBrushVerticesCommand

        const Command::CommandType SnapBrushVerticesCommand::Type = Command::freeType();

        std::unique_ptr<SnapBrushVerticesCommand> SnapBrushVerticesCommand::snap(const FloatType snapTo) {
            return std::make_unique<SnapBrushVerticesCommand>(snapTo);
        }

        SnapBrushVerticesCommand::SnapBrushVerticesCommand(const FloatType snapTo) :
        SnapshotCommand(Type, "Snap Brush Vertices"),
        m_snapTo(snapTo) {}

        std::unique_ptr<CommandResult> SnapBrushVerticesCommand::doPerformDo(MapDocumentCommandFacade* document) {
            const auto success = document->performSnapVertices(m_snapTo);
            return std::make_unique<CommandResult>(success);
        }

        bool SnapBrushVerticesCommand::doIsRepeatable(MapDocumentCommandFacade*) const {
            return false;
        }

        bool SnapBrushVerticesCommand::doCollateWith(UndoableCommand* command) {
            SnapBrushVerticesCommand* other = static_cast<SnapBrushVerticesCommand*>(command);
            return other->m_snapTo == m_snapTo;
        }

        // SnapSpecificBrushVerticesCommand

        const Command::CommandType SnapSpecificBrushVerticesCommand::Type = Command::freeType();

        std::unique_ptr<SnapSpecificBrushVerticesCommand> SnapSpecificBrushVerticesCommand::snap(const FloatType snapTo, const VertexToBrushesMap& vertices) {
            std::vector<Model::BrushNode*> brushes;
            BrushVerticesMap brushVertices;
            std::vector<vm::vec3> vertexPositions;
            extractVertexMap(vertices, brushes, brushVertices, vertexPositions);

            return std::make_unique<SnapSpecificBrushVerticesCommand>(snapTo, brushes, brushVertices, vertexPositions);
        }

        SnapSpecificBrushVerticesCommand::SnapSpecificBrushVerticesCommand(const FloatType snapTo, const std::vector<Model::BrushNode*>& brushes, const BrushVerticesMap& vertices, const std::vector<vm::vec3>& vertexPositions) :
        VertexCommand(Type, "Snap Brush Vertices", brushes),
        m_snapTo(snapTo),
        m_vertices(vertices),
        m_oldVertexPositions(vertexPositions) {}

        bool SnapSpecificBrushVerticesCommand::doCanDoVertexOperation(const MapDocument* document) const {
            return true;
        }

        bool SnapSpecificBrushVerticesCommand::doVertexOperation(MapDocumentCommandFacade* document) {
            m_newVertexPositions = document->performSnapVertices(m_vertices, m_snapTo);
            return true;
        }

        bool SnapSpecificBrushVerticesCommand::doCollateWith(UndoableCommand*) {
            return false;
        }

        void SnapSpecificBrushVerticesCommand::doSelectNewHandlePositions(VertexHandleManagerBaseT<vm::vec3>& manager) const {
            manager.select(std::begin(m_newVertexPositions), std::end(m_newVertexPositions));
        }

        void SnapSpecificBrushVerticesCommand::doSelectOldHandlePositions(VertexHandleManagerBaseT<vm::vec3>& manager) const {
            manager.select(std::begin(m_oldVertexPositions), std::end(m_oldVertexPositions));
        }

        // SnapSpecificBrushEdgesCommand

        const Command::CommandType SnapSpecificBrushEdgesCommand::Type = Command::freeType();

        std::unique_ptr<SnapSpecificBrushEdgesCommand> SnapSpecificBrushEdgesCommand::snap(const FloatType snapTo, const EdgeToBrushesMap& edges) {
            std::vector<Model::BrushNode*> brushes;
            BrushEdgesMap brushEdges;
            std::vector<vm::segment3> edgePositions;
            extractEdgeMap(edges, brushes, brushEdges, edgePositions);

            return std::make_unique<SnapSpecificBrushEdgesCommand>(snapTo, brushes, brushEdges, edgePositions);
        }

        SnapSpecificBrushEdgesCommand::SnapSpecificBrushEdgesCommand(const FloatType snapTo, const std::vector<Model::BrushNode*>& brushes, const BrushEdgesMap& edges, const std::vector<vm::segment3>& edgePositions) :
        VertexCommand(Type, "Snap Brush Vertices", brushes),
        m_snapTo(snapTo),
        m_edges(edges),
        m_oldEdgePositions(edgePositions) {}

        bool SnapSpecificBrushEdgesCommand::doCanDoVertexOperation(const MapDocument* document) const {
            return true;
        }

        bool SnapSpecificBrushEdgesCommand::doVertexOperation(MapDocumentCommandFacade* document) {
            m_newEdgePositions = document->performSnapEdges(m_edges, m_snapTo);
            return true;
        }

        bool SnapSpecificBrushEdgesCommand::doCollateWith(UndoableCommand*) {
            return false;
        }

        void SnapSpecificBrushEdgesCommand::doSelectNewHandlePositions(VertexHandleManagerBaseT<vm::segment3>& manager) const {
            manager.select(std::begin(m_newEdgePositions), std::end(m_newEdgePositions));
        }

        void SnapSpecificBrushEdgesCommand::doSelectOldHandlePositions(VertexHandleManagerBaseT<vm::segment3>& manager) const {
            manager.select(std::begin(m_oldEdgePositions), std::end(m_oldEdgePositions));
        }

        // SnapSpecificBrushFacesCommand

        const Command::CommandType SnapSpecificBrushFacesCommand::Type = Command::freeType();

        std::unique_ptr<SnapSpecificBrushFacesCommand> SnapSpecificBrushFacesCommand::snap(const FloatType snapTo, const FaceToBrushesMap& faces) {
            std::vector<Model::BrushNode*> brushes;
            BrushFacesMap brushFaces;
            std::vector<vm::polygon3> facePositions;
            extractFaceMap(faces, brushes, brushFaces, facePositions);

            return std::make_unique<SnapSpecificBrushFacesCommand>(snapTo, brushes, brushFaces, facePositions);
        }

        SnapSpecificBrushFacesCommand::SnapSpecificBrushFacesCommand(const FloatType snapTo, const std::vector<Model::BrushNode*>& brushes, const BrushFacesMap& faces, const std::vector<vm::polygon3>& facePositions) :
        VertexCommand(Type, "Snap Brush Vertices", brushes),
        m_snapTo(snapTo),
        m_faces(faces),
        m_oldFacePositions(facePositions) {}

        bool SnapSpecificBrushFacesCommand::doCanDoVertexOperation(const MapDocument* document) const {
            return true;
        }

        bool SnapSpecificBrushFacesCommand::doVertexOperation(MapDocumentCommandFacade* document) {
            m_newFacePositions = document->performSnapFaces(m_faces, m_snapTo);
            return true;
        }

        bool SnapSpecificBrushFacesCommand::doCollateWith(UndoableCommand*) {
            return false;
        }

        void SnapSpecificBrushFacesCommand::doSelectNewHandlePositions(VertexHandleManagerBaseT<vm::polygon3>& manager) const {
            manager.select(std::begin(m_newFacePositions), std::end(m_newFacePositions));
        }

        void SnapSpecificBrushFacesCommand::doSelectOldHandlePositions(VertexHandleManagerBaseT<vm::polygon3>& manager) const {
            manager.select(std::begin(m_oldFacePositions), std::end(m_oldFacePositions));
        }
    }
}
