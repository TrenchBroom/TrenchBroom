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

#include "BrushGeometry.h"

#include "CollectionUtils.h"
#include "Model/BrushEdge.h"
#include "Model/BrushFace.h"
#include "Model/BrushFaceGeometry.h"
#include "Model/BrushVertex.h"
#include "Model/IntersectBrushGeometryWithFace.h"
#include "Model/MoveBrushEdgesAlgorithm.h"
#include "Model/MoveBrushFacesAlgorithm.h"
#include "Model/MoveBrushVerticesAlgorithm.h"
#include "Model/SnapBrushVerticesAlgorithm.h"
#include "Model/SplitBrushEdgeAlgorithm.h"
#include "Model/SplitBrushFaceAlgorithm.h"

#include <map>

namespace TrenchBroom {
    namespace Model {
        void BrushAlgorithmResult::append(const BrushAlgorithmResult& other) {
            addedFaces = VectorUtils::difference(addedFaces, other.droppedFaces);
            VectorUtils::append(addedFaces, other.addedFaces);
            
            droppedFaces = VectorUtils::difference(droppedFaces, other.addedFaces);
            VectorUtils::append(droppedFaces, other.droppedFaces);
        }
        
        BrushAlgorithmResult::BrushAlgorithmResult(const BrushFaceList& i_addedFaces, const BrushFaceList& i_droppedFaces) :
        addedFaces(i_addedFaces),
        droppedFaces(i_droppedFaces) {}

        AddFaceResult::AddFaceResult(const Code i_resultCode, const BrushFaceList& i_addedFaces, const BrushFaceList& i_droppedFaces) :
        BrushAlgorithmResult(i_addedFaces, i_droppedFaces),
        resultCode(i_resultCode) {}
        
        MoveVerticesResult::MoveVerticesResult(const Vec3::List& i_newVertexPositions, const BrushFaceList& i_addedFaces, const BrushFaceList& i_droppedFaces) :
        BrushAlgorithmResult(i_addedFaces, i_droppedFaces),
        newVertexPositions(i_newVertexPositions) {}
        
        MoveVerticesResult::MoveVerticesResult(Vec3::List& i_newVertexPositions, const BrushFaceList& i_addedFaces, const BrushFaceList& i_droppedFaces) :
        BrushAlgorithmResult(i_addedFaces, i_droppedFaces) {
            using std::swap;
            swap(newVertexPositions, i_newVertexPositions);
        }

        MoveEdgesResult::MoveEdgesResult(const Edge3::List& i_newEdgePositions, const BrushFaceList& i_addedFaces, const BrushFaceList& i_droppedFaces) :
        BrushAlgorithmResult(i_addedFaces, i_droppedFaces),
        newEdgePositions(i_newEdgePositions) {}

        MoveEdgesResult::MoveEdgesResult(Edge3::List& i_newEdgePositions, const BrushFaceList& i_addedFaces, const BrushFaceList& i_droppedFaces) :
        BrushAlgorithmResult(i_addedFaces, i_droppedFaces) {
            using std::swap;
            swap(newEdgePositions, i_newEdgePositions);
        }

        MoveFacesResult::MoveFacesResult(const Polygon3::List& i_newFacePositions, const BrushFaceList& i_addedFaces, const BrushFaceList& i_droppedFaces) :
        BrushAlgorithmResult(i_addedFaces, i_droppedFaces),
        newFacePositions(i_newFacePositions) {}
        
        MoveFacesResult::MoveFacesResult(Polygon3::List& i_newFacePositions, const BrushFaceList& i_addedFaces, const BrushFaceList& i_droppedFaces) :
        BrushAlgorithmResult(i_addedFaces, i_droppedFaces) {
            using std::swap;
            swap(newFacePositions, i_newFacePositions);
        }

        SplitResult::SplitResult(const Vec3& i_newVertexPosition, const BrushFaceList& i_addedFaces, const BrushFaceList& i_droppedFaces) :
        BrushAlgorithmResult(i_addedFaces, i_droppedFaces),
        newVertexPosition(i_newVertexPosition) {}

        SnapVerticesResult::SnapVerticesResult(const Vec3::List& i_newVertexPositions, const BrushFaceList& i_addedFaces, const BrushFaceList& i_droppedFaces) :
        BrushAlgorithmResult(i_addedFaces, i_droppedFaces),
        newVertexPositions(i_newVertexPositions) {}

        BrushGeometry::BrushGeometry(const BrushGeometry& original) {
            copy(original);
        }

        BrushGeometry::BrushGeometry(const BBox3& worldBounds) :
        bounds(worldBounds) {
            initializeWithBounds(worldBounds.expanded(1.0));
        }

        BrushGeometry::~BrushGeometry() {
            VectorUtils::clearAndDelete(sides);
            VectorUtils::clearAndDelete(edges);
            VectorUtils::clearAndDelete(vertices);
        }

        BrushFaceGeometryList BrushGeometry::incidentSides(const BrushVertex* vertex) const {
            return vertex->incidentSides(edges);
        }

        AddFaceResult BrushGeometry::addFaces(const BrushFaceList& faces) {
            AddFaceResult totalResult(AddFaceResult::Code_BrushSplit);
            
            BrushFaceList::const_iterator it, end;
            for (it = faces.begin(), end = faces.end(); it != end; ++it) {
                BrushFace* face = *it;
                const AddFaceResult result = addFace(face);
                if (result.resultCode == AddFaceResult::Code_BrushNull)
                    return AddFaceResult(AddFaceResult::Code_BrushNull);
                totalResult.append(result);
            }
            
            updateBounds();
            return totalResult;
        }
        
        bool BrushGeometry::canMoveVertices(const BBox3& worldBounds, const Vec3::List& vertexPositions, const Vec3& delta) {
            MoveBrushVerticesAlgorithm algorithm(*this, worldBounds, vertexPositions, delta);
            return algorithm.canExecute();
        }
        
        MoveVerticesResult BrushGeometry::moveVertices(const BBox3& worldBounds, const Vec3::List& vertexPositions, const Vec3& delta) {
            MoveBrushVerticesAlgorithm algorithm(*this, worldBounds, vertexPositions, delta);
            return algorithm.execute();
        }

        bool BrushGeometry::canMoveEdges(const BBox3& worldBounds, const Edge3::List& edgePositions, const Vec3& delta) {
            MoveBrushEdgesAlgorithm algorithm(*this, worldBounds, edgePositions, delta);
            return algorithm.canExecute();
        }
        
        MoveEdgesResult BrushGeometry::moveEdges(const BBox3& worldBounds, const Edge3::List& edgePositions, const Vec3& delta) {
            MoveBrushEdgesAlgorithm algorithm(*this, worldBounds, edgePositions, delta);
            return algorithm.execute();
        }

        bool BrushGeometry::canMoveFaces(const BBox3& worldBounds, const Polygon3::List& facePositions, const Vec3& delta) {
            MoveBrushFacesAlgorithm algorithm(*this, worldBounds, facePositions, delta);
            return algorithm.canExecute();
        }
        
        MoveFacesResult BrushGeometry::moveFaces(const BBox3& worldBounds, const Polygon3::List& facePositions, const Vec3& delta) {
            MoveBrushFacesAlgorithm algorithm(*this, worldBounds, facePositions, delta);
            return algorithm.execute();
        }

        bool BrushGeometry::canSplitEdge(const BBox3& worldBounds, const Edge3& edgePosition, const Vec3& delta) {
            SplitBrushEdgeAlgorithm algorithm(*this, worldBounds, edgePosition, delta);
            return algorithm.canExecute();
        }
        
        SplitResult BrushGeometry::splitEdge(const BBox3& worldBounds, const Edge3& edgePosition, const Vec3& delta) {
            SplitBrushEdgeAlgorithm algorithm(*this, worldBounds, edgePosition, delta);
            return algorithm.execute();
        }

        bool BrushGeometry::canSplitFace(const BBox3& worldBounds, const Polygon3& facePosition, const Vec3& delta) {
            SplitBrushFaceAlgorithm algorithm(*this, worldBounds, facePosition, delta);
            return algorithm.canExecute();
        }
        
        SplitResult BrushGeometry::splitFace(const BBox3& worldBounds, const Polygon3& facePosition, const Vec3& delta) {
            SplitBrushFaceAlgorithm algorithm(*this, worldBounds, facePosition, delta);
            return algorithm.execute();
        }

        SnapVerticesResult BrushGeometry::snapVertices(const Vec3::List& vertexPositions, const size_t snapTo) {
            SnapBrushVerticesAlgorithm algorithm(*this, vertexPositions, snapTo);
            return algorithm.execute();
        }

        void BrushGeometry::restoreFaceGeometries() {
            BrushFaceGeometryList::iterator it, end;
            for (it = sides.begin(), end = sides.end(); it != end; ++it) {
                BrushFaceGeometry* side = *it;
                if (side->face != NULL)
                    side->face->setSide(side);
            }
        }

        void BrushGeometry::updateBounds() {
            assert(vertices.size() > 0);
            bounds = BBox3(vertices[0]->position, vertices[0]->position);
            for (size_t i = 1; i < vertices.size(); ++i)
                bounds.mergeWith(vertices[i]->position);
        }

        bool BrushGeometry::isClosed() const {
            for (size_t i = 0; i < sides.size(); ++i)
                if (sides[i]->face == NULL)
                    return false;
            return true;
        }

        bool BrushGeometry::sanityCheck() const {
            // check Euler characteristic http://en.wikipedia.org/wiki/Euler_characteristic
            size_t sideCount = 0;
            for (size_t i = 0; i < sides.size(); ++i)
                if (sides[i]->face != NULL)
                    ++sideCount;
            if (vertices.size() - edges.size() + sideCount != 2) {
                fprintf(stdout, "failed Euler check\n");
                return false;
            }
            
			std::vector<size_t> vVisits;
			vVisits.resize(vertices.size());
            for (size_t i = 0; i < vertices.size(); ++i)
                vVisits[i] = 0;
            
			std::vector<size_t> eVisits;
			eVisits.resize(edges.size());
            for (size_t i = 0; i < edges.size(); ++i)
                eVisits[i] = 0;
            
            for (size_t i = 0; i < sides.size(); ++i) {
                const BrushFaceGeometry* side = sides[i];
                
                if (side->edges.size() != side->vertices.size()) {
                    fprintf(stdout, "side with index %zu has differing vertex and edge counts\n", i);
                    return false;
                }
                
                size_t index = 0;
                for (size_t j = 0; j < side->edges.size(); ++j) {
                    BrushEdge* edge = side->edges[j];
                    if (edge->left != side && edge->right != side) {
                        fprintf(stdout, "edge with index %zu of side with index %zu does not actually belong to it\n", j, i);
                        return false;
                    }
                    
                    index = VectorUtils::indexOf(edges, edge);
                    if (index == edges.size()) {
                        fprintf(stdout, "edge with index %zu of side with index %zu is missing from vertex data\n", j, i);
                        return false;
                    }
                    eVisits[index]++;
                    
                    BrushVertex* vertex = edge->startVertex(side);
                    if (side->vertices[j] != vertex) {
                        fprintf(stdout, "start vertex of edge with index %zu of side with index %zu is not at position %zu in the side's vertex list\n", j, i, j);
                        return false;
                    }
                    
                    index = VectorUtils::indexOf(vertices, vertex);
                    if (index == vertices.size()) {
                        fprintf(stdout, "start vertex of edge with index %zu of side with index %zu is missing from vertex data\n", j, i);
                        return false;
                    }
                    vVisits[index]++;
                }
            }
            
            for (size_t i = 0; i < vertices.size(); ++i) {
                if (vVisits[i] == 0) {
                    fprintf(stdout, "vertex with index %zu does not belong to any side\n", i);
                    return false;
                }
                
                for (size_t j = i + 1; j < vertices.size(); ++j)
                    if (vertices[i]->position.equals(vertices[j]->position)) {
                        fprintf(stdout, "vertex with index %zu is identical to vertex with index %zu\n", i, j);
                        return false;
                    }
            }
            
            for (size_t i = 0; i < edges.size(); ++i) {
                if (eVisits[i] != 2) {
                    fprintf(stdout, "edge with index %zu was visited %zu times, should have been 2\n", i, eVisits[i]);
                    return false;
                }
                
                if (edges[i]->start->position.equals(edges[i]->end->position)) {
                    fprintf(stdout, "edge with index %zu has identical vertices", i);
                    return false;
                }
                
                if (edges[i]->left == edges[i]->right) {
                    fprintf(stdout, "edge with index %zu has identical sides", i);
                    return false;
                }
                
                BrushEdge* edge1 = edges[i];
                for (size_t j = i + 1; j < edges.size(); ++j) {
                    BrushEdge* edge2 = edges[j];
                    if (edge1->hasPositions(edge2->start->position, edge2->end->position)) {
                        fprintf(stdout, "edge with index %zu is identical to edge with index %zu\n", i, j);
                        return false;
                    }
                }
            }
            
            return true;
        }

        void BrushGeometry::copy(const BrushGeometry& original) {
            typedef std::map<BrushVertex*, BrushVertex*> VertexMap;
            typedef std::map<BrushEdge*, BrushEdge*> EdgeMap;
            
            VertexMap vertexMap;
            EdgeMap edgeMap;
            
            VectorUtils::clearAndDelete(vertices);
            VectorUtils::clearAndDelete(edges);
            VectorUtils::clearAndDelete(sides);
            
            vertices.reserve(original.vertices.size());
            edges.reserve(original.edges.size());
            sides.reserve(original.sides.size());
            
            for (size_t i = 0; i < original.vertices.size(); ++i) {
                BrushVertex* originalVertex = original.vertices[i];
                BrushVertex* copyVertex = new BrushVertex(*originalVertex);
                vertexMap[originalVertex] = copyVertex;
                vertices.push_back(copyVertex);
            }
            
            for (size_t i = 0; i < original.edges.size(); ++i) {
                BrushEdge* originalEdge = original.edges[i];
                BrushEdge* copyEdge = new BrushEdge(*originalEdge);
                copyEdge->start = vertexMap[originalEdge->start];
                copyEdge->end = vertexMap[originalEdge->end];
                edgeMap[originalEdge] = copyEdge;
                edges.push_back(copyEdge);
            }
            
            for (size_t i = 0; i < original.sides.size(); ++i) {
                BrushFaceGeometry* originalSide = original.sides[i];
                BrushFaceGeometry* copySide = new BrushFaceGeometry(*originalSide);
                copySide->vertices.clear();
                copySide->edges.clear();
                
                for (size_t j = 0; j < originalSide->edges.size(); ++j) {
                    BrushEdge* originalEdge = originalSide->edges[j];
                    BrushEdge* copyEdge = edgeMap[originalEdge];
                    
                    if (originalEdge->left == originalSide)
                        copyEdge->left = copySide;
                    else
                        copyEdge->right = copySide;
                    copySide->edges.push_back(copyEdge);
                    copySide->vertices.push_back(copyEdge->startVertex(copySide));
                }
                
                sides.push_back(copySide);
            }
            
            bounds = original.bounds;
        }

        AddFaceResult BrushGeometry::addFace(BrushFace* face) {
            IntersectBrushGeometryWithFace algorithm(*this, face);
            const AddFaceResult::Code resultCode = algorithm.execute();
            switch (resultCode) {
                case AddFaceResult::Code_BrushNull:
                    break;
                case AddFaceResult::Code_FaceRedundant:
                    break;
                case AddFaceResult::Code_BrushSplit:
                    vertices = algorithm.vertices();
                    edges = algorithm.edges();
                    sides = algorithm.sides();
                    break;
            }
            
            return AddFaceResult(resultCode, algorithm.addedFaces(), algorithm.removedFaces());
        }

        void BrushGeometry::initializeWithBounds(const BBox3& bounds) {
            BrushVertex* v000 = new BrushVertex(bounds.vertex(BBox3::Corner_Min, BBox3::Corner_Min, BBox3::Corner_Min));
            BrushVertex* v001 = new BrushVertex(bounds.vertex(BBox3::Corner_Min, BBox3::Corner_Min, BBox3::Corner_Max));
            BrushVertex* v010 = new BrushVertex(bounds.vertex(BBox3::Corner_Min, BBox3::Corner_Max, BBox3::Corner_Min));
            BrushVertex* v011 = new BrushVertex(bounds.vertex(BBox3::Corner_Min, BBox3::Corner_Max, BBox3::Corner_Max));
            BrushVertex* v100 = new BrushVertex(bounds.vertex(BBox3::Corner_Max, BBox3::Corner_Min, BBox3::Corner_Min));
            BrushVertex* v101 = new BrushVertex(bounds.vertex(BBox3::Corner_Max, BBox3::Corner_Min, BBox3::Corner_Max));
            BrushVertex* v110 = new BrushVertex(bounds.vertex(BBox3::Corner_Max, BBox3::Corner_Max, BBox3::Corner_Min));
            BrushVertex* v111 = new BrushVertex(bounds.vertex(BBox3::Corner_Max, BBox3::Corner_Max, BBox3::Corner_Max));
            
            BrushEdge* v000v001 = new BrushEdge(v000, v001);
            BrushEdge* v001v101 = new BrushEdge(v001, v101);
            BrushEdge* v101v100 = new BrushEdge(v101, v100);
            BrushEdge* v100v000 = new BrushEdge(v100, v000);
            BrushEdge* v010v110 = new BrushEdge(v010, v110);
            BrushEdge* v110v111 = new BrushEdge(v110, v111);
            BrushEdge* v111v011 = new BrushEdge(v111, v011);
            BrushEdge* v011v010 = new BrushEdge(v011, v010);
            BrushEdge* v000v010 = new BrushEdge(v000, v010);
            BrushEdge* v011v001 = new BrushEdge(v011, v001);
            BrushEdge* v101v111 = new BrushEdge(v101, v111);
            BrushEdge* v110v100 = new BrushEdge(v110, v100);
            
            BrushFaceGeometry* top = new BrushFaceGeometry();
            BrushFaceGeometry* bottom = new BrushFaceGeometry();
            BrushFaceGeometry* front = new BrushFaceGeometry();
            BrushFaceGeometry* back = new BrushFaceGeometry();
            BrushFaceGeometry* left = new BrushFaceGeometry();
            BrushFaceGeometry* right = new BrushFaceGeometry();
            
            top->addBackwardEdge(v011v001);
            top->addBackwardEdge(v111v011);
            top->addBackwardEdge(v101v111);
            top->addBackwardEdge(v001v101);
            
            bottom->addBackwardEdge(v100v000);
            bottom->addBackwardEdge(v110v100);
            bottom->addBackwardEdge(v010v110);
            bottom->addBackwardEdge(v000v010);

            front->addForwardEdge(v000v001);
            front->addForwardEdge(v001v101);
            front->addForwardEdge(v101v100);
            front->addForwardEdge(v100v000);
            
            back->addForwardEdge(v010v110);
            back->addForwardEdge(v110v111);
            back->addForwardEdge(v111v011);
            back->addForwardEdge(v011v010);
            
            left->addBackwardEdge(v000v001);
            left->addForwardEdge(v000v010);
            left->addBackwardEdge(v011v010);
            left->addForwardEdge(v011v001);
            
            right->addBackwardEdge(v101v100);
            right->addForwardEdge(v101v111);
            right->addBackwardEdge(v110v111);
            right->addForwardEdge(v110v100);

            vertices.push_back(v000);
            vertices.push_back(v001);
            vertices.push_back(v010);
            vertices.push_back(v011);
            vertices.push_back(v100);
            vertices.push_back(v101);
            vertices.push_back(v110);
            vertices.push_back(v111);
            
            edges.push_back(v000v001);
            edges.push_back(v001v101);
            edges.push_back(v101v100);
            edges.push_back(v100v000);
            edges.push_back(v010v110);
            edges.push_back(v110v111);
            edges.push_back(v111v011);
            edges.push_back(v011v010);
            edges.push_back(v000v010);
            edges.push_back(v011v001);
            edges.push_back(v101v111);
            edges.push_back(v110v100);
            
            sides.push_back(top);
            sides.push_back(bottom);
            sides.push_back(front);
            sides.push_back(back);
            sides.push_back(left);
            sides.push_back(right);
            
            assert(vertices.size() == 8);
            assert(edges.size() == 12);
            assert(sides.size() == 6);
            assert(top->isClosed());
            assert(bottom->isClosed());
            assert(front->isClosed());
            assert(back->isClosed());
            assert(left->isClosed());
            assert(right->isClosed());
        }
    }
}
