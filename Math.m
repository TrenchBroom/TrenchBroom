//
//  Math.m
//  TrenchBroom
//
//  Created by Kristian Duske on 24.08.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "Math.h"
#import "math.h"
#import "Vector2f.h"
#import "Vector3f.h"

float const AlmostZero = 0.001f;

BOOL fzero(float v) {
    return fabsf(v) <= AlmostZero;
}

BOOL fpos(float v) {
    return v > AlmostZero;
}

BOOL fneg(float v) {
    return v < -AlmostZero;
}

BOOL feq(float v1, float v2) {
    return fabsf(v1 - v2) < AlmostZero;
}

BOOL fgt(float v1, float v2) {
    return v1 > v2 + AlmostZero;
}

BOOL flt(float v1, float v2) {
    return v1 < v2 - AlmostZero;
}

BOOL fgte(float v1, float v2) {
    return !flt(v1, v2);
}

BOOL flte(float v1, float v2) {
    return !fgt(v1, v2);
}
             
BOOL finxx(float v, float b1, float b2) {
    return b1 < b2 ? fgt(v, b1) && flt(v, b2) : fgt(v, b2) && flt(v, b1);
}

BOOL finxi(float v, float b1, float b2) {
    return b1 < b2 ? fgt(v, b1) && flte(v, b2) : fgt(v, b2) && flte(v, b1);
}

BOOL finix(float v, float b1, float b2) {
    return b1 < b2 ? fgte(v, b1) && flt(v, b2) : fgte(v, b2) && flt(v, b1);
}

BOOL finii(float v, float b1, float b2) {
    return b1 < b2 ? fgte(v, b1) && flte(v, b2) : fgte(v, b2) && flte(v, b1);
}

#pragma mark TVector3f functions

TVector3f* newV3f(TVector3f* v) {
    TVector3f* r = malloc(sizeof(TVector3f));
    setV3f(r, v);
    return r;
}

void setV3f(TVector3f* l, TVector3f* r) {
    l->x = r->x;
    l->y = r->y;
    l->z = r->z;
}

void addV3f(TVector3f* l, TVector3f* r) {
    l->x += r->x;
    l->y += r->y;
    l->z += r->z;
}

void subV3f(TVector3f* l, TVector3f* r) {
    l->x -= r->x;
    l->y -= r->y;
    l->z -= r->z;
}

float dotV3f(TVector3f* l, TVector3f* r) {
    return l->x * r->x + l->y * r->y + l->z * r->z;
}

void crossV3f(TVector3f* l, TVector3f* r) {
    float xt = l->y * r->z - l->z * r->y;
    float yt = l->z * r->x - l->x * r->z;
    l->z = l->x * r->y - l->y * r->x;
    l->x = xt;
    l->y = yt;
}

void scaleV3f(TVector3f* v, float f) {
    v->x *= f;
    v->y *= f;
    v->z *= f;
}

float lengthV3f(TVector3f* v) {
    return sqrt(lengthSquaredV3f(v));
}

float lengthSquaredV3f(TVector3f* v) {
    return dotV3f(v, v);
}

void normalizeV3f(TVector3f* v) {
    float l = lengthV3f(v);
    v->x /= l;
    v->y /= l;
    v->z /= l;
}

BOOL equalV3f(TVector3f* l, TVector3f* r) {
    return feq(l->x, r->x) && feq(l->y, r->y) && feq(l->z, r->z);
}

# pragma mark TPlane functions

BOOL pointAbovePlane(TPlane* p, TVector3f* v) {
    TVector3f* t = newV3f(v);
    subV3f(t, &p->point);

    BOOL a = fpos(dotV3f(&p->norm, t));
    free(t);
    return a;
}

float intersectPlaneWithLine(TPlane* p, TLine* l) {
    float d = dotV3f(&l->direction, &p->norm);
    if (fzero(d))
        return NAN;
    
    TVector3f* v = newV3f(&p->point);
    subV3f(v, &l->point);
    float r = dotV3f(v, &p->norm) / d;

    free(v);
    return r;
}

float intersectPlaneWithRay(TPlane* p, TRay* r) {
    float d = dotV3f(&r->direction, &p->norm);
    if (fzero(d))
        return NAN;
    
    TVector3f* v = newV3f(&p->point);
    subV3f(v, &r->origin);
    float s = dotV3f(v, &p->norm) / d;
    free(v);
    
    if (fneg(s))
        return NAN;
    
    return s;
}

#pragma mark TRay functions

TVector3f* rayPointAtDistance(TRay* r, float d) {
    TVector3f* v = newV3f(&r->direction);
    scaleV3f(v, d);
    addV3f(v, &r->origin);
    return v;
}

#pragma mark TLine functions

TVector3f* linePointAtDistance(TLine* l, float d) {
    TVector3f* v = newV3f(&l->direction);
    scaleV3f(v, d);
    addV3f(v, &l->point);
    return v;
}

# pragma mark TQuaternion functions

void setQ(TQuaternion* l, TQuaternion* r) {
    l->scalar = r->scalar;
    setV3f(&l->vector, &r->vector);
}

void setAngleAndAxisQ(TQuaternion* q, float a, TVector3f* x) {
    q->scalar = cos(a / 2);
    setV3f(&q->vector, x);
    scaleV3f(&q->vector, sin(a / 2));
}

void mulQ(TQuaternion* l, TQuaternion* r) {
    float a = l->scalar;
    TVector3f* v = &l->vector;
    float b = r->scalar;
    TVector3f* w = &r->vector;
    
    float nx = a * w->x + b * v->x + v->y * w->z - v->z * w->y;
    float ny = a * w->y + b * v->y + v->z * w->x - v->x * w->z;
    float nz = a * w->z + b * v->z + v->x * w->y - v->y * w->x;
    
    a = a * b - dotV3f(v, w);
    v->x = nx;
    v->y = ny;
    v->z = nz;
}

void conjugateQ(TQuaternion* q) {
    scaleV3f(&q->vector, -1);
}

void rotateQ(TQuaternion* q, TVector3f* v) { 
    TQuaternion* t = malloc(sizeof(TQuaternion));
    setQ(t, q);

    TQuaternion* p = malloc(sizeof(TQuaternion));
    p->scalar = 0;
    setV3f(&p->vector, v);
    
    TQuaternion* c = malloc(sizeof(TQuaternion));
    setQ(c, q);
    conjugateQ(c);
    
    mulQ(t, p);
    mulQ(t, c);
    setV3f(v, &t->vector);
    
    free(t);
    free(p);
    free(c);
}


NSArray* makeCircle(float radius, int segments) {
    NSMutableArray* points = [[NSMutableArray alloc] initWithCapacity:segments];
    
    float d = 2 * M_PI / segments;
    float a = 0;
    for (int i = 0; i < segments; i++) {
        float s = sin(a);
        float c = cos(a);
        Vector3f* point = [[Vector3f alloc] initWithFloatX:radius * s y:radius * c z:0];
        [points addObject:point];
        [point release];
        a += d;
    }
    
    return [points autorelease];
}

NSArray* makeRing(float innerRadius, float outerRadius, int segments) {
    NSMutableArray* points = [[NSMutableArray alloc] initWithCapacity:2 * segments + 2];
    
    float d = 2 * M_PI / (2 * segments);
    float a = 0;
    for (int i = 0; i < 2 * segments; i++) {
        float s = sin(a);
        float c = cos(a);
        float r = i % 2 == 0 ? innerRadius : outerRadius;
        
        Vector3f* point = [[Vector3f alloc] initWithFloatX:r * s y:r * c z:0];
        [points addObject:point];
        [point release];
        a += d;
    }
    
    [points addObject:[points objectAtIndex:0]];
    [points addObject:[points objectAtIndex:1]];
    
    return [points autorelease];
}

int smallestXVertex2D(NSArray *vertices) {
    int s = 0;
    Vector3f* v = [vertices objectAtIndex:0];
    float x = [v x];
    float y = [v y];
    
    for (int i = 1; i < [vertices count]; i++) {
        v = [vertices objectAtIndex:i];
        if (flt([v x], x) || (feq([v x], x) && flt([v y], y))) {
            x = [v x];
            y = [v y];
            s = i;
        }
    }
    
    return s;
}

int smallestYVertex2D(NSArray *vertices)  {
    int s = 0;
    Vector3f* v = [vertices objectAtIndex:0];
    float x = [v x];
    float y = [v y];
    
    for (int i = 1; i < [vertices count]; i++) {
        v = [vertices objectAtIndex:i];
        if (flt([v y], y) || (feq([v y], y) && flt([v x], x))) {
            x = [v x];
            y = [v y];
            s = i;
        }
    }
    
    return s;
}
