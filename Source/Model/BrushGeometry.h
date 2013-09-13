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

#ifndef __TrenchBroo_BrushGeometry__
#define __TrenchBroo_BrushGeometry__

#include "IO/ByteBuffer.h"
#include "Model/BrushGeometryTypes.h"
#include "Model/FaceTypes.h"
#include "Model/MapExceptions.h"
#include "Utility/Allocator.h"
#include "Utility/VecMath.h"

#include <iostream>

using namespace TrenchBroom::VecMath;

namespace TrenchBroom {
    namespace Model {
        class Vertex : public Utility::Allocator<Vertex> {
        public:
            enum Mark {
                Drop,
                Keep,
                Undecided,
                New,
                Unknown
            };
        public:
            Vec3f position;
            Mark mark;

            Vertex(float x, float y, float z) :
            position(Vec3f(x, y, z)),
            mark(New) {}

            Vertex() : mark(New) {}

			~Vertex() {
                position = Vec3f::NaN;
                mark = Drop;
            }

            SideList incidentSides(const EdgeList& edges) const;
        };

        class Side;

        class Edge : public Utility::Allocator<Edge> {
        public:
            enum Mark {
                Drop,
                Keep,
                Split,
                Undecided,
                New,
                Unknown
            };
        public:
            Vertex* start;
            Vertex* end;
            Side* left;
            Side* right;
            Mark mark;

            Edge(Vertex* i_start, Vertex* i_end, Side* i_left, Side* i_right) :
            start(i_start),
            end(i_end),
            left(i_left),
            right(i_right),
            mark(New) {}

            Edge(Vertex* i_start, Vertex* i_end) :
            start(i_start),
            end(i_end),
            left(NULL),
            right(NULL),
            mark(New) {}

            Edge() :
            start(NULL),
            end(NULL),
            left(NULL),
            right(NULL),
            mark(New) {}

            ~Edge() {
                start = NULL;
                end = NULL;
                left = NULL;
                right = NULL;
                mark = Drop;
            }

            inline Vertex* startVertex(const Side* side) const {
                if (left == side)
                    return end;
                if (right == side)
                    return start;
                return NULL;
            }

            inline Vertex* endVertex(const Side* side) const {
                if (left == side)
                    return start;
                if (right == side)
                    return end;
                return NULL;
            }

            inline Vec3f vector() const {
                return end->position - start->position;
            }

            inline Vec3f vector(const Side* side) const {
                return endVertex(side)->position - startVertex(side)->position;
            }

            inline Vec3f center() const {
                return (start->position + end->position) / 2.0f;
            }

            inline bool incidentWith(const Edge* edge) const {
                return start == edge->start || start == edge->end || end == edge->start || end == edge->end;
            }

            inline bool contains(const Vec3f& point, float maxDistance = Math<float>::AlmostZero) const {
                const Vec3f edgeVec = vector();
                const Vec3f edgeDir = edgeVec.normalized();
                const float dot = (point - start->position).dot(edgeDir);

                // determine the closest point on the edge
                Vec3f closestPoint;
                if (dot < 0.0f)
                    closestPoint = start->position;
                else if ((dot * dot) > edgeVec.lengthSquared())
                    closestPoint = end->position;
                else
                    closestPoint = start->position + edgeDir * dot;

                const float distance2 = (point - closestPoint).lengthSquared();
                return distance2 <= (maxDistance * maxDistance);
            }

            inline bool connects(const Vertex* vertex1, const Vertex* vertex2) const {
                return (start == vertex1 && end == vertex2) || (start == vertex2 && end == vertex1);
            }
            
            inline bool connects(const Vec3f& vertex1, const Vec3f& vertex2, const float epsilon = Math<float>::AlmostZero) const {
                return ((start->position.equals(vertex1, epsilon) && end->position.equals(vertex2, epsilon)) ||
                        (start->position.equals(vertex2, epsilon) && end->position.equals(vertex1, epsilon)));
            }

            void updateMark();

            Vertex* split(const Planef& plane);

            inline void flip() {
                std::swap(left, right);
                std::swap(start, end);
            }

            inline bool intersectWithRay(const Rayf& ray, float& distanceToRaySquared, float& distanceOfClosestPoint) const {
                Vec3f u = vector();
                Vec3f w = start->position - ray.origin;

                float a = u.dot(u);
                float b = u.dot(ray.direction);
                float c = ray.direction.dot(ray.direction);
                float d = u.dot(w);
                float e = ray.direction.dot(w);
                float D = a * c - b * b;
                float sN, sD = D;
                float tN, tD = D;

                if (Math<float>::zero(D)) {
                    sN = 0.0f;
                    sD = 1.0f;
                    tN = e;
                    tD = c;
                } else {
                    sN = (b * e - c * d);
                    tN = (a * e - b * d);
                    if (sN < 0.0f) {
                        sN = 0.0f;
                        tN = e;
                        tD = c;
                    } else if (sN > sD) {
                        sN = sD;
                        tN = e + b;
                        tD = c;
                    }
                }

                if (tN < 0.0f)
                    return false;

                float sc = Math<float>::zero(sN) ? 0.0f : sN / sD;
                float tc = Math<float>::zero(tN) ? 0.0f : tN / tD;

                Vec3f dP = w + u * sc - ray.direction * tc;
                distanceToRaySquared = dP.lengthSquared();
                distanceOfClosestPoint = tc;

                return true;
            }

            inline EdgeInfo info() const {
                return EdgeInfo(start->position, end->position);
            }
        };

        class Face;

        class Side : public Utility::Allocator<Side> {
        public:
            enum Mark {
                Keep,
                Drop,
                Split,
                New,
                Unknown
            };
        public:
            VertexList vertices;
            EdgeList edges;
            Face* face;
            Mark mark;

            Side() :
            face(NULL),
            mark(New) {}
            Side(Edge* newEdges[], bool invert[], unsigned int count);
            Side(Face& face, EdgeList& newEdges);
			~Side();

            float intersectWithRay(const Rayf& ray);
            void replaceEdges(size_t index1, size_t index2, Edge* edge);
            Edge* split();
            void chop(size_t index, Side*& newSide, Edge*& newEdge);
            void flip();
            void shift(size_t offset);
            bool isDegenerate();
            size_t isCollinearTriangle();

            inline bool hasVertices(const Vec3f::List& vecs, float epsilon = Math<float>::AlmostZero) const {
                if (vertices.size() != vecs.size())
                    return false;

                size_t count = vecs.size();
                for (size_t i = 0; i < count; i++) {
                    bool equal = true;
                    for (size_t j = 0; j < count && equal; j++) {
                        equal = vertices[(i + j) % count]->position.equals(vecs[j], epsilon);
                    }
                    if (equal)
                        return true;
                }
                return false;
            }

            inline FaceInfo info() const {
                FaceInfo result;
                for (size_t i = 0; i < vertices.size(); i++)
                    result.vertices.push_back(vertices[i]->position);
                return result;
            }
        };

        struct MoveVertexResult {
            typedef enum {
                VertexMoved,
                VertexDeleted,
                VertexUnchanged
            } Type;

            const Type type;
            Vertex* vertex;

            MoveVertexResult(Type i_type, Vertex* i_vertex = NULL) :
            type(i_type),
            vertex(i_vertex) {}
        };

        class BrushGeometry {
        public:
            enum CutResult {
                Redundant,  // the given face is redundant and need not be added to the brush
                Null,       // the given face has nullified the entire brush
                Split       // the given face has split the brush
            };
        private:
            class FaceManager {
            private:
                typedef std::map<Face*, FaceSet> CopyMap;
                CopyMap m_newFaces;
                FaceSet m_droppedFaces;
            public:
                ~FaceManager();

                void addFace(Face* original, Face* copy);
                void dropFace(Side* side);
                void getFaces(FaceSet& newFaces, FaceSet& droppedFaces);
            };

            void deleteDegenerateTriangle(Side* side, Edge* edge, FaceManager& faceManager);
            void mergeEdges();
            void mergeNeighbours(Side* side, size_t edgeIndex, FaceManager& faceManager);
            void mergeSides(FaceManager& faceManager);

            MoveVertexResult moveVertex(Vertex* vertex, bool mergeWithAdjacentVertex, const Vec3f& start, const Vec3f& end, FaceManager& faceManager);
            Vertex* splitEdge(Edge* edge);
            Vertex* splitFace(Face* face, FaceManager& faceManager);

            void copy(const BrushGeometry& original);
            bool sanityCheck();
        public:
            VertexList vertices;
            EdgeList edges;
            SideList sides;
            Vec3f center;
            BBoxf bounds;

            BrushGeometry(const BBoxf& bounds);
            BrushGeometry(const BrushGeometry& original);
            BrushGeometry(const Model::VertexList& i_vertices, const Model::EdgeList& i_edges, const Model::SideList& i_sides);
            ~BrushGeometry();

            bool closed() const;
            void restoreFaceSides();

            CutResult addFace(Face& face, FaceSet& droppedFaces);
            bool addFaces(const FaceList& faces, FaceSet& droppedFaces);

            void updateFacePoints(FaceManager& faceManager);

            void correct(FaceSet& newFaces, FaceSet& droppedFaces, float epsilon);
            void snap(FaceSet& newFaces, FaceSet& droppedFaces, unsigned int snapTo);

            SideList incidentSides(const Vertex* vertex);

            bool canMoveVertices(const BBoxf& worldBounds, const Vec3f::List& vertexPositions, const Vec3f& delta);
            Vec3f::List moveVertices(const BBoxf& worldBounds, const Vec3f::List& vertexPositions, const Vec3f& delta, FaceSet& newFaces, FaceSet& droppedFaces);
            bool canMoveEdges(const BBoxf& worldBounds, const EdgeInfoList& edgeInfos, const Vec3f& delta);
            EdgeInfoList moveEdges(const BBoxf& worldBounds, const EdgeInfoList& edgeInfos, const Vec3f& delta, FaceSet& newFaces, FaceSet& droppedFaces);
            bool canMoveFaces(const BBoxf& worldBounds, const FaceInfoList& faceInfos, const Vec3f& delta);
            FaceInfoList moveFaces(const BBoxf& worldBounds, const FaceInfoList& faceInfos, const Vec3f& delta, FaceSet& newFaces, FaceSet& droppedFaces);

            bool canSplitEdge(const BBoxf& worldBounds, const EdgeInfo& edgeInfo, const Vec3f& delta);
            Vec3f splitEdge(const BBoxf& worldBounds, const EdgeInfo& edgeInfo, const Vec3f& delta, FaceSet& newFaces, FaceSet& droppedFaces);
            bool canSplitFace(const BBoxf& worldBounds, const FaceInfo& faceInfo, const Vec3f& delta);
            Vec3f splitFace(const BBoxf& worldBounds, const FaceInfo& faceInfo, const Vec3f& delta, FaceSet& newFaces, FaceSet& droppedFaces);
        };

        template <class T>
        inline size_t findElement(const std::vector<T*>& vec, const T* element) {
//            return vec.find(element) - vec.begin();
            for (size_t i = 0; i < vec.size(); i++)
                if (vec[i] == element)
                    return i;
            return vec.size();
        }

        template <class T>
        inline bool removeElement(std::vector<T*>& vec, T* element) {
            typename std::vector<T*>::iterator elementIt = find(vec.begin(), vec.end(), element);
            if (elementIt == vec.end())
                return false;
            vec.erase(elementIt);
            return true;
        }

        template <class T>
        inline bool deleteElement(std::vector<T*>& vec, T* element) {
            if (!removeElement(vec, element))
                return false;
            delete element;
            return true;
        }

        Vertex* findVertex(const VertexList& vertices, const Vec3f& position, float epsilon = Math<float>::AlmostZero);
        Edge* findEdge(const EdgeList& edges, const Vec3f& vertexPosition1, const Vec3f& vertexPosition2, float epsilon = Math<float>::AlmostZero);
        Side* findSide(const SideList& sides, const Vec3f::List& vertexPositions, float epsilon = Math<float>::AlmostZero);

        Vec3f centerOfVertices(const VertexList& vertices);
        BBoxf boundsOfVertices(const VertexList& vertices);
        PointStatus::Type vertexStatusFromRay(const Vec3f& origin, const Vec3f& direction, const VertexList& vertices);
    }
}


#endif /* defined(__TrenchBroo_BrushGeometry__) */
