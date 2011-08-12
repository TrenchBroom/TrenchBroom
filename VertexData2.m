//
//  VertexData2.m
//  TrenchBroom
//
//  Created by Kristian Duske on 12.08.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "VertexData2.h"
#import "Face.h"
#import "MutableFace.h"

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
        return e->startVertex;
    else if (e->rightSide == s)
        return e->endVertex;
    return NULL;
}

TVertex* endVertexOfEdge(const TEdge* e, const TSide* s) {
    if (e->leftSide == s)
        return e->endVertex;
    else if (e->rightSide == s)
        return e->startVertex;
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

void splitEdge(const TPlane* p, TEdge* e, TVertex* v) {
    TLine l;
    setLinePoints(&l, &e->startVertex->vector, &e->endVertex->vector);
    
    float dist = intersectPlaneWithLine(p, &l);
    linePointAtDistance(&l, dist, &v->vector);
    
    if (e->startVertex->mark == VM_DROP)
        e->startVertex = v;
    else
        e->endVertex = v;
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

    for (int i = 0; i < c; i++) {
        if (!f[i])
            e[i]->rightSide = s;
        else
            e[i]->leftSide = s;
        s->edges[i] = e[i];
        s->vertices[i] = startVertexOfEdge(e[i], s);
    }
    
    s->edgeCount = c;
}

void initSideWithFace(MutableFace* f, TEdge** e, int c, TSide* s) {
    s->face = f;
    if (s->edges != NULL)
        free(s->edges);
    if (s->vertices != NULL)
        free(s->vertices);
    
    s->edges = malloc(c * sizeof(TEdge *));
    s->vertices = malloc(c * sizeof(TVertex *));

    for (int i = 0; i < c; i++) {
        e[i]->leftSide = s;
        s->edges[i] = e[i];
        s->vertices[i] = startVertexOfEdge(e[i], s);
    }
    
    s->edgeCount = c;
    [f setSide:s];
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
    s->face = nil;
    s->mark = SM_UNKNOWN;
}

BOOL splitSide(TSide* s, TEdge* e) {
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
        return NO;
    }
    
    if (undecided == 1 && keep == s->edgeCount - 1) {
        s->mark = SM_KEEP;
        e = undecidedEdge;
        return YES;
    }
    
    if (drop + undecided == s->edgeCount) {
        s->mark = drop;
        return NO;
    }
    
    s->mark = SM_SPLIT;
    
    e->startVertex = endVertexOfEdge(s->edges[splitIndex1], s);
    e->endVertex = startVertexOfEdge(s->edges[splitIndex2], s);
    e->rightSide = s;
    e->mark = EM_NEW;

    int newEdgeCount;
    TEdge** newEdges;
    TVertex** newVertices;
    
    if (splitIndex2 > splitIndex1) {
        newEdgeCount = s->edgeCount - (splitIndex2 - splitIndex1 - 1) + 1;
        newEdges = malloc(newEdgeCount * sizeof(TEdge *));
        
        int j = 0;
        for (int i = 0; i <= splitIndex1; i++, j++) {
            newEdges[j] = s->edges[i];
            newVertices[j] = startVertexOfEdge(s->edges[i], s);
        }
        
        newEdges[j] = e;
        newVertices[j] = startVertexOfEdge(e, s);
        j++;
        
        for (int i = splitIndex2; i < s->edgeCount; i++, j++) {
            newEdges[j++] = s->edges[i];
            newVertices[j] = startVertexOfEdge(s->edges[i], s);
        }
    } else {
        newEdgeCount = splitIndex1 - splitIndex2 + 2;
        newEdges = malloc(newEdgeCount * sizeof(TEdge *));
        
        int j = 0;
        for (int i = splitIndex2; i <= splitIndex1; i++, j++) {
            newEdges[j++] = s->edges[i];
            newVertices[j] = startVertexOfEdge(s->edges[i], s);
        }
        newEdges[j] = e;
        newVertices[j] = startVertexOfEdge(e, s);
    }

    free(s->vertices);
    free(s->edges);
    s->vertices = newVertices;
    s->edges = newEdges;
    s->edgeCount = newEdgeCount;
    
    return YES;
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

void initVertexDataWithBounds(TVertexData* vd, const TBoundingBox* b) {
    freeVertexData(vd);
    vd->vertexCount = 8;
    vd->vertices = malloc(vd->vertexCount * sizeof(TVertex));
    vd->vertexCapacity = vd->vertexCount;
    vd->edgeCount = 12;
    vd->edges = malloc(vd->edgeCount * sizeof(TEdge));
    vd->edgeCapacity = vd->edgeCount;
    vd->sideCount = 6;
    vd->sides = malloc(vd->sideCount * sizeof(TSide));
    vd->sideCapacity = vd->sideCount;
    
    const TVector3f* min = &b->min;
    const TVector3f* max = &b->max;

    int esb = 0;
    vd->vertices[esb].vector.x  = min->x - 1;
    vd->vertices[esb].vector.y  = min->y - 1;
    vd->vertices[esb].vector.z  = min->z - 1;
    vd->vertices[esb].mark      = VM_UNKNOWN;
    
    int est = 1;
    vd->vertices[est].vector.x  = min->x - 1;
    vd->vertices[est].vector.y  = min->y - 1;
    vd->vertices[est].vector.z  = max->z + 1;
    vd->vertices[est].mark      = VM_UNKNOWN;
    
    int enb = 2;
    vd->vertices[enb].vector.x  = min->x - 1;
    vd->vertices[enb].vector.y  = max->y + 1;
    vd->vertices[enb].vector.z  = min->z - 1;
    vd->vertices[enb].mark      = VM_UNKNOWN;
    
    int ent = 3;
    vd->vertices[ent].vector.x  = min->x - 1;
    vd->vertices[ent].vector.y  = max->y + 1;
    vd->vertices[ent].vector.z  = max->z + 1;
    vd->vertices[ent].mark      = VM_UNKNOWN;

    int wsb = 4;
    vd->vertices[wsb].vector.x  = max->x + 1;
    vd->vertices[wsb].vector.y  = min->y - 1;
    vd->vertices[wsb].vector.z  = min->z - 1;
    vd->vertices[wsb].mark      = VM_UNKNOWN;

    int wst = 5;
    vd->vertices[wst].vector.x  = max->x + 1;
    vd->vertices[wst].vector.y  = min->y - 1;
    vd->vertices[wst].vector.z  = max->z + 1;
    vd->vertices[wst].mark      = VM_UNKNOWN;
    
    int wnb = 6;
    vd->vertices[wnb].vector.x  = max->x + 1;
    vd->vertices[wnb].vector.y  = max->y + 1;
    vd->vertices[wnb].vector.z  = min->z - 1;
    vd->vertices[wnb].mark      = VM_UNKNOWN;
    
    int wnt = 7;
    vd->vertices[wnt].vector.x  = max->x + 1;
    vd->vertices[wnt].vector.y  = max->y + 1;
    vd->vertices[wnt].vector.z  = max->z + 1;
    vd->vertices[wnt].mark      = VM_UNKNOWN;
    
    int esbwsb = 0;
    vd->edges[esbwsb].startVertex   = &vd->vertices[esb];
    vd->edges[esbwsb].endVertex     = &vd->vertices[wsb];
    vd->edges[esbwsb].mark          = EM_UNKNOWN;

    int wsbwst = 1;
    vd->edges[wsbwst].startVertex   = &vd->vertices[wsb];
    vd->edges[wsbwst].endVertex     = &vd->vertices[wst];
    vd->edges[wsbwst].mark          = EM_UNKNOWN;
    
    int wstest = 2;
    vd->edges[wstest].startVertex   = &vd->vertices[wst];
    vd->edges[wstest].endVertex     = &vd->vertices[est];
    vd->edges[wstest].mark          = EM_UNKNOWN;
    
    int estesb = 3;
    vd->edges[estesb].startVertex   = &vd->vertices[est];
    vd->edges[estesb].endVertex     = &vd->vertices[esb];
    vd->edges[estesb].mark          = EM_UNKNOWN;
    
    int wnbenb = 4;
    vd->edges[wnbenb].startVertex   = &vd->vertices[wnb];
    vd->edges[wnbenb].endVertex     = &vd->vertices[enb];
    vd->edges[wnbenb].mark          = EM_UNKNOWN;
    
    int enbent = 5;
    vd->edges[enbent].startVertex   = &vd->vertices[enb];
    vd->edges[enbent].endVertex     = &vd->vertices[ent];
    vd->edges[enbent].mark          = EM_UNKNOWN;
    
    int entwnt = 6;
    vd->edges[entwnt].startVertex   = &vd->vertices[ent];
    vd->edges[entwnt].endVertex     = &vd->vertices[wnt];
    vd->edges[entwnt].mark          = EM_UNKNOWN;
    
    int wntwnb = 7;
    vd->edges[wntwnb].startVertex   = &vd->vertices[wnt];
    vd->edges[wntwnb].endVertex     = &vd->vertices[wnb];
    vd->edges[wntwnb].mark          = EM_UNKNOWN;
    
    int enbesb = 8;
    vd->edges[enbesb].startVertex   = &vd->vertices[enb];
    vd->edges[enbesb].endVertex     = &vd->vertices[esb];
    vd->edges[enbesb].mark          = EM_UNKNOWN;
    
    int estent = 9;
    vd->edges[estent].startVertex   = &vd->vertices[est];
    vd->edges[estent].endVertex     = &vd->vertices[ent];
    vd->edges[estent].mark          = EM_UNKNOWN;
    
    int wsbwnb = 10;
    vd->edges[wsbwnb].startVertex   = &vd->vertices[wsb];
    vd->edges[wsbwnb].endVertex     = &vd->vertices[wnb];
    vd->edges[wsbwnb].mark          = EM_UNKNOWN;
    
    int wntwst = 11;
    vd->edges[wntwst].startVertex   = &vd->vertices[wnt];
    vd->edges[wntwst].endVertex     = &vd->vertices[wst];
    vd->edges[wntwst].mark          = EM_UNKNOWN;
    
    TEdge* southEdges[] = {&vd->edges[esbwsb], &vd->edges[estesb], &vd->edges[wstest], &vd->edges[wsbwst]};
    BOOL southFlipped[] = {YES, YES, YES, YES};
    initSideWithEdges(southEdges, southFlipped, 4, &vd->sides[0]);
    
    TEdge* northEdges[] = {&vd->edges[wnbenb], &vd->edges[wntwnb], &vd->edges[entwnt], &vd->edges[enbent]};
    BOOL northFlipped[] = {YES, YES, YES, YES};
    initSideWithEdges(northEdges, northFlipped, 4, &vd->sides[1]);
    
    TEdge* westEdges[] = {&vd->edges[wsbwnb], &vd->edges[wsbwst], &vd->edges[wntwst], &vd->edges[wntwnb]};
    BOOL westFlipped[] = {YES, NO, YES, NO};
    initSideWithEdges(westEdges, westFlipped, 4, &vd->sides[2]);
    
    TEdge* eastEdges[] = {&vd->edges[enbesb], &vd->edges[enbent], &vd->edges[estent], &vd->edges[estesb]};
    BOOL eastFlipped[] = {YES, NO, YES, NO};
    initSideWithEdges(eastEdges, eastFlipped, 4, &vd->sides[3]);
    
    TEdge* topEdges[] = {&vd->edges[wstest], &vd->edges[estent], &vd->edges[entwnt], &vd->edges[wntwst]};
    BOOL topFlipped[] = {NO, NO, NO, NO};
    initSideWithEdges(topEdges, topFlipped, 4, &vd->sides[4]);

    TEdge* bottomEdges[] = {&vd->edges[esbwsb], &vd->edges[wsbwnb], &vd->edges[wnbenb], &vd->edges[enbesb]};
    BOOL bottomFlipped[] = {NO, NO, NO, NO};
    initSideWithEdges(bottomEdges, bottomFlipped, 4, &vd->sides[5]);
    
    vd->valid = NO;
}

BOOL initVertexDataWithFaces(TVertexData* vd, const TBoundingBox* b, NSArray* f, NSMutableSet** d) {
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
        free(vd->vertices);
        vd->vertices = NULL;
    }
    if (vd->edges != NULL) {
        free(vd->edges);
        vd->edges = NULL;
    }
    if (vd->sides != NULL) {
        for (int i = 0; i < vd->sideCount; i++)
             freeSide(&vd->sides[i]);
        free(vd->sides);
        vd->sides = NULL;
    }
    vd->valid = NO;
}

void addVertex(TVertexData* vd, TVertex* v) {
    if (vd->vertexCount == vd->vertexCapacity) {
        TVertex* vt = vd->vertices;
        
        vd->vertexCapacity *= 2;
        vd->vertices = malloc(vd->vertexCapacity * sizeof(TVertex));
        memcpy(&vd->vertices, &vt, vd->vertexCount);
        free(vt);
    }
    
    vd->vertices[vd->vertexCount++] = *v;
}

void addEdge(TVertexData* vd, TEdge* e) {
    if (vd->edgeCount == vd->edgeCapacity) {
        TEdge* et = vd->edges;
        
        vd->edgeCapacity *= 2;
        vd->edges = malloc(vd->edgeCapacity * sizeof(TEdge));
        memcpy(&vd->edges, &et, vd->edgeCount);
        free(et);
    }
    
    vd->edges[vd->edgeCount++] = *e;
}

void addSide(TVertexData* vd, TSide* s) {
    if (vd->sideCount == vd->sideCapacity) {
        TSide* st = vd->sides;
        
        vd->sideCapacity *= 2;
        vd->sides = malloc(vd->sideCapacity * sizeof(TSide));
        memcpy(&vd->sides, &st, vd->sideCount);
        free(st);
    }
    
    vd->sides[vd->sideCount++] = *s;
}

void removeSide(TVertexData* vd, int s) {
    for (int i = s; i < vd->sideCount - 1; i++)
        vd->sides[i] = vd->sides[i + 1];
}

BOOL cutVertexData(TVertexData* vd, MutableFace* f, NSMutableSet** d) {
    const TPlane* p = [f boundary];
    
    int keep = 0;
    int drop = 0;
    int undecided = 0;
    
    // mark vertices
    for (int i = 0; i < vd->vertexCount; i++) {
        TVertex* v = &vd->vertices[i];
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
            *d = [NSMutableSet set];
        [*d addObject:f];
        return YES;
    }
    
    if (drop + undecided == vd->vertexCount)
        return NO;
    
    // mark and split edges
    int ec = 0;
    for (int i = 0; i < vd->edgeCount; i++) {
        TEdge* e = &vd->edges[i];
        updateEdgeMark(e);
        if (e->mark == EM_SPLIT) {
            TVertex v;
            v.mark = VM_UNKNOWN;
            
            splitEdge(p, e, &v);
            addVertex(vd, &v);
            ec++;
        }
    }
    
    // mark, split and drop sides
    TEdge* es = malloc(ec * sizeof(TEdge));
    int j = 0;
    for (int i = 0; i < vd->sideCount; i++) {
        TSide* s = &vd->sides[i];
        TEdge e;
        BOOL ex = splitSide(s, &e);
        
        if (s->mark == SM_DROP) {
            if (s->face != nil) {
                if (*d == nil)
                    *d = [NSMutableSet set];
                [*d addObject:s->face];
                [s->face setSide:NULL];
            }
            freeSide(s);
            removeSide(vd, i--);
        } else if (s->mark == SM_SPLIT) {
            addEdge(vd, &e);
            es[j++] = e;
            s->mark = SM_UNKNOWN;
        } else if (s->mark == SM_KEEP && ex) {
            // the edge is an undecided edge, so it needs to be flipped in order to act as a new edge
            if (e.rightSide != s)
                flipEdge(&e);
            es[j++] = e;
            s->mark = SM_UNKNOWN;
        } else {
            s->mark = SM_UNKNOWN;
        }
    }
    
    // create new side from newly created edges
    // first, sort the new edges to form a polygon in clockwise order
    for (int i = 0; i < ec - 1; i++) {
        TEdge* e = &es[i];
        for (int j = i + 2; j < ec; j++) {
            TEdge* c = &es[j];
            if (e->startVertex == c->endVertex) {
                TEdge t = es[j];
                es[j] = es[i + 1];
                es[i + 1] = t;
            }
        }
    }
    
    // now create the new side
    TSide side;
    initSideWithFace(f, &es, ec, &side);
    addSide(vd, &side);
    free(es);
    
    // clean up
    // delete dropped vertices
    int l = -1;
    int lc = 0;
    for (int i = 0; i < vd->vertexCount; i++) {
        if (vd->vertices[i].mark == VM_DROP) {
            if (l >= 0) {
                memcpy(&vd->vertices[l - lc], &vd->vertices[l + 1], (i - l - 1) * sizeof(TVertex));
                lc++;
            }
            l = i;
        } else {
            vd->vertices[i].mark = VM_UNKNOWN;
        }
    }
    
    if (l >= 0) {
        memcpy(&vd->vertices[l - lc], &vd->vertices[l + 1], (vd->vertexCount - l - 1) * sizeof(TVertex));
        vd->vertexCount -= ++lc;
    }
    
    // delete dropped edges
    l = -1;
    lc = 0;
    for (int i = 0; i < vd->edgeCount; i++) {
        if (vd->edges[i].mark == EM_DROP) {
            if (l >= 0) {
                memcpy(&vd->edges[l - lc], &vd->edges[l + 1], (i - l - 1) * sizeof(TEdge));
                lc++;
            }
            l = i;
        } else {
            vd->edges[i].mark = EM_UNKNOWN;
        }
    }
    
    if (l >= 0) {
        memcpy(&vd->edges[l - lc], &vd->edges[l + 1], (vd->edgeCount - l - 1) * sizeof(TEdge));
        vd->edgeCount -= ++lc;
    }
    
    vd->valid = NO;
    return YES;
}

void validateVertexData(TVertexData* vd) {
    if (!vd->valid) {
        vd->bounds.min = vd->vertices[0].vector;
        vd->bounds.max = vd->vertices[0].vector;
        vd->center = vd->vertices[0].vector;
        
        for  (int i = 1; i < vd->vertexCount; i++) {
            mergeBoundsWithPoint(&vd->bounds, &vd->vertices[i].vector, &vd->bounds);
            addV3f(&vd->center, &vd->vertices[i].vector, &vd->center);
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
