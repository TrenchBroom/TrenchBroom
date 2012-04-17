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

#include <vector>
#include "Model/Map/Face.h"

using namespace std;

namespace TrenchBroom {
    namespace Model {
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
        
        class Vertex {
        private:
            static Vertex* pool;
            static int poolSize;
            Vertex* next;
        public:
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
            static Edge* pool;
            static int poolSize;
            Edge* next;
        public:
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
            static Side* pool;
            static int poolSize;
            Side* next;
        public:
            vector<Vertex*> vertices;
            vector<Edge*> edges;
            Face* face;
            ESideMark mark;
            void* operator new(size_t size);
            void operator delete(void* pointer);
            Side() : mark(TB_SM_NEW), face(NULL) {}
            Side(Edge* newEdges[], bool invert[], int count);
            Side(Face& face, vector<Edge*>& newEdges);
            float intersectWithRay(const Ray& ray);
            void replaceEdges(size_t index1, size_t index2, Edge* edge);
            Edge* split();
            void flip();
            void shift(size_t offset);
        };
        
        class BrushGeometry {
        private:
            vector<Side*> incidentSides(size_t vertexIndex);
            void deleteDegenerateTriangle(Side* side, Edge* edge, vector<Face*>& newFaces, vector<Face*>& droppedFaces);
            void triangulateSide(Side* side, size_t vertexIndex, vector<Face*>& newFaces);
            void splitSide(Side* side, size_t vertexIndex, vector<Face*>& newFaces);
            void splitSides(vector<Side*>& sides, Ray ray, size_t vertexIndex, vector<Face*>& newFaces, vector<Face*>& droppedFaces);
            void mergeVertices(Vertex* keepVertex, Vertex* dropVertex, vector<Face*>& newFaces, vector<Face*>& droppedFaces);
            void mergeEdges();
            void mergeNeighbours(Side* side, size_t edgeIndex);
            void mergeSides(vector<Face*>& newFaces, vector<Face*>&droppedFaces);
            float minVertexMoveDist(const vector<Side*>& sides, const Vertex* vertex, Ray ray, float maxDist);
            MoveResult moveVertex(size_t vertexIndex, bool mergeIncidentVertex, const Vec3f& delta, vector<Face*>& newFaces, vector<Face*>& droppedFaces);
            MoveResult splitAndMoveEdge(size_t index, const Vec3f& delta, vector<Face*>& newFaces, vector<Face*>& droppedFaces);
            MoveResult splitAndMoveSide(size_t sideIndex, const Vec3f& delta, vector<Face*>& newFaces, vector<Face*>& droppedFaces);
            void copy(const BrushGeometry& original);
        public:
            vector<Vertex*> vertices;
            vector<Edge*> edges;
            vector<Side*> sides;
            BBox bounds;
            
            BrushGeometry(const BBox& bounds);
            BrushGeometry(const BrushGeometry& original);
            ~BrushGeometry();
            
            bool closed() const;
            void restoreFaceSides();
            
            ECutResult addFace(Face& face, vector<Face*>& droppedFaces);
            bool addFaces(vector<Face*>& faces, vector<Face*>& droppedFaces);
            
            void translate(Vec3f delta);
            void rotate90(EAxis axis, Vec3f center, bool clockwise);
            void rotate(Quat rotation, Vec3f center);
            void flip(EAxis axis, Vec3f center);
            void snap();
            
            MoveResult moveVertex(size_t vertexIndex, Vec3f delta, vector<Face*>& newFaces, vector<Face*>& droppedFaces);
            MoveResult moveEdge(size_t edgeIndex, Vec3f delta, vector<Face*>& newFaces, vector<Face*>& droppedFaces);
            MoveResult moveSide(size_t sideIndex, Vec3f delta, vector<Face*>& newFaces, vector<Face*>& droppedFaces);
        };
        
        
        template <class T> size_t indexOf(const vector<T*>& vec, const T* element);
        template <class T> bool removeElement(vector<T*>& vec, T* element);
        template <class T> bool deleteElement(vector<T*>& vec, T* element);
        size_t indexOf(const vector<Vertex*>& vertices, Vec3f v);
        size_t indexOf(const vector<Edge*>& edges, Vec3f v1, Vec3f v2);
        size_t indexOf(const vector<Side*>& sides, const vector<Vec3f>& vertices);
        
        Vec3f centerOfVertices(const vector<Vertex*>& vertices);
        BBox boundsOfVertices(const vector<Vertex*>& vertices);
        EPointStatus vertexStatusFromRay(Vec3f origin, Vec3f direction, const vector<Vertex*>& vertices);
    }
}
#endif
