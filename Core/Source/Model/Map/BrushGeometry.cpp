/*
 Copyright (C) 2010-2012 Kristian Duske
 
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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "BrushGeometry.h"

#include "Model/Map/Face.h"
#include "Utilities/VecMath.h"

#include <cassert>
#include <cmath>
#include <map>
#include <algorithm>

#define VERTEX_MAX_POOL_SIZE 256
#define EDGE_MAX_POOL_SIZE 256
#define SIDE_MAX_POOL_SIZE 256

namespace TrenchBroom {
    namespace Model {
        Vertex* Vertex::pool = NULL;
        Edge* Edge::pool = NULL;
        Side* Side::pool = NULL;
        int Vertex::poolSize = 0;
        int Edge::poolSize = 0;
        int Side::poolSize = 0;
        
        void* Vertex::operator new(size_t size) {
            if (pool != NULL) {
                Vertex* vertex = pool;
                pool = pool->next;
                poolSize--;
                return vertex;
            }
            
            Vertex* vertex = (Vertex*)malloc(size);
            vertex->next = NULL;
            return vertex;
        }
        
        void Vertex::operator delete(void* pointer) {
            if (poolSize < VERTEX_MAX_POOL_SIZE) {
                Vertex* vertex = (Vertex*)pointer;
                vertex->next = pool;
                pool = vertex;
                poolSize++;
            } else {
                free(pointer);
            }
        }

        Vertex::Vertex(float x, float y, float z) {
            position.x = x;
            position.y = y;
            position.z = z;
            mark = TB_VM_NEW;
        }
        
        Vertex::Vertex() {
            mark = TB_VM_NEW;
        }
        
        void* Edge::operator new(size_t size) {
            if (pool != NULL) {
                Edge* edge = pool;
                pool = pool->next;
                poolSize--;
                return edge;
            }
            
            Edge* edge = (Edge*)malloc(size);
            edge->next = NULL;
            return edge;
        }
        
        void Edge::operator delete(void* pointer) {
            if (poolSize < EDGE_MAX_POOL_SIZE) {
                Edge* edge = (Edge*)pointer;
                edge->next = pool;
                pool = edge;
                poolSize++;
            } else {
                free(pointer);
            }
        }

        Edge::Edge(Vertex* start, Vertex* end) : start(start), end(end), mark((TB_EM_NEW)), left(NULL), right(NULL) {}
        Edge::Edge() : start(NULL), end(NULL), mark(TB_EM_NEW), left(NULL), right(NULL) {}
        
        Vertex* Edge::startVertex(Side* side) {
            if (left == side) return end;
            if (right == side) return start;
            return NULL;
        }
        
        Vertex* Edge::endVertex(Side* side) {
            if (left == side) return start;
            if (right == side) return end;
            return NULL;
        }
        
        void Edge::updateMark() {
            int keep = 0;
            int drop = 0;
            int undecided = 0;
            
            if (start->mark == TB_VM_KEEP) keep++;
            else if (start->mark == TB_VM_DROP) drop++;
            else if (start->mark == TB_VM_UNDECIDED) undecided++;
            
            if (end->mark == TB_VM_KEEP) keep++;
            else if (end->mark == TB_VM_DROP) drop++;
            else if (end->mark == TB_VM_UNDECIDED) undecided++;
            
            assert(keep + drop + undecided == 2);
            
            if (keep == 1 && drop == 1) mark = TB_EM_SPLIT;
            else if (keep > 0) mark = TB_EM_KEEP;
            else if (drop > 0) mark = TB_EM_DROP;
            else mark = TB_EM_UNDECIDED;
        }
        
        Vec3f Edge::vector() {
            return end->position - start->position;
        }
        
        Vec3f Edge::center() {
            return (start->position + end->position) / 2;
        }
        
        Vertex* Edge::split(Plane plane) {
            Line line(start->position, (end->position - start->position).normalize());
            Vertex* newVertex = new Vertex();
            
            float dist = plane.intersectWithLine(line);
            newVertex->position = line.pointAtDistance(dist).snap();
            newVertex->mark = TB_VM_NEW;
            
            if (start->mark == TB_VM_DROP) start = newVertex;
            else end = newVertex;
            
            return newVertex;
        }
        
        void Edge::flip() {
            Side* tempSide = left;
            left = right;
            right = tempSide;
            Vertex* tempVertex = start;
            start = end;
            end = tempVertex;
        }
        
        void* Side::operator new(size_t size) {
            if (pool != NULL) {
                Side* side = pool;
                pool = pool->next;
                poolSize--;
                return side;
            }
            
            Side* side = (Side*)malloc(size);
            side->next = NULL;
            return side;
        }
        
        void Side::operator delete(void* pointer) {
            if (poolSize < SIDE_MAX_POOL_SIZE) {
                Side* side = (Side*)pointer;
                side->next = pool;
                pool = side;
                poolSize++;
            } else {
                free(pointer);
            }
        }

        Side::Side(Edge* newEdges[], bool invert[], unsigned int count) : mark(TB_SM_NEW), face(NULL) {
            vertices.reserve(count);
            edges.reserve(count);
            for (unsigned int i = 0; i < count; i++) {
                Edge* edge = newEdges[i];
                edges.push_back(edge);
                if (invert[i]) {
                    edge->left = this;
                    vertices.push_back(edge->end);
                } else {
                    edge->right = this;
                    vertices.push_back(edge->start);
                }
            }
        }
        
        Side::Side(Face& face, std::vector<Edge*>& newEdges) : mark(TB_SM_NEW), face(&face) {
            vertices.reserve(newEdges.size());
            edges.reserve(newEdges.size());
            for (unsigned int i = 0; i < newEdges.size(); i++) {
                Edge* edge = newEdges[i];
                edge->left = this;
                edges.push_back(edge);
                vertices.push_back(edge->startVertex(this));
            }
            
            this->face->side = this;
        }
        
        float Side::intersectWithRay(const Ray& ray) {
            assert(face != NULL);
            
            const Plane& boundary = face->boundary;
            float dot = boundary.normal | ray.direction;
            if (!Math::fneg(dot)) return std::numeric_limits<float>::quiet_NaN();
            
            float dist = boundary.intersectWithRay(ray);
            if (Math::isnan(dist)) return std::numeric_limits<float>::quiet_NaN();
            
            Vec3f hit, projectedHit, v0, v1;
            CoordinatePlane cPlane = CoordinatePlane::plane(boundary.normal);
            
            hit = ray.pointAtDistance(dist);
            projectedHit = cPlane.project(hit);
            
            const Vertex* vertex = vertices.back();
            v0 = cPlane.project(vertex->position) - projectedHit;
            
            int c = 0;
            for (unsigned int i = 0; i < vertices.size(); i++) {
                vertex = vertices[i];
                v1 = cPlane.project(vertex->position) - projectedHit;
                
                if ((Math::fzero(v0.x) && Math::fzero(v0.y)) || (Math::fzero(v1.x) && Math::fzero(v1.y))) {
                    // the point is identical to a polygon vertex, cancel search
                    c = 1;
                    break;
                }
                
                /*
                 * A polygon edge intersects with the positive X axis if the
                 * following conditions are met: The Y coordinates of its
                 * vertices must have different signs (we assign a negative sign
                 * to 0 here in order to count it as a negative number) and one
                 * of the following two conditions must be met: Either the X
                 * coordinates of the vertices are both positive or the X
                 * coordinates of the edge have different signs (again, we
                 * assign a negative sign to 0 here). In the latter case, we
                 * must calculate the point of intersection between the edge and
                 * the X axis and determine whether its X coordinate is positive
                 * or zero.
                 */
                
                // do the Y coordinates have different signs?
                if ((v0.y > 0 && v1.y <= 0) || (v0.y <= 0 && v1.y > 0)) {
                    // Is segment entirely on the positive side of the X axis?
                    if (v0.x > 0 && v1.x > 0) {
                        c += 1; // edge intersects with the X axis
                        // if not, do the X coordinates have different signs?
                    } else if ((v0.x > 0 && v1.x <= 0) || (v0.x <= 0 && v1.x > 0)) {
                        // calculate the point of intersection between the edge
                        // and the X axis
                        float x = -v0.y * (v1.x - v0.x) / (v1.y - v0.y) + v0.x;
                        if (x >= 0)
                            c += 1; // edge intersects with the X axis
                    }
                }
                
                v0 = v1;
            }
            
            if (c % 2 == 0) return std::numeric_limits<float>::quiet_NaN();
            return dist;
        }
        
        void Side::replaceEdges(size_t index1, size_t index2, Edge* edge) {
            if (index2 > index1) {
                vertices.erase(vertices.begin() + index1 + 1, vertices.begin() + index2 + 1);
                edges.erase(edges.begin() + index1 + 1, edges.begin() + index2);
                vertices.insert(vertices.begin() + index1 + 1, edge->startVertex(this));
                vertices.insert(vertices.begin() + index1 + 2, edge->endVertex(this));

                assert(edge->startVertex(this) == vertices[index1 + 1]);
                assert(edge->endVertex(this) == vertices[index1 + 2]);
                edges.insert(edges.begin() + index1 + 1, edge);
            } else {
                vertices.erase(vertices.begin() + index1 + 1, vertices.end());
                vertices.erase(vertices.begin(), vertices.begin() + index2 + 1);
                edges.erase(edges.begin() + index1 + 1, edges.end());
                edges.erase(edges.begin(), edges.begin() + index2);
                vertices.push_back(edge->startVertex(this));
                vertices.insert(vertices.begin(), edge->endVertex(this));
                
                assert(edge->startVertex(this) == vertices.back());
                assert(edge->endVertex(this) == vertices.front());
                edges.push_back(edge);
            }

            assert(vertices.size() == edges.size());
        }
        
        Edge* Side::split() {
            unsigned int keep = 0;
            unsigned int drop = 0;
            unsigned int split = 0;
            unsigned int undecided = 0;
            Edge* undecidedEdge = NULL;
            
            int splitIndex1 = -2;
            int splitIndex2 = -2;
            
            assert(!edges.empty());
            
            Edge* edge = edges.back();
            EEdgeMark lastMark = edge->mark;
            for (unsigned int i = 0; i < edges.size(); i++) {
                edge = edges[i];
                EEdgeMark currentMark = edge->mark;
                if (currentMark == TB_EM_SPLIT) {
                    Vertex* start = edge->startVertex(this);
                    if (start->mark == TB_VM_KEEP)
                        splitIndex1 = i;
                    else
                        splitIndex2 = i;
                    split++;
                } else if (currentMark == TB_EM_UNDECIDED) {
                    undecided++;
                    undecidedEdge = edge;
                } else if (currentMark == TB_EM_KEEP) {
                    if (lastMark == TB_EM_DROP)
                        splitIndex2 = i;
                    keep++;
                } else if (currentMark == TB_EM_DROP) {
                    if (lastMark == TB_EM_KEEP)
                        splitIndex1 = i > 0 ? i - 1 : (int)edges.size() - 1;
                    drop++;
                }
                lastMark = currentMark;
            }
            
            if (keep == edges.size()) {
                mark = TB_SM_KEEP;
                return NULL;
            }
            
            if (undecided == 1 && keep == edges.size() - 1) {
                mark = TB_SM_KEEP;
                return undecidedEdge;
            }
            
            if (drop + undecided == edges.size()) {
                mark = TB_SM_DROP;
                return NULL;
            }
            
            assert(splitIndex1 >= 0 && splitIndex2 >= 0);
            mark = TB_SM_SPLIT;
            
            Edge* newEdge = new Edge();
            newEdge->start = edges[splitIndex1]->endVertex(this);
            newEdge->end = edges[splitIndex2]->startVertex(this);
            newEdge->left = NULL;
            newEdge->right = this;
            newEdge->mark = TB_EM_NEW;
            
            replaceEdges(splitIndex1, splitIndex2, newEdge);
            return newEdge;
        }
        
        void Side::flip() {
            Vertex* tempVertex;
            size_t j;
            for (unsigned int i = 0; i < vertices.size() / 2; i++) {
                j = vertices.size() - i - 1;
                tempVertex = vertices[i];
                vertices[i] = vertices[j];
                vertices[j] = tempVertex;
            }
        }
        
        void Side::shift(int offset) {
            assert(offset >= 0);
            
            if (offset == 0)
                return;
            
            std::vector<Edge*> newEdges;
            std::vector<Vertex*> newVertices;
            
            int count = static_cast<int>(edges.size());
            for (int i = 0; i < count; i++) {
                size_t index = (i + offset + count) % count;
                newEdges.push_back(edges[index]);
                newVertices.push_back(vertices[index]);
            }
            
            edges = newEdges;
            vertices = newVertices;
        }
        
        std::vector<Side*> BrushGeometry::incidentSides(size_t vertexIndex) {
            std::vector<Side*> result;
            Vertex* vertex = vertices[vertexIndex];
            
            // find any edge that is incident to vertex
            Edge* edge = NULL;
            for (unsigned int i = 0; i < edges.size() && edge == NULL; i++) {
                Edge* candidate = edges[i];
                if (candidate->start == vertex || candidate->end == vertex)
                    edge = candidate;
            }
            
            Side* side = edge->start == vertex ? edge->right : edge->left;
            do {
                result.push_back(side);
                size_t i = indexOf<Edge>(side->edges, edge);
                edge = side->edges[(i - 1 + side->edges.size()) % side->edges.size()];
                side = edge->start == vertex ? edge->right : edge->left;
            } while (side != result[0]);
            
            return result;
        }
        
        void BrushGeometry::deleteDegenerateTriangle(Side* side, Edge* edge, std::vector<Face*>& newFaces, std::vector<Face*>& droppedFaces) {
            assert(side->edges.size() == 3);
            
            side->shift(indexOf(side->edges, edge));
            
            Edge* keepEdge = side->edges[1];
            Edge* dropEdge = side->edges[2];
            Side* neighbour = dropEdge->left == side ? dropEdge->right : dropEdge->left;
            
            if (keepEdge->left == side)
                keepEdge->left = neighbour;
            else
                keepEdge->right = neighbour;
            
            size_t deleteIndex = indexOf(neighbour->edges, dropEdge);
            size_t prevIndex = (deleteIndex - 1 + neighbour->edges.size()) % neighbour->edges.size();
            size_t nextIndex = (deleteIndex + 1) % neighbour->edges.size();
            neighbour->replaceEdges(prevIndex, nextIndex, keepEdge);
            
            std::vector<Face*>::iterator faceIt = find(newFaces.begin(), newFaces.end(), side->face);
            if (faceIt == newFaces.end()) droppedFaces.push_back(side->face);
            else newFaces.erase(faceIt);    
            
            deleteElement(sides, side);
            deleteElement(edges, dropEdge);
        }
        
        void BrushGeometry::triangulateSide(Side* side, size_t vertexIndex, std::vector<Face*>& newFaces) {
            Side* newSide;
            Vertex* vertex = vertices[vertexIndex];
            size_t sideVertexIndex = indexOf<Vertex>(side->vertices, vertex);
            assert(sideVertexIndex >= 0);
            
            Edge* sideEdges[3];
            bool flipped[3];
            sideEdges[0] = side->edges[sideVertexIndex];
            flipped[0] = sideEdges[0]->left == side;
            sideEdges[1] = side->edges[(sideVertexIndex + 1) % side->edges.size()];
            flipped[1] = sideEdges[1]->left == side;
            
            for (unsigned int i = 0; i < side->edges.size() - 3; i++) {
                sideEdges[2] = new Edge();
                sideEdges[2]->start = side->vertices[(sideVertexIndex + 2) % side->vertices.size()];
                sideEdges[2]->end = vertex;
                sideEdges[2]->left= NULL;
                sideEdges[2]->right = NULL;
                sideEdges[2]->mark = TB_EM_NEW;
                flipped[2] = false;
                edges.push_back(sideEdges[2]);
                
                newSide = new Side(sideEdges, flipped, 3);
                newSide->face = new Face(side->face->worldBounds, *side->face);
                sides.push_back(newSide);
                newFaces.push_back(newSide->face);
                
                sideEdges[0] = sideEdges[2];
                flipped[0] = true;
                edges[1] = side->edges[(sideVertexIndex + 2) % side->edges.size()];
                flipped[1] = sideEdges[1]->left == side;
                
                sideVertexIndex = (sideVertexIndex + 1) % side->edges.size();
            }
            
            sideEdges[2] = side->edges[(sideVertexIndex + 2) % side->edges.size()];
            flipped[2] = sideEdges[2]->left == side;
            
            newSide = new Side(sideEdges, flipped, 3);
            newSide->face = new Face(side->face->worldBounds, *side->face);
            sides.push_back(newSide);
            newFaces.push_back(newSide->face);
        }
        
        void BrushGeometry::splitSide(Side* side, size_t vertexIndex, std::vector<Face*>& newFaces) {
            Side* newSide;
            Vertex* vertex = vertices[vertexIndex];
            size_t sideVertexIndex = indexOf<Vertex>(side->vertices, vertex);
            assert(sideVertexIndex >= 0);
            
            Edge* sideEdges[3];
            bool flipped[3];
            sideEdges[0] = side->edges[(sideVertexIndex + side->edges.size() - 1) % side->edges.size()];
            flipped[0] = sideEdges[0]->left == side;
            sideEdges[1] = side->edges[sideVertexIndex % side->edges.size()];
            flipped[1] = sideEdges[1]->left == side;
            sideEdges[2] = new Edge();
            sideEdges[2]->start = side->vertices[(sideVertexIndex + side->edges.size() - 1) % side->vertices.size()];
            sideEdges[2]->end = side->vertices[(sideVertexIndex + 1) % side->vertices.size()];
            sideEdges[2]->left = NULL;
            sideEdges[2]->right = side;
            sideEdges[2]->mark = TB_EM_NEW;
            flipped[2] = true;
            edges.push_back(sideEdges[2]);
            side->replaceEdges((sideVertexIndex + side->edges.size() - 2) % side->edges.size(), (sideVertexIndex + 1) % side->edges.size(), sideEdges[2]);
            
            newSide = new Side(sideEdges, flipped, 3);
            newSide->face = new Face(side->face->worldBounds, *side->face);
            sides.push_back(newSide);
            newFaces.push_back(newSide->face);
            
        }
        
        void BrushGeometry::splitSides(std::vector<Side*>& sides, const Ray& ray, size_t vertexIndex, std::vector<Face*>& newFaces, std::vector<Face*>& droppedFaces) {
            Vec3f v1, v2;
            
            for (unsigned int i = 0; i < sides.size(); i++) {
                Side* side = sides[i];
                if (side->vertices.size() > 3) {
                    v1 = side->vertices[2]->position - side->vertices[0]->position;
                    v2 = side->vertices[1]->position - side->vertices[0]->position;
                    v1 = v1 % v2;
                    
                    if (Math::fneg((v1 | ray.direction))) {
                        splitSide(side, vertexIndex, newFaces);
                    } else {
                        triangulateSide(side, vertexIndex, newFaces);
                        std::vector<Face*>::iterator faceIt = find(newFaces.begin(), newFaces.end(), side->face);
                        if (faceIt == newFaces.end()) {
                            delete *faceIt;
                            newFaces.erase(faceIt);
                        } else {
                            newFaces.push_back(side->face);
                        }
                        
                        deleteElement(sides, side);
                    }
                }
            }
        }
        
        void BrushGeometry::mergeVertices(Vertex* keepVertex, Vertex* dropVertex, std::vector<Face*>& newFaces, std::vector<Face*>& droppedFaces) {
            // find the edge incident to both vertex and candidate
            Edge* dropEdge = NULL;
            for (unsigned int j = 0; j < edges.size() && dropEdge == NULL; j++) {
                Edge* edge =edges[j];
                if ((edge->start == keepVertex && edge->end == dropVertex) ||
                    (edge->end == keepVertex && edge->start == dropVertex))
                    dropEdge = edge;
            }
            
            // because the algorithm should not allow non-adjacent vertices to be merged in the first place
            assert(dropEdge != NULL); 
            assert(dropEdge->left->vertices.size() == 3);
            assert(dropEdge->right->vertices.size() == 3);
            
            for (unsigned int j = 0; j < edges.size(); j++) {
                Edge* edge = edges[j];
                if (edge != dropEdge && (edge->start == dropVertex || edge->end == dropVertex)) {
                    if (edge->start == dropVertex)
                        edge->start = keepVertex;
                    else
                        edge->end = keepVertex;
                    
                    int index = indexOf(edge->left->vertices, dropVertex);
                    if (index != -1)
                        edge->left->vertices[index] = keepVertex;
                    
                    index = indexOf(edge->right->vertices, dropVertex);
                    if (index != -1)
                        edge->right->vertices[index] = keepVertex;
                }
            }
            
            deleteDegenerateTriangle(dropEdge->left, dropEdge, newFaces, droppedFaces);
            deleteDegenerateTriangle(dropEdge->right, dropEdge, newFaces, droppedFaces);
            
            deleteElement(edges, dropEdge);
            deleteElement(vertices, dropVertex);
        }
        
        void BrushGeometry::mergeEdges() {
            Vec3f v1, v2;
            
            for (unsigned int i = 0; i < edges.size(); i++) {
                Edge* edge = edges[i];
                v1 = edge->vector();
                for (unsigned int j = i + 1; j < edges.size(); j++) {
                    Edge* candidate = edges[j];
                    v2 = candidate->vector();
                    v2 = v1 % v2;
                    if (v2.equals(Null3f)) {
                        if (edge->end == candidate->end)
                            candidate->flip();
                        if (edge->end == candidate->start) {
                            // we sometimes crash here because we meet two identical edges with opposite directions
                            assert(edge->start != candidate->end);
                            assert(edge->left == candidate->left);
                            assert(edge->right == candidate->right);
                            
                            Side* leftSide = edge->left;
                            Side* rightSide = edge->right;
                            
                            assert(leftSide != rightSide);
                            
                            Edge* newEdge = new Edge(edge->start, candidate->end);
                            newEdge->left = leftSide;
                            newEdge->right = rightSide;
                            edges.push_back(newEdge);
                            
                            size_t leftIndex = indexOf<Edge>(leftSide->edges, candidate);
                            size_t leftCount = leftSide->edges.size();
                            size_t rightIndex = indexOf<Edge>(rightSide->edges, candidate);
                            size_t rightCount = rightSide->edges.size();
                            
                            leftSide->replaceEdges((leftIndex - 1 + leftCount) % leftCount, (leftIndex + 2) % leftCount, newEdge);
                            rightSide->replaceEdges((rightIndex - 2 + rightCount) % rightCount, (rightIndex + 1) % rightCount, newEdge);
                            
                            deleteElement<Vertex>(vertices, candidate->start);
                            deleteElement<Edge>(edges, candidate);
                            deleteElement<Edge>(edges, edge);
                            
                            break;
                        }
                        
                        if (edge->start == candidate->start)
                            candidate->flip();
                        if (edge->start == candidate->end) {
                            assert(edge->left == candidate->left);
                            assert(edge->right == candidate->right);
                            
                            Side* leftSide = edge->left;
                            Side* rightSide = edge->right;
                            
                            assert(leftSide != rightSide);
                            
                            Edge* newEdge = new Edge(candidate->start, edge->end);
                            newEdge->left = leftSide;
                            newEdge->right = rightSide;
                            edges.push_back(newEdge);
                            
                            size_t leftIndex = indexOf<Edge>(leftSide->edges, candidate);
                            size_t leftCount = leftSide->edges.size();
                            size_t rightIndex = indexOf<Edge>(rightSide->edges, candidate);
                            size_t rightCount = rightSide->edges.size();
                            
                            leftSide->replaceEdges((leftIndex - 2 + leftCount) % leftCount, (leftIndex + 1) % leftCount, newEdge);
                            rightSide->replaceEdges((rightIndex - 1 + rightCount) % rightCount, (rightIndex + 2) % rightCount, newEdge);
                            
                            deleteElement<Vertex>(vertices, candidate->end);
                            deleteElement<Edge>(edges, candidate);
                            deleteElement<Edge>(edges, edge);
                            
                            break;
                        }
                    }
                }
            }
        }
        
        void BrushGeometry::mergeNeighbours(Side* side, size_t edgeIndex) {
            Vertex* vertex;
            Edge* edge = side->edges[edgeIndex];
            Side* neighbour = edge->left != side ? edge->left : edge->right;
            size_t sideEdgeIndex = edgeIndex;
            size_t neighbourEdgeIndex = indexOf<Edge>(neighbour->edges, edge);
            
            do {
                sideEdgeIndex = (sideEdgeIndex + 1) % side->edges.size();
                neighbourEdgeIndex = (neighbourEdgeIndex - 1 + neighbour->edges.size()) % neighbour->edges.size();
            } while (side->edges[sideEdgeIndex] == neighbour->edges[neighbourEdgeIndex]);
            
            // now sideEdgeIndex points to the last edge (in CW order) of side that should not be deleted
            // and neighbourEdgeIndex points to the first edge (in CW order) of neighbour that should not be deleted
            
            int count = -1;
            do {
                sideEdgeIndex = (sideEdgeIndex - 1 + side->edges.size()) % side->edges.size();
                neighbourEdgeIndex = (neighbourEdgeIndex + 1) % neighbour->edges.size();
                count++;
            } while (side->edges[sideEdgeIndex] == neighbour->edges[neighbourEdgeIndex]);
            
            // now sideEdgeIndex points to the first edge (in CW order) of side that should not be deleted
            // now neighbourEdgeIndex points to the last edge (in CW order) of neighbour that should not be deleted
            // and count is the number of shared edges between side and neighbour
            
            // shift the two sides so that their shared edges are at the end of both's edge lists
            side->shift((sideEdgeIndex + count + 1) % side->edges.size());
            neighbour->shift(neighbourEdgeIndex);
            
            side->edges.resize(side->edges.size() - count);
            side->vertices.resize(side->vertices.size() - count);
            
            for (unsigned int i = 0; i < neighbour->edges.size() - count; i++) {
                edge = neighbour->edges[i];
                vertex = neighbour->vertices[i];
                if (edge->left == neighbour)
                    edge->left = side;
                else
                    edge->right = side;
                side->edges.push_back(edge);
                side->vertices.push_back(vertex);
            }
            
            for (size_t i = neighbour->edges.size() - count; i < neighbour->edges.size(); i++) {
                deleteElement<Edge>(edges, neighbour->edges[i]);
                if (i > neighbour->edges.size() - count)
                    deleteElement<Vertex>(vertices, neighbour->vertices[i]);
            }
            
            neighbour->face->side = NULL;
            deleteElement<Side>(sides, neighbour);
        }
        
        void BrushGeometry::mergeSides(std::vector<Face*>& newFaces, std::vector<Face*>&droppedFaces) {
            for (unsigned int i = 0; i < sides.size(); i++) {
                Side* side = sides[i];
                Plane sideBoundary;
                sideBoundary.setPoints(side->vertices[0]->position, 
                                       side->vertices[1]->position, 
                                       side->vertices[2]->position);
                
                for (unsigned int j = 0; j < side->edges.size(); j++) {
                    Edge* edge = side->edges[j];
                    Side* neighbour = edge->left != side ? edge->left : edge->right;
                    Plane neighbourBoundary;
                    neighbourBoundary.setPoints(neighbour->vertices[0]->position, 
                                                neighbour->vertices[1]->position, 
                                                neighbour->vertices[2]->position);
                    
                    if (sideBoundary.equals(neighbourBoundary)) {
                        Face* neighbourFace = neighbour->face;
                        mergeNeighbours(side, j);
                        
                        std::vector<Face*>::iterator faceIt = find(newFaces.begin(), newFaces.end(), neighbourFace);
                        if (faceIt == newFaces.end()) {
                            delete *faceIt;
                            newFaces.erase(faceIt);
                        } else {
                            newFaces.push_back(side->face);
                        }
                        
                        i -= 1;
                        break;
                    }
                }
            }
        }
        
        float BrushGeometry::minVertexMoveDist(const std::vector<Side*>& sides, const Vertex* vertex, const Ray& ray, float maxDist) {
            float minDist;
            Plane plane;
            
            minDist = maxDist;
            for (unsigned int i = 0; i < sides.size(); i++) {
                Side* side = sides[i];
                Side* succ = sides[(i + 1) % sides.size()];
                
                side->shift(indexOf<Vertex>(side->vertices, vertex));
                succ->shift(indexOf<Vertex>(succ->vertices, vertex));
                
                plane.setPoints(side->vertices[1]->position, 
                                side->vertices[2]->position, 
                                succ->vertices[2]->position);
                
                float sideDist = plane.intersectWithRay(ray);
                
                Edge* neighbourEdge = side->edges[1];
                Side* neighbourSide = neighbourEdge->left != side ? neighbourEdge->left : neighbourEdge->right;
                
                plane = neighbourSide->face->boundary;
                float neighbourDist = plane.intersectWithRay(ray);
                
                if (!Math::isnan(sideDist) && Math::fpos(sideDist) && Math::flt(sideDist, minDist))
                    minDist = sideDist;
                if (!Math::isnan(neighbourDist) && Math::fpos(neighbourDist) && Math::flt(neighbourDist, minDist))
                    minDist = neighbourDist;
            }
            
            return minDist;
        }
        
        MoveResult BrushGeometry::moveVertex(size_t vertexIndex, bool mergeIncidentVertex, const Vec3f& delta, std::vector<Face*>& newFaces, std::vector<Face*>& droppedFaces) {
            Vertex* vertex;
            Vec3f newPosition;
            Ray ray;
            float actualMoveDist, moveDist, dot1, dot2;
            std::vector<Side*> incSides;
            size_t actualVertexIndex;
            Vec3f v1, v2, cross, edgeVector;
            MoveResult result;
            
            assert(vertexIndex >= 0 && vertexIndex < vertices.size());
            
            moveDist = delta.length();
            if (moveDist == 0) {
                result.moved = false;
                result.index = vertexIndex;
                return result;
            }
            
            actualVertexIndex = vertexIndex;
            vertex = vertices[actualVertexIndex];
            ray.origin = vertex->position;
            ray.direction = delta / moveDist;
            
            incSides = incidentSides(actualVertexIndex);
            splitSides(incSides, ray, actualVertexIndex, newFaces, droppedFaces);
            
            incSides = incidentSides(actualVertexIndex);
            actualMoveDist = minVertexMoveDist(incSides, vertex, ray, moveDist);
            
            vertex->position = ray.pointAtDistance(actualMoveDist);
            newPosition = vertex->position;
            
            // check whether the vertex is dragged onto a non-incident edge
            for (unsigned int i = 0; i < edges.size(); i++) {
                Edge* edge = edges[i];
                if (edge->start != vertex && edge->end != vertex) {
                    v1 = vertex->position - edge->start->position;
                    v2 = vertex->position - edge->end->position;
                    cross = v1 % v2;
                    
                    if (cross.equals(Null3f)) {
                        edgeVector = edge->vector();
                        dot1 = v1 | edgeVector;
                        dot2 = v2 | edgeVector;
                        if ((dot1 > 0 && dot2 < 0) || (dot1 < 0 && dot2 > 0)) {
                            // undo the vertex move
                            vertex->position = ray.origin;
                            mergeSides(newFaces, droppedFaces);
                            mergeEdges();
                            
                            result.moved = false;
                            result.index = indexOf<Vertex>(vertices, vertex);
                            return result;
                        }
                    }
                }
            }
            
            // check whether the vertex is dragged onto another vertex, if so, kill that vertex
            for (unsigned int i = 0; i < vertices.size(); i++) {
                if (i != vertexIndex) {
                    Vertex* candidate = vertices[i];
                    if (vertex->position.equals(candidate->position)) {
                        if (mergeIncidentVertex) {
                            mergeVertices(vertex, candidate, newFaces, droppedFaces);
                            break;
                        } else {
                            // undo the vertex move
                            vertex->position = ray.origin;
                            mergeSides(newFaces, droppedFaces);
                            mergeEdges();
                            
                            result.moved = false;
                            result.index = indexOf<Vertex>(vertices, vertex);
                            return result;
                        }
                    }
                }
            }
            
            // now merge all mergeable sides back together
            // then check for consecutive edges that can be merged
            mergeSides(newFaces, droppedFaces);
            mergeEdges();
            bounds = boundsOfVertices(vertices);
            
            // find the index of the dragged vertex
            int newVertexIndex = indexOf(vertices, newPosition);
            
            // drag is concluded
            if (newVertexIndex == -1 || actualMoveDist == moveDist) {
                for (unsigned int i = 0; i < vertices.size(); i++)
                    vertices[i]->position = vertices[i]->position.snap();
                for (unsigned int i = 0; i < sides.size(); i++)
                    sides[i]->face->updatePoints();
                
                result.moved = true;
                result.index = vertexIndex;
                return result;
            }
            
            // now safe
            vertexIndex = static_cast<size_t>(newVertexIndex);
            
            // drag is not concluded, calculate the new delta and call self
            ray.direction *= (moveDist - actualMoveDist);
            return moveVertex(vertexIndex, mergeIncidentVertex, ray.direction, newFaces, droppedFaces);
        }
        
        MoveResult BrushGeometry::splitAndMoveEdge(size_t index, const Vec3f& delta, std::vector<Face*>& newFaces, std::vector<Face*>& droppedFaces) {
            Edge* edge;
            Vertex* vertex;
            Vec3f edgeVertices[2];
            Vec3f leftNorm, rightNorm;
            MoveResult result;
            size_t edgeIndex;
            
            edgeIndex = index - vertices.size();
            edge = edges[edgeIndex];
            
            // detect whether the drag would make the incident faces invalid
            leftNorm = edge->left->face->boundary.normal;
            rightNorm = edge->right->face->boundary.normal;
            if (Math::fneg((delta | leftNorm)) || 
                Math::fneg((delta | rightNorm))) {
                result.moved = false;
                result.index = index;
                return result;
            }
            
            edgeVertices[0] = edge->start->position;
            edgeVertices[1] = edge->end->position;
            
            // split the edge
            edge->left->shift(indexOf<Edge>(edge->left->edges, edge) + 1);
            edge->right->shift(indexOf<Edge>(edge->right->edges, edge) + 1);
            
            vertex = new Vertex();
            vertex->position = edge->center();
            
            vertices.push_back(vertex);
            edge->left->vertices.push_back(vertex);
            edge->right->vertices.push_back(vertex);
            
            Edge* newEdge1 = new Edge(edge->start, vertex);
            newEdge1->left = edge->left;
            newEdge1->right = edge->right;
            Edge* newEdge2 = new Edge(vertex, edge->end);
            newEdge2->left = edge->left;
            newEdge2->right = edge->right;
            
            edge->left->edges.erase(edge->left->edges.end() - 1);
            edge->right->edges.erase(edge->right->edges.end() - 1);
            
            edges.push_back(newEdge1);
            edges.push_back(newEdge2);
            edge->left->edges.push_back(newEdge2);
            edge->left->edges.push_back(newEdge1);
            edge->right->edges.push_back(newEdge1);
            edge->right->edges.push_back(newEdge2);
            
            edges.erase(edges.begin() + edgeIndex);
            
            result = moveVertex(vertices.size() - 1, true, delta, newFaces, droppedFaces);
            if (result.index == -1)
                result.index = vertices.size() + indexOf(edges, edgeVertices[0], edgeVertices[1]);
            
            return result;
        }
        
        MoveResult BrushGeometry::splitAndMoveSide(size_t sideIndex, const Vec3f& delta, std::vector<Face*>& newFaces, std::vector<Face*>& droppedFaces) {
            Side* side;
            Vertex* vertex;
            std::vector<Vec3f> sideVertices;
            Vec3f norm;
            MoveResult result;    
            size_t index;
            
            index = sideIndex - edges.size() - vertices.size();
            side = sides[index];
            
            // detect whether the drag would lead to an indented face
            norm = side->face->boundary.normal;
            if (Math::fzero((delta | norm))) {
                result.moved = false;
                result.index = sideIndex;
                return result;
            }
            
            // store the side's vertices for later
            for (unsigned int i = 0; i < side->vertices.size(); i++)
                sideVertices.push_back(side->vertices[i]->position);
            
            vertex = new Vertex();
            vertex->position = centerOfVertices(side->vertices);
            vertices.push_back(vertex);
            
            Edge* firstEdge = new Edge(vertex, side->edges[0]->startVertex(side));
            edges.push_back(firstEdge);
            
            Edge* lastEdge = firstEdge;
            for (unsigned int i = 0; i < side->edges.size(); i++) {
                Edge* sideEdge = side->edges[i];
                
                Edge* newEdge;
                if (i == side->edges.size() - 1) {
                    newEdge = firstEdge;
                } else {
                    newEdge = new Edge(vertex, sideEdge->endVertex(side));
                    edges.push_back(newEdge);
                }
                
                Side* newSide = new Side();
                newSide->vertices.push_back(vertex);
                newSide->edges.push_back(lastEdge);
                lastEdge->right = newSide;
                
                newSide->vertices.push_back(lastEdge->end);
                newSide->edges.push_back(sideEdge);
                if (sideEdge->left == side) sideEdge->left = newSide;
                else sideEdge->right = newSide;
                
                newSide->vertices.push_back(newEdge->end);
                newSide->edges.push_back(newEdge);
                newEdge->left = newSide;
                
                newSide->face = new Face(side->face->worldBounds, *side->face);
                sides.push_back(newSide);
                newFaces.push_back(newSide->face);
                
                lastEdge = newEdge;
            }
            
            droppedFaces.push_back(side->face);
            sides.erase(sides.begin() + index);
            delete side;
            
            result = moveVertex(vertices.size() - 1, true, delta, newFaces, droppedFaces);
            result.index = indexOf(sides, sideVertices);
            
            return result;
        }
        
        void BrushGeometry::copy(const BrushGeometry& original) {
            std::map<Vertex*, Vertex*> vertexMap;
            std::map<Edge*, Edge*> edgeMap;
            std::map<Side*, Side*> sideMap;
            
            vertices.reserve(original.vertices.size());
            edges.reserve(original.edges.size());
            sides.reserve(original.sides.size());
            
            for (unsigned int i = 0; i < original.vertices.size(); i++) {
                Vertex* originalVertex = original.vertices[i];
                Vertex* copyVertex = new Vertex(*originalVertex);
                vertexMap[originalVertex] = copyVertex;
                vertices.push_back(copyVertex);
            }
            
            for (unsigned int i = 0; i < original.edges.size(); i++) {
                Edge* originalEdge = original.edges[i];
                Edge* copyEdge = new Edge(*originalEdge);
                copyEdge->start = vertexMap[originalEdge->start];
                copyEdge->end = vertexMap[originalEdge->end];
                edgeMap[originalEdge] = copyEdge;
                edges.push_back(copyEdge);
            }
            
            for (unsigned int i = 0; i < original.sides.size(); i++) {
                Side* originalSide = original.sides[i];
                Side* copySide = new Side(*originalSide);
                copySide->vertices.clear();
                copySide->edges.clear();
                
                for (unsigned int j = 0; j < originalSide->edges.size(); j++) {
                    Edge* originalEdge = originalSide->edges[j];
                    Edge* copyEdge = edgeMap[originalEdge];
                    
                    if (originalEdge->left == originalSide) copyEdge->left = copySide;
                    else copyEdge->right = copySide;
                    copySide->edges.push_back(copyEdge);
                    copySide->vertices.push_back(copyEdge->startVertex(copySide));
                }
            }
            
            bounds = original.bounds;
        }
        
        BrushGeometry::BrushGeometry(const BBox& bounds) {
            Vertex* lfd = new Vertex(bounds.min.x, bounds.min.y, bounds.min.z);
            Vertex* lfu = new Vertex(bounds.min.x, bounds.min.y, bounds.max.z);
            Vertex* lbd = new Vertex(bounds.min.x, bounds.max.y, bounds.min.z);
            Vertex* lbu = new Vertex(bounds.min.x, bounds.max.y, bounds.max.z);
            Vertex* rfd = new Vertex(bounds.max.x, bounds.min.y, bounds.min.z);
            Vertex* rfu = new Vertex(bounds.max.x, bounds.min.y, bounds.max.z);
            Vertex* rbd = new Vertex(bounds.max.x, bounds.max.y, bounds.min.z);
            Vertex* rbu = new Vertex(bounds.max.x, bounds.max.y, bounds.max.z);
            
            Edge* lfdlbd = new Edge(lfd, lbd);
            Edge* lbdlbu = new Edge(lbd, lbu);
            Edge* lbulfu = new Edge(lbu, lfu);
            Edge* lfulfd = new Edge(lfu, lfd);
            Edge* rfdrfu = new Edge(rfd, rfu);
            Edge* rfurbu = new Edge(rfu, rbu);
            Edge* rburbd = new Edge(rbu, rbd);
            Edge* rbdrfd = new Edge(rbd, rfd);
            Edge* lfurfu = new Edge(lfu, rfu);
            Edge* rfdlfd = new Edge(rfd, lfd);
            Edge* lbdrbd = new Edge(lbd, rbd);
            Edge* rbulbu = new Edge(rbu, lbu);
            
            bool invertNone[4] = {false, false, false, false};
            bool invertAll[4] = {true, true, true, true};
            bool invertOdd[4] = {false, true, false, true};
            
            Edge* leftEdges[4] = {lfdlbd, lbdlbu, lbulfu, lfulfd};
            Side* left = new Side(leftEdges, invertNone, 4);
            
            Edge* rightEdges[4] = {rfdrfu, rfurbu, rburbd, rbdrfd};
            Side* right = new Side(rightEdges, invertNone, 4);
            
            Edge* frontEdges[4] = {lfurfu, rfdrfu, rfdlfd, lfulfd};
            Side* front = new Side(frontEdges, invertOdd, 4);
            
            Edge* backEdges[4] = {rbulbu, lbdlbu, lbdrbd, rburbd};
            Side* back = new Side(backEdges, invertOdd, 4);
            
            Edge* topEdges[4] = {lbulfu, rbulbu, rfurbu, lfurfu};
            Side* top = new Side(topEdges, invertAll, 4);
            
            Edge* downEdges[4] = {rfdlfd, rbdrfd, lbdrbd, lfdlbd};
            Side* down = new Side(downEdges, invertAll, 4);
            
            vertices.resize(8);
            vertices[0] = lfd;
            vertices[1] = lfu;
            vertices[2] = lbd;
            vertices[3] = lbu;
            vertices[4] = rfd;
            vertices[5] = rfu;
            vertices[6] = rbd;
            vertices[7] = rbu;
            
            edges.resize(12);
            edges[ 0] = lfdlbd;
            edges[ 1] = lbdlbu;
            edges[ 2] = lbulfu;
            edges[ 3] = lfulfd;
            edges[ 4] = rfdrfu;
            edges[ 5] = rfurbu;
            edges[ 6] = rburbd;
            edges[ 7] = rbdrfd;
            edges[ 8] = lfurfu;
            edges[ 9] = rfdlfd;
            edges[10] = lbdrbd;
            edges[11] = rbulbu;
            
            sides.resize(6);
            sides[0] = left;
            sides[1] = right;
            sides[2] = front;
            sides[3] = back;
            sides[4] = top;
            sides[5] = down;
            
            this->bounds = bounds;
        }
        
        BrushGeometry::BrushGeometry(const BrushGeometry& original) {
            copy(original);
        }
        
        BrushGeometry::~BrushGeometry() {
            while(!sides.empty()) delete sides.back(), sides.pop_back();
            while(!edges.empty()) delete edges.back(), edges.pop_back();
            while(!vertices.empty()) delete vertices.back(), vertices.pop_back();
        }
        
        bool BrushGeometry::closed() const {
            for (unsigned int i = 0; i < sides.size(); i++)
                if (sides[i]->face == NULL)
                    return false;
            return true;
        }
        
        void BrushGeometry::restoreFaceSides() {
            for (unsigned int i = 0; i < sides.size(); i++)
                sides[i]->face->side = sides[i];
        }
        
        ECutResult BrushGeometry::addFace(Face& face, std::vector<Face*>& droppedFaces) {
            Plane boundary = face.boundary;
            
            unsigned int keep = 0;
            unsigned int drop = 0;
            unsigned int undecided = 0;
            
            // mark vertices
            for (unsigned int i = 0; i < vertices.size(); i++) {
                Vertex& vertex = *vertices[i];
                EPointStatus vs = boundary.pointStatus(vertex.position);
                if (vs == TB_PS_ABOVE) {
                    vertex.mark = TB_VM_DROP;
                    drop++;
                } else if (vs == TB_PS_BELOW) {
                    vertex.mark  = TB_VM_KEEP;
                    keep++;
                } else {
                    vertex.mark = TB_VM_UNDECIDED;
                    undecided++;
                }
            }
            
            if (keep + undecided == vertices.size())
                return TB_CR_REDUNDANT;
            
            if (drop + undecided == vertices.size())
                return TB_CR_NULL;
            
            // mark and split edges
            for (unsigned int i = 0; i < edges.size(); i++) {
                Edge& edge = *edges[i];
                edge.updateMark();
                if (edge.mark == TB_EM_SPLIT) {
                    Vertex* vertex = edge.split(boundary);
                    vertices.push_back(vertex);
                }
            }
            
            // mark, split and drop sides
            std::vector<Edge*> newEdges;
            std::vector<Side*>::iterator sideIt = sides.begin();
            while (sideIt != sides.end()) {
                Side* side = *sideIt;
                Edge* newEdge = side->split();
                
                if (side->mark == TB_SM_DROP) {
                    Face* face = side->face;
                    if (face != NULL) {
                        droppedFaces.push_back(face);
                        face->side = NULL;
                    }
                    delete side;
                    sideIt = sides.erase(sideIt);
                } else if (side->mark == TB_SM_SPLIT) {
                    edges.push_back(newEdge);
                    newEdges.push_back(newEdge);
                    side->mark = TB_SM_UNKNOWN;
                    ++sideIt;
                } else if (side->mark == TB_SM_KEEP && newEdge != NULL) {
                    // the edge is an undecided edge, so it needs to be flipped in order to act as a new edge
                    if (newEdge->right != side)
                        newEdge->flip();
                    newEdges.push_back(newEdge);
                    side->mark = TB_SM_UNKNOWN;
                    ++sideIt;
                } else {
                    side->mark = TB_SM_UNKNOWN;
                    ++sideIt;
                }
            }
            
            // create new side from newly created edges
            // first, sort the new edges to form a polygon in clockwise order
            for (unsigned int i = 0; i < newEdges.size() - 1; i++) {
                Edge* edge = newEdges[i];
                for (unsigned int j = i + 2; j < newEdges.size(); j++) {
                    Edge* candidate = newEdges[j];
                    if (edge->start == candidate->end) {
                        newEdges[j] = newEdges[i + 1];
                        newEdges[i + 1] = candidate;
                        break;
                    }
                }
            }
            
            // now create the new side
            Side* newSide = new Side(face, newEdges);
            sides.push_back(newSide);
            
            // sanity checks
            for (unsigned int i = 0; i < sides.size(); i++) {
                Side* side = sides[i];
                std::vector<Vertex*>& vertices = side->vertices;
                std::vector<Edge*>& edges = side->edges;
                assert(vertices.size() == edges.size());
                for (unsigned int j = 0; j < vertices.size(); j++) {
                    assert(vertices[j]->mark != TB_VM_DROP);
                    assert(edges[j]->mark != TB_EM_DROP);
                    assert(edges[j]->startVertex(side) == vertices[j]);
                }
            }
            
            // clean up
            // delete dropped vertices
            std::vector<Vertex*>::iterator vertexIt = vertices.begin();
            while (vertexIt != vertices.end()) {
                Vertex* vertex = *vertexIt;
                if (vertex->mark == TB_VM_DROP) {
                    delete vertex;
                    vertexIt = vertices.erase(vertexIt);
                } else {
                    vertex->mark = TB_VM_UNKNOWN;
                    ++vertexIt;
                }
            }
            
            // delete dropped edges
            std::vector<Edge*>::iterator edgeIt = edges.begin();
            while (edgeIt != edges.end()) {
                Edge* edge = *edgeIt;
                if (edge->mark == TB_EM_DROP) {
                    delete edge;
                    edgeIt = edges.erase(edgeIt);
                } else {
                    edge->mark = TB_EM_UNKNOWN;
                    ++edgeIt;
                }
            }
            
            bounds = boundsOfVertices(vertices);
            return TB_CR_SPLIT;
        }
        
        bool BrushGeometry::addFaces(std::vector<Face*>& faces, std::vector<Face*>& droppedFaces) {
            for (unsigned int i = 0; i < faces.size(); i++)
                if (addFace(*faces[i], droppedFaces) == TB_CR_NULL)
                    return false;
            return true;
        }
        
        void BrushGeometry::translate(const Vec3f& delta) {
            for (unsigned int i = 0; i < vertices.size(); i++)
                vertices[i]->position += delta;
            bounds = bounds.translate(delta);
        }
        
        void BrushGeometry::rotate90(EAxis axis, const Vec3f& center, bool clockwise) {
            for (unsigned int i = 0; i < vertices.size(); i++)
                vertices[i]->position = vertices[i]->position.rotate90(axis, center, clockwise);
            bounds = bounds.rotate90(axis, center, clockwise);
        }
        
        void BrushGeometry::rotate(const Quat& rotation, const Vec3f& center) {
            for (unsigned int i = 0; i < vertices.size(); i++)
                vertices[i]->position = rotation * (vertices[i]->position - center) + center;
            bounds = bounds.rotate(rotation, center);
        }
        
        void BrushGeometry::flip(EAxis axis, const Vec3f& center) {
            for (unsigned int i = 0; i < vertices.size(); i++)
                vertices[i]->position = vertices[i]->position.flip(axis, center);
            bounds = bounds.flip(axis, center);

            for (unsigned int i = 0; i < edges.size(); i++)
                edges[i]->flip();
            for (unsigned int i = 0; i < sides.size(); i++)
                sides[i]->flip();
        }
        
        void BrushGeometry::snap() {
        }
        
        MoveResult BrushGeometry::moveVertex(size_t vertexIndex, const Vec3f& delta, std::vector<Face*>& newFaces, std::vector<Face*>& droppedFaces) {
            assert(vertexIndex >= 0);
            assert(vertexIndex < vertices.size() + edges.size() + sides.size());
            
            MoveResult result;
            if (delta.lengthSquared() == 0)
                result = MoveResult(vertexIndex, false);
            else if (vertexIndex < vertices.size())
                result = moveVertex(vertexIndex, true, delta, newFaces, droppedFaces);
            else if (vertexIndex < vertices.size() + edges.size())
                result = splitAndMoveEdge(vertexIndex, delta, newFaces, droppedFaces);
            else
                result = splitAndMoveSide(vertexIndex, delta, newFaces, droppedFaces);
            
            return result;
        }
        
        MoveResult BrushGeometry::moveEdge(size_t edgeIndex, const Vec3f& delta, std::vector<Face*>& newFaces, std::vector<Face*>& droppedFaces) {
            assert(edgeIndex >= 0 && edgeIndex < edges.size());
            
            if (delta.lengthSquared() == 0)
                return MoveResult(edgeIndex, false);
            
            BrushGeometry testGeometry(*this);
            testGeometry.restoreFaceSides();
            
            Vec3f dir;
            Edge* edge = testGeometry.edges[edgeIndex];
            Vec3f start = edge->start->position;
            Vec3f end = edge->end->position;
            dir = end - start;
            start += delta;
            end += delta;
            
            MoveResult result;
            if ((dir | delta) > 0) {
                result = testGeometry.moveVertex(indexOf<Vertex>(testGeometry.vertices, edge->end), false, delta, newFaces, droppedFaces);
                if (result.moved)
                    result = testGeometry.moveVertex(indexOf<Vertex>(testGeometry.vertices, edge->start), false, delta, newFaces, droppedFaces);
            } else {
                result = testGeometry.moveVertex(indexOf<Vertex>(testGeometry.vertices, edge->start), false, delta, newFaces, droppedFaces);
                if (result.moved)
                    result = testGeometry.moveVertex(indexOf<Vertex>(testGeometry.vertices, edge->end), false, delta, newFaces, droppedFaces);
            }
            
            if (result.moved) {
                result.index = indexOf(testGeometry.edges, start, end);
                copy(testGeometry);
            } else {
                result.index = edgeIndex;
                newFaces.clear();
                droppedFaces.clear();
            }
            
            restoreFaceSides();
            return result;
        }
        
        MoveResult BrushGeometry::moveSide(size_t sideIndex, const Vec3f& delta, std::vector<Face*>& newFaces, std::vector<Face*>& droppedFaces) {
            assert(sideIndex >= 0 && sideIndex < sides.size());
            
            float dist = delta.length();
            if (dist == 0)
                return MoveResult(sideIndex, false);
            
            BrushGeometry testGeometry(*this);
            testGeometry.restoreFaceSides();
            
            Vec3f diff;
            Vec3f dir = delta / dist;
            Side* side = testGeometry.sides[sideIndex];
            Vec3f center = centerOfVertices(side->vertices);
            
            size_t sideVertexCount = side->vertices.size();
            std::vector<Vec3f> sideVertices(sideVertexCount);
            std::vector<size_t> indices(sideVertexCount);
            std::vector<float> dots(sideVertexCount);
            for (unsigned int i = 0; i < sideVertexCount; i++) {
                sideVertices[i] = side->vertices[i]->position;
                Vec3f diff = sideVertices[i] - center;
                dots[i] = diff | dir;
                indices[i] = indexOf<Vertex>(testGeometry.vertices, side->vertices[i]);
                sideVertices[i] += delta;
            }
            
            // sort indices by dot value, eek, bubblesort
            bool switched = true;
            for (long j = static_cast<long>(sideVertexCount) - 1; j >= 0 && switched; j--) {
                switched = false;
                for (long i = 0; i < j; i++) {
                    if (dots[i] > dots[i + 1]) {
                        float dt = dots[i];
                        dots[i] = dots[i + 1];
                        dots[i + 1] = dt;
                        
                        size_t di = indices[i];
                        indices[i] = indices[i + 1];
                        indices[i + 1] = di;
                        switched = true;
                    }
                }
            }
            
            MoveResult result(-1, true);
            for (unsigned int i = 0; i < sideVertexCount && result.moved; i++)
                result = moveVertex(indices[i], false, delta, newFaces, droppedFaces);
            
            if (result.moved) {
                result.index = indexOf(testGeometry.sides, sideVertices);
                copy(testGeometry);
            } else {
                result.index = sideIndex;
                newFaces.clear();
                droppedFaces.clear();
            }
            
            restoreFaceSides();
            return result;
        }
        
        template <class T> int indexOf(const std::vector<T*>& vec, const T* element) {
            for (unsigned int i = 0; i < vec.size(); i++)
                if (vec[i] == element)
                    return i;
            return -1;
        }
        
        template <class T> bool removeElement(std::vector<T*>& vec, T* element) {
            typename std::vector<T*>::iterator elementIt = find(vec.begin(), vec.end(), element);
            if (elementIt == vec.end())
                return false;
            vec.erase(elementIt);
            return true;
        }
        
        template <class T> bool deleteElement(std::vector<T*>& vec, T* element) {
            if (!removeElement(vec, element))
                return false;
            delete element;
            return true;
        }
        
        int indexOf(const std::vector<Vertex*>& vertices, const Vec3f& v) {
            for (unsigned int i = 0; i < vertices.size(); i++)
                if (vertices[i]->position.equals(v)) return i;
            return -1;
        }
        
        int indexOf(const std::vector<Edge*>& edges, const Vec3f& v1, const Vec3f& v2) {
            for (unsigned int i = 0; i < edges.size(); i++) {
                Edge* edge = edges[i];
                if ((edge->start->position.equals(v1) && edge->end->position.equals(v2)) ||
                    (edge->start->position.equals(v2) && edge->end->position.equals(v1)))
                    return i;
            }
            return -1;
        }
        
        int indexOf(const std::vector<Side*>& sides, const std::vector<Vec3f>& vertices) {
            for (unsigned int i = 0; i < sides.size(); i++) {
                Side* side = sides[i];
                if (side->vertices.size() == vertices.size()) {
                    for (unsigned int j = 0; j < vertices.size(); j++) {
                        unsigned int k = 0;
                        while (k < vertices.size() && side->vertices[(j + k) % vertices.size()]->position.equals(vertices[k]))
                            k++;
                        
                        if (k == vertices.size())
                            return i;
                    }
                }
            }
            
            return -1;    
        }
        
        Vec3f centerOfVertices(const std::vector<Vertex*>& vertices) {
            Vec3f center = vertices[0]->position;
            for (unsigned int i = 1; i < vertices.size(); i++)
                center += vertices[i]->position;
            center /= static_cast<float>(vertices.size());
            return center;
        }
        
        BBox boundsOfVertices(const std::vector<Vertex*>& vertices) {
            BBox bounds;
            bounds.min = vertices[0]->position;
            bounds.max = vertices[0]->position;
            
            for (unsigned int i = 1; i < vertices.size(); i++)
                bounds += vertices[i]->position;
            return bounds;
        }
        
        EPointStatus vertexStatusFromRay(const Vec3f& origin, const Vec3f& direction, const std::vector<Vertex*>& vertices) {
            Ray ray(origin, direction);
            int above = 0;
            int below = 0;
            for (unsigned int i = 0; i < vertices.size(); i++) {
                EPointStatus status = ray.pointStatus(vertices[i]->position); 
                if (status == TB_PS_ABOVE)
                    above++;
                else if (status == TB_PS_BELOW)
                    below++;
                if (above > 0 && below > 0)
                    return TB_PS_INSIDE;
            }
            
            return above > 0 ? TB_PS_ABOVE : TB_PS_BELOW;
        }
    }
}