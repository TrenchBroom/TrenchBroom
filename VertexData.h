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

extern int const PS_CONVEX;
extern int const PS_CONCAVE;

struct TEdgeTag;
struct TSideTag;

typedef struct {
    TVector3f position;
    EVertexMark mark;
} TVertex;

typedef struct {
    int* items;
    int count;
    int capacity;
} TIndexList;

typedef struct {
    TVertex** items;
    int count;
    int capacity;
} TVertexList;

typedef struct TEdgeTag {
    TVertex* startVertex;
    TVertex* endVertex;
    struct TSideTag* leftSide;
    struct TSideTag* rightSide;
    EEdgeMark mark;
} TEdge;

typedef struct {
    TEdge** items;
    int count;
    int capacity;
} TEdgeList;

typedef struct TSideTag {
    TVertexList vertices;
    TEdgeList edges;
    MutableFace* face;
    ESideMark mark;
} TSide;

typedef struct {
    TSide** items;
    int count;
    int capacity;
} TSideList;

typedef struct {
    TVertexList vertices;
    TEdgeList edges;
    TSideList sides;
    TBoundingBox bounds;
} TVertexData;

void initIndexList(TIndexList* l, int c);
void addIndexToList(TIndexList* l, int i);
void removeIndexFromList(TIndexList* l, int i);
void removeSuffixFromIndexList(TIndexList* l, int i);
void clearIndexList(TIndexList* l);
void appendIndexList(const TIndexList* s, int si, int sc, TIndexList* d);
int indexIndex(const TIndexList* l, int i);
void freeIndexList(TIndexList* l);

void initVertexList(TVertexList* l, int c);
void addVertexToList(TVertexList* l, TVertex* v);
void removeVertexFromList(TVertexList* l, int i);
void removeSuffixFromVertexList(TVertexList* l, int i);
void clearVertexList(TVertexList* l);
void appendVertexList(const TVertexList* s, int si, int sc, TVertexList* d);
int vertexIndex(const TVertexList* l, const TVertex* v);
void freeVertexList(TVertexList* l);

void initEdgeList(TEdgeList* l, int c);
void addEdgeToList(TEdgeList* l, TEdge* e);
void removeEdgeFromList(TEdgeList* l, int i);
void removeSuffixFromEdgeList(TEdgeList* l, int i);
void clearEdgeList(TEdgeList* l);
void appendEdgeList(const TEdgeList* s, int si, int sc, TEdgeList* d);
int edgeIndex(const TEdgeList* l, const TEdge* e);
void freeEdgeList(TEdgeList* l);

void initSideList(TSideList* l, int c);
void addSideToList(TSideList* l, TSide* s);
void removeSideFromList(TSideList* l, int i);
void removeSuffixFromSideList(TSideList* l, int i);
void clearSideList(TSideList* l);
void appendSideList(const TSideList* s, int si, int sc, TSideList* d);
int sideIndex(const TSideList* l, const TSide* s);
void freeSideList(TSideList* l);

void centerOfVertices(const TVertexList* v, TVector3f* c);
void edgeVector(const TEdge* e, TVector3f* v);
void centerOfEdge(const TEdge* e, TVector3f* v);
id <Face> frontFaceOfEdge(const TEdge* e, const TRay* r);
id <Face> backFaceOfEdge(const TEdge* e, const TRay* r);
TVertex* startVertexOfEdge(const TEdge* e, const TSide* s);
TVertex* endVertexOfEdge(const TEdge* e, const TSide* s);
void flipEdge(TEdge* e);
TVertex* splitEdge(const TPlane* p, TEdge* e);
void updateEdgeMark(TEdge* e);

void initSideWithEdges(TEdge** e, BOOL* f, int c, TSide* s);
void initSideWithFace(MutableFace* f, TEdgeList* e, TSide* s);
void freeSide(TSide* s);
TEdge* splitSide(TSide* s);
void flipSide(TSide* s);
float pickSide(const TSide* s, const TRay* r, TVector3f* h);
void shiftSide(TSide* s, int o);

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
void boundsOfVertexData(TVertexData* vd, TBoundingBox* b);
ECutResult cutVertexData(TVertexData* vd, MutableFace* f, NSMutableArray** d);
void translateVertexData(TVertexData* vd, const TVector3f* d);
void rotateVertexData90CW(TVertexData* vd, EAxis a, const TVector3f* c);
void rotateVertexData90CCW(TVertexData* vd, EAxis a, const TVector3f* c);
void rotateVertexData(TVertexData* vd, const TQuaternion* r, const TVector3f* c);
void flipVertexData(TVertexData* vd, EAxis a, const TVector3f* c);
BOOL vertexDataContainsPoint(TVertexData* vd, TVector3f* p);
EPointStatus vertexStatusFromRay(const TVector3f* o, const TVector3f* d, const TVertexList* ps);

int dragVertex(TVertexData* vd, int v, TVector3f d, NSMutableArray* newFaces, NSMutableArray* removedFaces);

void snapVertexData(TVertexData* vd);
BOOL sanityCheck(const TVertexData* vd, BOOL cc);
