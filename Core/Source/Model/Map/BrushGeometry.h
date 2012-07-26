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

#ifndef TrenchBroom_BrushGeometry_h
#define TrenchBroom_BrushGeometry_h

#include "Model/Map/BrushGeometryTypes.h"
#include "Model/Map/FaceTypes.h"
#include "Model/Map/MapExceptions.h"
#include "Utilities/VecMath.h"

#include <cassert>
#include <list>

namespace TrenchBroom {
    namespace Model {
        class Face;
        
        typedef enum {
            TB_CR_REDUNDANT, // the given face is redundant and need not be added to the brush
            TB_CR_NULL, // the given face has nullified the entire brush
            TB_CR_SPLIT // the given face has split the brush
        } ECutResult;
        
        typedef enum {
            TB_VM_DROP,
            TB_VM_KEEP,
            TB_VM_UNDECIDED,
            TB_VM_NEW,
            TB_VM_UNKNOWN
        } EVertexMark;
        
        typedef enum {
            TB_EM_KEEP,
            TB_EM_DROP,
            TB_EM_SPLIT,
            TB_EM_UNDECIDED,
            TB_EM_NEW,
            TB_EM_UNKNOWN
        } EEdgeMark;
        
        typedef enum {
            TB_SM_KEEP,
            TB_SM_DROP,
            TB_SM_SPLIT,
            TB_SM_NEW,
            TB_SM_UNKNOWN
        } ESideMark;
        
        class MoveResult {
        public:
            size_t index;
            bool moved;
            MoveResult() {};
            MoveResult(size_t index, bool moved) : index(index), moved(moved) {}
        };
        
		template <typename T>
		class Pool {
		private:
			unsigned int maxSize;
			std::list<T*> items;
		public:
			Pool(unsigned int maxSize) : maxSize(maxSize) {}
			~Pool() {
				while (!items.empty()) {
					free(items.back());
					items.pop_back();
				}
			}

			inline bool empty() {
				return items.empty();
			}

			inline size_t size() {
				return items.size();
			}

			inline bool push(T* item) {
				if (items.size() == maxSize)
					return false;
				items.push_back(item);
				return true;
			}

			inline T* pop() {
				assert(!items.empty());
				T* item = items.front();
				items.pop_front();
				return item;
			}
		};

        class Vertex {
        private:
			static Pool<Vertex> pool;
        public:
			~Vertex();

            Vec3f position;
            EVertexMark mark;

            void* operator new(size_t size);
            void operator delete(void* pointer);
            Vertex(float x, float y, float z);
            Vertex();
        };
        
        class Side;
        class Edge {
        private:
			static Pool<Edge> pool;
        public:
			~Edge();
		
            Vertex* start;
            Vertex* end;
            Side* left;
            Side* right;
            EEdgeMark mark;

            void* operator new(size_t size);
            void operator delete(void* pointer);
            Edge(Vertex* start, Vertex* end);
            Edge();
            Vertex* startVertex(Side* side);
            Vertex* endVertex(Side* side);
            Vec3f vector();
            Vec3f center();
            void updateMark();
            Vertex* split(Plane plane);
            void flip();
        };
        
        class Face;
        class Side {
        private:
			static Pool<Side> pool;
        public:
			~Side();
            VertexList vertices;
            EdgeList edges;
            Face* face;
            ESideMark mark;

            void* operator new(size_t size);
            void operator delete(void* pointer);
            Side() : mark(TB_SM_NEW), face(NULL) {}
            Side(Edge* newEdges[], bool invert[], unsigned int count);
            Side(Face& face, EdgeList& newEdges);
            float intersectWithRay(const Ray& ray);
            void replaceEdges(size_t index1, size_t index2, Edge* edge);
            Edge* split() throw (GeometryException);
            void flip();
            void shift(size_t offset);
        };
        
        class BrushGeometry {
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
            float minVertexMoveDist(const SideList& sides, const Vertex* vertex, const Ray& ray, float maxDist);
            MoveResult moveVertex(size_t vertexIndex, bool mergeIncidentVertex, const Vec3f& delta, FaceList& newFaces, FaceList& droppedFaces);
            MoveResult splitAndMoveEdge(size_t index, const Vec3f& delta, FaceList& newFaces, FaceList& droppedFaces);
            MoveResult splitAndMoveSide(size_t sideIndex, const Vec3f& delta, FaceList& newFaces, FaceList& droppedFaces);
            void copy(const BrushGeometry& original);
            bool sanityCheck();
        public:
            VertexList vertices;
            EdgeList edges;
            SideList sides;
            BBox bounds;
            
            BrushGeometry(const BBox& bounds);
            BrushGeometry(const BrushGeometry& original);
            ~BrushGeometry();
            
            bool closed() const;
            void restoreFaceSides();
            
            ECutResult addFace(Face& face, FaceList& droppedFaces) throw (GeometryException);
            bool addFaces(FaceList& faces, FaceList& droppedFaces) throw (GeometryException);
            
            void translate(const Vec3f& delta);
            void rotate90(EAxis axis, const Vec3f& center, bool clockwise);
            void rotate(const Quat& rotation, const Vec3f& center);
            void flip(EAxis axis, const Vec3f& center);
            void snap();
            
            MoveResult moveVertex(size_t vertexIndex, const Vec3f& delta, FaceList& newFaces, FaceList& droppedFaces);
            MoveResult moveEdge(size_t edgeIndex, const Vec3f& delta, FaceList& newFaces, FaceList& droppedFaces);
            MoveResult moveSide(size_t sideIndex, const Vec3f& delta, FaceList& newFaces, FaceList& droppedFaces);
        };
        
        
        template <class T> size_t indexOf(const std::vector<T*>& vec, const T* element) {
            for (unsigned int i = 0; i < vec.size(); i++)
                if (vec[i] == element)
                    return i;
            return vec.size();
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
        
        size_t indexOf(const VertexList& vertices, const Vec3f& v);
        size_t indexOf(const EdgeList& edges, const Vec3f& v1, const Vec3f& v2);
        size_t indexOf(const SideList& sides, const Vec3fList& vertices);
        
        inline size_t succ(size_t index, size_t count, size_t offset = 1) {
            return (index + offset) % count;
        }
        
        inline size_t pred(size_t index, size_t count, size_t offset = 1) {
            return ((index + count) - (offset % count)) % count;
        }
        
        Vec3f centerOfVertices(const VertexList& vertices);
        BBox boundsOfVertices(const VertexList& vertices);
        EPointStatus vertexStatusFromRay(const Vec3f& origin, const Vec3f& direction, const VertexList& vertices);
    }
}
#endif
