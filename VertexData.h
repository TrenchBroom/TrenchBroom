/*
Copyright (C) 2010-2011 Kristian Duske

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

#import <Foundation/Foundation.h>
#import "Math.h"

@class PickingHitList;
@class MutableFace;
@protocol Face;

typedef enum {
    CR_REDUNDANT, // the given face is redundant and need not be added to the brush
    CR_NULL, // the given face has nullified the entire brush
    CR_SPLIT // the given face has split the brush
} ECutResult;

typedef enum {
    VM_DROP,
    VM_KEEP,
    VM_UNDECIDED,
    VM_NEW,
    VM_UNKNOWN
} EVertexMark;

typedef enum {
    EM_KEEP,
    EM_DROP,
    EM_SPLIT,
    EM_UNDECIDED,
    EM_NEW,
    EM_UNKNOWN
} EEdgeMark;

typedef enum {
    SM_KEEP,
    SM_DROP,
    SM_SPLIT,
    SM_NEW,
    SM_UNKNOWN
} ESideMark;

struct TEdgeTag;
struct TSideTag;

typedef struct {
    TVector3f vector;
    EVertexMark mark;
} TVertex;

typedef struct TEdgeTag {
    TVertex* startVertex;
    TVertex* endVertex;
    struct TSideTag* leftSide;
    struct TSideTag* rightSide;
    EEdgeMark mark;
} TEdge;

typedef struct TSideTag {
    TVertex** vertices;
    TEdge** edges;
    int edgeCount;
    TVector3f center;
    MutableFace* face;
    ESideMark mark;
} TSide;

typedef struct {
    TVertex** vertices;
    int vertexCount;
    int vertexCapacity;
    TEdge** edges;
    int edgeCount;
    int edgeCapacity;
    TSide** sides;
    int sideCount;
    int sideCapacity;
    
    TBoundingBox bounds;
    TVector3f center;
} TVertexData;

void centerOfVertices(TVertex** v, int n, TVector3f* c);
void edgeVector(const TEdge* e, TVector3f* v);
id <Face> frontFaceOfEdge(const TEdge* e, const TRay* r);
id <Face> backFaceOfEdge(const TEdge* e, const TRay* r);
TVertex* startVertexOfEdge(const TEdge* e, const TSide* s);
TVertex* endVertexOfEdge(const TEdge* e, const TSide* s);
void flipEdge(TEdge* e);
TVertex* splitEdge(const TPlane* p, TEdge* e);
void updateEdgeMark(TEdge* e);

void initSideWithEdges(TEdge** e, BOOL* f, int c, TSide* s);
void initSideWithFace(MutableFace* f, TEdge** e, int c, TSide* s);
void freeSide(TSide* s);
TEdge* splitSide(TSide* s);
void flipSide(TSide* s);
float pickSide(const TSide* s, const TRay* r, TVector3f* h);

void initVertexData(TVertexData* vd);
void initVertexDataWithBounds(TVertexData* vd, const TBoundingBox* b);
BOOL initVertexDataWithFaces(TVertexData* vd, const TBoundingBox* b, NSArray* f, NSMutableArray** d);
void freeVertexData(TVertexData* vd);
void addVertex(TVertexData* vd, TVertex* v);
void deleteVertex(TVertexData* vd, int v);
void addEdge(TVertexData* vd, TEdge* e);
void deleteEdge(TVertexData* vd, int e);
void addSide(TVertexData* vd, TSide* s);
void deleteSide(TVertexData* vd, int s);
ECutResult cutVertexData(TVertexData* vd, MutableFace* f, NSMutableArray** d);
void translateVertexData(TVertexData* vd, const TVector3f* d);
void rotateVertexData90CW(TVertexData* vd, EAxis a, const TVector3f* c);
void rotateVertexData90CCW(TVertexData* vd, EAxis a, const TVector3f* c);
void rotateVertexData(TVertexData* vd, const TQuaternion* r, const TVector3f* c);
void flipVertexData(TVertexData* vd, EAxis a, const TVector3f* c);
BOOL vertexDataContainsPoint(TVertexData* vd, TVector3f* p);
EPointStatus vertexStatusFromRay(const TVector3f* o, const TVector3f* d, TVertex** ps, int c);

int translateVertex(TVertexData* vd, int v, const TVector3f* d);
