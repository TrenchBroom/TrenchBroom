//
//  Math.m
//  TrenchBroom
//
//  Created by Kristian Duske on 24.08.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "Math.h"
#import "math.h"

float const AlmostZero = 0.001f;
TVector3f const XAxisPos = {+1,  0,  0};
TVector3f const XAxisNeg = {-1,  0,  0};
TVector3f const YAxisPos = { 0, +1,  0};
TVector3f const YAxisNeg = { 0, -1,  0};
TVector3f const ZAxisPos = { 0,  0, +1};
TVector3f const ZAxisNeg = { 0,  0, -1};
TVector3f const NullVector = {0, 0, 0};

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

void addV3f(const TVector3f* l, const TVector3f* r, TVector3f* o) {
    o->x = l->x + r->x;
    o->y = l->y + r->y;
    o->z = l->z + r->z;
}

void subV3f(const TVector3f* l, const TVector3f* r, TVector3f* o) {
    o->x = l->x - r->x;
    o->y = l->y - r->y;
    o->z = l->z - r->z;
}

float dotV3f(const TVector3f* l, const TVector3f* r) {
    return l->x * r->x + l->y * r->y + l->z * r->z;
}

void crossV3f(const TVector3f* l, const TVector3f* r, TVector3f* o) {
    float xt = l->y * r->z - l->z * r->y;
    float yt = l->z * r->x - l->x * r->z;
    o->z = l->x * r->y - l->y * r->x;
    o->x = xt;
    o->y = yt;
}

void scaleV3f(const TVector3f* v, float f, TVector3f* o) {
    o->x = f * v->x;
    o->y = f * v->y;
    o->z = f * v->z;
}

float lengthV3f(const TVector3f* v) {
    return sqrt(lengthSquaredV3f(v));
}

float lengthSquaredV3f(const TVector3f* v) {
    return dotV3f(v, v);
}

void normalizeV3f(const TVector3f* v, TVector3f* o) {
    float l = lengthV3f(v);
    o->x = v->x / l;
    o->y = v->y / l;
    o->z = v->z / l;
}

BOOL equalV3f(const TVector3f* l, const TVector3f* r) {
    return feq(l->x, r->x) && feq(l->y, r->y) && feq(l->z, r->z);
}

EAxis largestComponentV3f(const TVector3f* v) {
    float xa = fabs(v->x);
    float ya = fabs(v->y);
    float za = fabs(v->z);
    
    if (xa >= ya && xa >= za)
        return A_X;
    if (ya >= xa && ya >= za)
        return A_Y;
    return A_Z;
}

float componentV3f(const TVector3f* v, EAxis a) {
    switch (a) {
        case A_X:
            return v->x;
        case A_Y:
            return v->y;
        case A_Z:
            return v->z;
        default:
            [NSException raise:NSInvalidArgumentException format:@"invalid vector component %i", a];
            return NAN;
    }
}

void setComponentV3f(TVector3f* v, EAxis a, float f) {
    switch (a) {
        case A_X:
            v->x = f;
            break;
        case A_Y:
            v->y = f;
            break;
        case A_Z:
            v->z = f;
            break;
        default:
            [NSException raise:NSInvalidArgumentException format:@"invalid vector component %i", a];
    }
}

void roundV3f(const TVector3f* v, TVector3i* o) {
    o->x = roundf(v->x);
    o->y = roundf(v->y);
    o->z = roundf(v->z);
}

void setV3f(TVector3f* l, const TVector3i* r) {
    l->x = r->x;
    l->y = r->y;
    l->z = r->z;
}

BOOL parseV3f(NSString* s, NSRange r, TVector3f* o) {
    int comp = -1;
    BOOL dot = NO;
    int b, l;
    for (int i = r.location; i < r.location + r.length; i++) {
        char c = [s characterAtIndex:i];
        switch (c) {
            case '+':
            case '-':
                if (comp > 0) {
                    return NO;
                } else {
                    comp *= -1;
                    b = i;
                    l = 1;
                }
                break;
            case '.':
                if (dot)
                    return NO;
                else
                    dot = YES;
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                if (comp < 0) {
                    comp *= -1;
                    b = i;
                    l = 1;
                } else {
                    l++;
                }
                break;
            default:
                if (comp > 0) {
                    NSString* p = [s substringWithRange:NSMakeRange(b, l)];
                    float f = [p floatValue];
                    if (comp == 1)
                        o->x = f;
                    else if (comp == 2)
                        o->y = f;
                    else if (comp == 3)
                        o->z = f;
                    comp++;
                    comp *= -1;
                    dot = NO;
                }
                break;
        }
    }
    
    if (comp == 3) {
        NSString* p = [s substringWithRange:NSMakeRange(b, l)];
        o->z = [p floatValue];
    } else if (comp != -3) {
        return NO;
    }
    
    return YES;
}

# pragma mark TVector3i functions

void addV3i(const TVector3i* l, const TVector3i* r, TVector3i* o) {
    o->x = l->x + r->x;
    o->y = l->y + r->y;
    o->z = l->z + r->z;
}

void subV3i(const TVector3i* l, const TVector3i* r, TVector3i* o) {
    o->x = l->x - r->x;
    o->y = l->y - r->y;
    o->z = l->z - r->z;
}

BOOL equalV3i(const TVector3i* l, const TVector3i* r) {
    return l->x == r->x && l->y == r->y && l->z == r->z;
}

BOOL parseV3i(NSString* s, NSRange r, TVector3i* o) {
    int comp = -1;
    int b, l;
    for (int i = r.location; i < r.location + r.length; i++) {
        char c = [s characterAtIndex:i];
        switch (c) {
            case '+':
            case '-':
                if (comp > 0) {
                    return NO;
                } else {
                    comp *= -1;
                    b = i;
                    l = 1;
                }
                break;
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                if (comp < 0) {
                    comp *= -1;
                    b = i;
                    l = 1;
                } else {
                    l++;
                }
                break;
            default:
                if (comp > 0) {
                    NSString* p = [s substringWithRange:NSMakeRange(b, l)];
                    int f = [p intValue];
                    if (comp == 1)
                        o->x = f;
                    else if (comp == 2)
                        o->y = f;
                    else if (comp == 3)
                        o->z = f;
                    comp++;
                    comp *= -1;
                }
                break;
        }
    }
    
    if (comp == 3) {
        NSString* p = [s substringWithRange:NSMakeRange(b, l)];
        o->z = [p floatValue];
    } else if (comp != -3) {
        return NO;
    }
    
    return YES;
}


# pragma mark TVecto3fList functions

TVector3fList* newVector3fList(int c) {
    assert(c > 0);
    
    TVector3fList* l = malloc(sizeof(TVector3fList));
    l->items = malloc(c * sizeof(TVector3f));
    l->capacity = c;
    l->count = 0;
    return l;
}

void freeVector3flist(TVector3fList* l) {
    assert(l != NULL);
    
    free(l->items);
    l->items = NULL;
    free(l);
}

void addVector3fToList(TVector3f v, TVector3fList* l) {
    assert(l != NULL);
    
    if (l->count == l->capacity) {
        TVector3f* temp = malloc(2 * l->capacity * sizeof(TVector3f));
        free(l->items);
        l->items = temp;
        l->capacity *= 2;
    }
    
    l->items[l->count++] = v;
}

void removeVector3fFromList(int idx, TVector3fList* l) {
    assert(idx >= 0 && idx < l->count);
    
    for (int i = idx; i < l->count - 1; i++)
        l->items[i] = l->items[i + 1];
    l->count--;
}

# pragma mark TLine functions

void setLinePoints(TLine* l, TVector3f* p1, TVector3f* p2) {
    l->point = *p1;
    subV3f(p2, p1, &l->direction);
    normalizeV3f(&l->direction, &l->direction);
}

void linePointAtDistance(TLine* l, float d, TVector3f* p) {
    scaleV3f(&l->direction, d, p);
    addV3f(p, &l->point, p);
}

# pragma mark TPlane functions

void setPlanePoints(TPlane* p, const TVector3i* p1, const TVector3i* p2, const TVector3i* p3) {
    TVector3f v1, v2;
    
    setV3f(&p->point, p1);
    
    v1.x = p2->x - p1->x;
    v1.y = p2->y - p1->y;
    v1.z = p2->z - p1->z;

    v2.x = p3->x - p1->x;
    v2.y = p3->y - p1->y;
    v2.z = p3->z - p1->z;
    
    crossV3f(&v2, &v1, &p->norm);
    normalizeV3f(&p->norm, &p->norm);
}

EPointStatus pointStatus(const TPlane* p, const TVector3f* v) {
    TVector3f t;

    subV3f(v, &p->point, &t);
    float d = dotV3f(&p->norm, &t);
    if (fpos(d))
        return PS_ABOVE;
    
    if (fneg(d))
        return PS_BELOW;
    
    return PS_INSIDE;
}

float intersectPlaneWithLine(const TPlane* p, const TLine* l) {
    float d = dotV3f(&l->direction, &p->norm);
    if (fzero(d))
        return NAN;
    
    TVector3f v;
    subV3f(&p->point, &l->point, &v);
    return dotV3f(&v, &p->norm) / d;
}

float intersectPlaneWithRay(const TPlane* p, const TRay* r) {
    float d = dotV3f(&r->direction, &p->norm);
    if (fzero(d))
        return NAN;
    
    TVector3f v;
    subV3f(&p->point, &r->origin, &v);
    float s = dotV3f(&v, &p->norm) / d;
    if (fneg(s))
        return NAN;
    
    return s;
}

float planeX(TPlane* p, float y, float z) {
    float l = dotV3f(&p->norm, &p->point);
    return (l - p->norm.y * y - p->norm.z * z) / p->norm.x;
}

float planeY(TPlane* p, float x, float z) {
    float l = dotV3f(&p->norm, &p->point);
    return (l - p->norm.x * x - p->norm.z * z) / p->norm.y;
}

float planeZ(TPlane* p, float x, float y) {
    float l = dotV3f(&p->norm, &p->point);
    return (l - p->norm.x * x - p->norm.y * y) / p->norm.z;
}

#pragma mark TRay functions

float intersectSphereWithRay(const TVector3f* c, float ra, const TRay* r) {
    TVector3f diff;
    subV3f(&r->origin, c, &diff);
    
    float p = 2 * dotV3f(&r->direction, &diff);
    float q = lengthSquaredV3f(&diff) - ra * ra;
    
    float d = p * p - 4 * q;
    if (d < 0)
        return NAN;
    
    float s = sqrt(d);
    float t0 = -p + s;
    float t1 = -p - s;
    
    if (t0 < 0 && t1 < 0)
        return NAN;
    if (t0 > 0 && t1 > 0)
        return fmin(t0, t1);
    return fmax(t0, t1);
}

void rayPointAtDistance(const TRay* r, float d, TVector3f* p) {
    scaleV3f(&r->direction, d, p);
    addV3f(p, &r->origin, p);
}

#pragma mark TLine functions

#pragma mark TBoundingBox functions

void centerOfBounds(const TBoundingBox* b, TVector3f* o) {
    subV3f(&b->max, &b->min, o);
    scaleV3f(o, 0.5f, o);
    addV3f(o, &b->min, o);
}

void mergeBoundsWithPoint(const TBoundingBox* b, const TVector3f* p, TBoundingBox* o) {
    o->min.x = fmin(p->x, b->min.x);
    o->min.y = fmin(p->y, b->min.y);
    o->min.z = fmin(p->z, b->min.z);
    o->max.x = fmax(p->x, b->max.x);
    o->max.y = fmax(p->y, b->max.y);
    o->max.z = fmax(p->z, b->max.z);
}

void mergeBoundsWithBounds(const TBoundingBox* b, const TBoundingBox* c, TBoundingBox* o) {
    o->min.x = fmin(b->min.x, c->min.x);
    o->min.y = fmin(b->min.y, c->min.y);
    o->min.z = fmin(b->min.z, c->min.z);
    o->max.x = fmax(b->max.x, c->max.x);
    o->max.y = fmax(b->max.y, c->max.y);
    o->max.z = fmax(b->max.z, c->max.z);
}

void expandBounds(const TBoundingBox* b, float f, TBoundingBox* o) {
    *o = *b;
    o->min.x -= f;
    o->min.y -= f;
    o->min.z -= f;
    o->max.x += f;
    o->max.y += f;
    o->max.z += f;
}

void sizeOfBounds(const TBoundingBox* b, TVector3f* o) {
    subV3f(&b->max, &b->min, o);
}

# pragma mark TQuaternion functions

void setQ(TQuaternion* l, const TQuaternion* r) {
    l->scalar = r->scalar;
    l->vector = r->vector;
}

void setAngleAndAxisQ(TQuaternion* q, float a, const TVector3f* x) {
    q->scalar = cos(a / 2);
    scaleV3f(x, sin(a / 2), &q->vector);
}

void mulQ(const TQuaternion* l, const TQuaternion* r, TQuaternion* o) {
    float a = l->scalar;
    const TVector3f* v = &l->vector;
    float b = r->scalar;
    const TVector3f* w = &r->vector;
    
    float nx = a * w->x + b * v->x + v->y * w->z - v->z * w->y;
    float ny = a * w->y + b * v->y + v->z * w->x - v->x * w->z;
    float nz = a * w->z + b * v->z + v->x * w->y - v->y * w->x;
    
    o->scalar = a * b - dotV3f(v, w);
    o->vector.x = nx;
    o->vector.y = ny;
    o->vector.z = nz;
}

void conjugateQ(const TQuaternion* q, TQuaternion* o) {
    o->scalar = q->scalar;
    scaleV3f(&q->vector, -1, &o->vector);
}

void rotateQ(const TQuaternion* q, const TVector3f* v, TVector3f* o) { 
    TQuaternion t, p, c;
    setQ(&t, q);

    p.scalar = 0;
    p.vector = *v;
    
    setQ(&c, q);
    conjugateQ(&c, &c);
    
    mulQ(&t, &p, &t);
    mulQ(&t, &c, &t);
    *o = t.vector;
}

# pragma mark Coordinate plane functions

void projectOntoPlane(EPlane plane, const TVector3f* v, TVector3f* o) {
    switch (plane) {
        case P_XY:
            o->x = v->x;
            o->y = v->y;
            o->z = v->z;
            break;
        case P_XZ:
            o->x = v->x;
            o->y = v->z;
            o->z = v->y;
            break;
        default:
            o->x = v->y;
            o->y = v->z;
            o->z = v->x;
            break;
    }
}


void makeCircle(float radius, int segments, TVector3f* points) {
    float d = 2 * M_PI / segments;
    float a = 0;
    for (int i = 0; i < segments; i++) {
        float s = sin(a);
        float c = cos(a);
        points[i].x = radius * s;
        points[i].y = radius * c;
        points[i].z = 0;
        a += d;
    }
}

void makeRing(float innerRadius, float outerRadius, int segments, TVector3f* points) {
    float d = 2 * M_PI / (2 * segments);
    float a = 0;
    for (int i = 0; i < 2 * segments; i++) {
        float s = sin(a);
        float c = cos(a);
        float r = i % 2 == 0 ? innerRadius : outerRadius;
        
        points[i].x = r * s;
        points[i].y = r * c;
        points[i].z = 0;
        a += d;
    }

    points[2 * segments] = points[0];
    points[2 * segments + 1] = points[1];
}
