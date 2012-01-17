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

#import "VertexData.h"
#import "Face.h"
#import "MutableFace.h"
#import "assert.h"

int const PS_CONVEX = -1;
int const PS_CONCAVE = -2;

void initVertexList(TVertexList* l, int c) {
    assert(c >= 0);
    
    l->capacity = c;
    l->count = 0;
    l->items = malloc(c * sizeof(TVertex *));
}

void addVertexToList(TVertexList* l, TVertex* v) {
    assert(l != NULL);
    assert(v != NULL);
    
    if (l->count == l->capacity) {
        TVertex** t = l->items;
        l->capacity *= 2;
        l->items = malloc(l->capacity * sizeof(TVertex *));
        memcpy(l->items, t, l->count * sizeof(TVertex *));
        free(t);
    }

    l->items[l->count++] = v;
}

void removeVertexFromList(TVertexList* l, int i) {
    assert(l != NULL);
    assert(i >= 0 && i < l->count);
    
    if (i < l->count - 1)
        memcpy(&l->items[i], &l->items[i + 1], (l->count - i - 1) * sizeof(TVertex *));

    l->count--;
    l->items[l->count] = NULL;
}

void clearVertexList(TVertexList* l) {
    l->count = 0;
}

void appendVertexList(const TVertexList* s, int si, int sc, TVertexList* d) {
    for (int i = si; i < mini(s->count - si, sc); i++)
        addVertexToList(d, s->items[i]);
}

int vertexIndex(const TVertexList* l, const TVertex* v) {
    for (int i = 0; i < l->count; i++)
        if (l->items[i] == v)
            return i;
    return -1;
}

void freeVertexList(TVertexList* l) {
    assert(l->items != NULL);
    free(l->items);
    l->items = NULL;
    l->count = 0;
    l->capacity = 0;
}

void initEdgeList(TEdgeList* l, int c) {
    assert(c >= 0);
    
    l->capacity = c;
    l->count = 0;
    l->items = malloc(c * sizeof(TEdge *));
}

void addEdgeToList(TEdgeList* l, TEdge* e) {
    assert(l != NULL);
    assert(e != NULL);
    
    if (l->count == l->capacity) {
        TEdge** t = l->items;
        l->capacity *= 2;
        l->items = malloc(l->capacity * sizeof(TEdge *));
        memcpy(l->items, t, l->count * sizeof(TEdge *));
        free(t);
    }
    
    l->items[l->count++] = e;
}

void removeEdgeFromList(TEdgeList* l, int i) {
    assert(l != NULL);
    assert(i >= 0 && i < l->count);
    
    if (i < l->count - 1)
        memcpy(&l->items[i], &l->items[i + 1], (l->count - i - 1) * sizeof(TEdge *));
    
    l->count--;
    l->items[l->count] = NULL;
}

void clearEdgeList(TEdgeList* l) {
    l->count = 0;
}

void appendEdgeList(const TEdgeList* s, int si, int sc, TEdgeList* d) {
    for (int i = si; i < mini(s->count - si, sc); i++)
        addEdgeToList(d, s->items[i]);
}

int edgeIndex(const TEdgeList* l, const TEdge* e) {
    for (int i = 0; i < l->count; i++)
        if (l->items[i] == e)
            return i;
    return -1;
}

void freeEdgeList(TEdgeList* l) {
    assert(l->items != NULL);
    free(l->items);
    l->items = NULL;
    l->count = 0;
    l->capacity = 0;
}

void initSideList(TSideList* l, int c) {
    assert(c >= 0);
    
    l->capacity = c;
    l->count = 0;
    l->items = malloc(c * sizeof(TSide *));
}

void addSideToList(TSideList* l, TSide* s) {
    assert(l != NULL);
    assert(s != NULL);
    
    if (l->count == l->capacity) {
        TSide** t = l->items;
        l->capacity *= 2;
        l->items = malloc(l->capacity * sizeof(TSide *));
        memcpy(l->items, t, l->count * sizeof(TSide *));
        free(t);
    }
    
    l->items[l->count++] = s;
}

void removeSideFromList(TSideList* l, int i) {
    assert(l != NULL);
    assert(i >= 0 && i < l->count);
    
    if (i < l->count - 1)
        memcpy(&l->items[i], &l->items[i + 1], (l->count - i - 1) * sizeof(TSide *));
    
    l->count--;
    l->items[l->count] = NULL;
}

void clearSideList(TSideList* l) {
    l->count = 0;
}

void appendSideList(const TSideList* s, int si, int sc, TSideList* d) {
    for (int i = si; i < mini(s->count - si, sc); i++)
        addSideToList(d, s->items[i]);
}

int sideIndex(const TSideList* l, const TSide* s) {
    for (int i = 0; i < l->count; i++)
        if (l->items[i] == s)
            return i;
    return -1;
}

void freeSideList(TSideList* l) {
    assert(l->items != NULL);
    free(l->items);
    l->items = NULL;
    l->count = 0;
    l->capacity = 0;
}

void centerOfVertices(TVertexList* v, TVector3f* c) {
    assert(v->count > 0);
    
    *c = v->items[0]->vector;
    for (int i = 1; i < v->count; i++)
        addV3f(c, &v->items[i]->vector, c);
    scaleV3f(c, 1.0f / v->count, c);
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
    initEdgeList(&s->edges, c);
    initVertexList(&s->vertices, c);

    for (int i = 0; i < c; i++) {
        if (f[i])
            e[i]->leftSide = s;
        else
            e[i]->rightSide = s;
        
        addEdgeToList(&s->edges, e[i]);
        addVertexToList(&s->vertices, startVertexOfEdge(e[i], s));
    }
    
    centerOfVertices(&s->vertices, &s->center);

    s->face = nil;
    s->mark = SM_UNKNOWN;
}

void initSideWithFace(MutableFace* f, TEdgeList* e, TSide* s) {
    initEdgeList(&s->edges, e->count);
    initVertexList(&s->vertices, e->count);

    for (int i = 0; i < e->count; i++) {
        e->items[i]->leftSide = s;
        addEdgeToList(&s->edges, e->items[i]);
        addVertexToList(&s->vertices, startVertexOfEdge(e->items[i], s));
    }
    
    centerOfVertices(&s->vertices, &s->center);

    s->face = f;
    [f setSide:s];
    s->mark = SM_UNKNOWN;
}

void freeSide(TSide* s) {
    if (s->vertices.items != NULL)
        freeVertexList(&s->vertices);
    if (s->edges.items != NULL)
        freeEdgeList(&s->edges);

    s->mark = SM_UNKNOWN;
    if (s->face != nil) {
        [s->face setSide:NULL];
        s->face = nil;
    }
}

void replaceEdges(TSide* s, int i1, int i2, TEdge* e) {
    int newEdgeCount;
    TEdgeList newEdges;
    TVertexList newVertices;
    
    if (i2 > i1) {
        newEdgeCount = s->edges.count - (i2 - i1 - 1) + 1;
        initEdgeList(&newEdges, newEdgeCount);
        initVertexList(&newVertices, newEdgeCount);
        
        for (int i = 0; i <= i1; i++) {
            addEdgeToList(&newEdges, s->edges.items[i]);
            addVertexToList(&newVertices, startVertexOfEdge(s->edges.items[i], s));
        }
        
        addEdgeToList(&newEdges, e);
        addVertexToList(&newVertices, startVertexOfEdge(e, s));
        
        for (int i = i2; i < s->edges.count; i++) {
            addEdgeToList(&newEdges, s->edges.items[i]);
            addVertexToList(&newVertices, startVertexOfEdge(s->edges.items[i], s));
        }
    } else {
        newEdgeCount = i1 - i2 + 2;
        initEdgeList(&newEdges, newEdgeCount);
        initVertexList(&newVertices, newEdgeCount);
        
        for (int i = i2; i <= i1; i++) {
            addEdgeToList(&newEdges, s->edges.items[i]);
            addVertexToList(&newVertices, startVertexOfEdge(s->edges.items[i], s));
        }
        
        addEdgeToList(&newEdges, e);
        addVertexToList(&newVertices, startVertexOfEdge(e, s));
    }

    clearEdgeList(&s->edges);
    appendEdgeList(&newEdges, 0, newEdges.count, &s->edges);
    freeEdgeList(&newEdges);
    
    clearVertexList(&s->vertices);
    appendVertexList(&newVertices, 0, newVertices.count, &s->vertices);
    freeVertexList(&newVertices);
}

TEdge* splitSide(TSide* s) {
    int keep = 0;
    int drop = 0;
    int split = 0;
    int undecided = 0;
    TEdge* undecidedEdge = NULL;
    
    int splitIndex1 = -2;
    int splitIndex2 = -2;
    
    TEdge* edge = s->edges.items[s->edges.count - 1];
    EEdgeMark lastMark = edge->mark;
    for (int i = 0; i < s->edges.count; i++) {
        edge = s->edges.items[i];
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
                splitIndex1 = i > 0 ? i - 1 : s->edges.count - 1;
            drop++;
        }
        lastMark = currentMark;
    }
    
    if (keep == s->edges.count) {
        s->mark = SM_KEEP;
        return NULL;
    }
    
    if (undecided == 1 && keep == s->edges.count - 1) {
        s->mark = SM_KEEP;
        return undecidedEdge;
    }
    
    if (drop + undecided == s->edges.count) {
        s->mark = SM_DROP;
        return NULL;
    }
    
    assert(splitIndex1 >= 0 && splitIndex2 >= 0);
    s->mark = SM_SPLIT;
    
    TEdge* newEdge = malloc(sizeof(TEdge));
    newEdge->startVertex = endVertexOfEdge(s->edges.items[splitIndex1], s);
    newEdge->endVertex = startVertexOfEdge(s->edges.items[splitIndex2], s);
    newEdge->leftSide = NULL;
    newEdge->rightSide = s;
    newEdge->mark = EM_NEW;

    replaceEdges(s, splitIndex1, splitIndex2, newEdge);
    
    return newEdge;
}

void flipSide(TSide* s) {
    TVertex* t;
    for (int i = 0; i < s->vertices.count / 2; i++) {
        t = s->vertices.items[i];
        s->vertices.items[i] = s->vertices.items[s->vertices.count - i - 1];
        s->vertices.items[s->vertices.count - i - 1] = t;
    }
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
    projectOntoCoordinatePlane(cPlane, &is, &pis);
    
    TVertex* v = s->vertices.items[s->vertices.count - 1];
    projectOntoCoordinatePlane(cPlane, &v->vector, &v0);
    subV3f(&v0, &pis, &v0);
    
    int c = 0;
    for (int i = 0; i < s->vertices.count; i++) {
        v = s->vertices.items[i];
        projectOntoCoordinatePlane(cPlane, &v->vector, &v1);
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

void shiftSide(TSide* s, int o) {
    TEdgeList newEdges;
    TVertexList newVertices;
    int c;
    
    assert(s->edges.count == s->vertices.count); // to be safe
    
    if (o == 0)
        return;
    
    c = s->edges.count;
    
    initEdgeList(&newEdges, c);
    initVertexList(&newVertices, c);
    
    for (int i = 0; i < c; i++) {
        addEdgeToList(&newEdges, s->edges.items[(i + o + c) % c]);
        addVertexToList(&newVertices, s->vertices.items[(i + o + c) % c]);
    }
    
    clearEdgeList(&s->edges);
    appendEdgeList(&newEdges, 0, newEdges.count, &s->edges);
    freeEdgeList(&newEdges);

    clearVertexList(&s->vertices);
    appendVertexList(&newVertices, 0, newVertices.count, &s->vertices);
    freeVertexList(&newVertices);
}

void initVertexData(TVertexData* vd) {
    vd->vertexList.capacity = 0;
    vd->vertexList.count = 0;
    vd->vertexList.items = NULL;
    vd->edgeList.capacity = 0;
    vd->edgeList.count = 0;
    vd->edgeList.items = NULL;
    vd->sideList.capacity = 0;
    vd->sideList.count = 0;
    vd->sideList.items = NULL;
    vd->bounds.min = NullVector;
    vd->bounds.max = NullVector;
    vd->center = NullVector;
}

void initVertexDataWithBounds(TVertexData* vd, const TBoundingBox* b) {
    vd->vertexList.capacity = 8;
    vd->vertexList.count = 0;
    vd->vertexList.items = malloc(vd->vertexList.capacity * sizeof(TVertex *));
    vd->edgeList.capacity = 12;
    vd->edgeList.count = 0;
    vd->edgeList.items = malloc(vd->edgeList.capacity * sizeof(TEdge *));
    vd->sideList.capacity = 6;
    vd->sideList.count = 0;
    vd->sideList.items = malloc(vd->sideList.capacity * sizeof(TSide *));

    const TVector3f* min = &b->min;
    const TVector3f* max = &b->max;
    
    vd->bounds.min.x = min->x - 1;
    vd->bounds.min.y = min->y - 1;
    vd->bounds.min.z = min->z - 1;
    vd->bounds.max.x = max->x + 1;
    vd->bounds.max.y = max->y + 1;
    vd->bounds.max.z = max->z + 1;
    centerOfBounds(&vd->bounds, &vd->center);
    
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
    
    TSide* southSide = malloc(sizeof(TSide));
    TEdge* southEdges[] = {esbwsb, estesb, wstest, wsbwst};
    BOOL southFlipped[] = {YES, YES, YES, YES};
    initSideWithEdges(southEdges, southFlipped, 4, southSide);
    addSide(vd, southSide);
    
    TSide* northSide = malloc(sizeof(TSide));
    TEdge* northEdges[] = {wnbenb, wntwnb, entwnt, enbent};
    BOOL northFlipped[] = {YES, YES, YES, YES};
    initSideWithEdges(northEdges, northFlipped, 4, northSide);
    addSide(vd, northSide);
    
    TSide* westSide = malloc(sizeof(TSide));
    TEdge* westEdges[] = {wsbwnb, wsbwst, wntwst, wntwnb};
    BOOL westFlipped[] = {YES, NO, YES, NO};
    initSideWithEdges(westEdges, westFlipped, 4, westSide);
    addSide(vd, westSide);
    
    TSide* eastSide = malloc(sizeof(TSide));
    TEdge* eastEdges[] = {enbesb, enbent, estent, estesb};
    BOOL eastFlipped[] = {YES, NO, YES, NO};
    initSideWithEdges(eastEdges, eastFlipped, 4, eastSide);
    addSide(vd, eastSide);
    
    TSide* topSide = malloc(sizeof(TSide));
    TEdge* topEdges[] = {wstest, estent, entwnt, wntwst};
    BOOL topFlipped[] = {NO, NO, NO, NO};
    initSideWithEdges(topEdges, topFlipped, 4, topSide);
    addSide(vd, topSide);

    TSide* bottomSide = malloc(sizeof(TSide));
    TEdge* bottomEdges[] = {esbwsb, wsbwnb, wnbenb, enbesb};
    BOOL bottomFlipped[] = {NO, NO, NO, NO};
    initSideWithEdges(bottomEdges, bottomFlipped, 4, bottomSide);
    addSide(vd, bottomSide);
}

BOOL initVertexDataWithFaces(TVertexData* vd, const TBoundingBox* b, NSArray* f, NSMutableArray** d) {
    initVertexDataWithBounds(vd, b);
    
    NSEnumerator* faceEn = [f objectEnumerator];
    id <Face> face;
    while ((face = [faceEn nextObject])) {
        switch (cutVertexData(vd, face, d)) {
            case CR_REDUNDANT:
                if (*d == nil)
                    *d = [NSMutableArray array];
                [*d addObject:face];
                break;
            case CR_NULL:
                freeVertexData(vd);
                return NO;
            default:
                break;
        }
    }
    
    return YES;
}

void freeVertexData(TVertexData* vd) {
    if (vd->vertexList.items != NULL) {
        for (int i = 0; i < vd->vertexList.count; i++)
            free(vd->vertexList.items[i]);
        freeVertexList(&vd->vertexList);
    }
    if (vd->edgeList.items != NULL) {
        for (int i = 0; i < vd->edgeList.count; i++)
            free(vd->edgeList.items[i]);
        freeEdgeList(&vd->edgeList);
    }
    if (vd->sideList.items != NULL) {
        for (int i = 0; i < vd->sideList.count; i++) {
            freeSide(vd->sideList.items[i]);
            free(vd->sideList.items[i]);
        }
        freeSideList(&vd->sideList);
    }
}

void addVertex(TVertexData* vd, TVertex* v) {
    addVertexToList(&vd->vertexList, v);
}

void deleteVertex(TVertexData* vd, int v) {
    assert(vd != NULL);
    assert(v >= 0 && v < vd->vertexList.count);
    
    free(vd->vertexList.items[v]);
    removeVertexFromList(&vd->vertexList, v);
}

void addEdge(TVertexData* vd, TEdge* e) {
    addEdgeToList(&vd->edgeList, e);
}

void deleteEdge(TVertexData* vd, int e) {
    assert(vd != NULL);
    assert(e >= 0 && e < vd->edgeList.count);
    
    free(vd->edgeList.items[e]);
    removeEdgeFromList(&vd->edgeList, e);
}

void addSide(TVertexData* vd, TSide* s) {
    addSideToList(&vd->sideList, s);
}

void deleteSide(TVertexData* vd, int s) {
    assert(vd != NULL);
    assert(s >= 0 && s < vd->sideList.count);
    
    freeSide(vd->sideList.items[s]);
    free(vd->sideList.items[s]);
    removeSideFromList(&vd->sideList, s);
}

void boundsOfVertexData(TVertexData* vd, TBoundingBox* b, TVector3f* c) {
    b->min = vd->vertexList.items[0]->vector;
    b->max = vd->vertexList.items[0]->vector;
    *c = vd->vertexList.items[0]->vector;
    
    for  (int i = 1; i < vd->vertexList.count; i++) {
        mergeBoundsWithPoint(b, &vd->vertexList.items[i]->vector, b);
        addV3f(c, &vd->vertexList.items[i]->vector, c);
    }
    
    scaleV3f(c, 1.0f / vd->vertexList.count,c);
}

ECutResult cutVertexData(TVertexData* vd, MutableFace* f, NSMutableArray** d) {
    const TPlane* p = [f boundary];
    
    int keep = 0;
    int drop = 0;
    int undecided = 0;
    
    // mark vertices
    for (int i = 0; i < vd->vertexList.count; i++) {
        TVertex* v = vd->vertexList.items[i];
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
    
    if (keep + undecided == vd->vertexList.count)
        return CR_REDUNDANT;
    
    if (drop + undecided == vd->vertexList.count)
        return CR_NULL;
    
    // mark and split edges
    for (int i = 0; i < vd->edgeList.count; i++) {
        TEdge* edge = vd->edgeList.items[i];
        updateEdgeMark(edge);
        if (edge->mark == EM_SPLIT) {
            TVertex* newVertex = splitEdge(p, edge);
            addVertex(vd, newVertex);
        }
    }
    
    // mark, split and drop sides
    TEdgeList newEdges;
    initEdgeList(&newEdges, vd->sideList.count); // alloc enough space for new edges
    for (int i = 0; i < vd->sideList.count; i++) {
        TSide* side = vd->sideList.items[i];
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
            addEdgeToList(&newEdges, newEdge);
            side->mark = SM_UNKNOWN;
            centerOfVertices(&side->vertices, &side->center);
        } else if (side->mark == SM_KEEP && newEdge != NULL) {
            // the edge is an undecided edge, so it needs to be flipped in order to act as a new edge
            if (newEdge->rightSide != side)
                flipEdge(newEdge);
            addEdgeToList(&newEdges, newEdge);
            side->mark = SM_UNKNOWN;
        } else {
            side->mark = SM_UNKNOWN;
        }
    }
    
    // create new side from newly created edges
    // first, sort the new edges to form a polygon in clockwise order
    for (int i = 0; i < newEdges.count - 1; i++) {
        TEdge* edge = newEdges.items[i];
        for (int j = i + 2; j < newEdges.count; j++) {
            TEdge* candidate = newEdges.items[j];
            if (edge->startVertex == candidate->endVertex) {
                TEdge* t = newEdges.items[j];
                newEdges.items[j] = newEdges.items[i + 1];
                newEdges.items[i + 1] = t;
            }
        }
    }
    
    // now create the new side
    TSide* newSide = malloc(sizeof(TSide));
    newSide->face = nil;
    newSide->mark = SM_NEW;

    initSideWithFace(f, &newEdges, newSide);
    addSide(vd, newSide);
    freeEdgeList(&newEdges);
    
    // clean up
    // delete dropped vertices
    for (int i = 0; i < vd->vertexList.count; i++)
        if (vd->vertexList.items[i]->mark == VM_DROP)
            deleteVertex(vd, i--);
        else
            vd->vertexList.items[i]->mark = VM_UNDECIDED;
    
    // delete dropped edges
    for (int i = 0; i < vd->edgeList.count; i++)
        if (vd->edgeList.items[i]->mark == EM_DROP)
            deleteEdge(vd, i--);
        else
            vd->edgeList.items[i]->mark = EM_UNDECIDED;

    boundsOfVertexData(vd, &vd->bounds, &vd->center);
    
    return CR_SPLIT;
}

void translateVertexData(TVertexData* vd, const TVector3f* d) {
    for (int i = 0; i < vd->vertexList.count; i++)
        addV3f(&vd->vertexList.items[i]->vector, d, &vd->vertexList.items[i]->vector);

    for (int i = 0; i < vd->sideList.count; i++)
        addV3f(&vd->sideList.items[i]->center, d, &vd->sideList.items[i]->center);
    
    translateBounds(&vd->bounds, d, &vd->bounds);
    addV3f(&vd->center, d, &vd->center);
}

void rotateVertexData90CW(TVertexData* vd, EAxis a, const TVector3f* c) {
    for (int i = 0; i < vd->vertexList.count; i++) {
        subV3f(&vd->vertexList.items[i]->vector, c, &vd->vertexList.items[i]->vector);
        rotate90CWV3f(&vd->vertexList.items[i]->vector, a, &vd->vertexList.items[i]->vector);
        addV3f(&vd->vertexList.items[i]->vector, c, &vd->vertexList.items[i]->vector);
    }

    for (int i = 0; i < vd->sideList.count; i++) {
        subV3f(&vd->sideList.items[i]->center, c, &vd->sideList.items[i]->center);
        rotate90CWV3f(&vd->sideList.items[i]->center, a, &vd->sideList.items[i]->center);
        addV3f(&vd->sideList.items[i]->center, c, &vd->sideList.items[i]->center);
    }
    
    rotateBounds90CW(&vd->bounds, a, c, &vd->bounds);
    subV3f(&vd->center, c, &vd->center);
    rotate90CWV3f(&vd->center, a, &vd->center);
    addV3f(&vd->center, c, &vd->center);
}

void rotateVertexData90CCW(TVertexData* vd, EAxis a, const TVector3f* c) {
    for (int i = 0; i < vd->vertexList.count; i++) {
        subV3f(&vd->vertexList.items[i]->vector, c, &vd->vertexList.items[i]->vector);
        rotate90CCWV3f(&vd->vertexList.items[i]->vector, a, &vd->vertexList.items[i]->vector);
        addV3f(&vd->vertexList.items[i]->vector, c, &vd->vertexList.items[i]->vector);
    }
    
    for (int i = 0; i < vd->sideList.count; i++) {
        subV3f(&vd->sideList.items[i]->center, c, &vd->sideList.items[i]->center);
        rotate90CCWV3f(&vd->sideList.items[i]->center, a, &vd->sideList.items[i]->center);
        addV3f(&vd->sideList.items[i]->center, c, &vd->sideList.items[i]->center);
    }
    
    rotateBounds90CCW(&vd->bounds, a, c, &vd->bounds);
    subV3f(&vd->center, c, &vd->center);
    rotate90CCWV3f(&vd->center, a, &vd->center);
    addV3f(&vd->center, c, &vd->center);
}

void rotateVertexData(TVertexData* vd, const TQuaternion* r, const TVector3f* c) {
    for (int i = 0; i < vd->vertexList.count; i++) {
        subV3f(&vd->vertexList.items[i]->vector, c, &vd->vertexList.items[i]->vector);
        rotateQ(r, &vd->vertexList.items[i]->vector, &vd->vertexList.items[i]->vector);
        addV3f(&vd->vertexList.items[i]->vector, c, &vd->vertexList.items[i]->vector);
    }
    
    for (int i = 0; i < vd->sideList.count; i++) {
        subV3f(&vd->sideList.items[i]->center, c, &vd->sideList.items[i]->center);
        rotateQ(r, &vd->sideList.items[i]->center, &vd->sideList.items[i]->center);
        addV3f(&vd->sideList.items[i]->center, c, &vd->sideList.items[i]->center);
    }
    
    rotateBounds(&vd->bounds, r, c, &vd->bounds);
    subV3f(&vd->center, c, &vd->center);
    rotateQ(r, &vd->center, &vd->center);
    addV3f(&vd->center, c, &vd->center);
}

void flipVertexData(TVertexData* vd, EAxis a, const TVector3f* c) {
    float min, max;
    switch (a) {
        case A_X:
            for (int i = 0; i < vd->vertexList.count; i++) {
                vd->vertexList.items[i]->vector.x -= c->x;
                vd->vertexList.items[i]->vector.x *= -1;
                vd->vertexList.items[i]->vector.x += c->x;
            }
            
            for (int i = 0; i < vd->sideList.count; i++) {
                vd->sideList.items[i]->center.x -= c->x;
                vd->sideList.items[i]->center.x *= -1;
                vd->sideList.items[i]->center.x += c->x;
            }
            
            min = vd->bounds.max.x;
            max = vd->bounds.min.x;
            min -= c->x;
            min *= -1;
            min += c->x;
            max -= c->x;
            max *= -1;
            max += c->x;
            vd->bounds.min.x = min;
            vd->bounds.max.x = max;
            
            vd->center.x -= c->x;
            vd->center.x *= -1;
            vd->center.x += c->x;
            break;
        case A_Y:
            for (int i = 0; i < vd->vertexList.count; i++) {
                vd->vertexList.items[i]->vector.y -= c->y;
                vd->vertexList.items[i]->vector.y *= -1;
                vd->vertexList.items[i]->vector.y += c->y;
            }
            
            for (int i = 0; i < vd->sideList.count; i++) {
                vd->sideList.items[i]->center.y -= c->y;
                vd->sideList.items[i]->center.y *= -1;
                vd->sideList.items[i]->center.y += c->y;
            }
            
            min = vd->bounds.max.y;
            max = vd->bounds.min.y;
            min -= c->y;
            min *= -1;
            min += c->y;
            max -= c->y;
            max *= -1;
            max += c->y;
            vd->bounds.min.y = min;
            vd->bounds.max.y = max;
            
            vd->center.y -= c->y;
            vd->center.y *= -1;
            vd->center.y += c->y;
            break;
        default:
            for (int i = 0; i < vd->vertexList.count; i++) {
                vd->vertexList.items[i]->vector.z -= c->z;
                vd->vertexList.items[i]->vector.z *= -1;
                vd->vertexList.items[i]->vector.z += c->z;
            }
            
            for (int i = 0; i < vd->sideList.count; i++) {
                vd->sideList.items[i]->center.z -= c->z;
                vd->sideList.items[i]->center.z *= -1;
                vd->sideList.items[i]->center.z += c->z;
            }
            
            min = vd->bounds.max.z;
            max = vd->bounds.min.z;
            min -= c->z;
            min *= -1;
            min += c->z;
            max -= c->z;
            max *= -1;
            max += c->z;
            vd->bounds.min.z = min;
            vd->bounds.max.z = max;
            
            vd->center.z -= c->z;
            vd->center.z *= -1;
            vd->center.z += c->z;
            break;
    }
    
    for (int i = 0; i < vd->edgeList.count; i++)
        flipEdge(vd->edgeList.items[i]);
    
    for (int i = 0; i < vd->sideList.count; i++)
        flipSide(vd->sideList.items[i]);
}

BOOL vertexDataContainsPoint(TVertexData* vd, TVector3f* p) {
    for (int i = 0; i < vd->sideList.count; i++)
        if (pointStatusFromPlane([vd->sideList.items[i]->face boundary], p) == PS_ABOVE)
            return NO;
    return YES;
}

EPointStatus vertexStatusFromRay(const TVector3f* o, const TVector3f* d, const TVertexList* ps) {
    int above = 0;
    int below = 0;
    for (int i = 0; i < ps->count; i++) {
        EPointStatus status = pointStatusFromRay(o, d, &ps->items[i]->vector);
        if (status == PS_ABOVE)
            above++;
        else if (status == PS_BELOW)
            below++;
        if (above > 0 && below > 0)
            return PS_INSIDE;
    }
    
    return above > 0 ? PS_ABOVE : PS_BELOW;
}

MutableFace* createFaceForSide(const TBoundingBox* w, TSide* s) {
    TPlane boundary;
    TVector3f v1, v2;
    TVector3i p1, p2, p3;

    MutableFace* newFace = [[MutableFace alloc] initWithWorldBounds:w];
    boundary.point = s->vertices.items[0]->vector;
    
    subV3f(&s->vertices.items[1]->vector, &s->vertices.items[0]->vector, &v1);
    subV3f(&s->vertices.items[2]->vector, &s->vertices.items[0]->vector, &v2);
    crossV3f(&v2, &v1, &boundary.norm);
    normalizeV3f(&boundary.norm, &boundary.norm);
    
    makePointsForPlane(&boundary, w, &p1, &p2, &p3);
    [newFace setPoint1:&p1 point2:&p2 point3:&p3];
    [newFace setSide:s];
    s->face = newFace;
    
    return [newFace autorelease];
}

void triangulateFace(TVertexData* vd, TSide* s, int v, NSMutableArray* newFaces) {
    TSide* newSide;
    MutableFace* newFace;
    TVertex* vertex = vd->vertexList.items[v];
    int vi = vertexIndex(&s->vertices, vertex);
    assert(vi >= 0);

    const TBoundingBox* worldBounds = [s->face worldBounds];
    
    TEdge* edges[3];
    BOOL flipped[3];
    edges[0] = s->edges.items[vi];
    flipped[0] = edges[0]->leftSide == s;
    edges[1] = s->edges.items[(vi + 1) % s->edges.count];
    flipped[1] = edges[1]->leftSide == s;
    
    for (int i = 0; i < s->edges.count - 3; i++) {
        edges[2] = malloc(sizeof(TEdge));
        edges[2]->startVertex = s->vertices.items[(vi + 2) % s->vertices.count];
        edges[2]->endVertex = vertex;
        edges[2]->leftSide = NULL;
        edges[2]->rightSide = NULL;
        edges[2]->mark = EM_NEW;
        flipped[2] = NO;

        newSide = malloc(sizeof(TSide));
        initSideWithEdges(edges, flipped, 3, newSide);

        addEdge(vd, edges[2]);
        addSide(vd, newSide);

        newFace = createFaceForSide(worldBounds, newSide);
        [newFace setTexture:[s->face texture]];
        [newFace setXOffset:[s->face xOffset]];
        [newFace setYOffset:[s->face yOffset]];
        [newFace setXScale:[s->face xScale]];
        [newFace setYScale:[s->face yScale]];
        [newFace setRotation:[s->face rotation]];
        [newFaces addObject:newFace];
        
        edges[0] = edges[2];
        flipped[0] = YES;
        edges[1] = s->edges.items[(vi + 2) % s->edges.count];
        flipped[1] = edges[1]->leftSide == s;
        
        vi = (vi + 1) % s->edges.count;
    }

    edges[2] = s->edges.items[(vi + 2) % s->edges.count];
    flipped[2] = edges[2]->leftSide == s;
    
    newSide = malloc(sizeof(TSide));
    initSideWithEdges(edges, flipped, 3, newSide);

    addSide(vd, newSide);
    
    newFace = createFaceForSide(worldBounds, newSide);
    [newFace setTexture:[s->face texture]];
    [newFace setXOffset:[s->face xOffset]];
    [newFace setYOffset:[s->face yOffset]];
    [newFace setXScale:[s->face xScale]];
    [newFace setYScale:[s->face yScale]];
    [newFace setRotation:[s->face rotation]];
    [newFaces addObject:newFace];
}

id <Face> splitFace(TVertexData* vd, TSide* s, int v) {
    TSide* newSide;
    MutableFace* newFace;
    TVertex* vertex = vd->vertexList.items[v];
    int vi = vertexIndex(&s->vertices, vertex);
    assert(vi >= 0);
    
    const TBoundingBox* worldBounds = [s->face worldBounds];
    
    TEdge* edges[3];
    BOOL flipped[3];
    edges[0] = s->edges.items[(vi + s->edges.count - 1) % s->edges.count];
    flipped[0] = edges[0]->leftSide == s;
    edges[1] = s->edges.items[vi % s->edges.count];
    flipped[1] = edges[1]->leftSide == s;
    edges[2] = malloc(sizeof(TEdge));
    edges[2]->startVertex = s->vertices.items[(vi + s->edges.count - 1) % s->vertices.count];
    edges[2]->endVertex = s->vertices.items[(vi + 1) % s->vertices.count];
    edges[2]->leftSide = NULL;
    edges[2]->rightSide = s;
    edges[2]->mark = EM_NEW;
    flipped[2] = YES;
    
    newSide = malloc(sizeof(TSide));
    initSideWithEdges(edges, flipped, 3, newSide);
    
    addEdge(vd, edges[2]);
    addSide(vd, newSide);
    
    replaceEdges(s, (vi + s->edges.count - 2) % s->edges.count, (vi + 1) % s->edges.count, edges[2]);
    
    newFace = createFaceForSide(worldBounds, newSide);
    [newFace setTexture:[s->face texture]];
    [newFace setXOffset:[s->face xOffset]];
    [newFace setYOffset:[s->face yOffset]];
    [newFace setXScale:[s->face xScale]];
    [newFace setYScale:[s->face yScale]];
    [newFace setRotation:[s->face rotation]];

    return newFace;
}

int polygonShape(const TVertexList* p, const TVector3f* n) {
    const TVector3f* a;
    
    if (!fzero(n->x))
        a = &XAxisPos;
    else if (!fzero(n->y))
        a = &YAxisPos;
    else
        a = &ZAxisPos;
    
    float d = dotV3f(n, a);
    
    const TVector3f* v1 = &p->items[p->count - 2]->vector;
    const TVector3f* v2 = &p->items[p->count - 1]->vector;
    const TVector3f* v3 = &p->items[0]->vector;
    
    TVector3f e1, e2, c;
    subV3f(v2, v1, &e1);
    subV3f(v3, v2, &e2);
    crossV3f(&e2, &e1, &c);
    
    if (nullV3f(&c))
        return p->count - 1;
    
    BOOL pos = (dotV3f(&c, a) / d) > 0;
    
    for (int i = 1; i < p->count; i++) {
        v1 = v2;
        v2 = v3;
        e1 = e2;
        
        v3 = &p->items[i]->vector;
        subV3f(v3, v2, &e2);
        crossV3f(&e2, &e1, &c);
        
        if (nullV3f(&c))
            return i - 1;
        
        if (pos != (dotV3f(&c, a) / d) > 0)
            return PS_CONCAVE;
    }
    
    return PS_CONVEX;
}

void mergeSides(TSide* s, int si) {
    TEdge* e = s->edges.items[si];
    TSide* n = e->leftSide != s ? e->leftSide : e->rightSide;
    int ni = edgeIndex(&n->edges, e);
    
    // shift the two sides so that their shared edge is at the end of both's edge lists
    shiftSide(s, -s->edges.count + si + 1);
    shiftSide(n, -n->edges.count + ni + 1);
    
    removeEdgeFromList(&s->edges, s->edges.count - 1);
    removeEdgeFromList(&n->edges, n->edges.count - 1);

    removeVertexFromList(&s->vertices, s->vertices.count - 1);
    removeVertexFromList(&n->vertices, n->vertices.count - 1);
    
    for (int i = 0; i < n->edges.count; i++) {
        e = n->edges.items[i];
        if (e->leftSide == n)
            e->leftSide = s;
        else
            e->rightSide = s;
        addEdgeToList(&s->edges, e);
    }

    appendVertexList(&n->vertices, 0, n->vertices.count, &s->vertices);
}

int incidentSides(TVertexData* vd, int v, int* si) {
    int c = 0;
    TVertex* vertex = vd->vertexList.items[v];
    
    for (int i = 0; i < vd->sideList.count; i++) {
        TSide* side = vd->sideList.items[i];
        for (int j = 0; j < side->edges.count; j++) {
            TEdge* edge = side->edges.items[j];
            if (startVertexOfEdge(edge, side) == vertex)
                si[c++] = i;
        }
    }
    
    return c;
}

int incidentEdges(TVertexData* vd, int v, int* ei) {
    int c = 0;
    TVertex* vertex = vd->vertexList.items[v];
    
    for (int i = 0; i < vd->edgeList.count; i++) {
        TEdge* edge = vd->edgeList.items[i];
        if (edge->startVertex == vertex || edge->endVertex == vertex)
            ei[c++] = i;
    }
    
    return c;
}

int translateVertex(TVertexData* vd, int v, const TVector3f* d, NSMutableArray** newFaces, NSMutableArray** removedFaces) {
    int sides[vd->sideList.count];
    int sideCount;
    int edges[vd->edgeList.count];
    int edgeCount;
    TVertex* vertex;
    TPlane boundary;
    TVector3f v1, v2;
    TVector3i p1, p2, p3;
    
    edgeCount = incidentEdges(vd, v, edges);
    TVector3f edgeDirsBefore[edgeCount];
    TVector3f edgeDirsAfter[edgeCount];
    
    for (int i = 0; i < edgeCount; i++)
        edgeVector(vd->edgeList.items[edges[i]], &edgeDirsBefore[i]);
    
    vertex = vd->vertexList.items[v];
    addV3f(&vertex->vector, d, &vertex->vector);
    
    for (int i = 0; i < edgeCount; i++)
        edgeVector(vd->edgeList.items[edges[i]], &edgeDirsAfter[i]);
    
    for (int i = 0; i < edgeCount; i++)
        if (d <= 0)
            return -1;
    
    sideCount = incidentSides(vd, v, sides);
    for (int i = 0; i < sideCount; i++) {
        TSide* side = vd->sideList.items[sides[i]];
        if (side->vertices.count > 3 && 
            pointStatusFromPlane([side->face boundary], &vertex->vector) == PS_INSIDE && 
            polygonShape(&side->vertices, [side->face norm]) != PS_CONVEX)
            return -1;
    }
    
    for (int i = sideCount - 1; i >= 0; i--) {
        TSide* side = vd->sideList.items[sides[i]];
        if (side->vertices.count > 3) {
            EPointStatus vertexStatus = pointStatusFromPlane([side->face boundary], &vertex->vector);
            if (vertexStatus == PS_INSIDE) { // vertex is still coplanar
                centerOfVertices(&side->vertices, &side->center);
            } else if (vertexStatus == PS_ABOVE) { // vertex is moved "above" the face, so triangulate it
                if (*newFaces == nil)
                    *newFaces = [NSMutableArray array];
                if (*removedFaces == nil)
                    *removedFaces = [NSMutableArray array];
             
                triangulateFace(vd, side, v, *newFaces);

                [*removedFaces addObject:side->face];
                deleteSide(vd, sides[i]);
            } else if (vertexStatus == PS_BELOW) { // vertex is moved "below" the face, so split off one triangle
                if (*newFaces == nil)
                    *newFaces = [NSMutableArray array];
                
                id <Face> newFace = splitFace(vd, side, v);
                [*newFaces addObject:newFace];
            }
        } else if (side->vertices.count == 3) {
            boundary.point = side->vertices.items[0]->vector;
            
            subV3f(&side->vertices.items[1]->vector, &side->vertices.items[0]->vector, &v1);
            subV3f(&side->vertices.items[2]->vector, &side->vertices.items[0]->vector, &v2);
            crossV3f(&v2, &v1, &boundary.norm);
            normalizeV3f(&boundary.norm, &boundary.norm);
            
            makePointsForPlane(&boundary, [side->face worldBounds], &p1, &p2, &p3);
            [side->face setPoint1:&p1 point2:&p2 point3:&p3];

            centerOfVertices(&side->vertices, &side->center);
        }
    }

    for (int i = 0; i < vd->sideList.count; i++) {
        TSide* side = vd->sideList.items[i];
        const TPlane* boundary = [side->face boundary];
        for (int j = 0; j < side->edges.count; j++) {
            TEdge* edge = side->edges.items[j];
            TSide* neighbour = edge->leftSide != side ? edge->leftSide : edge->rightSide;
            if (equalPlane(boundary, [neighbour->face boundary])) {
                mergeSides(side, j);
                
                if (*removedFaces == nil)
                    *removedFaces = [NSMutableArray array];
                [*removedFaces addObject:neighbour->face];
                
                deleteEdge(vd, edgeIndex(&vd->edgeList, edge));
                deleteSide(vd, sideIndex(&vd->sideList, neighbour));
                
                centerOfVertices(&side->vertices, &side->center);

                i -= 1;
                break;
            }
        }
    }
    
    boundsOfVertexData(vd, &vd->bounds, &vd->center);
    
    return v;
}
