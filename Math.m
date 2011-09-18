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

NSString* const XAxisName = @"X";
NSString* const YAxisName = @"Y";
NSString* const ZAxisName = @"Z";

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

int mini(int v1, int v2) {
    if (v1 < v2)
        return v1;
    return v2;
}

int maxi(int v1, int v2) {
    if (v1 > v2)
        return v1;
    return v2;
}

BOOL segmentContainsPoint(float s11, float s12, float p) {
    return p >= s11 && p <= s12;
}

BOOL segmentIntersectsSegment(float s11, float s12, float s21, float s22) {
    return segmentContainsPoint(s11, s12, s21) ||
           segmentContainsPoint(s11, s12, s22) ||
           segmentContainsPoint(s21, s22, s11);
}

BOOL segmentContainsSegment(float s11, float s12, float s21, float s22) {
    return s21 >= s11 && s22 <= s12;
}

#pragma mark TVector2f functions

void addV2f(const TVector2f* l, const TVector2f* r, TVector2f* o) {
    o->x = l->x + r->x;
    o->y = l->y + r->y;
}

void subV2f(const TVector2f* l, const TVector2f* r, TVector2f* o) {
    o->x = l->x - r->x;
    o->y = r->y - r->y;
}

float dotV2f(const TVector2f* l, const TVector2f* r) {
    return l->x * r->x + l->y * r->y;
}

void scaleV2f(const TVector2f* v, float f, TVector2f* r) {
    r->x = f * v->x;
    r->y = f * v->y;
}

float lengthSquaredV2f(const TVector2f* v) {
    return dotV2f(v, v);
}

float lengthV2f(const TVector2f* v) {
    return sqrt(lengthSquaredV2f(v));
}

void normalizeV2f(const TVector2f* v, TVector2f* r) {
    float l = lengthV2f(v);
    r->x = v->x / l;
    r->y = v->y / l;
}

#pragma mark TVector3f functions

void addV3f(const TVector3f* l, const TVector3f* r, TVector3f* o) {
    o->x = l->x + r->x;
    o->y = l->y + r->y;
    o->z = l->z + r->z;
}

void sumV3f(const TVector3f* v, int c, TVector3f* o) {
    for (int i = 0; i < c; i++)
        addV3f(o, &v[i], o);
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

BOOL absEqualV3f(const TVector3f* l, const TVector3f* r) {
    return feq(fabsf(l->x), fabsf(r->x)) && feq(fabsf(l->y), fabsf(r->y)) && feq(fabsf(l->z), fabsf(r->z));
}

BOOL nullV3f(const TVector3f* v) {
    return equalV3f(v, &NullVector);
}

EAxis strongestComponentV3f(const TVector3f* v) {
    float xa = fabs(v->x);
    float ya = fabs(v->y);
    float za = fabs(v->z);
    
    if (xa >= ya && xa >= za)
        return A_X;
    if (ya >= xa && ya >= za)
        return A_Y;
    return A_Z;
}

EAxis weakestComponentV3f(const TVector3f* v) {
    float xa = fabs(v->x);
    float ya = fabs(v->y);
    float za = fabs(v->z);
    
    if (xa <= ya && xa <= za)
        return A_X;
    if (ya <= xa && ya <= za)
        return A_Y;
    return A_Z;
}

const TVector3f* closestAxisV3f(const TVector3f* v) {
    if (equalV3f(v, &NullVector)) {
        return &NullVector;
    } else {
        float xa = fabs(v->x);
        float ya = fabs(v->y);
        float za = fabs(v->z);
        
        if (xa >= ya && xa >= za) {
            if (v->x > 0)
                return &XAxisPos;
            else
                return &XAxisNeg;
        } else if (ya >= xa && ya >= za) {
            if (v->y > 0)
                return &YAxisPos;
            else
                return &YAxisNeg;
        } else {
            if (v->z > 0)
                return &ZAxisPos;
            else
                return &ZAxisNeg;
        }
    }
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

void rotateZ90CWV3f(const TVector3f* v, TVector3f *o) {
    float x = v->x;
    o->x = v->y;
    o->y = -x;
    o->z = v->z;
}

void rotateZ90CCWV3f(const TVector3f* v, TVector3f *o) {
    float x = v->x;
    o->x = -v->y;
    o->y = x;
    o->z = v->z;
}

BOOL parseV3f(NSString* s, NSRange r, TVector3f* o) {
    int comp = -1;
    BOOL dot = NO;
    int b, l;
    float x, y, z;
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
                    if (comp == 1)
                        x = [p floatValue];
                    else if (comp == 2)
                        y = [p floatValue];
                    else if (comp == 3)
                        z = [p floatValue];
                    comp++;
                    comp *= -1;
                    dot = NO;
                }
                break;
        }
    }
    
    if (comp == 3) {
        NSString* p = [s substringWithRange:NSMakeRange(b, l)];
        z = [p floatValue];
    } else if (comp != -3) {
        return NO;
    }
    
    o->x = x;
    o->y = y;
    o->z = z;
    
    return YES;
}

BOOL opposingV3f(const TVector3f* v1, const TVector3f* v2) {
    return !((v1->x > 0 && v2->x > 0 || v1->x < 0 && v2->x < 0) && 
             (v1->y > 0 && v2->y > 0 || v1->y < 0 && v2->y < 0) && 
             (v1->z > 0 && v2->z > 0 || v1->z < 0 && v2->z < 0));
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

void scaleV3i(const TVector3i* v, int i, TVector3i* o) {
    o->x = v->x * i;
    o->y = v->y * i;
    o->z = v->z * i;
}

BOOL equalV3i(const TVector3i* l, const TVector3i* r) {
    return l->x == r->x && l->y == r->y && l->z == r->z;
}

BOOL nullV3i(const TVector3i* v) {
    return v->x == 0 && v->y == 0 && v->z == 0;
}

void rotateZ90CWV3i(const TVector3i* v, TVector3i *o) {
    int x = v->x;
    o->x = v->y;
    o->y = -x;
    o->z = v->z;
}

void rotateZ90CCWV3i(const TVector3i* v, TVector3i *o) {
    int x = v->x;
    o->x = -v->y;
    o->y = x;
    o->z = v->z;
}


BOOL parseV3i(NSString* s, NSRange r, TVector3i* o) {
    int comp = -1;
    int b, l, x, y, z;
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
                    if (comp == 1)
                        x = [p intValue];
                    else if (comp == 2)
                        y = [p intValue];
                    else if (comp == 3)
                        z = [p intValue];
                    comp++;
                    comp *= -1;
                }
                break;
        }
    }
    
    if (comp == 3) {
        NSString* p = [s substringWithRange:NSMakeRange(b, l)];
        z = [p floatValue];
    } else if (comp != -3) {
        return NO;
    }
    
    o->x = x;
    o->y = y;
    o->z = z;
    
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

void setPlanePointsV3i(TPlane* p, const TVector3i* p1, const TVector3i* p2, const TVector3i* p3) {
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

void setPlanePointsV3f(TPlane* p, const TVector3f* p1, const TVector3f* p2, const TVector3f* p3) {
    TVector3f v1, v2;
    p->point = *p1;

    subV3f(p2, p1, &v1);
    subV3f(p3, p1, &v2);
    
    crossV3f(&v2, &v1, &p->norm);
    normalizeV3f(&p->norm, &p->norm);
}

EPointStatus pointStatusFromPlane(const TPlane* p, const TVector3f* v) {
    return pointStatusFromRay(&p->point, &p->norm, v);
}

EPointStatus pointStatusFromRay(const TVector3f* o, const TVector3f* d, const TVector3f* v) {
    TVector3f t;
    
    subV3f(v, o, &t);
    float c = dotV3f(d, &t);
    if (fpos(c))
        return PS_ABOVE;
    
    if (fneg(c))
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

float planeX(const TPlane* p, float y, float z) {
    float l = dotV3f(&p->norm, &p->point);
    return (l - p->norm.y * y - p->norm.z * z) / p->norm.x;
}

float planeY(const TPlane* p, float x, float z) {
    float l = dotV3f(&p->norm, &p->point);
    return (l - p->norm.x * x - p->norm.z * z) / p->norm.y;
}

float planeZ(const TPlane* p, float x, float y) {
    float l = dotV3f(&p->norm, &p->point);
    return (l - p->norm.x * x - p->norm.y * y) / p->norm.z;
}

NSString* axisName(EAxis a) {
    switch (a) {
        case A_X:
            return XAxisName;
        case A_Y:
            return YAxisName;
        default:
            return ZAxisName;
    }
}

#pragma mark TCubicBezierCurve functions

void pointOnQuadraticBezierCurve(const TQuadraticBezierCurve* c, float t, TVector3f* r) {
    TVector3f v;
    float tc = 1 - t;
    
    scaleV3f(&c->start, tc * tc, r);
    scaleV3f(&c->control, 2 * tc * t, &v);
    addV3f(r, &v, r);
    scaleV3f(&c->end, t * t, &v);
    addV3f(r, &v, r);
}

void pointOnCubicBezierCurve(const TCubicBezierCurve* c, float t, TVector3f* r) {
    TVector3f v;
    float tc = 1 - t;
    
    scaleV3f(&c->start, tc * tc * tc, r);
    scaleV3f(&c->startControl, 3 * tc * tc * t, &v);
    addV3f(r, &v, r);
    scaleV3f(&c->endControl, 3 * tc * t * t, &v);
    addV3f(r, &v, r);
    scaleV3f(&c->end, t * t * t, &v);
    addV3f(r, &v, r);
}

#pragma mark TRay functions

float intersectSphereWithRay(const TVector3f* c, float ra, const TRay* r) {
    TVector3f diff;
    subV3f(&r->origin, c, &diff);
    
    float p = 2 * dotV3f(&diff, &r->direction);
    float q = lengthSquaredV3f(&diff) - ra * ra;
    
    float d = p * p - 4 * q;
    if (d < 0)
        return NAN;
    
    float s = sqrt(d);
    float t0 = (-p + s) / 2;
    float t1 = (-p - s) / 2;
    
    if (t0 < 0 && t1 < 0)
        return NAN;
    if (t0 > 0 && t1 > 0)
        return fmin(t0, t1);
    return fmax(t0, t1);
}

float distanceOfPointAndRay(const TVector3f* c, const TRay* r) {
    float d = closestPointOnRay(c, r);
    if (isnan(d))
        return NAN;
    
    TVector3f p, pc;
    rayPointAtDistance(r, d, &p);
    subV3f(c, &p, &pc);
    return lengthV3f(&pc);
}

float closestPointOnRay(const TVector3f* c, const TRay* r) {
    TVector3f oc;
    subV3f(c, &r->origin, &oc);
    
    float d = dotV3f(&oc, &r->direction);
    if (d <= 0)
        return NAN;
    return d;
}

float distanceOfSegmentAndRaySquared(const TVector3f* ss, const TVector3f* se, const TRay* r, float* rd) {
    TVector3f u, v, w;
    subV3f(se, ss, &u);
    v = r->direction;
    subV3f(ss, &r->origin, &w);
    
    float a = dotV3f(&u, &u);
    float b = dotV3f(&u, &v);
    float c = dotV3f(&v, &v);
    float d = dotV3f(&u, &w);
    float e = dotV3f(&v, &w);
    float D = a * c - b * b;
    float sN, sD = D;
    float tN, tD = D;
    
    if (fzero(D)) {
        sN = 0;
        sD = 1;
        tN = e;
        tD = c;
    } else {
        sN = (b * e - c * d);
        tN = (a * e - b * d);
        if (sN < 0) {
            sN = 0;
            tN = e;
            tD = c;
        } else if (sN > sD) {
            sN = sD;
            tN = e + b;
            tD = c;
        }
    }
    
    if (tN < 0)
        return NAN;
    
    float sc = fzero(sN) ? 0.0 : sN / sD;
    float tc = fzero(tN) ? 0.0 : tN / tD;
    
    TVector3f dP;
    scaleV3f(&u, sc, &u);
    scaleV3f(&v, tc, &v);
    addV3f(&w, &u, &w);
    subV3f(&w, &v, &dP);

    *rd = tc;
    return lengthSquaredV3f(&dP);
}

float distanceOfSegmentAndRay(const TVector3f* ss, const TVector3f* se, const TRay* r, float* rd) {
    return sqrt(distanceOfSegmentAndRaySquared(ss, se, r, rd));
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

void roundedCenterOfBounds(const TBoundingBox* b, TVector3i* o) {
    TVector3f centerf;
    centerOfBounds(b, &centerf);
    roundV3f(&centerf, o);
}

void translateBounds(const TBoundingBox* b, const TVector3f* d, TBoundingBox* o) {
    addV3f(&b->min, d, &o->min);
    addV3f(&b->max, d, &o->max);
}

void rotateBoundsZ90CW(const TBoundingBox* b, const TVector3f* c, TBoundingBox* o) {
    TBoundingBox rotated;

    subV3f(&b->min, c, &rotated.min);
    subV3f(&b->max, c, &rotated.max);
    
    rotateZ90CWV3f(&rotated.min, &rotated.min);
    rotateZ90CWV3f(&rotated.max, &rotated.max);

    o->min.x = fminf(rotated.min.x, rotated.max.x);
    o->min.y = fminf(rotated.min.y, rotated.max.y);
    o->min.z = fminf(rotated.min.z, rotated.max.z);
    o->max.x = fmaxf(rotated.min.x, rotated.max.x);
    o->max.y = fmaxf(rotated.min.y, rotated.max.y);
    o->max.z = fmaxf(rotated.min.z, rotated.max.z);
    
    addV3f(&o->min, c, &o->min);
    addV3f(&o->max, c, &o->max);
}

void rotateBoundsZ90CCW(const TBoundingBox* b, const TVector3f* c, TBoundingBox* o) {
    TBoundingBox rotated;
    
    subV3f(&b->min, c, &rotated.min);
    subV3f(&b->max, c, &rotated.max);

    rotateZ90CCWV3f(&rotated.min, &rotated.min);
    rotateZ90CCWV3f(&rotated.max, &rotated.max);
    
    o->min.x = fminf(rotated.min.x, rotated.max.x);
    o->min.y = fminf(rotated.min.y, rotated.max.y);
    o->min.z = fminf(rotated.min.z, rotated.max.z);
    o->max.x = fmaxf(rotated.min.x, rotated.max.x);
    o->max.y = fmaxf(rotated.min.y, rotated.max.y);
    o->max.z = fmaxf(rotated.min.z, rotated.max.z);
    
    addV3f(&o->min, c, &o->min);
    addV3f(&o->max, c, &o->max);
}

void rotateBounds(const TBoundingBox* b, const TQuaternion* q, const TVector3f* c, TBoundingBox* o) {
    TBoundingBox rotated;
    
    subV3f(&b->min, c, &rotated.min);
    subV3f(&b->max, c, &rotated.max);
    
    rotateQ(q, &rotated.min, &rotated.min);
    rotateQ(q, &rotated.max, &rotated.max);
    
    o->min.x = fminf(rotated.min.x, rotated.max.x);
    o->min.y = fminf(rotated.min.y, rotated.max.y);
    o->min.z = fminf(rotated.min.z, rotated.max.z);
    o->max.x = fmaxf(rotated.min.x, rotated.max.x);
    o->max.y = fmaxf(rotated.min.y, rotated.max.y);
    o->max.z = fmaxf(rotated.min.z, rotated.max.z);
    
    addV3f(&o->min, c, &o->min);
    addV3f(&o->min, c, &o->min);
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

void roundedSizeOfBounds(const TBoundingBox* b, TVector3i* o) {
    TVector3f sizef;
    sizeOfBounds(b, &sizef);
    roundV3f(&sizef, o);
}

float radiusOfBounds(const TBoundingBox* b) {
    TVector3f s;
    sizeOfBounds(b, &s);
    return sqrt(s.x * s.x + s.y * s.y + s.z * s.z);
}

float intersectBoundsWithRay(const TBoundingBox* b, const TRay* ray, TVector3f* n) {
    TPlane plane;
    float dist;
    TVector3f point;
    BOOL hit;
    
    if (ray->direction.x < 0) {
        plane.point = b->max;
        plane.norm = XAxisPos;
        dist = intersectPlaneWithRay(&plane, ray);
        if (!isnan(dist)) {
            rayPointAtDistance(ray, dist, &point);
            hit = point.y >= b->min.y && point.y <= b->max.y && point.z >= b->min.z && point.z <= b->max.z;
            if (hit && n != NULL)
                *n = XAxisNeg;
        }
    } else if (ray->direction.x > 0) {
        plane.point = b->min;
        plane.norm = XAxisNeg;
        dist = intersectPlaneWithRay(&plane, ray);
        if (!isnan(dist)) {
            rayPointAtDistance(ray, dist, &point);
            hit = point.y >= b->min.y && point.y <= b->max.y && point.z >= b->min.z && point.z <= b->max.z;
            if (hit && n != NULL)
                *n = XAxisPos;
        }
    }
    
    if (!hit) {
        if (ray->direction.y < 0) {
            plane.point = b->max;
            plane.norm = YAxisPos;
            dist = intersectPlaneWithRay(&plane, ray);
            if (!isnan(dist)) {
                rayPointAtDistance(ray, dist, &point);
                hit = point.x >= b->min.x && point.x <= b->max.x && point.z >= b->min.z && point.z <= b->max.z;
                if (hit && n != NULL)
                    *n = YAxisNeg;
            }
        } else if (ray->direction.y > 0) {
            plane.point = b->min;
            plane.norm = YAxisNeg;
            dist = intersectPlaneWithRay(&plane, ray);
            if (!isnan(dist)) {
                rayPointAtDistance(ray, dist, &point);
                hit = point.x >= b->min.x && point.x <= b->max.x && point.z >= b->min.z && point.z <= b->max.z;
                if (hit && n != NULL)
                    *n = YAxisPos;
            }
        }
    }
    
    if (!hit) {
        if (ray->direction.z < 0) {
            plane.point = b->max;
            plane.norm = ZAxisPos;
            dist = intersectPlaneWithRay(&plane, ray);
            if (!isnan(dist)) {
                rayPointAtDistance(ray, dist, &point);
                hit = point.x >= b->min.x && point.x <= b->max.x && point.y >= b->min.y && point.y <= b->max.y;
                if (hit && n != NULL)
                    *n = ZAxisNeg;
            }
        } else if (ray->direction.z > 0) {
            plane.point = b->min;
            plane.norm = ZAxisNeg;
            dist = intersectPlaneWithRay(&plane, ray);
            if (!isnan(dist)) {
                rayPointAtDistance(ray, dist, &point);
                hit = point.x >= b->min.x && point.x <= b->max.x && point.y >= b->min.y && point.y <= b->max.y;
                if (hit && n != NULL)
                    *n = ZAxisPos;
            }
        }
    }

    if (!hit)
        return NAN;
    
    return dist;
}

BOOL boundsContainPoint(const TBoundingBox* b, const TVector3f* p) {
    return segmentContainsPoint(b->min.x, b->max.x, p->x) &&
           segmentContainsPoint(b->min.y, b->max.y, p->y) &&
           segmentContainsPoint(b->min.z, b->max.z, p->z);
}

BOOL boundsIntersectWithBounds(const TBoundingBox* b1, const TBoundingBox* b2) {
    return segmentIntersectsSegment(b1->min.x, b1->max.x, b2->min.x, b2->max.x) &&
           segmentIntersectsSegment(b1->min.y, b1->max.y, b2->min.y, b2->max.y) &&
           segmentIntersectsSegment(b1->min.z, b1->max.z, b2->min.z, b2->max.z);
}

BOOL boundsContainBounds(const TBoundingBox* b1, const TBoundingBox *b2) {
    return segmentContainsSegment(b1->min.x, b1->max.x, b2->min.x, b2->max.x) &&
           segmentContainsSegment(b1->min.y, b1->max.y, b2->min.y, b2->max.y) &&
           segmentContainsSegment(b1->min.z, b1->max.z, b2->min.z, b2->max.z);
}

# pragma mark TQuaternion functions

void setQ(TQuaternion* l, const TQuaternion* r) {
    l->scalar = r->scalar;
    l->vector = r->vector;
}

void setAngleAndAxisQ(TQuaternion* q, float a, const TVector3f* x) {
    q->scalar = cosf(a / 2);
    scaleV3f(x, sinf(a / 2), &q->vector);
}

BOOL nullQ(const TQuaternion* q) {
    return q->scalar == 1;
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

float radiansQ(const TQuaternion* q) {
    return 2 * acosf(q->scalar);
}

float degreesQ(const TQuaternion* q) {
    return radiansQ(q) * 180 / M_PI;
}

# pragma mark Coordinate plane functions

void projectOntoCoordinatePlane(EPlane plane, const TVector3f* v, TVector3f* o) {
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

BOOL projectVectorOntoPlane(const TVector3f* planeNorm, const TVector3f* dir, const TVector3f* v, TVector3f* o) {
    float d = dotV3f(dir, planeNorm);
    if (fzero(d))
        return NO;
    
    d = -dotV3f(v, planeNorm) / d;
    
    TVector3f w;
    scaleV3f(dir, d, &w);
    addV3f(&w, v, o);

    return YES;
}

void makePointsForPlane(const TPlane* p, const TBoundingBox* m, TVector3i* p1, TVector3i* p2, TVector3i* p3) {
    TLine l;
    TVector3f a,b,c;
    
    switch (strongestComponentV3f(&p->norm)) {
        case A_X:{
            l.direction = XAxisPos;

            l.point = m->min;
            linePointAtDistance(&l, intersectPlaneWithLine(p, &l), &a);
            
            l.point.y = m->max.y;
            linePointAtDistance(&l, intersectPlaneWithLine(p, &l), &b);
            
            l.point.z = m->max.z;
            linePointAtDistance(&l, intersectPlaneWithLine(p, &l), &c);
            
            if (p->norm.x < 0) {
                roundV3f(&a, p3);
                roundV3f(&b, p1);
                roundV3f(&c, p2);
            } else {
                roundV3f(&a, p2);
                roundV3f(&b, p1);
                roundV3f(&c, p3);
            }
            break;
        }
        case A_Y: {
            l.direction = YAxisPos;
            
            l.point = m->min;
            linePointAtDistance(&l, intersectPlaneWithLine(p, &l), &a);
            
            l.point.x = m->max.x;
            linePointAtDistance(&l, intersectPlaneWithLine(p, &l), &b);
            
            l.point.z = m->max.z;
            linePointAtDistance(&l, intersectPlaneWithLine(p, &l), &c);
            
            if (p->norm.y < 0) {
                roundV3f(&a, p2);
                roundV3f(&b, p1);
                roundV3f(&c, p3);
            } else {
                roundV3f(&a, p3);
                roundV3f(&b, p1);
                roundV3f(&c, p2);
            }
            break;
        }
        default: {
            l.direction = ZAxisPos;
            
            l.point = m->min;
            linePointAtDistance(&l, intersectPlaneWithLine(p, &l), &a);
            
            l.point.x = m->max.x;
            linePointAtDistance(&l, intersectPlaneWithLine(p, &l), &b);
            
            l.point.y = m->max.y;
            linePointAtDistance(&l, intersectPlaneWithLine(p, &l), &c);
            
            if (p->norm.z < 0) {
                roundV3f(&a, p3);
                roundV3f(&b, p1);
                roundV3f(&c, p2);
            } else {
                roundV3f(&a, p2);
                roundV3f(&b, p1);
                roundV3f(&c, p3);
            }
            break;
        }
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
    float d = M_PI / segments;
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

void makeTorus(float innerRadius, float outerRadius, int innerSegments, int outerSegments, TVector3f* points, TVector3f* normals) {
    float dTheta = 2 * M_PI / innerSegments;
    float dPhi = 2 * M_PI / outerSegments;
    
    float theta = 0;
    float phi = 0;
    
    if (normals != NULL) {
        TVector3f rc;
        rc.z = 0;
        
        for (int i = 0; i < innerSegments; i++) {
            float sTheta = sin(theta);
            float cTheta = cos(theta);
            
            rc.x = innerRadius * sTheta;
            rc.y = innerRadius * cTheta;
            
            for (int j = 0; j < outerSegments; j++) {
                float sPhi = sin(phi);
                float cPhi = cos(phi);
                
                int pa = i * outerSegments + j;
                points[pa].x = (innerRadius + outerRadius * sPhi) * cTheta;
                points[pa].y = (innerRadius + outerRadius * sPhi) * sTheta;
                points[pa].z = outerRadius * cPhi;
                
                subV3f(&points[pa], &rc, &normals[pa]);
                normalizeV3f(&normals[pa], &normals[pa]);
                
                phi += dPhi;
            }
            theta += dTheta;
        }
    } else {
        for (int i = 0; i < innerSegments; i++) {
            float sTheta = sin(theta);
            float cTheta = cos(theta);
            
            for (int j = 0; j < outerSegments; j++) {
                float sPhi = sin(phi);
                float cPhi = cos(phi);
                
                int pa = i * outerSegments + j;
                points[pa].x = (innerRadius + outerRadius * sPhi) * cTheta;
                points[pa].y = (innerRadius + outerRadius * sPhi) * sTheta;
                points[pa].z = outerRadius * cPhi;
                
                phi += dPhi;
            }
            theta += dTheta;
        }
    }
}

void makeTorusPart(float innerRadius, float outerRadius, int innerSegments, int outerSegments, float centerAngle, float angleLength, TVector3f* points, TVector3f* normals) {
    float dTheta = angleLength / innerSegments;
    float dPhi = 2 * M_PI / outerSegments;
    
    float theta = centerAngle - angleLength / 2;
    float phi = 0;
    
    if (normals != NULL) {
        TVector3f rc;
        rc.z = 0;
        
        for (int i = 0; i <= innerSegments; i++) {
            float sTheta = sin(theta);
            float cTheta = cos(theta);
            
            rc.x = innerRadius * sTheta;
            rc.y = innerRadius * cTheta;
            
            for (int j = 0; j < outerSegments; j++) {
                float sPhi = sin(phi);
                float cPhi = cos(phi);
                
                int pa = i * outerSegments + j;
                points[pa].x = (innerRadius + outerRadius * sPhi) * cTheta;
                points[pa].y = (innerRadius + outerRadius * sPhi) * sTheta;
                points[pa].z = outerRadius * cPhi;
                
                subV3f(&points[pa], &rc, &normals[pa]);
                normalizeV3f(&normals[pa], &normals[pa]);
                
                phi += dPhi;
            }
            theta += dTheta;
        }    
    } else {
        for (int i = 0; i <= innerSegments; i++) {
            float sTheta = sin(theta);
            float cTheta = cos(theta);
            
            for (int j = 0; j < outerSegments; j++) {
                float sPhi = sin(phi);
                float cPhi = cos(phi);
                
                int pa = i * outerSegments + j;
                points[pa].x = (innerRadius + outerRadius * sPhi) * cTheta;
                points[pa].y = (innerRadius + outerRadius * sPhi) * sTheta;
                points[pa].z = outerRadius * cPhi;
                
                phi += dPhi;
            }
            theta += dTheta;
        }    
    }
}

void makeCone(float radius, float height, int segments, TVector3f* points, TVector3f* normals) {
    points[0].x = 0;
    points[0].y = 0;
    points[0].z = height;
    
    makeCircle(radius, segments, &points[1]);
    points[segments + 1] = points[1];
    
    if (normals != NULL) {
        normals[0] = ZAxisPos;
        
        TVector3f csn, psn, tp1, tp2;
        subV3f(&points[segments + 1], &points[0], &tp1);
        subV3f(&points[segments], &points[0], &tp2);
        crossV3f(&tp1, &tp2, &psn);
        normalizeV3f(&psn, &psn);
        
        for (int i = 1; i < segments + 1; i++) {
            subV3f(&points[i + 1], &points[0], &tp1);
            subV3f(&points[i], &points[0], &tp2);
            crossV3f(&tp1, &tp2, &csn);
            normalizeV3f(&csn, &csn);
            
            addV3f(&psn, &csn, &normals[i]);
            addV3f(&ZAxisNeg, &normals[i], &normals[i]);
            scaleV3f(&normals[i], 1 / 3.0f, &normals[i]);
            
            psn = csn;
        }
        
        normals[segments + 1] = normals[1];
    }
}

void makeCylinder(int segments, TVector3f* points, TVector3f* normals) {
    float d = 2 * M_PI / segments;
    float a = 0;
    for (int i = 0; i < segments; i++) {
        float s = sin(a);
        float c = cos(a);
        
        points[2 * i].x = s;
        points[2 * i].y = c;
        points[2 * i].z = 0;
        points[2 * i + 1].x = s;
        points[2 * i + 1].y = c;
        points[2 * i + 1].z = 1;
        
        if (normals != NULL) {
            normals[2 * i].x = normals[2 * i + 1].x = s;
            normals[2 * i].y = normals[2 * i + 1].y = c;
            normals[2 * i].z = normals[2 * i + 1].z = 0;
        }
        
        a += d;
    }
    
    points[2 * segments] = points[0];
    points[2 * segments + 1] = points[1];
    if (normals != NULL) {
        normals[2 * segments] = normals[0];
        normals[2 * segments + 1] = normals[1];
    }
}