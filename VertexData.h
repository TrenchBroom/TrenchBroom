//
//  VertexData.h
//  TrenchBroom
//
//  Created by Kristian Duske on 12.08.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

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
    BOOL valid;
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
const TBoundingBox* vertexDataBounds(TVertexData* vd);
const TVector3f* vertexDataCenter(TVertexData* vd);
BOOL vertexDataContainsPoint(TVertexData* vd, TVector3f* p);
EPointStatus vertexStatusFromRay(const TVector3f* o, const TVector3f* d, TVertex** ps, int c);
