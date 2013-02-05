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

#include "Model/Face.h"

#include <map>
#include <cstdio>

namespace TrenchBroom {
    namespace Model {
        void Edge::updateMark() {
            unsigned int keep = 0;
            unsigned int drop = 0;
            unsigned int undecided = 0;

            if (start->mark == Vertex::Keep)
                keep++;
            else if (start->mark == Vertex::Drop)
                drop++;
            else if (start->mark == Vertex::Undecided)
                undecided++;

            if (end->mark == Vertex::Keep)
                keep++;
            else if (end->mark == Vertex::Drop)
                drop++;
            else if (end->mark == Vertex::Undecided)
                undecided++;

            assert(keep + drop + undecided == 2);

            if (keep == 1 && drop == 1)
                mark = Split;
            else if (keep > 0)
                mark = Keep;
            else if (drop > 0)
                mark = Drop;
            else
                mark = Undecided;
        }

        Vertex* Edge::split(const Plane& plane) {
            Line line(start->position, (end->position - start->position).normalized());
            Vertex* newVertex = new Vertex();

            float dist = plane.intersectWithLine(line);
            newVertex->position = line.pointAtDistance(dist).snapped();
            newVertex->mark = Vertex::New;

            if (start->mark == Vertex::Drop)
                start = newVertex;
            else
                end = newVertex;

            return newVertex;
        }

        Side::Side(Edge* newEdges[], bool invert[], unsigned int count) :
        face(NULL),
        mark(Side::New) {
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

        Side::Side(Face& i_face, EdgeList& newEdges) :
        face(&i_face),
        mark(Side::New) {
            vertices.reserve(newEdges.size());
            edges.reserve(newEdges.size());
            for (unsigned int i = 0; i < newEdges.size(); i++) {
                Edge* edge = newEdges[i];
                edge->left = this;
                edges.push_back(edge);
                vertices.push_back(edge->startVertex(this));
            }

            this->face->setSide(this);
        }

		Side::~Side() {
			vertices.clear();
			edges.clear();
			face = NULL;
			mark = Side::Drop;
		}

        float Side::intersectWithRay(const Ray& ray) {
            if (face == NULL)
                return Math::nan();

            const Plane& boundary = face->boundary();
            float dot = boundary.normal.dot(ray.direction);
            if (!Math::neg(dot))
                return Math::nan();

            float dist = boundary.intersectWithRay(ray);
            if (Math::isnan(dist))
                return Math::nan();

            Vec3f hit, projectedHit, v0, v1;
            CoordinatePlane cPlane = CoordinatePlane::plane(boundary.normal);

            hit = ray.pointAtDistance(dist);
            cPlane.project(hit, projectedHit);

            const Vertex* vertex = vertices.back();
            cPlane.project(vertex->position, v0);
            v0 -= projectedHit;

            int c = 0;
            for (unsigned int i = 0; i < vertices.size(); i++) {
                vertex = vertices[i];
                cPlane.project(vertex->position, v1);
                v1 -= projectedHit;

                if ((Math::zero(v0.x) && Math::zero(v0.y)) || (Math::zero(v1.x) && Math::zero(v1.y))) {
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

            if (c % 2 == 0)
                return Math::nan();
            return dist;
        }

        void Side::replaceEdges(size_t index1, size_t index2, Edge* edge) {
            VertexList::iterator vIt1, vIt2;
            EdgeList::iterator eIt1, eIt2;
            
            if (index2 > index1) {
                std::advance(vIt1 = vertices.begin(), index1 + 1);
                std::advance(vIt2 = vertices.begin(), index2 + 1);
                vertices.erase(vIt1, vIt2);
                
                std::advance(eIt1 = edges.begin(), index1 + 1);
                std::advance(eIt2 = edges.begin(), index2);
                edges.erase(eIt1, eIt2);

                std::advance(vIt1 = vertices.begin(), index1 + 1);
                vertices.insert(vIt1, edge->startVertex(this));
                
                std::advance(vIt1 = vertices.begin(), index1 + 2);
                vertices.insert(vIt1, edge->endVertex(this));
                assert(edge->startVertex(this) == vertices[index1 + 1]);
                assert(edge->endVertex(this) == vertices[index1 + 2]);
                
                std::advance(eIt1 = edges.begin(), index1 + 1);
                edges.insert(eIt1, edge);
            } else {
                std::advance(vIt1 = vertices.begin(), index1 + 1);
                vertices.erase(vIt1, vertices.end());
                
                std::advance(vIt2 = vertices.begin(), index2 + 1);
                vertices.erase(vertices.begin(), vIt2);
                
                std::advance(eIt1 = edges.begin(), index1 + 1);
                edges.erase(eIt1, edges.end());
                
                std::advance(eIt2 = edges.begin(), index2);
                edges.erase(edges.begin(), eIt2);
                
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
            Edge::Mark lastMark = edge->mark;
            for (unsigned int i = 0; i < edges.size(); i++) {
                edge = edges[i];
                Edge::Mark currentMark = edge->mark;
                if (currentMark == Edge::Split) {
                    Vertex* start = edge->startVertex(this);
                    if (start->mark == Vertex::Keep)
                        splitIndex1 = static_cast<int>(i);
                    else
                        splitIndex2 = static_cast<int>(i);
                    split++;
                } else if (currentMark == Edge::Undecided) {
                    undecided++;
                    undecidedEdge = edge;
                } else if (currentMark == Edge::Keep) {
                    if (lastMark == Edge::Drop)
                        splitIndex2 = static_cast<int>(i);
                    keep++;
                } else if (currentMark == Edge::Drop) {
                    if (lastMark == Edge::Keep)
                        splitIndex1 = i > 0 ? static_cast<int>(i) - 1 : static_cast<int>(edges.size() - 1);
                    drop++;
                }
                lastMark = currentMark;
            }

            if (keep == edges.size()) {
                mark = Side::Keep;
                return NULL;
            }

            if (undecided == 1 && keep == edges.size() - 1) {
                mark = Side::Keep;
                return undecidedEdge;
            }

            if (drop + undecided == edges.size()) {
                mark = Side::Drop;
                return NULL;
            }

            if (splitIndex1 < 0 || splitIndex2 < 0)
                throw GeometryException("Invalid brush detected during side split");

            assert(splitIndex1 >= 0 && splitIndex2 >= 0);

            mark = Side::Split;

            Edge* newEdge = new Edge();
            newEdge->start = edges[static_cast<size_t>(splitIndex1)]->endVertex(this);
            newEdge->end = edges[static_cast<size_t>(splitIndex2)]->startVertex(this);
            newEdge->left = NULL;
            newEdge->right = this;
            newEdge->mark = Edge::New;

            replaceEdges(static_cast<size_t>(splitIndex1), static_cast<size_t>(splitIndex2), newEdge);
            return newEdge;
        }
        
        void Side::chop(size_t index, Side*& newSide, Edge*& newEdge) {
            assert(vertices.size() > 3);
            assert(index < vertices.size());
            
            Vertex* nextVertex = vertices[succ(index, vertices.size())];
            Vertex* prevVertex = vertices[pred(index, vertices.size())];
            
            Edge* edge = edges[index];
            Edge* prevEdge = edges[pred(index, edges.size())];
            newEdge = new Edge(prevVertex, nextVertex, NULL, this);
            
            Edge* sideEdges[] = {prevEdge, edge, newEdge};
            bool flipped[] = {prevEdge->left == this, edge->left == this, true};

            newSide = new Side(sideEdges, flipped, 3);
            newSide->face = new Face(face->worldBounds(), *face);
            newSide->face->setSide(newSide);
            
            replaceEdges(pred(index, edges.size(), 2),
                         succ(index, edges.size()),
                         newEdge);
        }

        void Side::shift(size_t offset) {
            size_t count = edges.size();
            if (offset % count == 0)
                return;

            EdgeList newEdges;
            VertexList newVertices;

            for (size_t i = 0; i < count; i++) {
                size_t index = succ(i, count, offset);
                newEdges.push_back(edges[index]);
                newVertices.push_back(vertices[index]);
            }

            edges = newEdges;
            vertices = newVertices;
        }

        bool Side::isDegenerate() {
            Vec3f edgeVector, nextVector, cross;

            for (size_t i = 0; i < edges.size(); i++) {
                Edge* edge = edges[i];
                Edge* next = edges[succ(i, edges.size())];

                edgeVector = edge->vector(this);
                nextVector = next->vector(this);
                cross = nextVector.crossed(edgeVector);
                if (!Math::pos(cross.dot(face->boundary().normal)))
                    return true;
            }

            return false;
        }

        size_t Side::isCollinearTriangle() {
            assert(edges.size() >= 3);
            if (edges.size() > 3)
                return edges.size();

            Vec3f edgeVector1 = edges[0]->vector();
            Vec3f edgeVector2 = edges[1]->vector();

            if (edgeVector1.parallelTo(edgeVector2)) {
                Vec3f edgeVector3 = edges[2]->vector();
                assert(edgeVector1.parallelTo(edgeVector3));
                assert(edgeVector2.parallelTo(edgeVector3));

                float length1 = edgeVector1.lengthSquared();
                float length2 = edgeVector2.lengthSquared();
                float length3 = edgeVector3.lengthSquared();

                // we'll return the index of the longest of the three edges
                if (length1 > length2) {
                    if (length1 > length3)
                        return 0;
                    else
                        return 2;
                } else {
                    if (length2 > length3)
                        return 1;
                    else
                        return 2;
                }
            } else {
                Vec3f edgeVector3 = edges[2]->vector();
                assert(!edgeVector1.parallelTo(edgeVector3));
                assert(!edgeVector2.parallelTo(edgeVector3));

                return edges.size();
            }
        }

        BrushGeometry::FaceManager::~FaceManager() {
            CopyMap::iterator mapIt, mapEnd;
            for (mapIt = m_newFaces.begin(), mapEnd = m_newFaces.end(); mapIt != mapEnd; ++mapIt) {
                FaceSet& faces = mapIt->second;
                FaceSet::iterator faceIt, faceEnd;
                for (faceIt = faces.begin(), faceEnd = faces.end(); faceIt != faceEnd; ++faceIt)
                    delete *faceIt;
            }
        }

        void BrushGeometry::FaceManager::addFace(Face* original, Face* copy) {
            assert(original != NULL);
            assert(copy != NULL);
            assert(original != copy);
            m_newFaces[original].insert(copy);
        }
        
        void BrushGeometry::FaceManager::dropFace(Side* side) {
            assert(side != NULL);
            assert(side->face != NULL);
            
            CopyMap::iterator copyIt = m_newFaces.find(side->face);
            if (copyIt != m_newFaces.end()) {
                // the face is an original
                FaceSet& copies = copyIt->second;
				assert(!copies.empty());

				FaceSet::iterator faceIt = copies.begin();
                Face* copy = *faceIt;
                copies.erase(faceIt);
                
                Side* copySide = copy->side();
                copySide->face = side->face;
                copySide->face->setSide(copySide);
                
                if (copies.empty())
                    m_newFaces.erase(copyIt);
                
                delete copy;
            } else {
                bool wasCopy = false;
                CopyMap::iterator copyEnd;
                for (copyIt = m_newFaces.begin(), copyEnd = m_newFaces.end(); copyIt != copyEnd; ++copyIt) {
                    FaceSet& copies = copyIt->second;
					assert(!copies.empty());

					if (copies.erase(side->face) > 0) {
						wasCopy = true;
						if (copies.empty())
							m_newFaces.erase(copyIt);
						break;
					}
                }
                if (!wasCopy)
                    m_droppedFaces.insert(side->face);
            }
            side->face = NULL;
        }
        
        void BrushGeometry::FaceManager::getFaces(FaceSet& newFaces, FaceSet& droppedFaces) {
            newFaces.clear();
            
            CopyMap::const_iterator it, end;
            for (it = m_newFaces.begin(), end = m_newFaces.end(); it != end; ++it)
                newFaces.insert(it->second.begin(), it->second.end());
            droppedFaces = m_droppedFaces;

            m_newFaces.clear();
            m_droppedFaces.clear();
        }

        SideList BrushGeometry::incidentSides(Vertex* vertex) {
            SideList result;

            // find any edge that is incident to vertex
            Edge* edge = NULL;
            for (unsigned int i = 0; i < edges.size() && edge == NULL; i++) {
                Edge* candidate = edges[i];
                if (candidate->start == vertex || candidate->end == vertex)
                    edge = candidate;
            }

            // iterate over the incident sides in clockwise order
            Side* side = edge->start == vertex ? edge->right : edge->left;
            do {
                result.push_back(side);
                size_t i = findElement(side->edges, edge);
                edge = side->edges[pred(i, side->edges.size())];
                side = edge->start == vertex ? edge->right : edge->left;
            } while (side != result.front());

            return result;
        }

        void BrushGeometry::deleteDegenerateTriangle(Side* side, Edge* edge, FaceManager& faceManager) {
            assert(side->edges.size() == 3);

            side->shift(findElement(side->edges, edge));

            Edge* keepEdge = side->edges[1];
            Edge* dropEdge = side->edges[2];
            Side* neighbour = dropEdge->left == side ? dropEdge->right : dropEdge->left;

            if (keepEdge->left == side)
                keepEdge->left = neighbour;
            else
                keepEdge->right = neighbour;

            size_t deleteIndex = findElement(neighbour->edges, dropEdge);
            size_t prevIndex = pred(deleteIndex, neighbour->edges.size());
            size_t nextIndex = succ(deleteIndex, neighbour->edges.size());
            neighbour->replaceEdges(prevIndex, nextIndex, keepEdge);

            faceManager.dropFace(side);
            deleteElement(sides, side);
            deleteElement(edges, dropEdge);
        }

        void BrushGeometry::mergeEdges() {
            for (unsigned int i = 0; i < edges.size(); i++) {
                Edge* edge = edges[i];
                Vec3f edgeVector = edge->vector();
                for (unsigned int j = i + 1; j < edges.size(); j++) {
                    Edge* candidate = edges[j];
                    if (edge->incidentWith(candidate)) {
                        Vec3f candidateVector = candidate->vector();
                        if (edgeVector.parallelTo(candidateVector, 0.01f)) {
                            if (edge->end == candidate->end)
                                candidate->flip();
                            if (edge->end == candidate->start &&
                                edge->start != candidate->end) {
                                // we sometimes crash here because we meet two identical edges with opposite directions
                                assert(edge->start != candidate->end);
                                assert(edge->left == candidate->left);
                                assert(edge->right == candidate->right);
                                assert(edge->left->vertices.size() > 3);
                                assert(edge->right->vertices.size() > 3);

                                Side* leftSide = edge->left;
                                Side* rightSide = edge->right;

                                assert(leftSide != rightSide);

                                Edge* newEdge = new Edge(edge->start, candidate->end);
                                newEdge->left = leftSide;
                                newEdge->right = rightSide;
                                edges.push_back(newEdge);

                                size_t leftIndex = findElement(leftSide->edges, candidate);
                                size_t leftCount = leftSide->edges.size();
                                size_t rightIndex = findElement(rightSide->edges, candidate);
                                size_t rightCount = rightSide->edges.size();

                                leftSide->replaceEdges(pred(leftIndex, leftCount), succ(leftIndex, leftCount, 2), newEdge);
                                rightSide->replaceEdges(pred(rightIndex, rightCount, 2), succ(rightIndex, rightCount), newEdge);

                                deleteElement<Vertex>(vertices, candidate->start);
                                deleteElement<Edge>(edges, candidate);
                                deleteElement<Edge>(edges, edge);

                                break;
                            }

                            if (edge->start == candidate->start)
                                candidate->flip();
                            if (edge->start == candidate->end &&
                                edge->end != candidate->start) {
                                assert(edge->end != candidate->start);
                                assert(edge->left == candidate->left);
                                assert(edge->right == candidate->right);
                                assert(edge->left->vertices.size() > 3);
                                assert(edge->right->vertices.size() > 3);

                                Side* leftSide = edge->left;
                                Side* rightSide = edge->right;

                                assert(leftSide != rightSide);

                                Edge* newEdge = new Edge(candidate->start, edge->end);
                                newEdge->left = leftSide;
                                newEdge->right = rightSide;
                                edges.push_back(newEdge);

                                size_t leftIndex = findElement(leftSide->edges, candidate);
                                size_t leftCount = leftSide->edges.size();
                                size_t rightIndex = findElement(rightSide->edges, candidate);
                                size_t rightCount = rightSide->edges.size();

                                leftSide->replaceEdges(pred(leftIndex, leftCount, 2), succ(leftIndex, leftCount), newEdge);
                                rightSide->replaceEdges(pred(rightIndex, rightCount), succ(rightIndex, rightCount, 2), newEdge);

                                deleteElement<Vertex>(vertices, candidate->end);
                                deleteElement<Edge>(edges, candidate);
                                deleteElement<Edge>(edges, edge);

                                break;
                            }
                        }
                    }
                }
            }
        }

        void BrushGeometry::mergeNeighbours(Side* side, size_t edgeIndex, FaceManager& faceManager) {
            Vertex* vertex;
            Edge* edge = side->edges[edgeIndex];
            Side* neighbour = edge->left != side ? edge->left : edge->right;
            size_t sideEdgeIndex = edgeIndex;
            size_t neighbourEdgeIndex = findElement(neighbour->edges, edge);
            assert(neighbourEdgeIndex < neighbour->edges.size());

            do {
                sideEdgeIndex = succ(sideEdgeIndex, side->edges.size());
                neighbourEdgeIndex = pred(neighbourEdgeIndex, neighbour->edges.size());
            } while (side->edges[sideEdgeIndex] == neighbour->edges[neighbourEdgeIndex]);

            // now sideEdgeIndex points to the last edge (in CW order) of side that should not be deleted
            // and neighbourEdgeIndex points to the first edge (in CW order) of neighbour that should not be deleted

            int count = -1;
            do {
                sideEdgeIndex = pred(sideEdgeIndex, side->edges.size());
                neighbourEdgeIndex = succ(neighbourEdgeIndex, neighbour->edges.size());
                count++;
            } while (side->edges[sideEdgeIndex] == neighbour->edges[neighbourEdgeIndex]);

            // now sideEdgeIndex points to the first edge (in CW order) of side that should not be deleted
            // now neighbourEdgeIndex points to the last edge (in CW order) of neighbour that should not be deleted
            // and count is the number of shared edges between side and neighbour

            assert(count >= 0);
            size_t totalVertexCount = side->edges.size() + neighbour->edges.size() - static_cast<size_t>(2 * count);

            // shift the two sides so that their shared edges are at the end of both's edge lists
            side->shift(succ(sideEdgeIndex, side->edges.size(), static_cast<size_t>(count + 1)));
            neighbour->shift(neighbourEdgeIndex);

            side->edges.resize(side->edges.size() - static_cast<size_t>(count));
            side->vertices.resize(side->vertices.size() - static_cast<size_t>(count));

            for (size_t i = 0; i < neighbour->edges.size() - static_cast<size_t>(count); i++) {
                edge = neighbour->edges[i];
                vertex = neighbour->vertices[i];
                if (edge->left == neighbour)
                    edge->left = side;
                else
                    edge->right = side;
                side->edges.push_back(edge);
                side->vertices.push_back(vertex);
            }

            for (size_t i = neighbour->edges.size() - static_cast<size_t>(count); i < neighbour->edges.size(); i++) {
                bool success = deleteElement(edges, neighbour->edges[i]);
                assert(success);
                if (i > neighbour->edges.size() - static_cast<size_t>(count)) {
                    success = deleteElement(vertices, neighbour->vertices[i]);
                    assert(success);
                }
            }

            for (size_t i = 0; i < side->edges.size(); i++) {
                edge = side->edges[i];
                if (edge->left == side)
                    assert(edge->right != neighbour);
                else
                    assert(edge->left != neighbour);
            }

            faceManager.dropFace(neighbour);
            bool success = deleteElement<Side>(sides, neighbour);
            assert(success);

            assert(side->vertices.size() == totalVertexCount);
            assert(side->edges.size() == totalVertexCount);
        }

        void BrushGeometry::mergeSides(FaceManager& faceManager) {
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

                    if (sideBoundary.equals(neighbourBoundary, Math::ColinearEpsilon)) {
                        mergeNeighbours(side, j, faceManager);
                        i -= 1;
                        break;
                    }
                }
            }
        }

        MoveVertexResult BrushGeometry::moveVertex(Vertex* vertex, bool mergeWithAdjacentVertex, const Vec3f& start, const Vec3f& end, FaceManager& faceManager) {
            assert(vertex != NULL);
            assert(start != end);
            assert(sanityCheck());

            float lastFrac = 0.0f;
            while (!vertex->position.equals(end)) {
                const Vec3f lastPosition = vertex->position;
                SideList affectedSides = incidentSides(vertex);
                
                // First, we turn all sides incident to the vertex into triangles
                SideList::const_iterator sideIt, sideEnd;
                for (sideIt = affectedSides.begin(), sideEnd = affectedSides.end(); sideIt != sideEnd; ++sideIt) {
                    Side* side = *sideIt;
                    if (side->vertices.size() > 3) {
                        const Plane& boundary = side->face->boundary();
                        const float dot = end.dot(boundary.normal) - boundary.distance;
                        
                        if (Math::neg(dot)) {
                            // vertex will be moved below the boundary, so chop off one triangle
                            Side* newSide = NULL;
                            Edge* newEdge = NULL;
                            const size_t vertexIndex = findElement(side->vertices, vertex);
                            side->chop(vertexIndex, newSide, newEdge);
                            sides.push_back(newSide);
                            edges.push_back(newEdge);
                            faceManager.addFace(side->face, newSide->face);
                        } else {
                            // vertex will be moved above or parallel to the boundary, so create a triangle fan
                            for (unsigned int i = 1; i < side->vertices.size() - 1; i++) {
                                Side* newSide = NULL;
                                Edge* newEdge = NULL;
                                const size_t vertexIndex = findElement(side->vertices, vertex);
                                side->chop(succ(vertexIndex, side->vertices.size()), newSide, newEdge);
                                sides.push_back(newSide);
                                edges.push_back(newEdge);
                                faceManager.addFace(side->face, newSide->face);
                            }
                        }
                    }
                }
                affectedSides = incidentSides(vertex);
                
                // Now all sides incident to the vertex are triangles. We need to compute the next point to which the
                // vertex can be moved without making the brush convex. For that, we consider each incident side
                // and two of its neighbours: Its successor in the list of incident sides and its one neighbour that is
                // not incident to the vertex.
                
                float minFrac = 1.0f;
                for (size_t i = 0; i < affectedSides.size(); i++) {
                    Plane plane;
                    float startDot, endDot, frac;
                    
                    Side* side = affectedSides[i];
                    Side* next = affectedSides[succ(i, affectedSides.size())];
                    
                    /*
                     First, we consider the plane made up by the points p1, p2 and p3 of side and next. If the movement
                     of the vertex were to go through this plane, the brush would become convex, which we must prevent.
                     
                      v----p1
                      |\ s |
                      | \  |
                      |  \ |
                      | n \|
                     p3----p2
                     
                     */
                    
                    const size_t sideIndex0 = findElement(side->vertices, vertex);
                    const size_t nextIndex0 = findElement(next->vertices, vertex);
                    assert(sideIndex0 < side->vertices.size());
                    assert(nextIndex0 < next->vertices.size());
                    
                    const size_t sideIndex1 = succ(sideIndex0, side->vertices.size()); // index of next
                    const size_t sideIndex2 = succ(sideIndex0, side->vertices.size(), 2); // index of next but one
                    const size_t nextIndex1 = succ(nextIndex0, next->vertices.size(), 2); // index of next but one
                    
                    const Vec3f& p1 = side->vertices[sideIndex1]->position;
                    const Vec3f& p2 = side->vertices[sideIndex2]->position;
                    const Vec3f& p3 = next->vertices[nextIndex1]->position;
                    if (!plane.setPoints(p1, p2, p3)) {
                        // The points are colinear and we cannot determine the move distance - this is an error, but we
                        // gracefully stop the operation and return.
                        mergeSides(faceManager);
                        mergeEdges();
                        return MoveVertexResult(MoveVertexResult::VertexUnchanged, vertex);
                    }
                    
                    startDot = start.dot(plane.normal) - plane.distance;
                    endDot = end.dot(plane.normal) - plane.distance;
                    
                    if (std::abs(startDot) >= 0.001f || std::abs(endDot) >= 0.001f) {
                        if ((startDot > 0.0f) != (endDot >  0.0f)) {
                            frac = std::abs(startDot) < 0.001f ? 1.0f : std::abs(startDot) / (std::abs(startDot) + std::abs(endDot));
                            if (frac > lastFrac && frac < minFrac)
                                minFrac = frac;
                        }
                    }
                    
                    /*
                     Second, we consider the boundary plane of the one neighbour to side which is not incident to the
                     moved vertex. This neighbour is not necessarily a triangle, but that does not matter.
                     
                              ------
                             /   n  |
                            /    e  |
                     v-----/     i  |
                     |\ s |      g  |
                     | \  |      h  |
                     |  \ |      b  |
                     |   \|      o  |
                     -----\      u  |
                           \     r  |
                            --------
                     */
                    
                    const Edge* neighbourEdge = side->edges[sideIndex1];
                    const Side* neighbourSide = neighbourEdge->left == side ? neighbourEdge->right : neighbourEdge->left;
                    const Vec3f& b1 = neighbourSide->vertices[0]->position;
                    const Vec3f& b2 = neighbourSide->vertices[1]->position;
                    const Vec3f& b3 = neighbourSide->vertices[2]->position;
                    if (!plane.setPoints(b1, b2, b3)) { // Don't use the side face's boundary plane here as it might not yet be updated!
                        // The points are colinear and we cannot determine the move distance - this is an error, but we
                        // gracefully stop the operation and return.
                        mergeSides(faceManager);
                        mergeEdges();
                        return MoveVertexResult(MoveVertexResult::VertexUnchanged, vertex);
                    }

                    startDot = start.dot(plane.normal) - plane.distance;
                    endDot = end.dot(plane.normal) - plane.distance;
                    
                    if (std::abs(startDot) >= 0.001f || std::abs(endDot) >= 0.001f) {
                        if ((startDot > 0.0f) != (endDot >  0.0f)) {
                            frac = std::abs(startDot) < 0.001f ? 1.0f : std::abs(startDot) / (std::abs(startDot) + std::abs(endDot));
                            if (frac > lastFrac && frac < minFrac)
                                minFrac = frac;
                        }
                    }
                }

                assert(minFrac > lastFrac);
                lastFrac = minFrac;

                // We can now safely move the vertex to this point without the brush becoming convex:
                vertex->position = start + lastFrac * (end - start);
                
                // Now we check whether the vertex landed on another vertex. If so, we cancel the operation unless
                // that vertex is adjacent to the moved vertex and mergeWithAdjacentVertex is true.
                VertexList::const_iterator vertexIt, vertexEnd;
                for (vertexIt = vertices.begin(), vertexEnd = vertices.end(); vertexIt != vertexEnd; ++vertexIt) {
                    Vertex* candidate = *vertexIt;
                    if (vertex != candidate) {
                        if (vertex->position.equals(candidate->position)) {
                            Edge* connectingEdge = NULL;
                            EdgeList::const_iterator edgeIt, edgeEnd;
                            for (edgeIt = edges.begin(), edgeEnd = edges.end(); edgeIt != edgeEnd && connectingEdge == NULL; ++edgeIt) {
                                Edge* edge = *edgeIt;
                                if (edge->connects(vertex, candidate))
                                    connectingEdge = edge;
                            }
                            
                            if (connectingEdge != NULL && mergeWithAdjacentVertex) {
                                // The vertex was dragged onto an adjacent vertex and we are allowed to merge them.
                                for (edgeIt = edges.begin(), edgeEnd = edges.end(); edgeIt != edgeEnd; ++edgeIt) {
                                    Edge* edge = *edgeIt;
                                    if (edge != connectingEdge && (edge->start == candidate || edge->end == candidate)) {
                                        if (edge->start == candidate)
                                            edge->start = vertex;
                                        else
                                            edge->end = vertex;
                                        
                                        std::replace(edge->left->vertices.begin(), edge->left->vertices.end(), candidate, vertex);
                                        std::replace(edge->right->vertices.begin(), edge->right->vertices.end(), candidate, vertex);
                                    }
                                }
                                
                                deleteDegenerateTriangle(connectingEdge->left, connectingEdge, faceManager);
                                deleteDegenerateTriangle(connectingEdge->right, connectingEdge, faceManager);
                                deleteElement(edges, connectingEdge);
                                deleteElement(vertices, candidate);
                            } else {
                                // The vertex was either dragged onto a non-adjacent vertex or we weren't allowed to
                                // merge it with an adjacent vertex, so undo the operation and return.
                                vertex->position = lastPosition;
                                mergeSides(faceManager);
                                mergeEdges();
                                return MoveVertexResult(MoveVertexResult::VertexUnchanged, vertex);
                            }
                        }
                    }
                }
                
                // If any off the incident sides has become colinear, we abort the operation.
                affectedSides = incidentSides(vertex);
                for (sideIt = affectedSides.begin(), sideEnd = affectedSides.end(); sideIt != sideEnd; ++sideIt) {
                    Side* side = *sideIt;
                    if (side->isCollinearTriangle() < side->edges.size()) {
                        vertex->position = lastPosition;
                        mergeSides(faceManager);
                        mergeEdges();
                        return MoveVertexResult(MoveVertexResult::VertexUnchanged, vertex);
                    }
                }
            
                // affectedSides = incidentSides(vertex);
                // deleteCollinearTriangles(affectedSides, newFaces, droppedFaces);
                
                assert(sanityCheck());
                
                mergeSides(faceManager);
                mergeEdges();
                bounds = boundsOfVertices(vertices);
                center = centerOfVertices(vertices);
                
                assert(sanityCheck());
                
                bool vertexDeleted = std::find(vertices.begin(), vertices.end(), vertex) == vertices.end();
                if (vertexDeleted)
                    return MoveVertexResult(MoveVertexResult::VertexDeleted);
            }
            
            return MoveVertexResult(MoveVertexResult::VertexMoved, vertex);
        }
        
        Vertex* BrushGeometry::splitEdge(Edge* edge) {
            // split the edge
            edge->left->shift(findElement(edge->left->edges, edge) + 1);
            edge->right->shift(findElement(edge->right->edges, edge) + 1);
            
            // create a new vertex
            Vertex* newVertex = new Vertex();
            newVertex->position = edge->center();
            vertices.push_back(newVertex);
            edge->left->vertices.push_back(newVertex);
            edge->right->vertices.push_back(newVertex);
            
            // create the new edges
            Edge* newEdge1 = new Edge(edge->start, newVertex);
            newEdge1->left = edge->left;
            newEdge1->right = edge->right;
            Edge* newEdge2 = new Edge(newVertex, edge->end);
            newEdge2->left = edge->left;
            newEdge2->right = edge->right;
            edges.push_back(newEdge1);
            edges.push_back(newEdge2);
            
            // remove the split edge from the incident sides
            edge->left->edges.pop_back();
            edge->right->edges.pop_back();
            
            // add the new edges to the incident sides
            edge->left->edges.push_back(newEdge2);
            edge->left->edges.push_back(newEdge1);
            edge->right->edges.push_back(newEdge1);
            edge->right->edges.push_back(newEdge2);
            
            // delete the split edge
            edges.erase(std::remove(edges.begin(), edges.end(), edge), edges.end());
            delete edge;
            
            return newVertex;
        }
        
        Vertex* BrushGeometry::splitFace(Face* face, FaceManager& faceManager) {
            Side* side = face->side();
            
            // create a new vertex
            Vertex* newVertex = new Vertex();
            newVertex->position = centerOfVertices(side->vertices);
            vertices.push_back(newVertex);
            
            // create the new edges
            Edge* firstEdge = new Edge(newVertex, side->edges[0]->startVertex(side));
            edges.push_back(firstEdge);
            
            Edge* lastEdge = firstEdge;
            for (unsigned int i = 0; i < side->edges.size(); i++) {
                Edge* sideEdge = side->edges[i];
                
                Edge* newEdge;
                if (i == side->edges.size() - 1) {
                    newEdge = firstEdge;
                } else {
                    newEdge = new Edge(newVertex, sideEdge->endVertex(side));
                    edges.push_back(newEdge);
                }
                
                Side* newSide = new Side();
                newSide->vertices.push_back(newVertex);
                newSide->edges.push_back(lastEdge);
                lastEdge->right = newSide;
                
                newSide->vertices.push_back(lastEdge->end);
                newSide->edges.push_back(sideEdge);
                if (sideEdge->left == side)
                    sideEdge->left = newSide;
                else
                    sideEdge->right = newSide;
                
                newSide->vertices.push_back(newEdge->end);
                newSide->edges.push_back(newEdge);
                newEdge->left = newSide;
                
                newSide->face = new Face(side->face->worldBounds(), *side->face);
                newSide->face->setSide(newSide);
                sides.push_back(newSide);
                faceManager.addFace(side->face, newSide->face);
                
                lastEdge = newEdge;
            }
            
            // delete the split side
            faceManager.dropFace(side);
            bool success = deleteElement(sides, side);
            assert(success);
            
            return newVertex;
        }

        void BrushGeometry::copy(const BrushGeometry& original) {
            std::map<Vertex*, Vertex*> vertexMap;
            std::map<Edge*, Edge*> edgeMap;
            std::map<Side*, Side*> sideMap;

			while (!vertices.empty()) delete vertices.back(), vertices.pop_back();
			while (!edges.empty()) delete edges.back(), edges.pop_back();
			while (!sides.empty()) delete sides.back(), sides.pop_back();

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

                sides.push_back(copySide);
            }

            bounds = original.bounds;
        }

        bool BrushGeometry::sanityCheck() {
            // check Euler characteristic http://en.wikipedia.org/wiki/Euler_characteristic
            unsigned int sideCount = 0;
            for (unsigned int i = 0; i < sides.size(); i++)
                if (sides[i]->face != NULL)
                    sideCount++;
            if (vertices.size() - edges.size() + sideCount != 2) {
                fprintf(stdout, "failed Euler check\n");
                return false;
            }

			std::vector<int> vVisits;
			vVisits.resize(vertices.size());
            for (unsigned int i = 0; i < vertices.size(); i++)
                vVisits[i] = 0;

			std::vector<int> eVisits;
			eVisits.resize(edges.size());
            for (unsigned int i = 0; i < edges.size(); i++)
                eVisits[i] = 0;

            for (unsigned int i = 0; i < sides.size(); i++) {
                Side* side = sides[i];

                for (unsigned int j = 0; j < side->edges.size(); j++) {
                    Edge* edge = side->edges[j];
                    if (edge->left != side && edge->right != side) {
                        fprintf(stdout, "edge with index %i of side with index %i does not actually belong to it\n", j, i);
                        return false;
                    }

                    size_t index = findElement(edges, edge);
                    if (index == edges.size()) {
                        fprintf(stdout, "edge with index %i of side with index %i is missing from vertex data\n", j, i);
                        return false;
                    }
                    eVisits[index]++;

                    Vertex* vertex = edge->startVertex(side);
                    if (side->vertices[j] != vertex) {
                        fprintf(stdout, "start vertex of edge with index %i of side with index %i is not at position %i in the side's vertex list\n", j, i, j);
                        return false;
                    }

                    index = findElement(vertices, vertex);
                    if (index == vertices.size()) {
                        fprintf(stdout, "start vertex of edge with index %i of side with index %i is missing from vertex data\n", j, i);
                        return false;
                    }
                    vVisits[index]++;
                }
            }

            for (unsigned int i = 0; i < vertices.size(); i++) {
                if (vVisits[i] == 0) {
                    fprintf(stdout, "vertex with index %i does not belong to any side\n", i);
                    return false;
                }

                for (unsigned int j = i + 1; j < vertices.size(); j++)
                    if (vertices[i]->position.equals(vertices[j]->position)) {
                        fprintf(stdout, "vertex with index %i is identical to vertex with index %i\n", i, j);
                        return false;
                    }
            }

            for (unsigned int i = 0; i < edges.size(); i++) {
                if (eVisits[i] != 2) {
                    fprintf(stdout, "edge with index %i was visited %i times, should have been 2\n", i, eVisits[i]);
                    return false;
                }

                if (edges[i]->left == edges[i]->right) {
                    fprintf(stdout, "edge with index %i has equal sides", i);
                    return false;
                }

                Edge* edge1 = edges[i];
                for (unsigned int j = i + 1; j < edges.size(); j++) {
                    Edge* edge2 = edges[j];
                    if ((edge1->start == edge2->start && edge1->end == edge2->end) ||
                        (edge1->start == edge2->end && edge1->end == edge2->start)) {
                        fprintf(stdout, "edge with index %i is identical to edge with index %i\n", i, j);
                        return false;
                    }
                }
            }

            return true;
        }

        BrushGeometry::BrushGeometry(const BBox& i_bounds) {
            Vertex* lfd = new Vertex(i_bounds.min.x, i_bounds.min.y, i_bounds.min.z);
            Vertex* lfu = new Vertex(i_bounds.min.x, i_bounds.min.y, i_bounds.max.z);
            Vertex* lbd = new Vertex(i_bounds.min.x, i_bounds.max.y, i_bounds.min.z);
            Vertex* lbu = new Vertex(i_bounds.min.x, i_bounds.max.y, i_bounds.max.z);
            Vertex* rfd = new Vertex(i_bounds.max.x, i_bounds.min.y, i_bounds.min.z);
            Vertex* rfu = new Vertex(i_bounds.max.x, i_bounds.min.y, i_bounds.max.z);
            Vertex* rbd = new Vertex(i_bounds.max.x, i_bounds.max.y, i_bounds.min.z);
            Vertex* rbu = new Vertex(i_bounds.max.x, i_bounds.max.y, i_bounds.max.z);

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

            this->bounds = i_bounds;
            this->center = centerOfVertices(vertices);
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
                sides[i]->face->setSide(sides[i]);
        }

        BrushGeometry::CutResult BrushGeometry::addFace(Face& face, FaceList& droppedFaces) {
            Plane boundary = face.boundary();

            unsigned int keep = 0;
            unsigned int drop = 0;
            unsigned int undecided = 0;

            // mark vertices
            for (unsigned int i = 0; i < vertices.size(); i++) {
                Vertex& vertex = *vertices[i];
                PointStatus::Type vs = boundary.pointStatus(vertex.position);
                if (vs == PointStatus::PSAbove) {
                    vertex.mark = Vertex::Drop;
                    drop++;
                } else if (vs == PointStatus::PSBelow) {
                    vertex.mark  = Vertex::Keep;
                    keep++;
                } else {
                    vertex.mark = Vertex::Undecided;
                    undecided++;
                }
            }

            if (keep + undecided == vertices.size())
                return Redundant;

            if (drop + undecided == vertices.size())
                return Null;

            // mark and split edges
            for (unsigned int i = 0; i < edges.size(); i++) {
                Edge& edge = *edges[i];
                edge.updateMark();
                if (edge.mark == Edge::Split) {
                    Vertex* vertex = edge.split(boundary);
                    vertices.push_back(vertex);
                }
            }

            // mark, split and drop sides
            EdgeList newEdges;
            SideList::iterator sideIt = sides.begin();

            while (sideIt != sides.end()) {
                Side* side = *sideIt;
                Edge* newEdge = side->split();

                if (side->mark == Side::Drop) {
                    Face* dropFace = side->face;
                    if (dropFace != NULL) {
                        droppedFaces.push_back(dropFace);
                        dropFace->setSide(NULL);
                    }
                    delete side;
                    sideIt = sides.erase(sideIt);
                } else if (side->mark == Side::Split) {
                    edges.push_back(newEdge);
                    newEdges.push_back(newEdge);
                    side->mark = Side::Unknown;
                    ++sideIt;
                } else if (side->mark == Side::Keep && newEdge != NULL) {
                    // the edge is an undecided edge, so it needs to be flipped in order to act as a new edge
                    if (newEdge->right != side)
                        newEdge->flip();
                    newEdges.push_back(newEdge);
                    side->mark = Side::Unknown;
                    ++sideIt;
                } else {
                    side->mark = Side::Unknown;
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
                VertexList& sideVertices = side->vertices;
                EdgeList& sideEdges = side->edges;
                assert(sideVertices.size() == sideEdges.size());
                for (unsigned int j = 0; j < sideVertices.size(); j++) {
                    assert(sideVertices[j]->mark != Vertex::Drop);
                    assert(sideEdges[j]->mark != Edge::Drop);
                    assert(sideEdges[j]->startVertex(side) == sideVertices[j]);
                }
            }

            // clean up
            // delete dropped vertices
            VertexList::iterator vertexIt = vertices.begin();
            while (vertexIt != vertices.end()) {
                Vertex* vertex = *vertexIt;
                if (vertex->mark == Vertex::Drop) {
                    delete vertex;
                    vertexIt = vertices.erase(vertexIt);
                } else {
                    vertex->mark = Vertex::Unknown;
                    ++vertexIt;
                }
            }

            // delete dropped edges
            EdgeList::iterator edgeIt = edges.begin();
            while (edgeIt != edges.end()) {
                Edge* edge = *edgeIt;
                if (edge->mark == Edge::Drop) {
                    delete edge;
                    edgeIt = edges.erase(edgeIt);
                } else {
                    edge->mark = Edge::Unknown;
                    ++edgeIt;
                }
            }

            bounds = boundsOfVertices(vertices);
            center = centerOfVertices(vertices);
            return Split;
        }

        bool BrushGeometry::addFaces(FaceList& faces, FaceList& droppedFaces) {
            for (unsigned int i = 0; i < faces.size(); i++)
                if (addFace(*faces[i], droppedFaces) == Null)
                    return false;
            return true;
        }

        void BrushGeometry::translate(const Vec3f& delta) {
            for (unsigned int i = 0; i < vertices.size(); i++)
                vertices[i]->position += delta;
            bounds.translate(delta);
            center += delta;
        }

        void BrushGeometry::rotate90(Axis::Type axis, const Vec3f& rotationCenter, bool clockwise) {
            for (unsigned int i = 0; i < vertices.size(); i++)
                vertices[i]->position.rotate90(axis, rotationCenter, clockwise);
            bounds = boundsOfVertices(vertices);
            center = centerOfVertices(vertices);
        }

        void BrushGeometry::rotate(const Quat& rotation, const Vec3f& rotationCenter) {
            for (unsigned int i = 0; i < vertices.size(); i++)
                vertices[i]->position = rotation * (vertices[i]->position - rotationCenter) + rotationCenter;
            bounds = boundsOfVertices(vertices);
            center = centerOfVertices(vertices);
        }

        void BrushGeometry::flip(Axis::Type axis, const Vec3f& flipCenter) {
            for (unsigned int i = 0; i < vertices.size(); i++)
                vertices[i]->position.flip(axis, flipCenter);
            for (unsigned int i = 0; i < edges.size(); i++) {
                // std::swap(edges[i]->left, edges[i]->right);
                std::swap(edges[i]->start, edges[i]->end);
            }
            for (unsigned int i = 0; i < sides.size(); i++) {
                VertexList::iterator first = sides[i]->vertices.begin();
                std::advance(first, 1); // vertex 0 is invariant
                std::reverse(first, sides[i]->vertices.end());
                std::reverse(sides[i]->edges.begin(), sides[i]->edges.end());
            }
            
            bounds.flip(axis, flipCenter);
            center.flip(axis, flipCenter);
            
            assert(sanityCheck());
        }

        void BrushGeometry::updateFacePoints() {
            for (unsigned int i = 0; i < sides.size(); i++)
                sides[i]->face->updatePoints();
        }
        
        bool BrushGeometry::canMoveVertices(const Vec3f::List& vertexPositions, const Vec3f& delta) {
            FaceManager faceManager;
            
            BrushGeometry testGeometry(*this);
            testGeometry.restoreFaceSides();

            Vec3f::List sortedVertexPositions = vertexPositions;
            std::sort(sortedVertexPositions.begin(), sortedVertexPositions.end(), Vec3f::InverseDotOrder(delta));

            bool canMove = true;
            Vec3f::List::const_iterator vertexIt, vertexEnd;
            for (vertexIt = sortedVertexPositions.begin(), vertexEnd = sortedVertexPositions.end(); vertexIt != vertexEnd && canMove; ++vertexIt) {
                const Vec3f& vertexPosition = *vertexIt;
                Vertex* vertex = findVertex(testGeometry.vertices, vertexPosition);
                assert(vertex != NULL);
                
                const Vec3f start = vertex->position;
                const Vec3f end = start + delta;
                
                MoveVertexResult result = testGeometry.moveVertex(vertex, true, start, end, faceManager);
                canMove = result.type != MoveVertexResult::VertexUnchanged;
            }
            
            canMove &= testGeometry.sides.size() >= 3;

            restoreFaceSides();
            return canMove;
        }

        Vec3f::List BrushGeometry::moveVertices(const Vec3f::List& vertexPositions, const Vec3f& delta, FaceSet& newFaces, FaceSet& droppedFaces) {
            assert(canMoveVertices(vertexPositions, delta));
            
            FaceManager faceManager;
            VertexList movedVertices;
            Vec3f::List sortedVertexPositions = vertexPositions;
            std::sort(sortedVertexPositions.begin(), sortedVertexPositions.end(), Vec3f::InverseDotOrder(delta));

            Vec3f::List::const_iterator vertexIt, vertexEnd;
            for (vertexIt = sortedVertexPositions.begin(), vertexEnd = sortedVertexPositions.end(); vertexIt != vertexEnd; ++vertexIt) {
                const Vec3f& vertexPosition = *vertexIt;
                Vertex* vertex = findVertex(vertices, vertexPosition);
                assert(vertex != NULL);
                
                const Vec3f start = vertex->position;
                const Vec3f end = start + delta;

                MoveVertexResult result = moveVertex(vertex, true, start, end, faceManager);
                if (result.type == MoveVertexResult::VertexMoved)
                    movedVertices.push_back(result.vertex);
            }
            
            updateFacePoints();
            
            Vec3f::List newVertexPositions;
            newVertexPositions.reserve(movedVertices.size());
            for (unsigned int i = 0; i < movedVertices.size(); i++)
                newVertexPositions.push_back(movedVertices[i]->position);
            
            faceManager.getFaces(newFaces, droppedFaces);
            return newVertexPositions;
        }

        bool BrushGeometry::canMoveEdges(const EdgeList& i_edges, const Vec3f& delta) {
            FaceManager faceManager;
            
            BrushGeometry testGeometry(*this);
            testGeometry.restoreFaceSides();

            Vec3f::Set sortedVertexPositions;
            EdgeList::const_iterator edgeIt, edgeEnd;
            for (edgeIt = i_edges.begin(), edgeEnd = i_edges.end(); edgeIt != edgeEnd; ++edgeIt) {
                const Edge& edge = **edgeIt;
                sortedVertexPositions.insert(edge.start->position);
                sortedVertexPositions.insert(edge.end->position);
            }

            bool canMove = true;
            Vec3f::Set::const_iterator vertexIt, vertexEnd;
            for (vertexIt = sortedVertexPositions.begin(), vertexEnd = sortedVertexPositions.end(); vertexIt != vertexEnd; ++vertexIt) {
                const Vec3f& vertexPosition = *vertexIt;
                Vertex* vertex = findVertex(testGeometry.vertices, vertexPosition);
                if (vertex == NULL) {
                    canMove = false;
                    break;
                }
                
                const Vec3f start = vertex->position;
                const Vec3f end = start + delta;
                
                MoveVertexResult result = testGeometry.moveVertex(vertex, false, start, end, faceManager);
                if (result.type != MoveVertexResult::VertexMoved) {
                    canMove = false;
                    break;
                }
            }

            for (edgeIt = i_edges.begin(), edgeEnd = i_edges.end(); edgeIt != edgeEnd && canMove; ++edgeIt) {
                const Edge& edge = **edgeIt;
                canMove = findEdge(testGeometry.edges, (edge.start->position + delta).snapped(), (edge.end->position + delta).snapped()) != NULL;
            }
            
            canMove &= testGeometry.sides.size() >= 3;

            restoreFaceSides();
            return canMove;
        }
    
        void BrushGeometry::moveEdges(const EdgeList& i_edges, const Vec3f& delta, FaceSet& newFaces, FaceSet& droppedFaces) {
            assert(canMoveEdges(i_edges, delta));

            FaceManager faceManager;
            Vec3f::Set sortedVertexPositions;
            EdgeList::const_iterator edgeIt, edgeEnd;
            for (edgeIt = i_edges.begin(), edgeEnd = i_edges.end(); edgeIt != edgeEnd; ++edgeIt) {
                const Edge& edge = **edgeIt;
                sortedVertexPositions.insert(edge.start->position);
                sortedVertexPositions.insert(edge.end->position);
            }

            Vec3f::Set::const_iterator vertexIt, vertexEnd;
            for (vertexIt = sortedVertexPositions.begin(), vertexEnd = sortedVertexPositions.end(); vertexIt != vertexEnd; ++vertexIt) {
                const Vec3f& vertexPosition = *vertexIt;
                Vertex* vertex = findVertex(vertices, vertexPosition);
                assert(vertex != NULL);
                
                const Vec3f start = vertex->position;
                const Vec3f end = start + delta;
                
                MoveVertexResult result = moveVertex(vertex, false, start, end, faceManager);
                assert(result.type == MoveVertexResult::VertexMoved);
            }
            
            updateFacePoints();
            faceManager.getFaces(newFaces, droppedFaces);
        }

        bool BrushGeometry::canMoveFaces(const FaceList& faces, const Vec3f& delta) {
            FaceManager faceManager;

            typedef std::vector<size_t> VertexCounts;
            VertexCounts vertexCounts;
            
            BrushGeometry testGeometry(*this);
            testGeometry.restoreFaceSides();

            Vec3f::Set sortedVertexPositions;
            FaceList::const_iterator faceIt, faceEnd;
            for (faceIt = faces.begin(), faceEnd = faces.end(); faceIt != faceEnd; ++faceIt) {
                const Face& face = **faceIt;
                const VertexList& faceVertices = face.vertices();
                VertexList::const_iterator vertexIt, vertexEnd;
                for (vertexIt = faceVertices.begin(), vertexEnd = faceVertices.end(); vertexIt != vertexEnd; ++vertexIt) {
                    const Vertex& vertex = **vertexIt;
                    sortedVertexPositions.insert(vertex.position);
                }
                vertexCounts.push_back(face.vertices().size());
            }
            
            bool canMove = true;
            Vec3f::Set::const_iterator vertexIt, vertexEnd;
            for (vertexIt = sortedVertexPositions.begin(), vertexEnd = sortedVertexPositions.end(); vertexIt != vertexEnd; ++vertexIt) {
                const Vec3f& vertexPosition = *vertexIt;
                Vertex* vertex = findVertex(testGeometry.vertices, vertexPosition);
                if (vertex == NULL) {
                    canMove = false;
                    break;
                }
                
                const Vec3f start = vertex->position;
                const Vec3f end = start + delta;
                
                MoveVertexResult result = testGeometry.moveVertex(vertex, false, start, end, faceManager);
                if (result.type != MoveVertexResult::VertexMoved) {
                    canMove = false;
                    break;
                }
            }

            canMove &= testGeometry.sides.size() >= 3;

            for (size_t i = 0; i < faces.size() && canMove; i++)
                canMove = faces[i]->vertices().size() == vertexCounts[i];

            restoreFaceSides();
            return canMove;
        }
        
        void BrushGeometry::moveFaces(const FaceList& faces, const Vec3f& delta, FaceSet& newFaces, FaceSet& droppedFaces) {
            assert(canMoveFaces(faces, delta));
            
            FaceManager faceManager;
            Vec3f::Set sortedVertexPositions;
            FaceList::const_iterator faceIt, faceEnd;
            for (faceIt = faces.begin(), faceEnd = faces.end(); faceIt != faceEnd; ++faceIt) {
                const Face& face = **faceIt;
                const VertexList& faceVertices = face.vertices();

                VertexList::const_iterator vertexIt, vertexEnd;
                for (vertexIt = faceVertices.begin(), vertexEnd = faceVertices.end(); vertexIt != vertexEnd; ++vertexIt) {
                    const Vertex& vertex = **vertexIt;
                    sortedVertexPositions.insert(vertex.position);
                }
            }

            Vec3f::Set::const_iterator vertexIt, vertexEnd;
            for (vertexIt = sortedVertexPositions.begin(), vertexEnd = sortedVertexPositions.end(); vertexIt != vertexEnd; ++vertexIt) {
                const Vec3f& vertexPosition = *vertexIt;
                Vertex* vertex = findVertex(vertices, vertexPosition);
                assert(vertex != NULL);
                
                const Vec3f start = vertex->position;
                const Vec3f end = start + delta;
                
                MoveVertexResult result = moveVertex(vertex, false, start, end, faceManager);
                assert(result.type == MoveVertexResult::VertexMoved);
            }

            updateFacePoints();
            faceManager.getFaces(newFaces, droppedFaces);
        }

        bool BrushGeometry::canSplitEdge(Edge* edge, const Vec3f& delta) {
            // detect whether the drag would make the incident faces invalid
            const Vec3f& leftNorm = edge->left->face->boundary().normal;
            const Vec3f& rightNorm = edge->right->face->boundary().normal;
            if (Math::neg(delta.dot(leftNorm)) ||
                Math::neg(delta.dot(rightNorm)))
                return false;

            FaceManager faceManager;

            BrushGeometry testGeometry(*this);
            testGeometry.restoreFaceSides();
            
            // The given edge is not an edge of testGeometry!
            Edge* testEdge = findEdge(testGeometry.edges, edge->start->position, edge->end->position);
            assert(testEdge != NULL);
            
            Vertex* newVertex = testGeometry.splitEdge(testEdge);
            const Vec3f start = newVertex->position;
            const Vec3f end = start + delta;
            MoveVertexResult result = testGeometry.moveVertex(newVertex, false, start, end, faceManager);
            bool canSplit = result.type == MoveVertexResult::VertexMoved;
            canSplit &= testGeometry.sides.size() >= 3;

            restoreFaceSides();
            return canSplit;
        }

        Vec3f BrushGeometry::splitEdge(Edge* edge, const Vec3f& delta, FaceSet& newFaces, FaceSet& droppedFaces) {
            assert(canSplitEdge(edge, delta));
            
            FaceManager faceManager;
            Vertex* newVertex = splitEdge(edge);
            const Vec3f start = newVertex->position;
            const Vec3f end = start + delta;
            MoveVertexResult result = moveVertex(newVertex, false, start, end, faceManager);
            assert(result.type == MoveVertexResult::VertexMoved);

            updateFacePoints();
            faceManager.getFaces(newFaces, droppedFaces);
            return result.vertex->position;
        }

        bool BrushGeometry::canSplitFace(Face* face, const Vec3f& delta) {
            // detect whether the drag would lead to an indented face
            const Vec3f& norm = face->boundary().normal;
            if (Math::zero(delta.dot(norm)))
                return false;

            FaceManager faceManager;

            BrushGeometry testGeometry(*this);
            testGeometry.restoreFaceSides();

            Vertex* newVertex = testGeometry.splitFace(face, faceManager);
            const Vec3f start = newVertex->position;
            const Vec3f end = start + delta;
            MoveVertexResult result = testGeometry.moveVertex(newVertex, false, start, end, faceManager);
            bool canSplit = result.type == MoveVertexResult::VertexMoved;
            canSplit &= testGeometry.sides.size() >= 3;
            
            restoreFaceSides();
            return canSplit;
        }

        Vec3f BrushGeometry::splitFace(Face* face, const Vec3f& delta, FaceSet& newFaces, FaceSet& droppedFaces) {
            assert(canSplitFace(face, delta));
            
            FaceManager faceManager;
            Vertex* newVertex = splitFace(face, faceManager);
            const Vec3f start = newVertex->position;
            const Vec3f end = start + delta;
            MoveVertexResult result = moveVertex(newVertex, false, start, end, faceManager);
            assert(result.type == MoveVertexResult::VertexMoved);

            updateFacePoints();
            faceManager.getFaces(newFaces, droppedFaces);
            return result.vertex->position;
        }

        Vertex* findVertex(const VertexList& vertices, const Vec3f& position, float epsilon) {
            VertexList::const_iterator it, end;
            for (it = vertices.begin(), end = vertices.end(); it != end; ++it) {
                Vertex* vertex = *it;
                if (vertex->position.equals(position, epsilon))
                    return vertex;
            }
            return NULL;
        }
        
        Edge* findEdge(const EdgeList& edges, const Vec3f& vertexPosition1, const Vec3f& vertexPosition2, float epsilon) {
            EdgeList::const_iterator it, end;
            for (it = edges.begin(), end = edges.end(); it != end; ++it) {
                Edge* edge = *it;
                if ((edge->start->position.equals(vertexPosition1) && edge->end->position.equals(vertexPosition2, epsilon)) ||
                    (edge->start->position.equals(vertexPosition2) && edge->end->position.equals(vertexPosition1, epsilon)))
                    return edge;
            }
            return NULL;
        }
        
        Side* findSide(const SideList& sides, const Vec3f::List& vertexPositions, float epsilon) {
            SideList::const_iterator it, end;
            for (it = sides.begin(), end = sides.end(); it != end; ++it) {
                Side* side = *it;
                if (side->hasVertices(vertexPositions, epsilon))
                    return side;
            }
            return NULL;
        }
        
        Vec3f centerOfVertices(const VertexList& vertices) {
            Vec3f center = vertices[0]->position;
            for (unsigned int i = 1; i < vertices.size(); i++)
                center += vertices[i]->position;
            center /= static_cast<float>(vertices.size());
            return center;
        }

        BBox boundsOfVertices(const VertexList& vertices) {
            BBox bounds;
            bounds.min = vertices[0]->position;
            bounds.max = vertices[0]->position;

            for (unsigned int i = 1; i < vertices.size(); i++)
                bounds.mergeWith(vertices[i]->position);
            return bounds;
        }

        PointStatus::Type vertexStatusFromRay(const Vec3f& origin, const Vec3f& direction, const VertexList& vertices) {
            Ray ray(origin, direction);
            unsigned int above = 0;
            unsigned int below = 0;
            for (unsigned int i = 0; i < vertices.size(); i++) {
                PointStatus::Type status = ray.pointStatus(vertices[i]->position);
                if (status == PointStatus::PSAbove)
                    above++;
                else if (status == PointStatus::PSBelow)
                    below++;
                if (above > 0 && below > 0)
                    return PointStatus::PSInside;
            }

            return above > 0 ? PointStatus::PSAbove : PointStatus::PSBelow;
        }
    }
}
