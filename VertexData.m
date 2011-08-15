//
//  VertexData.m
//  TrenchBroom
//
//  Created by Kristian Duske on 12.08.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "VertexData.h"
#import "Face.h"
#import "MutableFace.h"
#import "assert.h"

void centerOfVertices(TVertex** v, int n, TVector3f* c) {
    *c = v[0]->vector;
    for (int i = 1; i < n; i++)
        addV3f(c, &v[i]->vector, c);
    scaleV3f(c, 1.0f / n, c);
}

void edgeVector(const TEdge* e, TVector3f* v) {
    subV3f(&e->endVertex->vector, &e->startVertex->vector, v);
}

id <Face> frontFaceOfEdge(const TEdge* e, const TRay* r) {
    id <Face> leftFace = e->leftSide->face;
    id <Face> rightFace = e->rightSide->face;
    
    return dotV3f([leftFace norm], &r->direction) >= 0 ? rightFace : leftFace;
}

id <Face> backFaceOfEdge(const TEdge* e, const TRay* r) {
    id <Face> leftFace = e->leftSide->face;
    id <Face> rightFace = e->rightSide->face;
    
    return dotV3f([leftFace norm], &r->direction) >= 0 ? leftFace : rightFace;
}

TVertex* startVertexOfEdge(const TEdge* e, const TSide* s) {
    if (e->leftSide == s)
        return e->endVertex;
    else if (e->rightSide == s)
        return e->startVertex;
    return NULL;
}

TVertex* endVertexOfEdge(const TEdge* e, const TSide* s) {
    if (e->leftSide == s)
        return e->startVertex;
    else if (e->rightSide == s)
        return e->endVertex;
    return NULL;
}

void flipEdge(TEdge* e) {
    TSide* ts = e->leftSide;
    e->leftSide = e->rightSide;
    e->rightSide = ts;
    
    TVertex* tv = e->startVertex;
    e->startVertex = e->endVertex;
    e->endVertex = tv;
}

TVertex* splitEdge(const TPlane* p, TEdge* e) {
    assert(e->startVertex->mark == VM_DROP || e->endVertex->mark == VM_DROP);
    
    TVertex* newVertex = malloc(sizeof(TVertex));
    TLine line;
    setLinePoints(&line, &e->startVertex->vector, &e->endVertex->vector);
    
    float dist = intersectPlaneWithLine(p, &line);
    linePointAtDistance(&line, dist, &newVertex->vector);
    newVertex->mark = VM_NEW;
    
    if (e->startVertex->mark == VM_DROP)
        e->startVertex = newVertex;
    else
        e->endVertex = newVertex;
    
    return newVertex;
}

void updateEdgeMark(TEdge* e) {
    int keep = 0;
    int drop = 0;
    int undecided = 0;

    EVertexMark sm = e->startVertex->mark;
    EVertexMark em = e->endVertex->mark;
    
    if (sm == VM_KEEP)
        keep++;
    else if (sm == VM_DROP)
        drop++;
    else if (sm == VM_UNDECIDED)
        undecided++;
    else
        [NSException raise:@"InvalidVertexStateException" format:@"invalid start vertex state: %i", sm];
    
    if (em == VM_KEEP)
        keep++;
    else if (em == VM_DROP)
        drop++;
    else if (em == VM_UNDECIDED)
        undecided++;
    else
        [NSException raise:@"InvalidVertexStateException" format:@"invalid end vertex state: %i", em];
    
    if (keep == 1 && drop == 1)
        e->mark = EM_SPLIT;
    else if (keep > 0)
        e->mark = EM_KEEP;
    else if (drop > 0)
        e->mark = EM_DROP;
    else
        e->mark = EM_UNDECIDED;
}

void initSideWithEdges(TEdge** e, BOOL* f, int c, TSide* s) {
    s->edges = malloc(c * sizeof(TEdge *));
    s->vertices = malloc(c * sizeof(TVertex *));

    s->edgeCount = c;
    for (int i = 0; i < c; i++) {
        if (!f[i])
            e[i]->rightSide = s;
        else
            e[i]->leftSide = s;
        s->edges[i] = e[i];
        s->vertices[i] = startVertexOfEdge(e[i], s);
    }
    
    s->face = nil;
    s->mark = SM_UNKNOWN;
}

void initSideWithFace(MutableFace* f, TEdge** e, int c, TSide* s) {
    if (s->edges != NULL)
        free(s->edges);
    if (s->vertices != NULL)
        free(s->vertices);
    
    s->edges = malloc(c * sizeof(TEdge *));
    s->vertices = malloc(c * sizeof(TVertex *));

    s->edgeCount = c;
    for (int i = 0; i < c; i++) {
        e[i]->leftSide = s;
        s->edges[i] = e[i];
        s->vertices[i] = startVertexOfEdge(e[i], s);
    }
    
    s->face = f;
    [f setSide:s];
    s->mark = SM_UNKNOWN;
}

void freeSide(TSide* s) {
    if (s->vertices != NULL) {
        free(s->vertices);
        s->vertices = NULL;
    }
    if (s->edges != NULL) {
        free(s->edges);
        s->edges = NULL;
    }
    s->edgeCount = 0;
    s->mark = SM_UNKNOWN;
    if (s->face != nil) {
        [s->face setSide:NULL];
        s->face = nil;
    }
}

TEdge* splitSide(TSide* s) {
    int keep = 0;
    int drop = 0;
    int split = 0;
    int undecided = 0;
    TEdge* undecidedEdge = NULL;
    
    int splitIndex1 = -2;
    int splitIndex2 = -2;
    
    TEdge* edge = s->edges[s->edgeCount - 1];
    EEdgeMark lastMark = edge->mark;
    for (int i = 0; i < s->edgeCount; i++) {
        edge = s->edges[i];
        EEdgeMark currentMark = edge->mark;
        if (currentMark == EM_SPLIT) {
            TVertex* sv = startVertexOfEdge(edge, s);
            if (sv->mark == VM_KEEP)
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
                splitIndex1 = i > 0 ? i - 1 : s->edgeCount - 1;
            drop++;
        }
        lastMark = currentMark;
    }
    
    if (keep == s->edgeCount) {
        s->mark = SM_KEEP;
        return NULL;
    }
    
    if (undecided == 1 && keep == s->edgeCount - 1) {
        s->mark = SM_KEEP;
        return undecidedEdge;
    }
    
    if (drop + undecided == s->edgeCount) {
        s->mark = SM_DROP;
        return NULL;
    }
    
    assert(splitIndex1 >= 0 && splitIndex2 >= 0);
    s->mark = SM_SPLIT;
    
    TEdge* newEdge = malloc(sizeof(TEdge));
    newEdge->startVertex = endVertexOfEdge(s->edges[splitIndex1], s);
    newEdge->endVertex = startVertexOfEdge(s->edges[splitIndex2], s);
    newEdge->leftSide = NULL;
    newEdge->rightSide = s;
    newEdge->mark = EM_NEW;

    int newEdgeCount;
    TEdge** newEdges;
    TVertex** newVertices;
    
    if (splitIndex2 > splitIndex1) {
        newEdgeCount = s->edgeCount - (splitIndex2 - splitIndex1 - 1) + 1;
        newEdges = malloc(newEdgeCount * sizeof(TEdge *));
        newVertices = malloc(newEdgeCount * sizeof(TVertex *));
        
        int j = 0;
        for (int i = 0; i <= splitIndex1; i++, j++) {
            newEdges[j] = s->edges[i];
            newVertices[j] = startVertexOfEdge(s->edges[i], s);
        }
        
        newEdges[j] = newEdge;
        newVertices[j] = startVertexOfEdge(newEdge, s);
        j++;
        
        for (int i = splitIndex2; i < s->edgeCount; i++, j++) {
            newEdges[j] = s->edges[i];
            newVertices[j] = startVertexOfEdge(s->edges[i], s);
        }
    } else {
        newEdgeCount = splitIndex1 - splitIndex2 + 2;
        newEdges = malloc(newEdgeCount * sizeof(TEdge *));
        newVertices = malloc(newEdgeCount * sizeof(TVertex *));
        
        int j = 0;
        for (int i = splitIndex2; i <= splitIndex1; i++, j++) {
            newEdges[j] = s->edges[i];
            newVertices[j] = startVertexOfEdge(s->edges[i], s);
        }
        newEdges[j] = newEdge;
        newVertices[j] = startVertexOfEdge(newEdge, s);
    }

    free(s->vertices);
    free(s->edges);
    s->vertices = newVertices;
    s->edges = newEdges;
    s->edgeCount = newEdgeCount;
    
    return newEdge;
}

float pickSide(const TSide* s, const TRay* r, TVector3f* h) {
    const TVector3f* norm = [s->face norm];
    float d = dotV3f(norm, &r->direction);
    if (!fneg(d))
        return NAN;

    const TPlane* plane = [s->face boundary];
    float dist = intersectPlaneWithRay(plane, r);
    if (isnan(dist))
        return NAN;

    TVector3f is, pis, v0, v1;
    EPlane cPlane;

    switch (strongestComponentV3f(norm)) {
        case A_X:
            cPlane = P_YZ;
            break;
        case A_Y:
            cPlane = P_XZ;
            break;
        case A_Z:
            cPlane = P_XY;
            break;
    }
    
    rayPointAtDistance(r, dist, &is);
    projectOntoPlane(cPlane, &is, &pis);
    
    TVertex* v = s->vertices[s->edgeCount - 1];
    projectOntoPlane(cPlane, &v->vector, &v0);
    subV3f(&v0, &pis, &v0);
    
    int c = 0;
    for (int i = 0; i < s->edgeCount; i++) {
        v = s->vertices[i];
        projectOntoPlane(cPlane, &v->vector, &v1);
        subV3f(&v1, &pis, &v1);
        
        if ((fzero(v0.x) && fzero(v0.y)) || (fzero(v1.x) && fzero(v1.y))) {
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
        return NAN;
    
    *h = is;
    return dist;
}

void initVertexData(TVertexData* vd) {
    vd->vertexCapacity = 0;
    vd->vertexCount = 0;
    vd->vertices = NULL;
    vd->edgeCapacity = 0;
    vd->edgeCount = 0;
    vd->edges = NULL;
    vd->sideCapacity = 0;
    vd->sideCount = 0;
    vd->sides = NULL;
    vd->bounds.min = NullVector;
    vd->bounds.max = NullVector;
    vd->center = NullVector;
    vd->valid = NO;
}

void initVertexDataWithBounds(TVertexData* vd, const TBoundingBox* b) {
    vd->vertexCapacity = 8;
    vd->vertices = malloc(vd->vertexCapacity * sizeof(TVertex *));
    vd->vertexCount = 0;
    vd->edgeCapacity = 12;
    vd->edges = malloc(vd->edgeCapacity * sizeof(TEdge *));
    vd->edgeCount = 0;
    vd->sideCapacity = 6;
    vd->sides = malloc(vd->sideCapacity * sizeof(TSide *));
    vd->sideCount = 0;
    vd->bounds.min = NullVector;
    vd->bounds.max = NullVector;
    vd->center = NullVector;
    vd->valid = NO;
    
    const TVector3f* min = &b->min;
    const TVector3f* max = &b->max;

    TVertex* esb = malloc(sizeof(TVertex));
    esb->vector.x  = min->x - 1;
    esb->vector.y  = min->y - 1;
    esb->vector.z  = min->z - 1;
    esb->mark      = VM_UNKNOWN;
    addVertex(vd, esb);
    
    TVertex* est = malloc(sizeof(TVertex));
    est->vector.x  = min->x - 1;
    est->vector.y  = min->y - 1;
    est->vector.z  = max->z + 1;
    est->mark      = VM_UNKNOWN;
    addVertex(vd, est);
    
    TVertex* enb = malloc(sizeof(TVertex));
    enb->vector.x  = min->x - 1;
    enb->vector.y  = max->y + 1;
    enb->vector.z  = min->z - 1;
    enb->mark      = VM_UNKNOWN;
    addVertex(vd, enb);
    
    TVertex* ent = malloc(sizeof(TVertex));
    ent->vector.x  = min->x - 1;
    ent->vector.y  = max->y + 1;
    ent->vector.z  = max->z + 1;
    ent->mark      = VM_UNKNOWN;
    addVertex(vd, ent);

    TVertex* wsb = malloc(sizeof(TVertex));
    wsb->vector.x  = max->x + 1;
    wsb->vector.y  = min->y - 1;
    wsb->vector.z  = min->z - 1;
    wsb->mark      = VM_UNKNOWN;
    addVertex(vd, wsb);

    TVertex* wst = malloc(sizeof(TVertex));
    wst->vector.x  = max->x + 1;
    wst->vector.y  = min->y - 1;
    wst->vector.z  = max->z + 1;
    wst->mark      = VM_UNKNOWN;
    addVertex(vd, wst);
    
    TVertex* wnb = malloc(sizeof(TVertex));
    wnb->vector.x  = max->x + 1;
    wnb->vector.y  = max->y + 1;
    wnb->vector.z  = min->z - 1;
    wnb->mark      = VM_UNKNOWN;
    addVertex(vd, wnb);
    
    TVertex* wnt = malloc(sizeof(TVertex));
    wnt->vector.x  = max->x + 1;
    wnt->vector.y  = max->y + 1;
    wnt->vector.z  = max->z + 1;
    wnt->mark      = VM_UNKNOWN;
    addVertex(vd, wnt);
    
    TEdge* esbwsb = malloc(sizeof(TEdge));
    esbwsb->startVertex     = esb;
    esbwsb->endVertex       = wsb;
    esbwsb->leftSide        = NULL;
    esbwsb->rightSide       = NULL;
    esbwsb->mark            = EM_UNKNOWN;
    addEdge(vd, esbwsb);

    TEdge* wsbwst = malloc(sizeof(TEdge));
    wsbwst->startVertex     = wsb;
    wsbwst->endVertex       = wst;
    wsbwst->leftSide        = NULL;
    wsbwst->rightSide       = NULL;
    wsbwst->mark            = EM_UNKNOWN;
    addEdge(vd, wsbwst);

    TEdge* wstest = malloc(sizeof(TEdge));
    wstest->startVertex     = wst;
    wstest->endVertex       = est;
    wstest->leftSide        = NULL;
    wstest->rightSide       = NULL;
    wstest->mark            = EM_UNKNOWN;
    addEdge(vd, wstest);

    TEdge* estesb = malloc(sizeof(TEdge));
    estesb->startVertex     = est;
    estesb->endVertex       = esb;
    estesb->leftSide        = NULL;
    estesb->rightSide       = NULL;
    estesb->mark            = EM_UNKNOWN;
    addEdge(vd, estesb);

    TEdge* wnbenb = malloc(sizeof(TEdge));
    wnbenb->startVertex     = wnb;
    wnbenb->endVertex       = enb;
    wnbenb->leftSide        = NULL;
    wnbenb->rightSide       = NULL;
    wnbenb->mark            = EM_UNKNOWN;
    addEdge(vd, wnbenb);

    TEdge* enbent = malloc(sizeof(TEdge));
    enbent->startVertex     = enb;
    enbent->endVertex       = ent;
    enbent->leftSide        = NULL;
    enbent->rightSide       = NULL;
    enbent->mark            = EM_UNKNOWN;
    addEdge(vd, enbent);
    
    TEdge* entwnt = malloc(sizeof(TEdge));
    entwnt->startVertex     = ent;
    entwnt->endVertex       = wnt;
    entwnt->leftSide        = NULL;
    entwnt->rightSide       = NULL;
    entwnt->mark            = EM_UNKNOWN;
    addEdge(vd, entwnt);
    
    TEdge* wntwnb = malloc(sizeof(TEdge));
    wntwnb->startVertex     = wnt;
    wntwnb->endVertex       = wnb;
    wntwnb->leftSide        = NULL;
    wntwnb->rightSide       = NULL;
    wntwnb->mark            = EM_UNKNOWN;
    addEdge(vd, wntwnb);
    
    TEdge* enbesb = malloc(sizeof(TEdge));
    enbesb->startVertex     = enb;
    enbesb->endVertex       = esb;
    enbesb->leftSide        = NULL;
    enbesb->rightSide       = NULL;
    enbesb->mark            = EM_UNKNOWN;
    addEdge(vd, enbesb);
    
    TEdge* estent = malloc(sizeof(TEdge));
    estent->startVertex     = est;
    estent->endVertex       = ent;
    estent->leftSide        = NULL;
    estent->rightSide       = NULL;
    estent->mark            = EM_UNKNOWN;
    addEdge(vd, estent);

    TEdge* wsbwnb = malloc(sizeof(TEdge));
    wsbwnb->startVertex     = wsb;
    wsbwnb->endVertex       = wnb;
    wsbwnb->leftSide        = NULL;
    wsbwnb->rightSide       = NULL;
    wsbwnb->mark            = EM_UNKNOWN;
    addEdge(vd, wsbwnb);
    
    TEdge* wntwst = malloc(sizeof(TEdge));
    wntwst->startVertex     = wnt;
    wntwst->endVertex       = wst;
    wntwst->leftSide        = NULL;
    wntwst->rightSide       = NULL;
    wntwst->mark            = EM_UNKNOWN;
    addEdge(vd, wntwst);
    
    TSide* south = malloc(sizeof(TSide));
    TEdge* southEdges[] = {esbwsb, estesb, wstest, wsbwst};
    BOOL southFlipped[] = {YES, YES, YES, YES};
    initSideWithEdges(southEdges, southFlipped, 4, south);
    addSide(vd, south);
    
    TSide* north = malloc(sizeof(TSide));
    TEdge* northEdges[] = {wnbenb, wntwnb, entwnt, enbent};
    BOOL northFlipped[] = {YES, YES, YES, YES};
    initSideWithEdges(northEdges, northFlipped, 4, north);
    addSide(vd, north);
    
    TSide* west = malloc(sizeof(TSide));
    TEdge* westEdges[] = {wsbwnb, wsbwst, wntwst, wntwnb};
    BOOL westFlipped[] = {YES, NO, YES, NO};
    initSideWithEdges(westEdges, westFlipped, 4, west);
    addSide(vd, west);
    
    TSide* east = malloc(sizeof(TSide));
    TEdge* eastEdges[] = {enbesb, enbent, estent, estesb};
    BOOL eastFlipped[] = {YES, NO, YES, NO};
    initSideWithEdges(eastEdges, eastFlipped, 4, east);
    addSide(vd, east);
    
    TSide* top = malloc(sizeof(TSide));
    TEdge* topEdges[] = {wstest, estent, entwnt, wntwst};
    BOOL topFlipped[] = {NO, NO, NO, NO};
    initSideWithEdges(topEdges, topFlipped, 4, top);
    addSide(vd, top);

    TSide* bottom = malloc(sizeof(TSide));
    TEdge* bottomEdges[] = {esbwsb, wsbwnb, wnbenb, enbesb};
    BOOL bottomFlipped[] = {NO, NO, NO, NO};
    initSideWithEdges(bottomEdges, bottomFlipped, 4, bottom);
    addSide(vd, bottom);
    
    vd->valid = NO;
}

BOOL initVertexDataWithFaces(TVertexData* vd, const TBoundingBox* b, NSArray* f, NSMutableArray** d) {
    initVertexDataWithBounds(vd, b);
    
    NSEnumerator* faceEn = [f objectEnumerator];
    id <Face> face;
    while ((face = [faceEn nextObject])) {
        if (!cutVertexData(vd, face, d)) {
            freeVertexData(vd);
            return NO;
        }
    }
    
    return YES;
}

void freeVertexData(TVertexData* vd) {
    if (vd->vertices != NULL) {
        for (int i = 0; i < vd->vertexCount; i++)
            free(vd->vertices[i]);
        free(vd->vertices);
        vd->vertices = NULL;
        vd->vertexCount = 0;
        vd->vertexCapacity = 0;
    }
    if (vd->edges != NULL) {
        for (int i = 0; i < vd->edgeCount; i++)
            free(vd->edges[i]);
        free(vd->edges);
        vd->edges = NULL;
        vd->edgeCount = 0;
        vd->edgeCapacity = 0;
    }
    if (vd->sides != NULL) {
        for (int i = 0; i < vd->sideCount; i++) {
            freeSide(vd->sides[i]);
            free(vd->sides[i]);
        }
        free(vd->sides);
        vd->sides = NULL;
        vd->sideCount = 0;
        vd->sideCapacity = 0;
    }
    vd->valid = NO;
}

void addVertex(TVertexData* vd, TVertex* v) {
    assert(vd != NULL);
    assert(v != NULL);
    
    if (vd->vertexCount == vd->vertexCapacity) {
        TVertex** vt = vd->vertices;
        
        vd->vertexCapacity *= 2;
        vd->vertices = malloc(vd->vertexCapacity * sizeof(TVertex *));
        memcpy(vd->vertices, vt, vd->vertexCount * sizeof(TVertex *));
        free(vt);
    }
    
    vd->vertices[vd->vertexCount++] = v;
}

void deleteVertex(TVertexData* vd, int v) {
    assert(vd != NULL);
    assert(v >= 0 && v < vd->vertexCount);
    
    free(vd->vertices[v]);
    if (v < vd->vertexCount - 1)
        memcpy(&vd->vertices[v], &vd->vertices[v + 1], (vd->vertexCount - v - 1) * sizeof(TVertex *));
    
    vd->vertexCount--;
    vd->vertices[vd->vertexCount] = NULL;
}

void addEdge(TVertexData* vd, TEdge* e) {
    assert(vd != NULL);
    assert(e != NULL);
    
    if (vd->edgeCount == vd->edgeCapacity) {
        TEdge** et = vd->edges;
        
        vd->edgeCapacity *= 2;
        vd->edges = malloc(vd->edgeCapacity * sizeof(TEdge *));
        memcpy(vd->edges, et, vd->edgeCount * sizeof(TEdge *));
        free(et);
    }
    
    vd->edges[vd->edgeCount++] = e;
}

void deleteEdge(TVertexData* vd, int e) {
    assert(vd != NULL);
    assert(e >= 0 && e < vd->edgeCount);
    
    free(vd->edges[e]);
    if (e < vd->edgeCount - 1)
        memcpy(&vd->edges[e], &vd->edges[e + 1], (vd->edgeCount - e - 1) * sizeof(TEdge *));

    vd->edgeCount--;
    vd->edges[vd->edgeCount] = NULL;
}

void addSide(TVertexData* vd, TSide* s) {
    assert(vd != NULL);
    assert(s != NULL);
    
    if (vd->sideCount == vd->sideCapacity) {
        TSide** st = vd->sides;
        
        vd->sideCapacity *= 2;
        vd->sides = malloc(vd->sideCapacity * sizeof(TSide *));
        memcpy(vd->sides, st, vd->sideCount * sizeof(TSide *));
        free(st);
    }
    
    vd->sides[vd->sideCount++] = s;
}

void deleteSide(TVertexData* vd, int s) {
    assert(vd != NULL);
    assert(s >= 0 && s < vd->sideCount);
    
    freeSide(vd->sides[s]);
    free(vd->sides[s]);
    if (s < vd->sideCount - 1)
        memcpy(&vd->sides[s], &vd->sides[s + 1], (vd->sideCount - s - 1) * sizeof(TSide *));

    vd->sideCount--;
    vd->sides[vd->sideCount] = NULL;
}

BOOL cutVertexData(TVertexData* vd, MutableFace* f, NSMutableArray** d) {
    const TPlane* p = [f boundary];
    
    int keep = 0;
    int drop = 0;
    int undecided = 0;
    
    // mark vertices
    for (int i = 0; i < vd->vertexCount; i++) {
        TVertex* v = vd->vertices[i];
        EPointStatus vs = pointStatusFromPlane(p, &v->vector);
        if (vs == PS_ABOVE) {
            v->mark = VM_DROP;
            drop++;
        } else if (vs == PS_BELOW) {
            v->mark  = VM_KEEP;
            keep++;
        } else {
            v->mark = VM_UNDECIDED;
            undecided++;
        }
    }
    
    if (keep + undecided == vd->vertexCount) {
        if (*d == nil)
            *d = [NSMutableArray array];
        [*d addObject:f];
        return YES;
    }
    
    if (drop + undecided == vd->vertexCount)
        return NO;
    
    // mark and split edges
    for (int i = 0; i < vd->edgeCount; i++) {
        TEdge* edge = vd->edges[i];
        updateEdgeMark(edge);
        if (edge->mark == EM_SPLIT) {
            TVertex* newVertex = splitEdge(p, edge);
            addVertex(vd, newVertex);
        }
    }
    
    // mark, split and drop sides
    int newEdgeCount = 0;
    TEdge** newEdges = malloc(vd->sideCount * sizeof(TEdge *)); // alloc enough space for new edges
    for (int i = 0; i < vd->sideCount; i++) {
        TSide* side = vd->sides[i];
        TEdge* newEdge = splitSide(side);
        
        if (side->mark == SM_DROP) {
            if (side->face != nil) {
                if (*d == nil)
                    *d = [NSMutableArray array];
                [*d addObject:side->face];
                [side->face setSide:NULL];
            }
            deleteSide(vd, i--);
        } else if (side->mark == SM_SPLIT) {
            addEdge(vd, newEdge);
            newEdges[newEdgeCount++] = newEdge;
            side->mark = SM_UNKNOWN;
        } else if (side->mark == SM_KEEP && newEdge != NULL) {
            // the edge is an undecided edge, so it needs to be flipped in order to act as a new edge
            if (newEdge->rightSide != side)
                flipEdge(newEdge);
            newEdges[newEdgeCount++] = newEdge;
            side->mark = SM_UNKNOWN;
        } else {
            side->mark = SM_UNKNOWN;
        }
    }
    
    // create new side from newly created edges
    // first, sort the new edges to form a polygon in clockwise order
    for (int i = 0; i < newEdgeCount - 1; i++) {
        TEdge* edge = newEdges[i];
        for (int j = i + 2; j < newEdgeCount; j++) {
            TEdge* candidate = newEdges[j];
            if (edge->startVertex == candidate->endVertex) {
                TEdge* t = newEdges[j];
                newEdges[j] = newEdges[i + 1];
                newEdges[i + 1] = t;
            }
        }
    }
    
    // now create the new side
    TSide* newSide = malloc(sizeof(TSide));
    newSide->face = nil;
    newSide->vertices = NULL;
    newSide->edges = NULL;
    newSide->edgeCount = 0;
    newSide->mark = SM_NEW;

    initSideWithFace(f, newEdges, newEdgeCount, newSide);
    addSide(vd, newSide);
    free(newEdges);
    
    // clean up
    // delete dropped vertices
    for (int i = 0; i < vd->vertexCount; i++)
        if (vd->vertices[i]->mark == VM_DROP)
            deleteVertex(vd, i--);
        else
            vd->vertices[i]->mark = VM_UNDECIDED;
    
    // delete dropped edges
    for (int i = 0; i < vd->edgeCount; i++)
        if (vd->edges[i]->mark == EM_DROP)
            deleteEdge(vd, i--);
        else
            vd->edges[i]->mark = EM_UNDECIDED;
    
    vd->valid = NO;
    return YES;
}

void validateVertexData(TVertexData* vd) {
    if (!vd->valid) {
        vd->bounds.min = vd->vertices[0]->vector;
        vd->bounds.max = vd->vertices[0]->vector;
        vd->center = vd->vertices[0]->vector;
        
        for  (int i = 1; i < vd->vertexCount; i++) {
            mergeBoundsWithPoint(&vd->bounds, &vd->vertices[i]->vector, &vd->bounds);
            addV3f(&vd->center, &vd->vertices[i]->vector, &vd->center);
        }
        
        scaleV3f(&vd->center, 1.0f / vd->vertexCount, &vd->center);
        vd->valid = YES;
    }
}

const TBoundingBox* vertexDataBounds(TVertexData* vd) {
    validateVertexData(vd);
    return &vd->bounds;
}

const TVector3f* vertexDataCenter(TVertexData* vd) {
    validateVertexData(vd);
    return &vd->center;
}

BOOL vertexDataContainsPoint(TVertexData* vd, TVector3f* p) {
    for (int i = 0; i < vd->sideCount; i++)
        if (pointStatusFromPlane([vd->sides[i]->face boundary], p) == PS_ABOVE)
            return NO;
    return YES;
}

EPointStatus vertexStatusFromRay(const TVector3f* o, const TVector3f* d, TVertex** ps, int c) {
    int above = 0;
    int below = 0;
    for (int i = 0; i < c; i++) {
        EPointStatus status = pointStatusFromRay(o, d, &ps[i]->vector);
        if (status == PS_ABOVE)
            above++;
        else if (status == PS_BELOW)
            below++;
        if (above > 0 && below > 0)
            return PS_INSIDE;
    }
    
    return above > 0 ? PS_ABOVE : PS_BELOW;
}

