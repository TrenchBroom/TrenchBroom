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

#include "Model/Brush.h"
#include "Model/BrushFace.h"
#include "Model/BrushGeometry.h"
#include "Model/Snapshot.h"
#include "View/MapDocumentCommandFacade.h"
#include "View/VertexTool.h"

namespace TrenchBroom {
    namespace View {
        VertexCommand::VertexCommand(const CommandType type, const String& name, const Model::BrushList& brushes) :
        DocumentCommand(type, name),
        m_brushes(brushes),
        m_snapshot(nullptr) {}

        VertexCommand::~VertexCommand() {
            if (m_snapshot != nullptr)
                deleteSnapshot();
        }
        
        void VertexCommand::extractVertexMap(const Model::VertexToBrushesMap& vertices, Model::BrushList& brushes, Model::BrushVerticesMap& brushVertices, Vec3::List& vertexPositions) {
            extract(vertices, brushes, brushVertices, vertexPositions);
        }

        void VertexCommand::extractEdgeMap(const Model::EdgeToBrushesMap& edges, Model::BrushList& brushes, Model::BrushEdgesMap& brushEdges, Edge3::List& edgePositions) {
            extract(edges, brushes, brushEdges, edgePositions);
        }

        void VertexCommand::extractFaceMap(const Model::FaceToBrushesMap& faces, Model::BrushList& brushes, Model::BrushFacesMap& brushFaces, Polygon3::List& facePositions) {
            extract(faces, brushes, brushFaces, facePositions);
        }

        void VertexCommand::extractEdgeMap(const Model::VertexToEdgesMap& edges, Model::BrushList& brushes, Model::BrushEdgesMap& brushEdges, Edge3::List& edgePositions) {
            
            for (const auto& entry : edges) {
                const Model::BrushEdgeSet& mappedEdges = entry.second;
                for (Model::BrushEdge* edge : mappedEdges) {
                    Model::Brush* brush = edge->firstFace()->payload()->brush();
                    const Edge3 edgePosition(edge->firstVertex()->position(), edge->secondVertex()->position());
                    
                    const auto result = brushEdges.insert(std::make_pair(brush, Edge3::List()));
                    if (result.second)
                        brushes.push_back(brush);
                    result.first->second.push_back(edgePosition);
                    edgePositions.push_back(edgePosition);
                }
            }
            
            assert(!brushes.empty());
            assert(brushes.size() == brushEdges.size());
        }

        void VertexCommand::extractFaceMap(const Model::VertexToFacesMap& faces, Model::BrushList& brushes, Model::BrushFacesMap& brushFaces, Polygon3::List& facePositions) {

            for (const auto& entry : faces) {
                const Model::BrushFaceSet& mappedFaces = entry.second;
                for (Model::BrushFace* face : mappedFaces) {
                    Model::Brush* brush = face->brush();
                    const Polygon3 facePosition(Vec3::asList(face->vertices().begin(), face->vertices().end(), Model::BrushGeometry::GetVertexPosition()));
                    
                    const auto result = brushFaces.insert(std::make_pair(brush, Polygon3::List()));
                    if (result.second)
                        brushes.push_back(brush);
                    result.first->second.push_back(facePosition);
                    facePositions.push_back(facePosition);
                }
            }
            
            VectorUtils::sort(facePositions);
            
            assert(!brushes.empty());
            assert(brushes.size() == brushFaces.size());
        }

        Model::BrushVerticesMap VertexCommand::brushVertexMap(const Model::BrushEdgesMap& edges) {
            Model::BrushVerticesMap result;
            for (const auto& entry : edges) {
                Model::Brush* brush = entry.first;
                const Edge3::List& edgeList = entry.second;
                
                Vec3::List vertices = Edge3::asVertexList(edgeList);
                VectorUtils::sortAndRemoveDuplicates(vertices);
                result.insert(std::make_pair(brush, vertices));
            }
            return result;
        }
        
        Model::BrushVerticesMap VertexCommand::brushVertexMap(const Model::BrushFacesMap& faces) {
            Model::BrushVerticesMap result;
            for (const auto& entry : faces) {
                Model::Brush* brush = entry.first;
                const Polygon3::List& faceList = entry.second;
                
                Vec3::List vertices = Polygon3::asVertexList(faceList);
                VectorUtils::sortAndRemoveDuplicates(vertices);
                result.insert(std::make_pair(brush, vertices));
            }
            return result;
        }

        bool VertexCommand::doPerformDo(MapDocumentCommandFacade* document) {
            if (m_snapshot != nullptr) {
                return doPerformUndo(document);
            } else {
                if (!doCanDoVertexOperation(document))
                    return false;

                takeSnapshot();
                return doVertexOperation(document);
            }
        }
        
        bool VertexCommand::doPerformUndo(MapDocumentCommandFacade* document) {
            ensure(m_snapshot != nullptr, "snapshot is null");

            Model::Snapshot* snapshot = nullptr;
            using std::swap; swap(m_snapshot, snapshot);
            takeSnapshot();

            document->restoreSnapshot(snapshot);

            delete snapshot;
            return true;
        }

        bool VertexCommand::doIsRepeatable(MapDocumentCommandFacade* document) const {
            return false;
        }

        void VertexCommand::takeSnapshot() {
            assert(m_snapshot == nullptr);
            m_snapshot = new Model::Snapshot(std::begin(m_brushes), std::end(m_brushes));
        }
        
        void VertexCommand::deleteSnapshot() {
            ensure(m_snapshot != nullptr, "snapshot is null");
            delete m_snapshot;
            m_snapshot = nullptr;
        }

        void VertexCommand::removeHandles(VertexHandleManagerBase& manager) {
            manager.removeHandles(std::begin(m_brushes), std::end(m_brushes));
        }
        
        void VertexCommand::addHandles(VertexHandleManagerBase& manager) {
            manager.addHandles(std::begin(m_brushes), std::end(m_brushes));
        }
        
        void VertexCommand::selectNewHandlePositions(VertexHandleManagerBaseT<Vec3>& manager) const {
            doSelectNewHandlePositions(manager);
        }
        
        void VertexCommand::selectOldHandlePositions(VertexHandleManagerBaseT<Vec3>& manager) const {
            doSelectOldHandlePositions(manager);
        }

        void VertexCommand::selectNewHandlePositions(VertexHandleManagerBaseT<Edge3>& manager) const {
            doSelectNewHandlePositions(manager);
        }
        
        void VertexCommand::selectOldHandlePositions(VertexHandleManagerBaseT<Edge3>& manager) const {
            doSelectOldHandlePositions(manager);
        }
        
        void VertexCommand::selectNewHandlePositions(VertexHandleManagerBaseT<Polygon3>& manager) const {
            doSelectNewHandlePositions(manager);
        }
        
        void VertexCommand::selectOldHandlePositions(VertexHandleManagerBaseT<Polygon3>& manager) const {
            doSelectOldHandlePositions(manager);
        }

        void VertexCommand::doSelectNewHandlePositions(VertexHandleManagerBaseT<Vec3>& manager) const {}
        void VertexCommand::doSelectOldHandlePositions(VertexHandleManagerBaseT<Vec3>& manager) const {}
        void VertexCommand::doSelectNewHandlePositions(VertexHandleManagerBaseT<Edge3>& manager) const {}
        void VertexCommand::doSelectOldHandlePositions(VertexHandleManagerBaseT<Edge3>& manager) const {}
        void VertexCommand::doSelectNewHandlePositions(VertexHandleManagerBaseT<Polygon3>& manager) const {}
        void VertexCommand::doSelectOldHandlePositions(VertexHandleManagerBaseT<Polygon3>& manager) const {}
    }
}
