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

#import "VertexData.h"
#import "Face.h"
#import "MutableFace.h"
#import "assert.h"

int const PS_CONVEX = -1;
int const PS_CONCAVE = -2;

void initIndexList(TIndexList* l, int c) {
    assert(c >= 0);
    
    l->capacity = c;
    l->count = 0;
    l->items = malloc(c * sizeof(int));
}

void addIndexToList(TIndexList* l, int i) {
    assert(l != NULL);
    
    if (l->count == l->capacity) {
        int* t = l->items;
        l->capacity *= 2;
        l->items = malloc(l->capacity * sizeof(int));
        memcpy(l->items, t, l->count * sizeof(int));
        free(t);
    }
    
    l->items[l->count++] = i;
}

void removeIndexFromList(TIndexList* l, int i) {
    assert(l != NULL);
    assert(i >= 0 && i < l->count);
    
    if (i < l->count - 1)
        memcpy(&l->items[i], &l->items[i + 1], (l->count - i - 1) * sizeof(int));
    
    l->count--;
}

void removeSuffixFromIndexList(TIndexList* l, int i) {
    assert(l != NULL);
    assert(i >= 0 && i < l->count);
    
    l->count = i;
}

void clearIndexList(TIndexList* l) {
    l->count = 0;
}

void appendIndexList(const TIndexList* s, int si, int sc, TIndexList* d) {
    for (int i = si; i < mini(s->count - si, sc); i++)
        addIndexToList(d, s->items[i]);
}

int indexIndex(const TIndexList* l, int i) {
    for (int j = 0; j < l->count; j++)
        if (l->items[j] == i)
            return j;
    return -1;
}

void freeIndexList(TIndexList* l) {
    assert(l->items != NULL);
    free(l->items);
    l->items = NULL;
    l->count = 0;
    l->capacity = 0;
}

void initVertexList(TVertexList* l, int c) {
    assert(c >= 0);
    
    l->capacity = c;
    l->count = 0;
    l->items = malloc(c * sizeof(TVertex **));
}

void addVertexToList(TVertexList* l, TVertex* v) {
    assert(l != NULL);
    assert(v != NULL);
    
    if (l->count == l->capacity) {
        TVertex** t = l->items;
        l->capacity *= 2;
        l->items = malloc(l->capacity * sizeof(TVertex **));
        memcpy(l->items, t, l->count * sizeof(TVertex **));
        free(t);
    }

    l->items[l->count++] = v;
}

void removeVertexFromList(TVertexList* l, int i) {
    assert(l != NULL);
    assert(i >= 0 && i < l->count);
    
    if (i < l->count - 1)
        memcpy(&l->items[i], &l->items[i + 1], (l->count - i - 1) * sizeof(TVertex **));

    l->count--;
}

void removeSuffixFromVertexList(TVertexList* l, int i) {
    assert(l != NULL);
    assert(i >= 0 && i < l->count);
    
    l->count = i;
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
    l->items = malloc(c * sizeof(TEdge **));
}

void addEdgeToList(TEdgeList* l, TEdge* e) {
    assert(l != NULL);
    assert(e != NULL);
    
    if (l->count == l->capacity) {
        TEdge** t = l->items;
        l->capacity *= 2;
        l->items = malloc(l->capacity * sizeof(TEdge **));
        memcpy(l->items, t, l->count * sizeof(TEdge **));
        free(t);
    }
    
    l->items[l->count++] = e;
}

void removeEdgeFromList(TEdgeList* l, int i) {
    assert(l != NULL);
    assert(i >= 0 && i < l->count);
    
    if (i < l->count - 1)
        memcpy(&l->items[i], &l->items[i + 1], (l->count - i - 1) * sizeof(TEdge **));
    
    l->count--;
    l->items[l->count] = NULL;
}

void removeSuffixFromEdgeList(TEdgeList* l, int i) {
    assert(l != NULL);
    assert(i >= 0 && i < l->count);
    
    l->count = i;
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
    l->items = malloc(c * sizeof(TSide **));
}

void addSideToList(TSideList* l, TSide* s) {
    assert(l != NULL);
    assert(s != NULL);
    
    if (l->count == l->capacity) {
        TSide** t = l->items;
        l->capacity *= 2;
        l->items = malloc(l->capacity * sizeof(TSide **));
        memcpy(l->items, t, l->count * sizeof(TSide **));
        free(t);
    }
    
    l->items[l->count++] = s;
}

void removeSideFromList(TSideList* l, int i) {
    assert(l != NULL);
    assert(i >= 0 && i < l->count);
    
    if (i < l->count - 1)
        memcpy(&l->items[i], &l->items[i + 1], (l->count - i - 1) * sizeof(TSide **));
    
    l->count--;
    l->items[l->count] = NULL;
}

void removeSuffixFromSideList(TSideList* l, int i) {
    assert(l != NULL);
    assert(i >= 0 && i < l->count);
    
    l->count = i;
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

void centerOfVertices(const TVertexList* v, TVector3f* c) {
    assert(v->count > 0);
    
    *c = v->items[0]->position;
    for (int i = 1; i < v->count; i++)
        addV3f(c, &v->items[i]->position, c);
    scaleV3f(c, 1.0f / v->count, c);
}


void boundsOfVertices(const TVertexList* l, TBoundingBox* b) {
    b->min = l->items[0]->position;
    b->max = l->items[0]->position;
    
    for  (int i = 1; i < l->count; i++)
        mergeBoundsWithPoint(b, & l->items[i]->position, b);
}

void edgeVector(const TEdge* e, TVector3f* v) {
    subV3f(&e->endVertex->position, &e->startVertex->position, v);
}

void centerOfEdge(const TEdge* e, TVector3f* v) {
    subV3f(&e->endVertex->position, &e->startVertex->position, v);
    scaleV3f(v, 0.5f, v);
    addV3f(&e->startVertex->position, v, v);
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
    setLinePoints(&line, &e->startVertex->position, &e->endVertex->position);
    
    float dist = intersectPlaneWithLine(p, &line);
    linePointAtDistance(&line, dist, &newVertex->position);
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
    projectOntoCoordinatePlane(cPlane, &v->position, &v0);
    subV3f(&v0, &pis, &v0);
    
    int c = 0;
    for (int i = 0; i < s->vertices.count; i++) {
        v = s->vertices.items[i];
        projectOntoCoordinatePlane(cPlane, &v->position, &v1);
        subV3f(&v1, &pis, &v1);
        
        if ((fzero(v0.x) && fzero(v0.y)) || (fzero(v1.x) && fzero(v1.y))) {
            // the point is identical to a polygon vertex, cancel search
//            NSLog(@"face %@ discarded because ray hit a vertex", s->face);
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
    
    if (c % 2 == 0) {
//        NSLog(@"face %@ discarded because hit count was %i", s->face, c);
        return NAN;
    }
    
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
    vd->vertices.capacity = 0;
    vd->vertices.count = 0;
    vd->vertices.items = NULL;
    vd->edges.capacity = 0;
    vd->edges.count = 0;
    vd->edges.items = NULL;
    vd->sides.capacity = 0;
    vd->sides.count = 0;
    vd->sides.items = NULL;
    vd->bounds.min = NullVector;
    vd->bounds.max = NullVector;
}

void initVertexDataWithBounds(TVertexData* vd, const TBoundingBox* b) {
    vd->vertices.capacity = 8;
    vd->vertices.count = 0;
    vd->vertices.items = malloc(vd->vertices.capacity * sizeof(TVertex *));
    vd->edges.capacity = 12;
    vd->edges.count = 0;
    vd->edges.items = malloc(vd->edges.capacity * sizeof(TEdge *));
    vd->sides.capacity = 6;
    vd->sides.count = 0;
    vd->sides.items = malloc(vd->sides.capacity * sizeof(TSide *));

    const TVector3f* min = &b->min;
    const TVector3f* max = &b->max;
    
    vd->bounds.min.x = min->x - 1;
    vd->bounds.min.y = min->y - 1;
    vd->bounds.min.z = min->z - 1;
    vd->bounds.max.x = max->x + 1;
    vd->bounds.max.y = max->y + 1;
    vd->bounds.max.z = max->z + 1;
    
    TVertex* esb = malloc(sizeof(TVertex));
    esb->position.x  = min->x - 1;
    esb->position.y  = min->y - 1;
    esb->position.z  = min->z - 1;
    esb->mark      = VM_UNKNOWN;
    addVertex(vd, esb);
    
    TVertex* est = malloc(sizeof(TVertex));
    est->position.x  = min->x - 1;
    est->position.y  = min->y - 1;
    est->position.z  = max->z + 1;
    est->mark      = VM_UNKNOWN;
    addVertex(vd, est);
    
    TVertex* enb = malloc(sizeof(TVertex));
    enb->position.x  = min->x - 1;
    enb->position.y  = max->y + 1;
    enb->position.z  = min->z - 1;
    enb->mark      = VM_UNKNOWN;
    addVertex(vd, enb);
    
    TVertex* ent = malloc(sizeof(TVertex));
    ent->position.x  = min->x - 1;
    ent->position.y  = max->y + 1;
    ent->position.z  = max->z + 1;
    ent->mark      = VM_UNKNOWN;
    addVertex(vd, ent);

    TVertex* wsb = malloc(sizeof(TVertex));
    wsb->position.x  = max->x + 1;
    wsb->position.y  = min->y - 1;
    wsb->position.z  = min->z - 1;
    wsb->mark      = VM_UNKNOWN;
    addVertex(vd, wsb);

    TVertex* wst = malloc(sizeof(TVertex));
    wst->position.x  = max->x + 1;
    wst->position.y  = min->y - 1;
    wst->position.z  = max->z + 1;
    wst->mark      = VM_UNKNOWN;
    addVertex(vd, wst);
    
    TVertex* wnb = malloc(sizeof(TVertex));
    wnb->position.x  = max->x + 1;
    wnb->position.y  = max->y + 1;
    wnb->position.z  = min->z - 1;
    wnb->mark      = VM_UNKNOWN;
    addVertex(vd, wnb);
    
    TVertex* wnt = malloc(sizeof(TVertex));
    wnt->position.x  = max->x + 1;
    wnt->position.y  = max->y + 1;
    wnt->position.z  = max->z + 1;
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
    if (vd->vertices.items != NULL) {
        for (int i = 0; i < vd->vertices.count; i++)
            free(vd->vertices.items[i]);
        freeVertexList(&vd->vertices);
    }
    if (vd->edges.items != NULL) {
        for (int i = 0; i < vd->edges.count; i++)
            free(vd->edges.items[i]);
        freeEdgeList(&vd->edges);
    }
    if (vd->sides.items != NULL) {
        for (int i = 0; i < vd->sides.count; i++) {
            freeSide(vd->sides.items[i]);
            free(vd->sides.items[i]);
        }
        freeSideList(&vd->sides);
    }
}

void addVertex(TVertexData* vd, TVertex* v) {
    addVertexToList(&vd->vertices, v);
}

void deleteVertex(TVertexData* vd, int v) {
    assert(vd != NULL);
    assert(v >= 0 && v < vd->vertices.count);
    
    free(vd->vertices.items[v]);
    removeVertexFromList(&vd->vertices, v);
}

void addEdge(TVertexData* vd, TEdge* e) {
    addEdgeToList(&vd->edges, e);
}

void deleteEdge(TVertexData* vd, int e) {
    assert(vd != NULL);
    assert(e >= 0 && e < vd->edges.count);
    
    free(vd->edges.items[e]);
    removeEdgeFromList(&vd->edges, e);
}

void addSide(TVertexData* vd, TSide* s) {
    addSideToList(&vd->sides, s);
}

void deleteSide(TVertexData* vd, int s) {
    assert(vd != NULL);
    assert(s >= 0 && s < vd->sides.count);
    
    freeSide(vd->sides.items[s]);
    free(vd->sides.items[s]);
    removeSideFromList(&vd->sides, s);
}

ECutResult cutVertexData(TVertexData* vd, MutableFace* f, NSMutableArray** d) {
    const TPlane* p = [f boundary];
    
    int keep = 0;
    int drop = 0;
    int undecided = 0;
    
    // mark vertices
    for (int i = 0; i < vd->vertices.count; i++) {
        TVertex* v = vd->vertices.items[i];
        EPointStatus vs = pointStatusFromPlane(p, &v->position);
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
    
    if (keep + undecided == vd->vertices.count)
        return CR_REDUNDANT;
    
    if (drop + undecided == vd->vertices.count)
        return CR_NULL;
    
    // mark and split edges
    for (int i = 0; i < vd->edges.count; i++) {
        TEdge* edge = vd->edges.items[i];
        updateEdgeMark(edge);
        if (edge->mark == EM_SPLIT) {
            TVertex* newVertex = splitEdge(p, edge);
            addVertex(vd, newVertex);
        }
    }
    
    // mark, split and drop sides
    TEdgeList newEdges;
    initEdgeList(&newEdges, vd->sides.count); // alloc enough space for new edges
    for (int i = 0; i < vd->sides.count; i++) {
        TSide* side = vd->sides.items[i];
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
    for (int i = 0; i < vd->vertices.count; i++)
        if (vd->vertices.items[i]->mark == VM_DROP)
            deleteVertex(vd, i--);
        else
            vd->vertices.items[i]->mark = VM_UNDECIDED;
    
    // delete dropped edges
    for (int i = 0; i < vd->edges.count; i++)
        if (vd->edges.items[i]->mark == EM_DROP)
            deleteEdge(vd, i--);
        else
            vd->edges.items[i]->mark = EM_UNDECIDED;

    boundsOfVertices(&vd->vertices, &vd->bounds);
    
    return CR_SPLIT;
}

void translateVertexData(TVertexData* vd, const TVector3f* d) {
    for (int i = 0; i < vd->vertices.count; i++)
        addV3f(&vd->vertices.items[i]->position, d, &vd->vertices.items[i]->position);

    translateBounds(&vd->bounds, d, &vd->bounds);
}

void rotateVertexData90CW(TVertexData* vd, EAxis a, const TVector3f* c) {
    for (int i = 0; i < vd->vertices.count; i++) {
        subV3f(&vd->vertices.items[i]->position, c, &vd->vertices.items[i]->position);
        rotate90CWV3f(&vd->vertices.items[i]->position, a, &vd->vertices.items[i]->position);
        addV3f(&vd->vertices.items[i]->position, c, &vd->vertices.items[i]->position);
    }

    rotateBounds90CW(&vd->bounds, a, c, &vd->bounds);
}

void rotateVertexData90CCW(TVertexData* vd, EAxis a, const TVector3f* c) {
    for (int i = 0; i < vd->vertices.count; i++) {
        subV3f(&vd->vertices.items[i]->position, c, &vd->vertices.items[i]->position);
        rotate90CCWV3f(&vd->vertices.items[i]->position, a, &vd->vertices.items[i]->position);
        addV3f(&vd->vertices.items[i]->position, c, &vd->vertices.items[i]->position);
    }
    
    rotateBounds90CCW(&vd->bounds, a, c, &vd->bounds);
}

void rotateVertexData(TVertexData* vd, const TQuaternion* r, const TVector3f* c) {
    for (int i = 0; i < vd->vertices.count; i++) {
        subV3f(&vd->vertices.items[i]->position, c, &vd->vertices.items[i]->position);
        rotateQ(r, &vd->vertices.items[i]->position, &vd->vertices.items[i]->position);
        addV3f(&vd->vertices.items[i]->position, c, &vd->vertices.items[i]->position);
    }
    
    rotateBounds(&vd->bounds, r, c, &vd->bounds);
}

void flipVertexData(TVertexData* vd, EAxis a, const TVector3f* c) {
    float min, max;
    switch (a) {
        case A_X:
            for (int i = 0; i < vd->vertices.count; i++) {
                vd->vertices.items[i]->position.x -= c->x;
                vd->vertices.items[i]->position.x *= -1;
                vd->vertices.items[i]->position.x += c->x;
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
            break;
        case A_Y:
            for (int i = 0; i < vd->vertices.count; i++) {
                vd->vertices.items[i]->position.y -= c->y;
                vd->vertices.items[i]->position.y *= -1;
                vd->vertices.items[i]->position.y += c->y;
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
            break;
        default:
            for (int i = 0; i < vd->vertices.count; i++) {
                vd->vertices.items[i]->position.z -= c->z;
                vd->vertices.items[i]->position.z *= -1;
                vd->vertices.items[i]->position.z += c->z;
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
            break;
    }
    
    for (int i = 0; i < vd->edges.count; i++)
        flipEdge(vd->edges.items[i]);
    
    for (int i = 0; i < vd->sides.count; i++)
        flipSide(vd->sides.items[i]);
}

BOOL vertexDataContainsPoint(TVertexData* vd, TVector3f* p) {
    for (int i = 0; i < vd->sides.count; i++)
        if (pointStatusFromPlane([vd->sides.items[i]->face boundary], p) == PS_ABOVE)
            return NO;
    return YES;
}

EPointStatus vertexStatusFromRay(const TVector3f* o, const TVector3f* d, const TVertexList* ps) {
    int above = 0;
    int below = 0;
    for (int i = 0; i < ps->count; i++) {
        EPointStatus status = pointStatusFromRay(o, d, &ps->items[i]->position);
        if (status == PS_ABOVE)
            above++;
        else if (status == PS_BELOW)
            below++;
        if (above > 0 && below > 0)
            return PS_INSIDE;
    }
    
    return above > 0 ? PS_ABOVE : PS_BELOW;
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
    
    const TVector3f* v1 = &p->items[p->count - 2]->position;
    const TVector3f* v2 = &p->items[p->count - 1]->position;
    const TVector3f* v3 = &p->items[0]->position;
    
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
        
        v3 = &p->items[i]->position;
        subV3f(v3, v2, &e2);
        crossV3f(&e2, &e1, &c);
        
        if (nullV3f(&c))
            return i - 1;
        
        if (pos != (dotV3f(&c, a) / d) > 0)
            return PS_CONCAVE;
    }
    
    return PS_CONVEX;
}

/**
 * This method merges a given side s with another side that shares at least one
 * edge with s. The shared edges and the shared vertices are deleted from the 
 * given polyhedron.
 *
 * @param vd the polyhedron
 * @param s the side which should be merged
 * @param si the index of the shared edge (relative to s)
 */
void mergeSides(TVertexData* vd, TSide* s, int si) {
    TVertex* v;
    TEdge* e = s->edges.items[si];
    TSide* n = e->leftSide != s ? e->leftSide : e->rightSide;
    int ni = edgeIndex(&n->edges, e);

    do {
        si = (si + 1) % s->edges.count;
        ni = (ni - 1 + n->edges.count) % n->edges.count;
    } while (s->edges.items[si] == n->edges.items[ni]);

    // now si points to the last edge (in CW order) of s that should not be deleted
    // and ni points to the first edge (in CW order) of n that should not be deleted
    
    int c = -1;
    do {
        si = (si - 1 + s->edges.count) % s->edges.count;
        ni = (ni + 1) % n->edges.count;
        c++;
    } while (s->edges.items[si] == n->edges.items[ni]);
    
    // now si points to the first edge (in CW order) of s that should not be deleted
    // now ni points to the last edge (in CW order) of n that should not be deleted
    // and c is the number of shared edges between s and n
    
    // shift the two sides so that their shared edges are at the end of both's edge lists
    shiftSide(s, (si + c + 1) % s->edges.count);
    shiftSide(n, ni);
    
    removeSuffixFromEdgeList(&s->edges, s->edges.count - c);
    removeSuffixFromVertexList(&s->vertices, s->vertices.count - c);
    
    for (int i = 0; i < n->edges.count - c; i++) {
        e = n->edges.items[i];
        v = n->vertices.items[i];
        if (e->leftSide == n)
            e->leftSide = s;
        else
            e->rightSide = s;
        addEdgeToList(&s->edges, e);
        addVertexToList(&s->vertices, v);
    }
    
    for (int i = n->edges.count - c; i < n->edges.count; i++) {
        deleteEdge(vd, edgeIndex(&vd->edges, n->edges.items[i]));
        if (i > n->edges.count - c)
            deleteVertex(vd, vertexIndex(&vd->vertices, n->vertices.items[i]));
    }
    
    deleteSide(vd, sideIndex(&vd->sides, n));
}

/**
 * This method determines the sides which are incident to the vertex with the
 * given index in the given polyhedron and adds them to the given side list in
 * clockwise order.
 *
 * @param vd the polyhedron
 * @param v the index of the vertex
 * @param l the list to which the incident sides are added in clockwise order
 */
void incidentSides(TVertexData* vd, int v, TSideList* l) {
    TVertex* vertex = vd->vertices.items[v];
    
    // find any edge that is incident to v
    TEdge* edge = NULL;
    for (int i = 0; i < vd->edges.count && edge == NULL; i++) {
        TEdge* candidate = vd->edges.items[i];
        if (candidate->startVertex == vertex || candidate->endVertex == vertex)
            edge = candidate;
    }
    
    TSide* side = edge->startVertex == vertex ? edge->rightSide : edge->leftSide;
    do {
        addSideToList(l, side);
        int i = edgeIndex(&side->edges, edge);
        edge = side->edges.items[(i - 1 + side->edges.count) % side->edges.count];
        side = edge->startVertex == vertex ? edge->rightSide : edge->leftSide;
    } while (side != l->items[0]);
}

void updateFaceOfSide(TSide* s) {
    int indices[3];
    int indexCount;
    TVector3f t;
    TVector3i p1, p2, p3;

    indexCount = 0;
    for (int i = 0; i < s->vertices.count && indexCount < 3; i++) {
        snapV3f(&s->vertices.items[i]->position, &t);
        if (equalV3f(&s->vertices.items[i]->position, &t)) {
            s->vertices.items[i]->position = t;
            indices[indexCount++] = i;
        }
    }
    
    assert(indexCount == 3);

    roundV3f(&s->vertices.items[indices[0]]->position, &p1);
    roundV3f(&s->vertices.items[indices[1]]->position, &p2);
    roundV3f(&s->vertices.items[indices[2]]->position, &p3);
    
    [s->face setPoint1:&p1 point2:&p2 point3:&p3];
}

void createFaceForSide(const TBoundingBox* w, TSide* s) {
    s->face = [[MutableFace alloc] initWithWorldBounds:w];
    [s->face setSide:s];
}

void deleteDegenerateTriangle(TVertexData* vd, TSide* s, TEdge* e, NSMutableArray* newFaces, NSMutableArray* removedFaces) {
    assert(s->edges.count == 3);
    
    shiftSide(s, edgeIndex(&s->edges, e));

    TEdge* keep = s->edges.items[1];
    TEdge* delete = s->edges.items[2];
    TSide* neighbor = delete->leftSide == s ? delete->rightSide : delete->leftSide;
    
    if (keep->leftSide == s)
        keep->leftSide = neighbor;
    else
        keep->rightSide = neighbor;
    
    int deleteIndex = edgeIndex(&neighbor->edges, delete);
    int prevIndex = (deleteIndex - 1 + neighbor->edges.count) % neighbor->edges.count;
    int nextIndex = (deleteIndex + 1) % neighbor->edges.count;
    replaceEdges(neighbor, prevIndex, nextIndex, keep);

    NSUInteger faceIndex = [newFaces indexOfObjectIdenticalTo:s->face];
    if (faceIndex != NSNotFound)
        [newFaces removeObjectAtIndex:faceIndex];
    else
        [removedFaces addObject:s->face];
    
    deleteSide(vd, sideIndex(&vd->sides, s));
    deleteEdge(vd, edgeIndex(&vd->edges, delete));
}

void triangulateFace(TVertexData* vd, TSide* s, int v, NSMutableArray* newFaces) {
    TSide* newSide;
    TVertex* vertex = vd->vertices.items[v];
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
        
        createFaceForSide(worldBounds, newSide);
        [newSide->face setTexture:[s->face texture]];
        [newSide->face setXOffset:[s->face xOffset]];
        [newSide->face setYOffset:[s->face yOffset]];
        [newSide->face setXScale:[s->face xScale]];
        [newSide->face setYScale:[s->face yScale]];
        [newSide->face setRotation:[s->face rotation]];
        [newFaces addObject:newSide->face];
        [newSide->face release];
        
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
    
    createFaceForSide(worldBounds, newSide);
    [newSide->face setTexture:[s->face texture]];
    [newSide->face setXOffset:[s->face xOffset]];
    [newSide->face setYOffset:[s->face yOffset]];
    [newSide->face setXScale:[s->face xScale]];
    [newSide->face setYScale:[s->face yScale]];
    [newSide->face setRotation:[s->face rotation]];
    [newFaces addObject:newSide->face];
    [newSide->face release];
}

void splitFace(TVertexData* vd, TSide* s, int v, NSMutableArray* newFaces) {
    TSide* newSide;
    TVertex* vertex = vd->vertices.items[v];
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
    
    createFaceForSide(worldBounds, newSide);
    [newSide->face setTexture:[s->face texture]];
    [newSide->face setXOffset:[s->face xOffset]];
    [newSide->face setYOffset:[s->face yOffset]];
    [newSide->face setXScale:[s->face xScale]];
    [newSide->face setYScale:[s->face yScale]];
    [newSide->face setRotation:[s->face rotation]];
    [newFaces addObject:newSide->face];
    [newSide->face release];
}

int performVertexDrag(TVertexData* vd, int v, const TVector3f d, NSMutableArray* newFaces, NSMutableArray* removedFaces) {
    TVertex* vertex;
    TVector3f newPosition, v1, v2;
    TRay dragRay;
    float dragDist;
    TSideList incSides;
    int vIndex;
    
    assert(vd != NULL);
    assert(v >= 0 && v < vd->vertices.count);
    
    vIndex = v;
    
    dragDist = lengthV3f(&d);
    if (dragDist == 0)
        return vIndex;
    
    assert(sanityCheck(vd, NO));
    
    vertex = vd->vertices.items[vIndex];
    dragRay.origin = vertex->position;
    scaleV3f(&d, 1 / dragDist, &dragRay.direction);
    
    // split all sides incident to v so that there remain only triangles
    // incident to v - it doesn't matter that some of those are coplanar, they
    // will be merged later
    
    initSideList(&incSides, vd->sides.count);
    incidentSides(vd, vIndex, &incSides);
    
    for (int i = 0; i < incSides.count; i++) {
        TSide* side = incSides.items[i];
        if (side->vertices.count > 3) {
            subV3f(&side->vertices.items[2]->position, &side->vertices.items[0]->position, &v1);
            subV3f(&side->vertices.items[1]->position, &side->vertices.items[0]->position, &v2);
            crossV3f(&v1, &v2, &v1);
            
            if (fneg(dotV3f(&v1, &dragRay.direction))) {
                splitFace(vd, side, vIndex, newFaces);
            } else {
                triangulateFace(vd, side, vIndex, newFaces);
                NSUInteger faceIndex = [newFaces indexOfObjectIdenticalTo:side->face];
                if (faceIndex != NSNotFound)
                    [newFaces removeObjectAtIndex:faceIndex];
                else
                    [removedFaces addObject:side->face];
                deleteSide(vd, sideIndex(&vd->sides, side));
            }
        }
    }
    
    assert(sanityCheck(vd, YES));
    
    // now find the shortest drag distance that will result in a merge of sides
    clearSideList(&incSides);
    incidentSides(vd, vIndex, &incSides);
    float actualDragDist = dragDist;
    
    for (int i = 0; i < incSides.count; i++) {
        TSide* side = incSides.items[i];
        TSide* succ = incSides.items[(i + 1) % incSides.count];
        
        shiftSide(side, vertexIndex(&side->vertices, vertex));
        shiftSide(succ, vertexIndex(&succ->vertices, vertex));
        
        TPlane plane;
        setPlanePointsV3f(&plane, &side->vertices.items[1]->position, 
                          &side->vertices.items[2]->position, 
                          &succ->vertices.items[2]->position);
        
        float sideDragDist = intersectPlaneWithRay(&plane, &dragRay);
        
        TEdge* neighborEdge = side->edges.items[1];
        TSide* neighbor = neighborEdge->leftSide != side ? neighborEdge->leftSide : neighborEdge->rightSide;
        float neighborDragDist = intersectPlaneWithRay([neighbor->face boundary], &dragRay);
        
        if (!isnan(sideDragDist) && fpos(sideDragDist) && flt(sideDragDist, actualDragDist))
            actualDragDist = sideDragDist;
        if (!isnan(neighborDragDist) && fpos(neighborDragDist) && flt(neighborDragDist, actualDragDist))
            actualDragDist = neighborDragDist;
    }
    
    // apply the shortest drag to the vertex
    rayPointAtDistance(&dragRay, actualDragDist, &vertex->position);
    newPosition = vertex->position;
    
    freeSideList(&incSides);
    
    // check whether the vertex is dragged onto another vertex
    for (int i = 0; i < vd->vertices.count; i++) {
        if (i != vIndex) {
            TVertex* vertexCandidate = vd->vertices.items[i];
            if (equalV3f(&vertex->position, &vertexCandidate->position)) {
                // find the edge incident to both vertex and candidate
                TEdge* delete = NULL;
                for (int j = 0; j < vd->edges.count && delete == NULL; j++) {
                    TEdge* edge = vd->edges.items[j];
                    if ((edge->startVertex == vertex && edge->endVertex == vertexCandidate) ||
                        (edge->endVertex == vertex && edge->startVertex == vertexCandidate))
                        delete = edge;
                }
                
                // because the algorithm should not allow non-adjacent vertices to be merged in the first place
                assert(delete != NULL); 
                assert(delete->leftSide->vertices.count == 3);
                assert(delete->rightSide->vertices.count == 3);
                
                for (int j = 0; j < vd->edges.count; j++) {
                    TEdge* edge = vd->edges.items[j];
                    if (edge != delete && (edge->startVertex == vertexCandidate || edge->endVertex == vertexCandidate)) {
                        if (edge->startVertex == vertexCandidate)
                            edge->startVertex = vertex;
                        else
                            edge->endVertex = vertex;
                        
                        int index = vertexIndex(&edge->leftSide->vertices, vertexCandidate);
                        if (index != -1)
                            edge->leftSide->vertices.items[index] = vertex;
                        
                        index = vertexIndex(&edge->rightSide->vertices, vertexCandidate);
                        if (index != -1)
                            edge->rightSide->vertices.items[index] = vertex;
                    }
                }
                
                deleteDegenerateTriangle(vd, delete->leftSide, delete, newFaces, removedFaces);
                deleteDegenerateTriangle(vd, delete->rightSide, delete, newFaces, removedFaces);
                
                deleteEdge(vd, edgeIndex(&vd->edges, delete));
                deleteVertex(vd, vertexIndex(&vd->vertices, vertexCandidate));
                
                break;
            }
        }
    }
    
    assert(sanityCheck(vd, YES));
    
    // now merge all mergeable sides back together
    for (int i = 0; i < vd->sides.count; i++) {
        TSide* side = vd->sides.items[i];
        TPlane sideBoundary;
        setPlanePointsV3f(&sideBoundary, 
                          &side->vertices.items[0]->position, 
                          &side->vertices.items[1]->position, 
                          &side->vertices.items[2]->position);

        for (int j = 0; j < side->edges.count; j++) {
            TEdge* edge = side->edges.items[j];
            TSide* neighbor = edge->leftSide != side ? edge->leftSide : edge->rightSide;
            TPlane neighborBoundary;
            setPlanePointsV3f(&neighborBoundary, 
                              &neighbor->vertices.items[0]->position, 
                              &neighbor->vertices.items[1]->position, 
                              &neighbor->vertices.items[2]->position);
            
            if (equalPlane(&sideBoundary, &neighborBoundary)) {
                id <Face> neighborFace = neighbor->face;
                mergeSides(vd, side, j);
                
                NSUInteger faceIndex = [newFaces indexOfObjectIdenticalTo:neighborFace];
                if (faceIndex != NSNotFound)
                    [newFaces removeObjectAtIndex:faceIndex];
                else
                    [removedFaces addObject:neighborFace];
                
                i -= 1;
                break;
            }
        }
    }
    
    assert(sanityCheck(vd, NO));
    
    // check for consecutive edges that can be merged
    for (int i = 0; i < vd->edges.count; i++) {
        TEdge* edge = vd->edges.items[i];
        edgeVector(edge, &v1);
        for (int j = i + 1; j < vd->edges.count; j++) {
            TEdge* candidate = vd->edges.items[j];
            edgeVector(candidate, &v2);
            crossV3f(&v1, &v2, &v2);
            if (nullV3f(&v2)) {
                if (edge->endVertex == candidate->endVertex)
                    flipEdge(candidate);
                if (edge->endVertex == candidate->startVertex) {
                    assert(edge->leftSide == candidate->leftSide);
                    assert(edge->rightSide == candidate->rightSide);
                    
                    TSide* leftSide = edge->leftSide;
                    TSide* rightSide = edge->rightSide;
                    
                    assert(leftSide != rightSide);
                    
                    TEdge* newEdge = malloc(sizeof(TEdge));
                    newEdge->mark = EM_UNKNOWN;
                    newEdge->leftSide = leftSide;
                    newEdge->rightSide = rightSide;
                    newEdge->startVertex = edge->startVertex;
                    newEdge->endVertex = candidate->endVertex;
                    
                    addEdge(vd, newEdge);
                    
                    int leftIndex = edgeIndex(&leftSide->edges, candidate);
                    int leftCount = leftSide->edges.count;
                    int rightIndex = edgeIndex(&rightSide->edges, candidate);
                    int rightCount = rightSide->edges.count;
                    
                    replaceEdges(leftSide, (leftIndex - 1 + leftCount) % leftCount, (leftIndex + 2) % leftCount, newEdge);
                    replaceEdges(rightSide, (rightIndex - 2 + rightCount) % rightCount, (rightIndex + 1) % rightCount, newEdge);
                    
                    deleteVertex(vd, vertexIndex(&vd->vertices, candidate->startVertex));
                    deleteEdge(vd, edgeIndex(&vd->edges, candidate));
                    deleteEdge(vd, edgeIndex(&vd->edges, edge));
                    
                    break;
                }
                
                if (edge->startVertex == candidate->startVertex)
                    flipEdge(candidate);
                if (edge->startVertex == candidate->endVertex) {
                    assert(edge->leftSide == candidate->leftSide);
                    assert(edge->rightSide == candidate->rightSide);
                    
                    TSide* leftSide = edge->leftSide;
                    TSide* rightSide = edge->rightSide;
                    
                    assert(leftSide != rightSide);
                    
                    TEdge* newEdge = malloc(sizeof(TEdge));
                    newEdge->mark = EM_UNKNOWN;
                    newEdge->leftSide = leftSide;
                    newEdge->rightSide = rightSide;
                    newEdge->startVertex = candidate->startVertex;
                    newEdge->endVertex = edge->endVertex;
                    
                    addEdge(vd, newEdge);
                    
                    int leftIndex = edgeIndex(&leftSide->edges, candidate);
                    int leftCount = leftSide->edges.count;
                    int rightIndex = edgeIndex(&rightSide->edges, candidate);
                    int rightCount = rightSide->edges.count;
                    
                    replaceEdges(leftSide, (leftIndex - 2 + leftCount) % leftCount, (leftIndex + 1) % leftCount, newEdge);
                    replaceEdges(rightSide, (rightIndex - 1 + rightCount) % rightCount, (rightIndex + 2) % rightCount, newEdge);
                    
                    deleteVertex(vd, vertexIndex(&vd->vertices, candidate->endVertex));
                    deleteEdge(vd, edgeIndex(&vd->edges, candidate));
                    deleteEdge(vd, edgeIndex(&vd->edges, edge));
                    
                    break;
                }
            }
        }
    }
    
    assert(sanityCheck(vd, YES));
    
    boundsOfVertices(&vd->vertices, &vd->bounds);
    
    // find the index of the dragged vertex
    vIndex = -1;
    for (int i = 0; i < vd->vertices.count; i++) {
        if (equalV3f(&newPosition, &vd->vertices.items[i]->position)) {
            vIndex = i;
            break;
        }
    }
    
    // drag is concluded
    if (vIndex == -1 || actualDragDist == dragDist) {
        for (int i = 0; i < vd->sides.count; i++)
            updateFaceOfSide(vd->sides.items[i]);
        return vIndex;
    }
    
    // drag is not concluded, calculate the new delta and call self
    scaleV3f(&dragRay.direction, dragDist - actualDragDist, &dragRay.direction);
    return performVertexDrag(vd, vIndex, dragRay.direction, newFaces, removedFaces);
}

int splitAndDragEdge(TVertexData* vd, int e, const TVector3f d, NSMutableArray* newFaces, NSMutableArray* removedFaces) {
    TEdge* edge;
    TVertex* vertex;
    TVector3f edgeVertices[2];
    int result;
    int index;

    index = e - vd->vertices.count;
    edge = vd->edges.items[index];
    
    // detect whether the drag would make the incident faces invalid
    if (fneg(dotV3f(&d, [edge->leftSide->face norm])) ||
        fneg(dotV3f(&d, [edge->rightSide->face norm])))
        return e;
    
    edgeVertices[0] = edge->startVertex->position;
    edgeVertices[1] = edge->endVertex->position;
    
    // split the edge
    shiftSide(edge->leftSide, edgeIndex(&edge->leftSide->edges, edge) + 1);
    shiftSide(edge->rightSide, edgeIndex(&edge->rightSide->edges, edge) + 1);
    
    vertex = malloc(sizeof(TVertex));
    vertex->mark = VM_UNKNOWN;
    centerOfEdge(edge, &vertex->position);
    
    addVertex(vd, vertex);
    addVertexToList(&edge->leftSide->vertices, vertex);
    addVertexToList(&edge->rightSide->vertices, vertex);
    
    TEdge* newEdge1 = malloc(sizeof(TEdge));
    newEdge1->mark = EM_UNKNOWN;
    newEdge1->leftSide = edge->leftSide;
    newEdge1->rightSide = edge->rightSide;
    newEdge1->startVertex = edge->startVertex;
    newEdge1->endVertex = vertex;
    
    TEdge* newEdge2 = malloc(sizeof(TEdge));
    newEdge2->mark = EM_UNKNOWN;
    newEdge2->leftSide = edge->leftSide;
    newEdge2->rightSide = edge->rightSide;
    newEdge2->startVertex = vertex;
    newEdge2->endVertex = edge->endVertex;
    
    removeEdgeFromList(&edge->leftSide->edges, edge->leftSide->edges.count - 1);
    removeEdgeFromList(&edge->rightSide->edges, edge->rightSide->edges.count - 1);
    
    addEdge(vd, newEdge1);
    addEdge(vd, newEdge2);
    addEdgeToList(&edge->leftSide->edges, newEdge2);
    addEdgeToList(&edge->leftSide->edges, newEdge1);
    addEdgeToList(&edge->rightSide->edges, newEdge1);
    addEdgeToList(&edge->rightSide->edges, newEdge2);
    
    deleteEdge(vd, index);

    result = performVertexDrag(vd, vd->vertices.count - 1, d, newFaces, removedFaces);
    
    for (int i = 0; i < vd->edges.count && result == -1; i++) {
        TEdge* edge = vd->edges.items[i];
        if ((equalV3f(&edge->startVertex->position, &edgeVertices[0]) && equalV3f(&edge->endVertex->position, &edgeVertices[1])) ||
            (equalV3f(&edge->startVertex->position, &edgeVertices[1]) && equalV3f(&edge->endVertex->position, &edgeVertices[0]))) {
            result = i;
        }
    }
    
    
    return result;
}

int splitAndDragSide(TVertexData* vd, int s, const TVector3f d, NSMutableArray* newFaces, NSMutableArray* removedFaces) {
    TSide* side;
    TVertex* vertex;
    TVector3f* sideVertices;
    int sideVertexCount;
    int result;    
    int index;
    
    index = s - vd->edges.count - vd->vertices.count;
    side = vd->sides.items[index];
    
    // detect whether the drag would lead to an indented face
    if (!fpos(dotV3f(&d, [side->face norm])))
        return s;
    
    // store the side's vertices for later
    sideVertexCount = side->vertices.count;
    sideVertices = malloc(sideVertexCount * sizeof(TVector3f));
    for (int i = 0; i < side->vertices.count; i++)
        sideVertices[i] = side->vertices.items[i]->position;
    
    vertex = malloc(sizeof(TVertex));
    vertex->mark = VM_UNKNOWN;
    centerOfVertices(&side->vertices, &vertex->position);
    
    addVertex(vd, vertex);
    
    TEdge* firstEdge = malloc(sizeof(TEdge));
    firstEdge->mark = EM_UNKNOWN;
    firstEdge->startVertex = vertex;
    firstEdge->endVertex = startVertexOfEdge(side->edges.items[0], side);
    addEdge(vd, firstEdge);
    
    TEdge* lastEdge = firstEdge;
    for (int i = 0; i < side->edges.count; i++) {
        TEdge* sideEdge = side->edges.items[i];
        
        TEdge* newEdge;
        if (i == side->edges.count - 1) {
            newEdge = firstEdge;
        } else {
            newEdge = malloc(sizeof(TEdge));
            newEdge->mark = EM_UNKNOWN;
            newEdge->startVertex = vertex;
            newEdge->endVertex = endVertexOfEdge(sideEdge, side);
            addEdge(vd, newEdge);
        }
        
        TSide* newSide = malloc(sizeof(TSide));
        initVertexList(&newSide->vertices, 3);
        initEdgeList(&newSide->edges, 3);
        
        addVertexToList(&newSide->vertices, vertex);
        addEdgeToList(&newSide->edges, lastEdge);
        lastEdge->rightSide = newSide;
        
        addVertexToList(&newSide->vertices, lastEdge->endVertex);
        addEdgeToList(&newSide->edges, sideEdge);
        if (sideEdge->leftSide == side) {
            sideEdge->leftSide = newSide;
        } else {
            sideEdge->rightSide = newSide;
        }
        
        addVertexToList(&newSide->vertices, newEdge->endVertex);
        addEdgeToList(&newSide->edges, newEdge);
        newEdge->leftSide = newSide;
        
        addSide(vd, newSide);
        
        createFaceForSide([side->face worldBounds], newSide);
        [newSide->face setTexture:[side->face texture]];
        [newSide->face setXOffset:[side->face xOffset]];
        [newSide->face setYOffset:[side->face yOffset]];
        [newSide->face setXScale:[side->face xScale]];
        [newSide->face setYScale:[side->face yScale]];
        [newSide->face setRotation:[side->face rotation]];
        [newFaces addObject:newSide->face];
        [newSide->face release];
        
        lastEdge = newEdge;
    }
    
    [removedFaces addObject:side->face];
    deleteSide(vd, index);
    
    result = performVertexDrag(vd, vd->vertices.count - 1, d, newFaces, removedFaces);

    for (int i = 0; i < vd->sides.count && result == -1; i++) {
        side = vd->sides.items[i];
        if (sideVertexCount == side->vertices.count) {
            for (int j = 0; j < sideVertexCount && result == -1; j++) {
                int k = 0;
                while (k < sideVertexCount && equalV3f(&side->vertices.items[(j + k) % sideVertexCount]->position, &sideVertices[k]))
                    k++;
                
                if (k == sideVertexCount)
                    result = i;
            }
        }
    }

    free(sideVertices);
    
    return result;
}

int dragVertex(TVertexData* vd, int v, const TVector3f d, NSMutableArray* newFaces, NSMutableArray* removedFaces) {
    assert(vd != NULL);
    assert(v >= 0 && v < vd->vertices.count + vd->edges.count + vd->sides.count);
    
    if (lengthV3f(&d) == 0)
        return v;

    if (v < vd->vertices.count)
        return performVertexDrag(vd, v, d, newFaces, removedFaces);
    if (v < vd->vertices.count + vd->edges.count)
        return splitAndDragEdge(vd, v, d, newFaces, removedFaces);
    return splitAndDragSide(vd, v, d, newFaces, removedFaces);
}

int dragEdge(TVertexData* vd, int e, TVector3f d, NSMutableArray* newFaces, NSMutableArray* removedFaces) {
    TEdge* edge;
    TVector3f start, end, dir;
    int result;
    
    assert(vd != NULL);
    assert(e >= 0 && e < vd->edges.count);

    if (lengthV3f(&d) == 0)
        return e;
    
    edge = vd->edges.items[e];
    start = edge->startVertex->position;
    end = edge->endVertex->position;
    subV3f(&end, &start, &dir);
    
    addV3f(&start, &d, &start);
    addV3f(&end, &d, &end);
    
    if (dotV3f(&dir, &d) > 0) {
        dragVertex(vd, vertexIndex(&vd->vertices, edge->endVertex), d, newFaces, removedFaces);
        dragVertex(vd, vertexIndex(&vd->vertices, edge->startVertex), d, newFaces, removedFaces);
    } else {
        dragVertex(vd, vertexIndex(&vd->vertices, edge->startVertex), d, newFaces, removedFaces);
        dragVertex(vd, vertexIndex(&vd->vertices, edge->endVertex), d, newFaces, removedFaces);
    }
    
    result = -1;
    for (int i = 0; i < vd->edges.count && result == -1; i++) {
        edge = vd->edges.items[i];
        if ((equalV3f(&edge->startVertex->position, &start) && equalV3f(&edge->endVertex->position, &end)) ||
            (equalV3f(&edge->startVertex->position, &end) && equalV3f(&edge->endVertex->position, &start)))
            result = i;
    }

    return result;
}

int dragSide(TVertexData* vd, int s, TVector3f d, NSMutableArray* newFaces, NSMutableArray* removedFaces) {
    TSide* side;
    TVector3f center, diff, dir;
    TVector3f* sideVertices;
    float length;
    int* indices;
    float* dots;
    int sideVertexCount;
    int result;
    BOOL switched;

    assert(vd != NULL);
    assert(s >= 0 && s < vd->sides.count);
    
    length = lengthV3f(&d);
    if (length == 0)
        return s;

    scaleV3f(&d, 1 / length, &dir);
    
    side = vd->sides.items[s];
    centerOfVertices(&side->vertices, &center);
    
    sideVertexCount = side->vertices.count;
    sideVertices = malloc(sideVertexCount * sizeof(TVector3f));
    indices = malloc(sideVertexCount * sizeof(int));
    dots = malloc(sideVertexCount * sizeof(float));
    
    for (int i = 0; i < side->vertices.count; i++) {
        sideVertices[i] = side->vertices.items[i]->position;
        subV3f(&sideVertices[i], &center, &diff);
        normalizeV3f(&diff, &diff);
        dots[i] = dotV3f(&diff, &dir);
        indices[i] = vertexIndex(&vd->vertices, side->vertices.items[i]);
        
        addV3f(&sideVertices[i], &d, &sideVertices[i]);
    }

    // sort indices by dot value, eek, bubblesort
    switched = YES;
    for (int j = sideVertexCount - 1; j >= 0 && switched; j--) {
        switched = NO;
        for (int i = 0; i < j; i++) {
            if (dots[i] > dots[i + 1]) {
                float dt = dots[i];
                dots[i] = dots[i + 1];
                dots[i + 1] = dt;
                
                int di = indices[i];
                indices[i] = indices[i + 1];
                indices[i + 1] = di;
                switched = YES;
            }
        }
    }
    
    for (int i = 0; i < sideVertexCount; i++)
        dragVertex(vd, indices[i], d, newFaces, removedFaces);
    
    result = -1;
    for (int i = 0; i < vd->sides.count && result == -1; i++) {
        side = vd->sides.items[i];
        if (sideVertexCount == side->vertices.count) {
            for (int j = 0; j < sideVertexCount && result == -1; j++) {
                int k = 0;
                while (k < sideVertexCount && equalV3f(&side->vertices.items[(j + k) % sideVertexCount]->position, &sideVertices[k]))
                    k++;
                
                if (k == sideVertexCount)
                    result = i;
            }
        }
    }
    
    free(sideVertices);
    free(indices);
    free(dots);
    
    return result;
}

void snapVertexData(TVertexData* vd) {
    assert(sanityCheck(vd, YES));

    for (int i = 0; i < vd->vertices.count; i++)
        snapV3f(&vd->vertices.items[i]->position, &vd->vertices.items[i]->position);
    
    // in some cases, we may now have an invalid brush, which must be fixed.
    
    for (int i = 0; i < vd->sides.count; i++)
        updateFaceOfSide(vd->sides.items[i]);
    
    assert(sanityCheck(vd, YES));
}

BOOL sanityCheck(const TVertexData* vd, BOOL cc) {
    // check Euler characteristic http://en.wikipedia.org/wiki/Euler_characteristic
    int sideCount = 0;
    for (int i = 0; i < vd->sides.count; i++)
        if (vd->sides.items[i]->face != nil)
            sideCount++;
    if (vd->vertices.count - vd->edges.count + sideCount != 2) {
        NSLog(@"failed Euler check");
        return NO;
    }

    for (int i = 0; i < vd->edges.count; i++) {
        if (vd->edges.items[i]->leftSide == vd->edges.items[i]->rightSide) {
            NSLog(@"edge with index %i has equal sides", i);
            return NO;
        }
    }
    
    int vVisits[vd->vertices.count];
    for (int i = 0; i < vd->vertices.count; i++)
        vVisits[i] = 0;

    int eVisits[vd->edges.count];
    for (int i = 0; i < vd->edges.count; i++)
        eVisits[i] = 0;
    
    for (int i = 0; i < vd->sides.count; i++) {
        TSide* side = vd->sides.items[i];
        /*
        if (cc && polygonShape(&side->vertices, [side->face norm]) != PS_CONVEX) {
            NSLog(@"side with index %i is not convex", i);
            return NO;
        }
         */
        
        for (int j = 0; j < side->edges.count; j++) {
            TEdge* edge = side->edges.items[j];
            if (edge->leftSide != side && edge->rightSide != side) {
                NSLog(@"edge with index %i of side with index %i does not actually belong to it", j, i);
                return NO;
            }

            int index = edgeIndex(&vd->edges, edge);
            if (index == -1) {
                NSLog(@"edge with index %i of side with index %i is missing from vertex data", j, i);
                return NO;
            }
            eVisits[index]++;

            TVertex* vertex = startVertexOfEdge(edge, side);
            if (side->vertices.items[j] != vertex) {
                NSLog(@"start vertex of edge with index %i of side with index %i is not at position %i in the side's vertex list", j, i, j);
                return NO;
            }
            
            index = vertexIndex(&vd->vertices, vertex);
            if (index == -1) {
                NSLog(@"start vertex of edge with index %i of side with index %i is missing from vertex data", j, i);
                return NO;
            }
            vVisits[index]++;
        }
    }
    
    for (int i = 0; i < vd->vertices.count; i++) {
        if (vVisits[i] == 0) {
            NSLog(@"vertex with index %i does not belong to any side", i);
            return NO;
        }
        
        for (int j = i + 1; j < vd->vertices.count; j++)
            if (equalV3f(&vd->vertices.items[i]->position, &vd->vertices.items[j]->position)) {
                NSLog(@"vertex with index %i is identical to vertex with index %i", i, j);
                return NO;
            }
    }
    
    for (int i = 0; i < vd->edges.count; i++) {
        if (eVisits[i] != 2) {
            NSLog(@"edge with index %i was visited %i times, should have been 2", i, eVisits[i]);
            return NO;
        }
    }
    
    return YES;
}
