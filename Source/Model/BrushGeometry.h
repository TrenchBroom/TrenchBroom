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

#include "Model/BrushGeometryTypes.h"
#include "Model/FaceTypes.h"
#include "Model/MapExceptions.h"
#include "Utility/Pool.h"
#include "Utility/VecMath.h"

#include <iostream>

using namespace TrenchBroom::Math;

namespace TrenchBroom {
    namespace Model {
        class Vertex {
        public:
            enum Mark {
                Drop,
                Keep,
                Undecided,
                New,
                Unknown
            };
        private:
			static Utility::Pool<Vertex> m_pool;
        public:
            Vec3f position;
            Mark mark;
            
            Vertex(float x, float y, float z) : mark(New) {
                position.x = x;
                position.y = y;
                position.z = z;
            }
            
            Vertex() : mark(New) {}
            
			~Vertex() {
                position = Vec3f::NaN;
                mark = Drop;
            }
            
            inline void* operator new(size_t size) {
                if (!m_pool.empty())
                    return m_pool.pop();
                    return malloc(size);
                    }
            
            inline void operator delete(void* pointer) {
                if (!m_pool.push(static_cast<Vertex*>(pointer)))
                    free(pointer);
                    }
        };
        
        class Side;
        
        class Edge {
        public:
            enum Mark {
                Drop,
                Keep,
                Split,
                Undecided,
                New,
                Unknown
            };
        private:
			static Utility::Pool<Edge> m_pool;
        public:
            Vertex* start;
            Vertex* end;
            Side* left;
            Side* right;
            Mark mark;
            
            Edge(Vertex* start, Vertex* end) : start(start), end(end), left(NULL), right(NULL), mark(New) {}
            Edge() : start(NULL), end(NULL), left(NULL), right(NULL), mark(New) {}
			
            ~Edge() {
                start = NULL;
                end = NULL;
                left = NULL;
                right = NULL;
                mark = Drop;
            }
            
            inline void* operator new(size_t size) {
                if (!m_pool.empty())
                    return m_pool.pop();
                    return malloc(size);
                    }
            
            inline void operator delete(void* pointer) {
                if (!m_pool.push(static_cast<Edge*>(pointer)))
                    free(pointer);
                    }
            
            inline Vertex* startVertex(const Side* side) {
                if (left == side)
                    return end;
                if (right == side)
                    return start;
                return NULL;
            }
            
            inline Vertex* endVertex(const Side* side) {
                if (left == side)
                    return start;
                if (right == side)
                    return end;
                return NULL;
            }
            
            inline Vec3f vector() {
                return start->position - end->position;
            }
            
            inline Vec3f vector(const Side* side) {
                return endVertex(side)->position - startVertex(side)->position;
            }
            
            inline Vec3f center() {
                return (start->position + end->position) / 2.0f;
            }
            
            inline bool incidentWith(const Edge* edge) {
                return start == edge->start || start == edge->end || end == edge->start || end == edge->end;
            }
            
            void updateMark();
            
            Vertex* split(const Plane& plane);
            
            inline void flip() {
                std::swap(left, right);
                std::swap(start, end);
            }
            
            inline bool intersectWithRay(const Ray& ray, float& distanceToRaySquared, float& distanceOfClosestPoint) {
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
                
                if (Math::zero(D)) {
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
                
                float sc = Math::zero(sN) ? 0.0f : sN / sD;
                float tc = Math::zero(tN) ? 0.0f : tN / tD;
                
                Vec3f dP = w + u * sc - ray.direction * tc;
                distanceToRaySquared = dP.lengthSquared();
                distanceOfClosestPoint = tc;
                
                return true;
            }
        };
        
        class Face;
        
        class Side {
        public:
            enum Mark {
                Keep,
                Drop,
                Split,
                New,
                Unknown
            };
        private:
			static Utility::Pool<Side> m_pool;
        public:
            VertexList vertices;
            EdgeList edges;
            Face* face;
            Mark mark;

            Side() : mark(New), face(NULL) {}
            Side(Edge* newEdges[], bool invert[], unsigned int count);
            Side(Face& face, EdgeList& newEdges);
			~Side();
            
            inline void* operator new(size_t size) {
                if (!m_pool.empty())
                    return m_pool.pop();
                return malloc(size);
            }
            
            inline void operator delete(void* pointer) {
                if (!m_pool.push(static_cast<Side*>(pointer)))
                    free(pointer);
            }
            
            float intersectWithRay(const Ray& ray);
            
            void replaceEdges(size_t index1, size_t index2, Edge* edge);
            
            Edge* split();
            void flip();
            void shift(size_t offset);
            bool isDegenerate();
            size_t isCollinearTriangle();
        };
        
        class VertexMoveResult {
        public:
            size_t index;
            bool moved;
            bool deleted;
            
            VertexMoveResult() {};
            VertexMoveResult(size_t index, bool moved, bool deleted) : index(index), moved(moved), deleted(deleted) {}
        };
        
        class BrushGeometry {
        public:
            enum CutResult {
                Redundant,  // the given face is redundant and need not be added to the brush
                Null,       // the given face has nullified the entire brush
                Split       // the given face has split the brush
            };
        private:
            SideList incidentSides(size_t vertexIndex);
            void deleteDegenerateTriangle(Side* side, Edge* edge, FaceList& newFaces, FaceList& droppedFaces);
            void triangulateSide(Side* sideToTriangluate, size_t vertexIndex, FaceList& newFaces);
            void splitSide(Side* sideToSplit, size_t vertexIndex, FaceList& newFaces);
            void splitSides(SideList& sidesToSplit, const Ray& ray, size_t vertexIndex, FaceList& newFaces, FaceList& droppedFaces);
            void mergeVertices(Vertex* keepVertex, Vertex* dropVertex, FaceList& newFaces, FaceList& droppedFaces);
            void mergeEdges();
            void mergeNeighbours(Side* side, size_t edgeIndex);
            void mergeSides(FaceList& newFaces, FaceList&droppedFaces);
            void deleteCollinearTriangles(SideList& incSides, FaceList& newFaces, FaceList& droppedFaces);
            float minVertexMoveDist(const SideList& incSides, const Vertex* vertex, const Ray& ray, float maxDist);
            VertexMoveResult moveVertex(size_t vertexIndex, bool mergeIncidentVertex, const Vec3f& delta, FaceList& newFaces, FaceList& droppedFaces);
            VertexMoveResult splitAndMoveEdge(size_t index, const Vec3f& delta, FaceList& newFaces, FaceList& droppedFaces);
            VertexMoveResult splitAndMoveSide(size_t sideIndex, const Vec3f& delta, FaceList& newFaces, FaceList& droppedFaces);
            void copy(const BrushGeometry& original);
            bool sanityCheck();
        public:
            VertexList vertices;
            EdgeList edges;
            SideList sides;
            Vec3f center;
            BBox bounds;
            
            BrushGeometry(const BBox& bounds);
            BrushGeometry(const BrushGeometry& original);
            ~BrushGeometry();
            
            bool closed() const;
            void restoreFaceSides();
            
            CutResult addFace(Face& face, FaceList& droppedFaces);
            bool addFaces(FaceList& faces, FaceList& droppedFaces);
            
            void translate(const Vec3f& delta);
            void rotate90(Axis::Type axis, const Vec3f& rotationCenter, bool clockwise);
            void rotate(const Quat& rotation, const Vec3f& rotationCenter);
            void flip(Axis::Type axis, const Vec3f& flipCenter);
            void snap();
            
            VertexMoveResult moveVertex(size_t vertexIndex, const Vec3f& delta, FaceList& newFaces, FaceList& droppedFaces);
            VertexMoveResult moveEdge(size_t edgeIndex, const Vec3f& delta, FaceList& newFaces, FaceList& droppedFaces);
            VertexMoveResult moveSide(size_t sideIndex, const Vec3f& delta, FaceList& newFaces, FaceList& droppedFaces);
        };
        
        template <class T>
        inline size_t findElement(const std::vector<T*>& vec, const T* element) {
//            return vec.find(element) - vec.begin();
            for (unsigned int i = 0; i < vec.size(); i++)
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
        
        size_t findVertex(const VertexList& vertices, const Vec3f& v);
        size_t findEdge(const EdgeList& edges, const Vec3f& v1, const Vec3f& v2);
        size_t findSide(const SideList& sides, const Vec3f::List& vertices);
        
        Vec3f centerOfVertices(const VertexList& vertices);
        BBox boundsOfVertices(const VertexList& vertices);
        PointStatus::Type vertexStatusFromRay(const Vec3f& origin, const Vec3f& direction, const VertexList& vertices);
    }
}


#endif /* defined(__TrenchBroo_BrushGeometry__) */
