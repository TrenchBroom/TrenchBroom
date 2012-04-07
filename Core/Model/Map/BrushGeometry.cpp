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
#include <cassert>
#include <cmath>
#include <map>
#include <algorithm>
#include "VecMath.h"

namespace TrenchBroom {
    namespace Model {
        Vertex::Vertex(float x, float y, float z) {
            position.x = x;
            position.y = y;
            position.z = z;
            mark = VM_NEW;
        }
        
        Vertex::Vertex() {
            mark = VM_NEW;
        }
        
        Edge::Edge(Vertex* start, Vertex* end) : start(start), end(end), mark((EM_NEW)) {}
        Edge::Edge() : mark(EM_NEW) {}
        
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
            
            if (start->mark == VM_KEEP) keep++;
            else if (start->mark == VM_DROP) drop++;
            else if (start->mark == VM_UNDECIDED) undecided++;
            
            if (end->mark == VM_KEEP) keep++;
            else if (end->mark == VM_DROP) drop++;
            else if (end->mark == VM_UNDECIDED) undecided++;
            
            if (keep == 1 && drop == 1) mark = EM_SPLIT;
            else if (keep > 0) mark = EM_KEEP;
            else if (drop > 0) mark = EM_DROP;
            else mark = EM_UNDECIDED;
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
            newVertex->mark = VM_NEW;
            
            if (start->mark == VM_DROP) start = newVertex;
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
        
        Side::Side(Edge* edges[], bool invert[], int count) {
            for (int i = 0; i < count; i++) {
                this->edges.push_back(edges[i]);
                if (invert[i]) {
                    edges[i]->left = this;
                    vertices.push_back(edges[i]->end);
                } else {
                    edges[i]->right = this;
                    vertices.push_back(edges[i]->start);
                }
            }
            
            face = NULL;
            mark = SM_NEW;
        }
        
        Side::Side(Face& face, vector<Edge*>& edges) {
            for (int i = 0; i < edges.size(); i++) {
                edges[i]->left = this;
                edges.push_back(edges[i]);
                vertices.push_back(edges[i]->startVertex(this));
            }
            
            this->face = &face;
            this->face->setSide(this);
            mark = SM_NEW;
        }
        
        void Side::replaceEdges(int index1, int index2, Edge* edge) {
            vector<Edge*> newEdges;
            vector<Vertex*> newVertices;
            
            if (index2 > index1) {
                for (int i = 0; i <= index1; i++) {
                    newEdges.push_back(edges[i]);
                    newVertices.push_back(edges[i]->startVertex(this));
                }
                
                newEdges.push_back(edge);
                newVertices.push_back(edge->startVertex(this));
                
                for (int i = index2; i < edges.size(); i++) {
                    newEdges.push_back(edges[i]);
                    newVertices.push_back(edges[i]->startVertex(this));
                }
            } else {
                for (int i = index2; i <= index1; i++) {
                    newEdges.push_back(edges[i]);
                    newVertices.push_back(edges[i]->startVertex(this));
                }
                
                newEdges.push_back(edge);
                newVertices.push_back(edge->startVertex(this));
            }
            
            edges = newEdges;
            vertices = newVertices;
        }
        
        Edge* Side::split() {
            int keep = 0;
            int drop = 0;
            int split = 0;
            int undecided = 0;
            Edge* undecidedEdge = NULL;
            
            int splitIndex1 = -2;
            int splitIndex2 = -2;
            
            Edge* edge = edges.back();
            EEdgeMark lastMark = edge->mark;
            for (int i = 0; i < edges.size(); i++) {
                edge = edges[i];
                EEdgeMark currentMark = edge->mark;
                if (currentMark == EM_SPLIT) {
                    Vertex* start = edge->startVertex(this);
                    if (start->mark == VM_KEEP)
                        splitIndex1 = i;
                    else
                        splitIndex2 = i;
                    split++;
                } else if (currentMark == EM_UNDECIDED) {
                    undecided++;
                    undecidedEdge = edge;
                } else if (currentMark == EM_KEEP) {
                    if (lastMark == EM_DROP)
                        splitIndex2 = i;
                    keep++;
                } else if (currentMark == EM_DROP) {
                    if (lastMark == EM_KEEP)
                        splitIndex1 = i > 0 ? i - 1 : edges.size() - 1;
                    drop++;
                }
                lastMark = currentMark;
            }
            
            if (keep == edges.size()) {
                mark = SM_KEEP;
                return NULL;
            }
            
            if (undecided == 1 && keep == edges.size() - 1) {
                mark = SM_KEEP;
                return undecidedEdge;
            }
            
            if (drop + undecided == edges.size()) {
                mark = SM_DROP;
                return NULL;
            }
            
            assert(splitIndex1 >= 0 && splitIndex2 >= 0);
            mark = SM_SPLIT;
            
            Edge* newEdge = new Edge();
            newEdge->start = edges[splitIndex1]->endVertex(this);
            newEdge->end = edges[splitIndex2]->startVertex(this);
            newEdge->left = NULL;
            newEdge->right = this;
            newEdge->mark = EM_NEW;
            
            replaceEdges(splitIndex1, splitIndex2, newEdge);
            return newEdge;
        }
        
        void Side::flip() {
            Vertex* tempVertex;
            int j;
            for (int i = 0; i < vertices.size() / 2; i++) {
                j = vertices.size() - i - 1;
                tempVertex = vertices[i];
                vertices[i] = vertices[j];
                vertices[j] = tempVertex;
            }
        }
        
        void Side::shift(int offset) {
            vector<Edge*> newEdges;
            vector<Vertex*> newVertices;
            int count;
            
            if (offset == 0)
                return;
            
            count = edges.size();
            for (int i = 0; i < count; i++) {
                int index = (i + offset + count) % count;
                newEdges.push_back(edges[index]);
                newVertices.push_back(vertices[index]);
            }
            
            edges = newEdges;
            vertices = newVertices;
        }
        
        vector<Side*> BrushGeometry::incidentSides(int vertexIndex) {
            vector<Side*> result;
            Vertex* vertex = vertices[vertexIndex];
            
            // find any edge that is incident to vertex
            Edge* edge = NULL;
            for (int i = 0; i < edges.size() && edge == NULL; i++) {
                Edge* candidate = edges[i];
                if (candidate->start == vertex || candidate->end == vertex)
                    edge = candidate;
            }
            
            Side* side = edge->start == vertex ? edge->right : edge->left;
            do {
                result.push_back(side);
                int i = indexOf<Edge>(side->edges, edge);
                edge = side->edges[(i - 1 + side->edges.size()) % side->edges.size()];
                side = edge->start == vertex ? edge->right : edge->left;
            } while (side != result[0]);
            
            return result;
        }
        
        void BrushGeometry::deleteDegenerateTriangle(Side* side, Edge* edge, vector<Face*>& newFaces, vector<Face*>& droppedFaces) {
            assert(side->edges.size() == 3);
            
            side->shift(indexOf(side->edges, edge));
            
            Edge* keepEdge = side->edges[1];
            Edge* dropEdge = side->edges[2];
            Side* neighbour = dropEdge->left == side ? dropEdge->right : dropEdge->left;
            
            if (keepEdge->left == side)
                keepEdge->left = neighbour;
            else
                keepEdge->right = neighbour;
            
            int deleteIndex = indexOf(neighbour->edges, dropEdge);
            int prevIndex = (deleteIndex - 1 + neighbour->edges.size()) % neighbour->edges.size();
            int nextIndex = (deleteIndex + 1) % neighbour->edges.size();
            neighbour->replaceEdges(prevIndex, nextIndex, keepEdge);
            
            vector<Face*>::iterator faceIt = find(newFaces.begin(), newFaces.end(), side->face);
            if (faceIt == newFaces.end()) droppedFaces.push_back(side->face);
            else newFaces.erase(faceIt);    
            
            deleteElement(sides, side);
            deleteElement(edges, dropEdge);
        }
        
        void BrushGeometry::triangulateSide(Side* side, int vertexIndex, vector<Face*>& newFaces) {
            Side* newSide;
            Vertex* vertex = vertices[vertexIndex];
            int sideVertexIndex = indexOf<Vertex>(side->vertices, vertex);
            assert(sideVertexIndex >= 0);
            
            Edge* sideEdges[3];
            bool flipped[3];
            sideEdges[0] = side->edges[sideVertexIndex];
            flipped[0] = sideEdges[0]->left == side;
            sideEdges[1] = side->edges[(sideVertexIndex + 1) % side->edges.size()];
            flipped[1] = sideEdges[1]->left == side;
            
            for (int i = 0; i < side->edges.size() - 3; i++) {
                sideEdges[2] = new Edge();
                sideEdges[2]->start = side->vertices[(sideVertexIndex + 2) % side->vertices.size()];
                sideEdges[2]->end = vertex;
                sideEdges[2]->left= NULL;
                sideEdges[2]->right = NULL;
                sideEdges[2]->mark = EM_NEW;
                flipped[2] = false;
                edges.push_back(sideEdges[2]);
                
                newSide = new Side(sideEdges, flipped, 3);
                newSide->face = new Face(side->face->worldBounds(), *side->face);
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
            newSide->face = new Face(side->face->worldBounds(), *side->face);
            sides.push_back(newSide);
            newFaces.push_back(newSide->face);
        }
        
        void BrushGeometry::splitSide(Side* side, int vertexIndex, vector<Face*>& newFaces) {
            Side* newSide;
            Vertex* vertex = vertices[vertexIndex];
            int sideVertexIndex = indexOf<Vertex>(side->vertices, vertex);
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
            sideEdges[2]->mark = EM_NEW;
            flipped[2] = true;
            edges.push_back(sideEdges[2]);
            side->replaceEdges((sideVertexIndex + side->edges.size() - 2) % side->edges.size(), (sideVertexIndex + 1) % side->edges.size(), sideEdges[2]);
            
            newSide = new Side(sideEdges, flipped, 3);
            newSide->face = new Face(side->face->worldBounds(), *side->face);
            sides.push_back(newSide);
            newFaces.push_back(newSide->face);
            
        }
        
        void BrushGeometry::splitSides(vector<Side*>& sides, Ray ray, int vertexIndex, vector<Face*>& newFaces, vector<Face*>& droppedFaces) {
            Vec3f v1, v2;
            
            for (int i = 0; i < sides.size(); i++) {
                Side* side = sides[i];
                if (side->vertices.size() > 3) {
                    v1 = side->vertices[2]->position - side->vertices[0]->position;
                    v2 = side->vertices[1]->position - side->vertices[0]->position;
                    v1 = v1 % v2;
                    
                    if ((v1 | ray.direction) <= -AlmostZero) {
                        splitSide(side, vertexIndex, newFaces);
                    } else {
                        triangulateSide(side, vertexIndex, newFaces);
                        vector<Face*>::iterator faceIt = find(newFaces.begin(), newFaces.end(), side->face);
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
        
        void BrushGeometry::mergeVertices(Vertex* keepVertex, Vertex* dropVertex, vector<Face*>& newFaces, vector<Face*>& droppedFaces) {
            // find the edge incident to both vertex and candidate
            Edge* dropEdge = NULL;
            for (int j = 0; j < edges.size() && dropEdge == NULL; j++) {
                Edge* edge =edges[j];
                if ((edge->start == keepVertex && edge->end == dropVertex) ||
                    (edge->end == keepVertex && edge->start == dropVertex))
                    dropEdge = edge;
            }
            
            // because the algorithm should not allow non-adjacent vertices to be merged in the first place
            assert(dropEdge != NULL); 
            assert(dropEdge->left->vertices.size() == 3);
            assert(dropEdge->right->vertices.size() == 3);
            
            for (int j = 0; j < edges.size(); j++) {
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
            
            for (int i = 0; i < edges.size(); i++) {
                Edge* edge = edges[i];
                v1 = edge->vector();
                for (int j = i + 1; j < edges.size(); j++) {
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
                            
                            int leftIndex = indexOf<Edge>(leftSide->edges, candidate);
                            int leftCount = leftSide->edges.size();
                            int rightIndex = indexOf<Edge>(rightSide->edges, candidate);
                            int rightCount = rightSide->edges.size();
                            
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
                            
                            int leftIndex = indexOf<Edge>(leftSide->edges, candidate);
                            int leftCount = leftSide->edges.size();
                            int rightIndex = indexOf<Edge>(rightSide->edges, candidate);
                            int rightCount = rightSide->edges.size();
                            
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
        
        void BrushGeometry::mergeNeighbours(Side* side, int edgeIndex) {
            Vertex* vertex;
            Edge* edge = side->edges[edgeIndex];
            Side* neighbour = edge->left != side ? edge->left : edge->right;
            int sideEdgeIndex = edgeIndex;
            int neighbourEdgeIndex = indexOf<Edge>(neighbour->edges, edge);
            
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
            
            for (int i = 0; i < neighbour->edges.size() - count; i++) {
                edge = neighbour->edges[i];
                vertex = neighbour->vertices[i];
                if (edge->left == neighbour)
                    edge->left = side;
                else
                    edge->right = side;
                side->edges.push_back(edge);
                side->vertices.push_back(vertex);
            }
            
            for (int i = neighbour->edges.size() - count; i < neighbour->edges.size(); i++) {
                deleteElement<Edge>(edges, neighbour->edges[i]);
                if (i > neighbour->edges.size() - count)
                    deleteElement<Vertex>(vertices, neighbour->vertices[i]);
            }
            
            neighbour->face->setSide(NULL);
            deleteElement<Side>(sides, neighbour);
        }
        
        void BrushGeometry::mergeSides(vector<Face*>& newFaces, vector<Face*>&droppedFaces) {
            for (int i = 0; i < sides.size(); i++) {
                Side* side = sides[i];
                Plane sideBoundary;
                sideBoundary.setPoints(side->vertices[0]->position, 
                                       side->vertices[1]->position, 
                                       side->vertices[2]->position);
                
                for (int j = 0; j < side->edges.size(); j++) {
                    Edge* edge = side->edges[j];
                    Side* neighbour = edge->left != side ? edge->left : edge->right;
                    Plane neighbourBoundary;
                    neighbourBoundary.setPoints(neighbour->vertices[0]->position, 
                                                neighbour->vertices[1]->position, 
                                                neighbour->vertices[2]->position);
                    
                    if (sideBoundary.equals(neighbourBoundary)) {
                        Face* neighbourFace = neighbour->face;
                        mergeNeighbours(side, j);
                        
                        vector<Face*>::iterator faceIt = find(newFaces.begin(), newFaces.end(), neighbourFace);
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
        
        float BrushGeometry::minVertexMoveDist(const vector<Side*>& sides, const Vertex* vertex, Ray ray, float maxDist) {
            float minDist;
            Plane plane;
            
            minDist = maxDist;
            for (int i = 0; i < sides.size(); i++) {
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
                
                plane = neighbourSide->face->boundary();
                float neighbourDist = plane.intersectWithRay(ray);
                
                if (!isnan(sideDist) && sideDist >= AlmostZero && sideDist < minDist - AlmostZero)
                    minDist = sideDist;
                if (!isnan(neighbourDist) && neighbourDist >= AlmostZero && neighbourDist < minDist - AlmostZero)
                    minDist = neighbourDist;
            }
            
            return minDist;
        }
        
        MoveResult BrushGeometry::moveVertex(int vertexIndex, bool mergeIncidentVertex, Vec3f delta, vector<Face*>& newFaces, vector<Face*>& droppedFaces) {
            Vertex* vertex;
            Vec3f newPosition;
            Ray ray;
            float actualMoveDist, moveDist, dot1, dot2;
            vector<Side*> incSides;
            int actualVertexIndex;
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
            for (int i = 0; i < edges.size(); i++) {
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
            for (int i = 0; i < vertices.size(); i++) {
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
            vertexIndex = indexOf(vertices, newPosition);
            
            // drag is concluded
            if (vertexIndex == -1 || actualMoveDist == moveDist) {
                for (int i = 0; i < vertices.size(); i++)
                    vertices[i]->position = vertices[i]->position.snap();
                for (int i = 0; i < sides.size(); i++)
                    sides[i]->face->updatePoints();
                
                result.moved = true;
                result.index = vertexIndex;
                return result;
            }
            
            // drag is not concluded, calculate the new delta and call self
            ray.direction *= (moveDist - actualMoveDist);
            return moveVertex(vertexIndex, mergeIncidentVertex, ray.direction, newFaces, droppedFaces);
        }
        
        MoveResult BrushGeometry::splitAndMoveEdge(int index, Vec3f delta, vector<Face*>& newFaces, vector<Face*>& droppedFaces) {
            Edge* edge;
            Vertex* vertex;
            Vec3f edgeVertices[2];
            Vec3f leftNorm, rightNorm;
            MoveResult result;
            int edgeIndex;
            
            edgeIndex = index - vertices.size();
            edge = edges[edgeIndex];
            
            // detect whether the drag would make the incident faces invalid
            leftNorm = edge->left->face->boundary().normal;
            rightNorm = edge->right->face->boundary().normal;
            if ((delta | leftNorm) <= -AlmostZero || 
                (delta | rightNorm) <= -AlmostZero) {
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
        
        MoveResult BrushGeometry::splitAndMoveSide(int sideIndex, Vec3f delta, vector<Face*>& newFaces, vector<Face*>& droppedFaces) {
            Side* side;
            Vertex* vertex;
            vector<Vec3f> sideVertices;
            Vec3f norm;
            MoveResult result;    
            int index;
            
            index = sideIndex - edges.size() - vertices.size();
            side = sides[index];
            
            // detect whether the drag would lead to an indented face
            norm = side->face->boundary().normal;
            if ((delta | norm) < AlmostZero) {
                result.moved = false;
                result.index = sideIndex;
                return result;
            }
            
            // store the side's vertices for later
            for (int i = 0; i < side->vertices.size(); i++)
                sideVertices.push_back(side->vertices[i]->position);
            
            vertex = new Vertex();
            vertex->position = centerOfVertices(side->vertices);
            vertices.push_back(vertex);
            
            Edge* firstEdge = new Edge(vertex, side->edges[0]->startVertex(side));
            edges.push_back(firstEdge);
            
            Edge* lastEdge = firstEdge;
            for (int i = 0; i < side->edges.size(); i++) {
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
                
                newSide->face = new Face(side->face->worldBounds(), *side->face);
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
            map<Vertex*, Vertex*> vertexMap;
            map<Edge*, Edge*> edgeMap;
            map<Side*, Side*> sideMap;
            
            vertices.reserve(original.vertices.size());
            edges.reserve(original.edges.size());
            sides.reserve(original.sides.size());
            
            for (int i = 0; i < original.vertices.size(); i++) {
                Vertex* originalVertex = original.vertices[i];
                Vertex* copyVertex = new Vertex(*originalVertex);
                vertexMap[originalVertex] = copyVertex;
                vertices.push_back(copyVertex);
            }
            
            for (int i = 0; i < original.edges.size(); i++) {
                Edge* originalEdge = original.edges[i];
                Edge* copyEdge = new Edge(*originalEdge);
                copyEdge->start = vertexMap[originalEdge->start];
                copyEdge->end = vertexMap[originalEdge->end];
                edgeMap[originalEdge] = copyEdge;
                edges.push_back(copyEdge);
            }
            
            for (int i = 0; i < original.sides.size(); i++) {
                Side* originalSide = original.sides[i];
                Side* copySide = new Side(*originalSide);
                copySide->vertices.clear();
                copySide->edges.clear();
                
                for (int j = 0; j < originalSide->edges.size(); j++) {
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
            for (int i = 0; i < sides.size(); i++)
                if (sides[i]->face == NULL)
                    return false;
            return true;
        }
        
        void BrushGeometry::restoreFaceSides() {
            for (int i = 0; i < sides.size(); i++)
                sides[i]->face->setSide(sides[i]);
        }
        
        ECutResult BrushGeometry::addFace(Face& face, vector<Face*>& droppedFaces) {
            Plane boundary = face.boundary();
            
            int keep = 0;
            int drop = 0;
            int undecided = 0;
            
            // mark vertices
            for (int i = 0; i < vertices.size(); i++) {
                Vertex& vertex = *vertices[i];
                EPointStatus vs = boundary.pointStatus(vertex.position);
                if (vs == PS_ABOVE) {
                    vertex.mark = VM_DROP;
                    drop++;
                } else if (vs == PS_BELOW) {
                    vertex.mark  = VM_KEEP;
                    keep++;
                } else {
                    vertex.mark = VM_UNDECIDED;
                    undecided++;
                }
            }
            
            if (keep + undecided == vertices.size())
                return CR_REDUNDANT;
            
            if (drop + undecided == vertices.size())
                return CR_NULL;
            
            // mark and split edges
            for (int i = 0; i < edges.size(); i++) {
                Edge& edge = *edges[i];
                edge.updateMark();
                if (edge.mark == EM_SPLIT) {
                    Vertex* vertex = edge.split(boundary);
                    vertices.push_back(vertex);
                }
            }
            
            // mark, split and drop sides
            vector<Edge*> newEdges;
            vector<Side*>::iterator sideIt;
            for (sideIt = sides.begin(); sideIt != sides.end(); sideIt++) {
                Side* side = *sideIt;
                Edge* newEdge = side->split();
                
                if (side->mark == SM_DROP) {
                    Face* face = side->face;
                    if (face != NULL) {
                        droppedFaces.push_back(face);
                        face->setSide(NULL);
                    }
                    delete side;
                    sideIt = sides.erase(sideIt);
                } else if (side->mark == SM_SPLIT) {
                    edges.push_back(newEdge);
                    newEdges.push_back(newEdge);
                    side->mark = SM_UNKNOWN;
                } else if (side->mark == SM_KEEP && newEdge != NULL) {
                    // the edge is an undecided edge, so it needs to be flipped in order to act as a new edge
                    if (newEdge->right != side)
                        newEdge->flip();
                    newEdges.push_back(newEdge);
                    side->mark = SM_UNKNOWN;
                } else {
                    side->mark = SM_UNKNOWN;
                }
            }
            
            // create new side from newly created edges
            // first, sort the new edges to form a polygon in clockwise order
            for (int i = 0; i < newEdges.size() - 1; i++) {
                Edge* edge = newEdges[i];
                for (int j = i + 2; j < newEdges.size(); j++) {
                    Edge* candidate = newEdges[j];
                    if (edge->start == candidate->end) {
                        newEdges[j] = newEdges[i + 1];
                        newEdges[i + 1] = candidate;
                    }
                }
            }
            
            // now create the new side
            Side* newSide = new Side(face, newEdges);
            sides.push_back(newSide);
            
            // clean up
            // delete dropped vertices
            vector<Vertex*>::iterator vertexIt;
            for (vertexIt = vertices.begin(); vertexIt != vertices.end(); vertexIt++) {
                Vertex* vertex = *vertexIt;
                if (vertex->mark == VM_DROP) {
                    delete vertex;
                    vertexIt = vertices.erase(vertexIt);
                } else {
                    vertex->mark = VM_UNDECIDED;
                }
            }
            
            // delete dropped edges
            vector<Edge*>::iterator edgeIt;
            for (edgeIt = edges.begin(); edgeIt != edges.end(); edgeIt++) {
                Edge* edge = *edgeIt;
                if (edge->mark == EM_DROP) {
                    delete edge;
                    edgeIt = edges.erase(edgeIt);
                } else {
                    edge->mark = EM_UNDECIDED;
                }
            }
            
            bounds = boundsOfVertices(vertices);
            return CR_SPLIT;
        }
        
        bool BrushGeometry::addFaces(vector<Face*>& faces, vector<Face*>& droppedFaces) {
            for (int i = 0; i < faces.size(); i++)
                if (addFace(*faces[i], droppedFaces) == CR_NULL)
                    return false;
            return true;
        }
        
        void BrushGeometry::translate(Vec3f delta) {
            for (int i = 0; i < vertices.size(); i++)
                vertices[i]->position += delta;
            bounds = bounds.translate(delta);
        }
        
        void BrushGeometry::rotate90CW(EAxis axis, Vec3f center) {
            for (int i = 0; i < vertices.size(); i++)
                vertices[i]->position = vertices[i]->position.rotate90(axis, center, true);
            bounds = bounds.rotate90(axis, center, true);
        }
        
        void BrushGeometry::rotate90CCW(EAxis axis, Vec3f center) {
            for (int i = 0; i < vertices.size(); i++)
                vertices[i]->position = vertices[i]->position.rotate90(axis, center, false);
            bounds = bounds.rotate90(axis, center, false);
        }
        
        void BrushGeometry::rotate(Quat rotation, Vec3f center) {
            for (int i = 0; i < vertices.size(); i++)
                vertices[i]->position = rotation * (vertices[i]->position - center) + center;
            bounds = bounds.rotate(rotation, center);
        }
        
        void BrushGeometry::flip(EAxis axis, Vec3f center) {
            for (int i = 0; i < vertices.size(); i++)
                vertices[i]->position = vertices[i]->position.flip(axis, center);
            bounds = bounds.flip(axis, center);

            for (int i = 0; i < edges.size(); i++)
                edges[i]->flip();
            for (int i = 0; i < sides.size(); i++)
                sides[i]->flip();
        }
        
        void BrushGeometry::snap() {
        }
        
        MoveResult BrushGeometry::moveVertex(int vertexIndex, Vec3f delta, vector<Face*>& newFaces, vector<Face*>& droppedFaces) {
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
        
        MoveResult BrushGeometry::moveEdge(int edgeIndex, Vec3f delta, vector<Face*>& newFaces, vector<Face*>& droppedFaces) {
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
        
        MoveResult BrushGeometry::moveSide(int sideIndex, Vec3f delta, vector<Face*>& newFaces, vector<Face*>& droppedFaces) {
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
            
            int sideVertexCount = side->vertices.size();
            vector<Vec3f> sideVertices(sideVertexCount);
            vector<int> indices(sideVertexCount);
            vector<float> dots(sideVertexCount);
            for (int i = 0; i < sideVertexCount; i++) {
                sideVertices[i] = side->vertices[i]->position;
                Vec3f diff = sideVertices[i] - center;
                dots[i] = diff | dir;
                indices[i] = indexOf<Vertex>(testGeometry.vertices, side->vertices[i]);
                sideVertices[i] += delta;
            }
            
            // sort indices by dot value, eek, bubblesort
            bool switched = true;
            for (int j = sideVertexCount - 1; j >= 0 && switched; j--) {
                switched = false;
                for (int i = 0; i < j; i++) {
                    if (dots[i] > dots[i + 1]) {
                        float dt = dots[i];
                        dots[i] = dots[i + 1];
                        dots[i + 1] = dt;
                        
                        int di = indices[i];
                        indices[i] = indices[i + 1];
                        indices[i + 1] = di;
                        switched = true;
                    }
                }
            }
            
            MoveResult result(-1, true);
            for (int i = 0; i < sideVertexCount && result.moved; i++)
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
        
        template <class T> int indexOf(const vector<T*>& vec, const T* element) {
            for (int i = 0; i < vec.size(); i++)
                if (vec[i] == element)
                    return i;
            return -1;
        }
        
        template <class T> bool removeElement(vector<T*>& vec, T* element) {
            typename vector<T*>::iterator elementIt = find(vec.begin(), vec.end(), element);
            if (elementIt == vec.end())
                return false;
            vec.erase(elementIt);
            return true;
        }
        
        template <class T> bool deleteElement(vector<T*>& vec, T* element) {
            if (!removeElement(vec, element))
                return false;
            delete element;
            return true;
        }
        
        int indexOf(const vector<Vertex*>& vertices, Vec3f v) {
            for (int i = 0; i < vertices.size(); i++)
                if (vertices[i]->position.equals(v)) return i;
            return -1;
        }
        
        int indexOf(const vector<Edge*>& edges, Vec3f v1, Vec3f v2) {
            for (int i = 0; i < edges.size(); i++) {
                Edge* edge = edges[i];
                if ((edge->start->position.equals(v1) && edge->end->position.equals(v2)) ||
                    (edge->start->position.equals(v2) && edge->end->position.equals(v1)))
                    return i;
            }
            return -1;
        }
        
        int indexOf(const vector<Side*>& sides, const vector<Vec3f>& vertices) {
            for (int i = 0; i < sides.size(); i++) {
                Side* side = sides[i];
                if (side->vertices.size() == vertices.size()) {
                    for (int j = 0; j < vertices.size(); j++) {
                        int k = 0;
                        while (k < vertices.size() && side->vertices[(j + k) % vertices.size()]->position.equals(vertices[k]))
                            k++;
                        
                        if (k == vertices.size())
                            return i;
                    }
                }
            }
            
            return -1;    
        }
        
        Vec3f centerOfVertices(const vector<Vertex*>& vertices) {
            Vec3f center = vertices[0]->position;
            for (int i = 1; i < vertices.size(); i++)
                center += vertices[i]->position;
            center /= vertices.size();
            return center;
        }
        
        BBox boundsOfVertices(const vector<Vertex*>& vertices) {
            BBox bounds;
            bounds.min = vertices[0]->position;
            bounds.max = vertices[0]->position;
            
            for  (int i = 1; i < vertices.size(); i++)
                bounds += vertices[i]->position;
            return bounds;
        }
        
        EPointStatus vertexStatusFromRay(Vec3f origin, Vec3f direction, const vector<Vertex*>& vertices) {
            Ray ray(origin, direction);
            int above = 0;
            int below = 0;
            for (int i = 0; i < vertices.size(); i++) {
                EPointStatus status = ray.pointStatus(vertices[i]->position); 
                if (status == PS_ABOVE)
                    above++;
                else if (status == PS_BELOW)
                    below++;
                if (above > 0 && below > 0)
                    return PS_INSIDE;
            }
            
            return above > 0 ? PS_ABOVE : PS_BELOW;
        }
    }
}