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

#import "Math.h"
#import "math.h"

float const AlmostZero = 0.001f;
float const PointStatusEpsilon = 0.01f;

TVector3f const XAxisPos = {+1,  0,  0};
TVector3f const XAxisNeg = {-1,  0,  0};
TVector3f const YAxisPos = { 0, +1,  0};
TVector3f const YAxisNeg = { 0, -1,  0};
TVector3f const ZAxisPos = { 0,  0, +1};
TVector3f const ZAxisNeg = { 0,  0, -1};
TVector3f const NullVector = {0, 0, 0};
TMatrix2f const IdentityM2f = {1, 0, 0, 1};
TMatrix3f const IdentityM3f = {1, 0, 0, 0, 1, 0, 0, 0, 1};
TMatrix4f const IdentityM4f = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
TMatrix4f const RotX90CWM4f = { 1,  0,  0,  0, 
                                0,  0, -1,  0, 
                                0,  1,  0,  0, 
                                0,  0,  0,  1 };
TMatrix4f const RotY90CWM4f = { 0,  0,  1,  0, 
                                0,  1,  0,  0, 
                               -1,  0,  0,  0, 
                                0,  0,  0,  1 };
TMatrix4f const RotZ90CWM4f = { 0, -1,  0,  0, 
                                1,  0,  0,  0, 
                                0,  0,  1,  0, 
                                0,  0,  0,  1 };
TMatrix4f const RotX90CCWM4f = { 1,  0,  0,  0, 
                                 0,  0,  1,  0, 
                                 0, -1,  0,  0, 
                                 0,  0,  0,  1 };
TMatrix4f const RotY90CCWM4f = { 0,  0, -1,  0, 
                                 0,  1,  0,  0, 
                                 1,  0,  0,  0, 
                                 0,  0,  0,  1 };
TMatrix4f const RotZ90CCWM4f = { 0,  1,  0,  0, 
                                -1,  0,  0,  0, 
                                 0,  0,  1,  0, 
                                 0,  0,  0,  1 };
TMatrix4f const MirXM4f = {-1,  0,  0,  0, 
                            0,  1,  0,  0, 
                            0,  0,  1,  0, 
                            0,  0,  0,  1 };
TMatrix4f const MirYM4f = { 1,  0,  0,  0, 
                            0, -1,  0,  0, 
                            0,  0,  1,  0, 
                            0,  0,  0,  1 };
TMatrix4f const MirZM4f = { 1,  0,  0,  0, 
                            0,  1,  0,  0, 
                            0,  0, -1,  0, 
                            0,  0,  0,  1 };

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

void absV3f(const TVector3f* v, TVector3f* o) {
    o->x = fabsf(v->x);
    o->y = fabsf(v->y);
    o->z = fabsf(v->z);
}

float lengthV3f(const TVector3f* v) {
    return sqrt(lengthSquaredV3f(v));
}

float lengthSquaredV3f(const TVector3f* v) {
    return dotV3f(v, v);
}

void normalizeV3f(const TVector3f* v, TVector3f* o) {
    float l = lengthV3f(v);
    assert(l != 0);
    
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

BOOL sameDirV3f(const TVector3f* v1, const TVector3f* v2) {
    TVector3f cross;
    crossV3f(v1, v2, &cross);
    if (!nullV3f(&cross))
        return NO;

    if (v1->x != 0)
        return v1->x > 0 == v2->x > 0;
    else if (v1->y != 0)
        return v1->y > 0 == v2->y > 0;
    else if (v1->z != 0)
        return v1->z > 0 == v2->z > 0;
    
    return nullV3f(v2);
}

BOOL intV3f(const TVector3f* v) {
    return v->x == (int)v->x && v->y == (int)v->y && v->z == (int)v->z;
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

const TVector3f* firstAxisV3f(const TVector3f* v) {
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

const TVector3f* firstAxisNegV3f(const TVector3f* v) {
    if (equalV3f(v, &NullVector)) {
        return &NullVector;
    } else {
        float xa = fabs(v->x);
        float ya = fabs(v->y);
        float za = fabs(v->z);
        
        if (xa >= ya && xa >= za) {
            if (v->x > 0)
                return &XAxisNeg;
            else
                return &XAxisPos;
        } else if (ya >= xa && ya >= za) {
            if (v->y > 0)
                return &YAxisNeg;
            else
                return &YAxisPos;
        } else {
            if (v->z > 0)
                return &ZAxisNeg;
            else
                return &ZAxisPos;
        }
    }
}

const TVector3f* secondAxisV3f(const TVector3f* v) {
    if (equalV3f(v, &NullVector)) {
        return &NullVector;
    } else {
        float xa = fabs(v->x);
        float ya = fabs(v->y);
        float za = fabs(v->z);
        
        if ((xa <= ya && xa >= za) || 
            (xa >= ya && xa <= za)) {
            if (v->x > 0)
                return &XAxisPos;
            else
                return &XAxisNeg;
        } else if ((ya <= xa && ya >= za) || 
                   (ya >= xa && ya <= za)) {
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

const TVector3f* secondAxisNegV3f(const TVector3f* v) {
    if (equalV3f(v, &NullVector)) {
        return &NullVector;
    } else {
        float xa = fabs(v->x);
        float ya = fabs(v->y);
        float za = fabs(v->z);
        
        if ((xa <= ya && xa >= za) || 
            (xa >= ya && xa <= za)) {
            if (v->x > 0)
                return &XAxisNeg;
            else
                return &XAxisPos;
        } else if ((ya <= xa && ya >= za) || 
                   (ya >= xa && ya <= za)) {
            if (v->y > 0)
                return &YAxisNeg;
            else
                return &YAxisPos;
        } else {
            if (v->z > 0)
                return &ZAxisNeg;
            else
                return &ZAxisPos;
        }
    }
}

const TVector3f* thirdAxisV3f(const TVector3f* v) {
    if (equalV3f(v, &NullVector)) {
        return &NullVector;
    } else {
        float xa = fabs(v->x);
        float ya = fabs(v->y);
        float za = fabs(v->z);
        
        if (xa <= ya && xa >= za) {
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

const TVector3f* thirdAxisNegV3f(const TVector3f* v) {
    if (equalV3f(v, &NullVector)) {
        return &NullVector;
    } else {
        float xa = fabs(v->x);
        float ya = fabs(v->y);
        float za = fabs(v->z);
        
        if (xa <= ya && xa >= za) {
            if (v->x > 0)
                return &XAxisNeg;
            else
                return &XAxisPos;
        } else if (ya >= xa && ya >= za) {
            if (v->y > 0)
                return &YAxisNeg;
            else
                return &YAxisPos;
        } else {
            if (v->z > 0)
                return &ZAxisNeg;
            else
                return &ZAxisPos;
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

void roundV3f(const TVector3f* v, TVector3f* o) {
    o->x = roundf(v->x);
    o->y = roundf(v->y);
    o->z = roundf(v->z);
}

void roundUpV3f(const TVector3f* v, TVector3f* o) {
    o->x = v->x > 0 ? ceilf(v->x) : floorf(v->x);
    o->y = v->y > 0 ? ceilf(v->y) : floorf(v->y);
    o->z = v->z > 0 ? ceilf(v->z) : floorf(v->z);
}

void roundDownV3f(const TVector3f* v, TVector3f* o) {
    o->x = v->x < 0 ? ceilf(v->x) : floorf(v->x);
    o->y = v->y < 0 ? ceilf(v->y) : floorf(v->y);
    o->z = v->z < 0 ? ceilf(v->z) : floorf(v->z);
}

void snapV3f(const TVector3f* v, TVector3f* o) {
    float rx = roundf(v->x);
    float ry = roundf(v->y);
    float rz = roundf(v->z);

    o->x = fabsf(v->x - rx) < AlmostZero ? rx : v->x;
    o->y = fabsf(v->y - ry) < AlmostZero ? ry : v->y;
    o->z = fabsf(v->z - rz) < AlmostZero ? rz : v->z;
}

void setV3f(TVector3f* l, const TVector3i* r) {
    l->x = r->x;
    l->y = r->y;
    l->z = r->z;
}

void rotate90CWV3f(const TVector3f* v, EAxis a, TVector3f *o) {
    switch (a) {
        case A_X: {
            float y = v->y;
            o->x = v->x;
            o->y = v->z;
            o->z = -y;
            break;
        }
        case A_Y: {
            float x = v->x;
            o->x = -v->z;
            o->y = v->y;
            o->z = x;
            break;
        }
        default: {
            float x = v->x;
            o->x = v->y;
            o->y = -x;
            o->z = v->z;
            break;
        }
    }
}

void rotate90CCWV3f(const TVector3f* v, EAxis a, TVector3f *o) {
    switch (a) {
        case A_X: {
            float y = v->y;
            o->x = v->x;
            o->y = -v->z;
            o->z = y;
            break;
        }
        case A_Y: {
            float x = v->x;
            o->x = v->z;
            o->y = v->y;
            o->z = -x;
            break;
        }
        default: {
            float x = v->x;
            o->x = -v->y;
            o->y = x;
            o->z = v->z;
            break;
        }
    }
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
    return !(((v1->x > 0 && v2->x > 0) || (v1->x < 0 && v2->x < 0)) && 
             ((v1->y > 0 && v2->y > 0) || (v1->y < 0 && v2->y < 0)) && 
             ((v1->z > 0 && v2->z > 0) || (v1->z < 0 && v2->z < 0)));
}

BOOL normV3f(const TVector3f* v1, const TVector3f* v2, const TVector3f* v3, TVector3f* o) {
    TVector3f w1, w2;
    
    subV3f(v3, v1, &w1);
    subV3f(v2, v1, &w2);
    crossV3f(&w1, &w2, o);
    
    if (nullV3f(o))
        return NO;

    normalizeV3f(o, o);
    return YES;
}

void avg3V3f(const TVector3f* v1, const TVector3f* v2, const TVector3f* v3, TVector3f* o) {
    addV3f(v1, v2, o);
    addV3f(o, v3, o);
    scaleV3f(o, 1 / 3.0f, o);
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

void setV3i(TVector3i* l, const TVector3f* r) {
    l->x = (int)r->x;
    l->y = (int)r->y;
    l->z = (int)r->z;
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

void rotate90CWV3i(const TVector3i* v, EAxis a, TVector3i *o) {
    switch (a) {
        case A_X: {
            int y = v->y;
            o->x = v->x;
            o->y = v->z;
            o->z = -y;
            break;
        }
        case A_Y: {
            int x = v->x;
            o->x = -v->z;
            o->y = v->y;
            o->z = x;
            break;
        }
        default: {
            int x = v->x;
            o->x = v->y;
            o->y = -x;
            o->z = v->z;
            break;
        }
    }
}

void rotate90CCWV3i(const TVector3i* v, EAxis a, TVector3i *o) {
    switch (a) {
        case A_X: {
            int y = v->y;
            o->x = v->x;
            o->y = -v->z;
            o->z = y;
            break;
        }
        case A_Y: {
            int x = v->x;
            o->x = v->z;
            o->y = v->y;
            o->z = -x;
            break;
        }
        default: {
            int x = v->x;
            o->x = -v->y;
            o->y = x;
            o->z = v->z;
            break;
        }
    }
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

# pragma mark TLine functions

void setLinePoints(TLine* l, TVector3f* p1, TVector3f* p2) {
    l->point = *p1;
    subV3f(p2, p1, &l->direction);
    normalizeV3f(&l->direction, &l->direction);
}

void linePointAtDistance(TLine* l, float d, TVector3f* p) {
    TVector3f r;
    scaleV3f(&l->direction, d, &r);
    addV3f(&r, &l->point, &r);
    *p = r;
}

# pragma mark TPlane functions

/*
 * The points are expected in clockwise orientation (view from above the plane):
 *
 *           p3
 *           |
 *           | v1
 *           |
 *   p2------p1
 *       v2
 */

BOOL setPlanePointsV3f(TPlane* p, const TVector3f* p1, const TVector3f* p2, const TVector3f* p3) {
    TVector3f v1, v2;
    p->point = *p1;

    subV3f(p3, p1, &v1);
    subV3f(p2, p1, &v2);
    
    crossV3f(&v1, &v2, &p->norm);
    if (nullV3f(&p->norm))
        return NO;
    
    normalizeV3f(&p->norm, &p->norm);
    return YES;
}

EPointStatus pointStatusFromPlane(const TPlane* p, const TVector3f* v) {
    return pointStatusFromRay(&p->point, &p->norm, v);
}

EPointStatus pointStatusFromRay(const TVector3f* o, const TVector3f* d, const TVector3f* v) {
    TVector3f t;
    
    subV3f(v, o, &t);
    float c = dotV3f(d, &t);
    if (c > PointStatusEpsilon)
        return PS_ABOVE;
    
    if (c < -PointStatusEpsilon)
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

BOOL equalPlane(const TPlane* p1, const TPlane* p2) {
    return equalV3f(&p1->norm, &p2->norm) && pointStatusFromPlane(p1, &p2->point) == PS_INSIDE;
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

void translateBounds(const TBoundingBox* b, const TVector3f* d, TBoundingBox* o) {
    addV3f(&b->min, d, &o->min);
    addV3f(&b->max, d, &o->max);
}

void rotateBounds90CW(const TBoundingBox* b, EAxis a, const TVector3f* c, TBoundingBox* o) {
    TBoundingBox rotated;

    subV3f(&b->min, c, &rotated.min);
    subV3f(&b->max, c, &rotated.max);
    
    rotate90CWV3f(&rotated.min, a, &rotated.min);
    rotate90CWV3f(&rotated.max, a, &rotated.max);

    o->min.x = fminf(rotated.min.x, rotated.max.x);
    o->min.y = fminf(rotated.min.y, rotated.max.y);
    o->min.z = fminf(rotated.min.z, rotated.max.z);
    o->max.x = fmaxf(rotated.min.x, rotated.max.x);
    o->max.y = fmaxf(rotated.min.y, rotated.max.y);
    o->max.z = fmaxf(rotated.min.z, rotated.max.z);
    
    addV3f(&o->min, c, &o->min);
    addV3f(&o->max, c, &o->max);
}

void rotateBounds90CCW(const TBoundingBox* b, EAxis a, const TVector3f* c, TBoundingBox* o) {
    TBoundingBox rotated;
    
    subV3f(&b->min, c, &rotated.min);
    subV3f(&b->max, c, &rotated.max);

    rotate90CCWV3f(&rotated.min, a, &rotated.min);
    rotate90CCWV3f(&rotated.max, a, &rotated.max);
    
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

float radiusOfBounds(const TBoundingBox* b) {
    TVector3f s;
    sizeOfBounds(b, &s);
    return sqrt(s.x * s.x + s.y * s.y + s.z * s.z);
}

float intersectBoundsWithRay(const TBoundingBox* b, const TRay* ray, TVector3f* n) {
    TPlane plane;
    float dist;
    TVector3f point;
    BOOL hit = NO;
    
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
    TQuaternion p, c;

    p.scalar = 0;
    p.vector = *v;
    
    conjugateQ(q, &c);
    
    mulQ(q, &p, &p);
    mulQ(&p, &c, &p);

    *o = p.vector;
}

float radiansQ(const TQuaternion* q) {
    return 2 * acosf(q->scalar);
}

float degreesQ(const TQuaternion* q) {
    return radiansQ(q) * 180 / M_PI;
}

#pragma mark TMatrix2f functions

void setMatrix2fAsSubMatrix(const TMatrix4f* m4f, int i, TMatrix2f* m2f) {
    switch (i) {
        case 0:
            m2f->values[0] = m4f->values[0];
            m2f->values[1] = m4f->values[1];
            m2f->values[2] = m4f->values[4];
            m2f->values[3] = m4f->values[5];
            break;
        case 1:
            m2f->values[0] = m4f->values[2];
            m2f->values[1] = m4f->values[3];
            m2f->values[2] = m4f->values[6];
            m2f->values[3] = m4f->values[7];
            break;
        case 2:
            m2f->values[0] = m4f->values[8];
            m2f->values[1] = m4f->values[9];
            m2f->values[2] = m4f->values[12];
            m2f->values[3] = m4f->values[13];
            break;
        case 3:
            m2f->values[0] = m4f->values[10];
            m2f->values[1] = m4f->values[11];
            m2f->values[2] = m4f->values[14];
            m2f->values[3] = m4f->values[15];
            break;
        default:
            [NSException raise:NSInvalidArgumentException format:@"sub matrix index out of bounds: %i", index];
            break;
    }
}

void setIdentityM2f(TMatrix2f* m) {
    m->values[0] = 1;
    m->values[1] = 0;
    m->values[2] = 0;
    m->values[3] = 1;
}

void setMinorM2f(const TMatrix3f* m3f, int col, int row, TMatrix2f* o) {
    int i = 0;
    for (int c = 0;  c < 3; c++)
        for (int r = 0; r < 3; r++)
            if (c != col && r != row)
                o->values[i++] = m3f->values[c * 3 + r];
}

void setColumnM2f(const TMatrix2f* m, const TVector2f* v, int col, TMatrix2f* o) {
    *o = *m;
    o->values[col * 2 + 0] = v->x;
    o->values[col * 2 + 1] = v->y;
}

void setValueM2f(const TMatrix2f* m, float v, int col, int row, TMatrix2f* o) {
    *o = *m;
    o->values[col * 2 + row] = v;
}

BOOL invertM2f(const TMatrix2f* m, TMatrix2f* o) {
    float det = determinantM2f(m);
    if (fzero(det))
        return NO;
    
    adjugateM2f(m, o);
    scaleM2f(o, 1 / det, o);
    return YES;
}

void adjugateM2f(const TMatrix2f* m, TMatrix2f* o) {
    float t = m->values[0];
    o->values[0] = m->values[3];
    o->values[3] = t;
    
    o->values[1] = -m->values[1];
    o->values[2] = -m->values[2];
}

float determinantM2f(const TMatrix2f* m) {
    return m->values[0] * m->values[3] - m->values[2] * m->values[1];
}

void negateM2f(const TMatrix2f* m, TMatrix2f* o) {
    scaleM2f(m, -1, o);
}

void transposeM2f(const TMatrix2f* m, TMatrix2f* o);

void addM2f(const TMatrix2f* l, const TMatrix2f* r, TMatrix2f* o) {
    for (int i = 0; i < 4; i++)
        o->values[i] = l->values[i] + r->values[i];
}

void subM2f(const TMatrix2f* l, const TMatrix2f* r, TMatrix2f* o) {
    for (int i = 0; i < 4; i++)
        o->values[i] = l->values[i] - r->values[i];
}

void mulM2f(const TMatrix2f* l, const TMatrix2f* r, TMatrix2f* o) {
    float a = l->values[0] * r->values[0] + l->values[2] * r->values[1];
    float c = l->values[1] * r->values[0] + l->values[3] * r->values[1];
    float b = l->values[0] * r->values[2] + l->values[2] * r->values[3];
    float d = l->values[1] * r->values[2] + l->values[3] * r->values[3];
    
    o->values[0] = a;
    o->values[1] = c;
    o->values[2] = b;
    o->values[3] = d;
}

void scaleM2f(const TMatrix2f* m, float s, TMatrix2f* o) {
    for (int i = 0; i < 4; i++)
        o->values[i] = m->values[i] * s;
}

#pragma mark TMatrix3f functions

void setIdentityM3f(TMatrix3f* m) {
    for (int r = 0; r < 3; r++)
        for (int c = 0; c < 3; c++)
            m->values[c * 3 + r] = r == c ? 1 : 0;
}

void setMinorM3f(const TMatrix4f* m4f, int col, int row, TMatrix3f* o) {
    int i = 0;
    for (int c = 0; c < 4; c++)
        for (int r = 0; r < 4; r++)
            if (c != col && r != row)
                o->values[i++] = m4f->values[c * 4 + r];
}

void setColumnM3f(const TMatrix3f* m, const TVector3f* v, int col, TMatrix3f* o) {
    if (o != m)
        *o = *m;
    
    o->values[col * 3 + 0] = v->x;
    o->values[col * 3 + 1] = v->y;
    o->values[col * 3 + 2] = v->z;
}

void setValueM3f(const TMatrix3f* m, float v, int col, int row, TMatrix3f* o) {
    if (o != m)
        *o = *m;
    
    o->values[col * 3 + row] = v;
}

BOOL invertM3f(const TMatrix3f* m, TMatrix3f* o) {
    float det = determinantM3f(m);
    if (fzero(det))
        return NO;
    
    adjugateM3f(m, o);
    scaleM3f(o, 1 / det, o);
    return YES;
}

void adjugateM3f(const TMatrix3f* m, TMatrix3f* o) {
    TMatrix2f m2f;
    for (int c = 0; c < 3; c++) {
        for (int r = 0; r < 3; r++) {
            setMinorM2f(m, c, r, &m2f);
            o->values[c * 3 + r] = ((c + r) % 2 == 0 ? 1 : -1) * determinantM2f(&m2f);
        }
    }
    
    transposeM3f(o, o);
}

float determinantM3f(const TMatrix3f* m) {
    return m->values[0] * m->values[4] * m->values[8]
         + m->values[3] * m->values[7] * m->values[2]
         + m->values[6] * m->values[1] * m->values[5]
         - m->values[2] * m->values[4] * m->values[6]
         - m->values[5] * m->values[7] * m->values[0]
         - m->values[8] * m->values[1] * m->values[3];
}

void negateM3f(const TMatrix3f* m, TMatrix3f* o) {
    scaleM3f(m, -1, o);
}

void transposeM3f(const TMatrix3f* m, TMatrix3f* o) {
    for (int c = 0; c < 3; c++) {
        o->values[c * 3 + c] = m->values[c * 3 + c];
        for (int r = c + 1; r < 3; r++) {
            float t = m->values[c * 3 + r];
            o->values[c * 3 + r] = m->values[r * 3 + c];
            o->values[r * 3 + c] = t;
        }
    }
}

void addM3f(const TMatrix3f* l, const TMatrix3f* r, TMatrix3f* o) {
    for (int i = 0; i < 9; i++)
        o->values[i] = l->values[i] + r->values[i];
}

void subM3f(const TMatrix3f* l, const TMatrix3f* r, TMatrix3f* o) {
    for (int i = 0; i < 9; i++)
        o->values[i] = l->values[i] - r->values[i];
}

void mulM3f(const TMatrix3f* l, const TMatrix3f* rm, TMatrix3f* o) {
    TMatrix3f t;
    for (int c = 0; c < 3; c++) {
        for (int r = 0; r < 3; r++) {
            t.values[c * 3 + r] = 0;
            for (int i = 0; i < 3; i++)
                t.values[c * 3 + r] += l->values[i * 3 + r] * rm->values[c * 3 + i];
        }
    }
    
    *o = t;
}

void scaleM3f(const TMatrix3f* m, float s, TMatrix3f* o) {
    for (int i = 0; i < 9; i++)
        o->values[i] = m->values[i] * s;
}

#pragma mark TMatrix4f functions

void setIdentityM4f(TMatrix4f* m) {
    for (int r = 0; r < 4; r++)
        for (int c = 0; c < 4; c++)
            m->values[c * 4 + r] = r == c ? 1 : 0;
}

void embedM4f(const TMatrix3f* m3f, TMatrix4f* m4f);

void setSubMatrixM4f(const TMatrix4f* m4f, const TMatrix2f* m2f, int i, TMatrix4f* o) {
    if (o != m4f)
        *o = *m4f;
    
    switch (i) {
        case 0:
            o->values[ 0] = m2f->values[0];
            o->values[ 1] = m2f->values[1];
            o->values[ 4] = m2f->values[2];
            o->values[ 5] = m2f->values[3];
            break;
        case 1:
            o->values[ 2] = m2f->values[0];
            o->values[ 3] = m2f->values[1];
            o->values[ 6] = m2f->values[2];
            o->values[ 7] = m2f->values[3];
            break;
        case 2:
            o->values[ 8] = m2f->values[0];
            o->values[ 9] = m2f->values[1];
            o->values[12] = m2f->values[2];
            o->values[13] = m2f->values[3];
            break;
        case 3:
            o->values[10] = m2f->values[0];
            o->values[11] = m2f->values[1];
            o->values[14] = m2f->values[2];
            o->values[15] = m2f->values[3];
            break;
        default:
            [NSException raise:NSInvalidArgumentException format:@"sub matrix index out of bounds: %i", index];
            break;
    }
}

void setColumnM4fV4f(const TMatrix4f* m, const TVector4f* v, int col, TMatrix4f* o) {
    if (o != m)
        *o = *m;

    o->values[col * 4 + 0] = v->x;
    o->values[col * 4 + 1] = v->y;
    o->values[col * 4 + 2] = v->z;
    o->values[col * 4 + 3] = v->w;
}

void setColumnM4fV3f(const TMatrix4f* m, const TVector3f* v, int col, TMatrix4f* o) {
    if (o != m)
        *o = *m;
    
    o->values[col * 4 + 0] = v->x;
    o->values[col * 4 + 1] = v->y;
    o->values[col * 4 + 2] = v->z;
    o->values[col * 4 + 3] = 0;
}

void setValueM4f(const TMatrix4f* m, float v, int col, int row, TMatrix4f* o) {
    if (o != m)
        *o = *m;
    
    o->values[col * 4 + row] = v;
}

BOOL invertM4f(const TMatrix4f* m, TMatrix4f* o) {
    float det = determinantM4f(m);
    if (fzero(det))
        return NO;
    
    TMatrix2f A, Ai;
    setMatrix2fAsSubMatrix(m, 0, &A);
    
    if (invertM2f(&A, &Ai)) { // use quick method
        TMatrix2f B, C, D, CAi, CAiB, AiB;
        
        setMatrix2fAsSubMatrix(m, 2, &B);
        setMatrix2fAsSubMatrix(m, 1, &C);
        setMatrix2fAsSubMatrix(m, 3, &D);
        
        mulM2f(&C, &Ai, &CAi);
        mulM2f(&CAi, &B, &CAiB);
        mulM2f(&Ai, &B, &AiB);
        
        // calculate D
        subM2f(&D, &CAiB, &D);
        invertM2f(&D, &D);
        
        // calculate -C and -B
        mulM2f(&D, &CAi, &C);
        mulM2f(&AiB, &D, &B);
        
        mulM2f(&B, &CAi, &A);
        addM2f(&A, &Ai, &A);
        
        negateM2f(&C, &C);
        negateM2f(&B, &B);

        setSubMatrixM4f(m, &A, 0, o);
        setSubMatrixM4f(o, &C, 1, o);
        setSubMatrixM4f(o, &B, 2, o);
        setSubMatrixM4f(o, &D, 3, o);
    } else { // use general but slower method
        adjugateM4f(m, o);
        scaleM4f(o, 1 / det, o);
    }
    
    return YES;
}

void adjugateM4f(const TMatrix4f* m, TMatrix4f* o) {
    TMatrix3f m3f;
    for (int c = 0; c < 4; c++) {
        for (int r = 0; r < 4; r++) {
            setMinorM3f(m, c, r, &m3f);
            o->values[c * 4 + r] = ((c + r) % 2 == 0 ? 1 : -1) * determinantM3f(&m3f);
        }
    }
    
    transposeM4f(o, o);
}

float determinantM4f(const TMatrix4f* m) {
    // Laplace after first col
    float det = 0;
    TMatrix3f m3f;
    
    for (int r = 0; r < 4; r++) {
        setMinorM3f(m, 0, r, &m3f);
        det += (r % 2 == 0 ? 1 : -1) * m->values[r] * determinantM3f(&m3f);
    }
    
    return det;
}

void negateM4f(const TMatrix4f* m, TMatrix4f* o) {
    for (int i = 0; i < 16; i++)
        o->values[i] = -1 * m->values[i];
}

void transposeM4f(const TMatrix4f* m, TMatrix4f* o) {
    for (int c = 0; c < 4; c++) {
        o->values[c * 4 + c] = m->values[c * 4 + c];
        for (int r = c + 1; r < 4; r++) {
            float t = m->values[c * 4 + r];
            o->values[c * 4 + r] = m->values[r * 4 + c];
            o->values[r * 4 + c] = t;
        }
    }
}

void addM4f(const TMatrix4f* l, const TMatrix4f* r, TMatrix4f* o) {
    for (int i = 0; i < 16; i++)
        o->values[i] = l->values[i] + r->values[i];
}

void subM4f(const TMatrix4f* l, const TMatrix4f* r, TMatrix4f* o) {
    for (int i = 0; i < 16; i++)
        o->values[i] = l->values[i] - r->values[i];
}

void mulM4f(const TMatrix4f* l, const TMatrix4f* rm, TMatrix4f* o) { // o = l * rm
    TMatrix4f t;
    for (int c = 0; c < 4; c++) {
        for (int r = 0; r < 4; r++) {
            t.values[c * 4 + r] = 0;
            for (int i = 0; i < 4; i++)
                t.values[c * 4 + r] += l->values[i * 4 + r] * rm->values[c * 4 + i];
        }
    }
    *o = t;
}

void scaleM4f(const TMatrix4f* m, float s, TMatrix4f* o) {
    for (int i = 0; i < 16; i++)
        o->values[i] = m->values[i] * s;
}

void rotateM4f(const TMatrix4f* m, const TVector3f* x, float a, TMatrix4f* o) {
    float s = sinf(a);
    float c = cosf(a);
    float i = 1 - c;
    
    float ix  = i  * x->x;
    float ix2 = ix * x->x;
    float ixy = ix * x->y;
    float ixz = ix * x->z;
    
    float iy  = i  * x->y;
    float iy2 = iy * x->y;
    float iyz = iy * x->z;
    
    float iz2 = i  * x->z * x->z;
    
    float sx = s * x->x;
    float sy = s * x->y;
    float sz = s * x->z;
    
    TMatrix4f t;
    
    t.values[ 0] = ix2 + c;
    t.values[ 1] = ixy - sz;
    t.values[ 2] = ixz + sy;
    t.values[ 3] = 0;
    
    t.values[ 4] = ixy + sz;
    t.values[ 5] = iy2 + c;
    t.values[ 6] = iyz - sx;
    t.values[ 7] = 0;
    
    t.values[ 8] = ixz - sy;
    t.values[ 9] = iyz + sx;
    t.values[10] = iz2 + c;
    t.values[11] = 0;
    
    t.values[12] = 0;
    t.values[13] = 0;
    t.values[14] = 0;
    t.values[15] = 1;
    
    mulM4f(m, &t, o);
}

void rotateM4fQ(const TMatrix4f* m, const TQuaternion* q, TMatrix4f* o) {
    TMatrix4f t;
    
    float a = q->scalar;
    float b = q->vector.x;
    float c = q->vector.y;
    float d = q->vector.z;
    
    float a2 = a * a;
    float b2 = b * b;
    float c2 = c * c;
    float d2 = d * d;
    
    t.values[ 0] = a2 + b2 - c2 - d2;
    t.values[ 1] = 2 * b * c + 2 * a * d;
    t.values[ 2] = 2 * b * d - 2 * a * c;
    t.values[ 3] = 0;

    t.values[ 4] = 2 * b * c - 2 * a * d;
    t.values[ 5] = a2 - b2 + c2 - d2;
    t.values[ 6] = 2 * c * d + 2 * a * b;
    t.values[ 7] = 0;
    
    t.values[ 8] = 2 * b * d + 2 * a * c;
    t.values[ 9] = 2 * c * d - 2 * a * b;
    t.values[10] = a2 - b2 - c2 + d2;
    t.values[11] = 0;

    t.values[12] = 0;
    t.values[13] = 0;
    t.values[14] = 0;
    t.values[15] = 1;
    
    mulM4f(m, &t, o);
}

void translateM4f(const TMatrix4f* m, const TVector3f* d, TMatrix4f* o) {
    TMatrix4f t;

    t = IdentityM4f;
    t.values[12] += d->x;
    t.values[13] += d->y;
    t.values[14] += d->z;
    
    mulM4f(m, &t, o);
}

void scaleM4fV3f(const TMatrix4f* m, const TVector3f* s, TMatrix4f* o) {
    o->values[ 0] = m->values[ 0] * s->x;
    o->values[ 1] = m->values[ 1] * s->y;
    o->values[ 2] = m->values[ 2] * s->z;
    o->values[ 3] = m->values[ 3];
    
    o->values[ 4] = m->values[ 4] * s->x;
    o->values[ 5] = m->values[ 5] * s->y;
    o->values[ 6] = m->values[ 6] * s->z;
    o->values[ 7] = m->values[ 7];
    
    o->values[ 8] = m->values[ 8] * s->x;
    o->values[ 9] = m->values[ 9] * s->y;
    o->values[10] = m->values[10] * s->z;
    o->values[11] = m->values[11];
    
    o->values[12] = m->values[12] * s->x;
    o->values[13] = m->values[13] * s->y;
    o->values[14] = m->values[14] * s->z;
    o->values[15] = m->values[15];
}

void transformM4fV3f(const TMatrix4f* m, const TVector3f* v, TVector3f* o) {
    TVector4f v4f;
    v4f.x = v->x;
    v4f.y = v->y;
    v4f.z = v->z;
    v4f.w = 1;
    
    transformM4fV4f(m, &v4f, &v4f);
    
    o->x = v4f.x / v4f.w;
    o->y = v4f.y / v4f.w;
    o->z = v4f.z / v4f.w;
}

void transformM4fV4f(const TMatrix4f* m, const TVector4f* v, TVector4f* o) { // o = m * v
    float x = m->values[ 0] * v->x + m->values[ 4] * v->y + m->values[ 8] * v->z + m->values[12] * v->w;
    float y = m->values[ 1] * v->x + m->values[ 5] * v->y + m->values[ 9] * v->z + m->values[13] * v->w;
    float z = m->values[ 2] * v->x + m->values[ 6] * v->y + m->values[10] * v->z + m->values[14] * v->w;
    float w = m->values[ 3] * v->x + m->values[ 7] * v->y + m->values[11] * v->z + m->values[15] * v->w;
    
    o->x = x;
    o->y = y;
    o->z = z;
    o->w = w;
}

# pragma mark Coordinate plane functions

void projectOntoCoordinatePlane(EPlane plane, const TVector3f* v, TVector3f* o) {
    switch (plane) {
        case P_XY: {
            o->x = v->x;
            o->y = v->y;
            o->z = v->z;
            break;
        }
        case P_XZ: {
            float y = v->y;
            o->x = v->x;
            o->y = v->z;
            o->z = y;
            break;
        }
        default: {
            float x = v->x;
            o->x = v->y;
            o->y = v->z;
            o->z = x;
            break;
        }
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

float measureDist(const TVector3f* v, EAxis measureDir) {
    if (measureDir == A_X)
        return fabsf(v->x - roundf(v->x));
    if (measureDir == A_Y)
        return fabsf(v->y - roundf(v->y));
    return fabsf(v->z - roundf(v->z));
}

/*
void findXWithMinYOffset(const TVector2f* v, TVector2f* r) {
    TVector2f prev, cur, t2f;
    float prevOffset, curOffset, grad, tf;
    int dX;
    
    grad = v->y / v->x;
    dX = v->x > 0 ? 1 : -1;
        
    prev.x = (int)v->x + dX;
    prev.y = grad * prev.x;
    prevOffset = fabsf(prev.y - (int)prev.y);

    cur.x = prev.x + dX;
    cur.y = grad * cur.x;
    curOffset = fabsf(cur.y - (int)cur.y);
    
    if (prevOffset == curOffset) {
        *r = prev;
        return;
    }
    
    if (curOffset > prevOffset) {
        dX *= -1;
        
        t2f = prev;
        prev = cur;
        cur = t2f;
        
        tf = prevOffset;
        prevOffset = curOffset;
        curOffset = tf;
    }
    
    while (curOffset < prevOffset) {
        prevOffset = curOffset;
        prev = cur;
        
        cur.x = prev.x + dX;
        cur.y = grad * cur.x;
        curOffset = fabsf(cur.y - (int)cur.y);
    }

    *r = prev;
}

void makePointsForPlane(const TPlane* p, const TBoundingBox* m, TVector3i* p1, TVector3i* p2, TVector3i* p3) {
    TLine l;
    TVector3f v3f;
    TVector2f v2f;
    float f;
    int x;
    
    switch (strongestComponentV3f(&p->norm)) {
        case A_X:
            break;
        case A_Y:
            break;
        default:
            break;
    }
}
 */
 

float updatePoint(const TPlane* pl, EAxis a, TVector3f* p) {
    switch (a) {
        case A_X:
            p->x = planeX(pl, p->y, p->z);
            return fabsf(p->x - roundf(p->x));
        case A_Y:
            p->y = planeY(pl, p->x, p->z);
            return fabsf(p->y - roundf(p->y));
        default:
            p->z = planeZ(pl, p->x, p->y);
            return fabsf(p->z - roundf(p->z));
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

void makeCylinder(float radius, float height, int segments, TVector3f* points, TVector3f* normals) {
    float d = 2 * M_PI / segments;
    float a = 0;
    for (int i = 0; i < segments; i++) {
        float s = sin(a);
        float c = cos(a);
        
        points[2 * i].x = radius * s;
        points[2 * i].y = radius * c;
        points[2 * i].z = height;
        points[2 * i + 1].x = radius * s;
        points[2 * i + 1].y = radius * c;
        points[2 * i + 1].z = 0;
        
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

void makeArrowTriangles(float baseLength, float baseWidth, float headLength, float headWidth, TVector3f* points) {
    points[0].x = 0;
    points[0].y = 0;
    points[0].z = 0;
    points[1].x = baseLength;
    points[1].y = baseWidth / 2;
    points[1].z = 0;
    points[2].x = baseLength;
    points[2].y = -baseWidth / 2;
    points[2].z = 0;
    points[3].x = baseLength + headLength;
    points[3].y = 0;
    points[3].z = 0;
    points[4].x = baseLength;
    points[4].y = -headWidth / 2;
    points[4].z = 0;
    points[5].x = baseLength;
    points[5].y = headWidth / 2;
    points[5].z = 0;
}

void makeArrowOutline(float baseLength, float baseWidth, float headLength, float headWidth, TVector3f* points) {
    points[0].x = 0;
    points[0].y = 0;
    points[0].z = 0;
    points[1].x = baseLength;
    points[1].y = baseWidth / 2;
    points[1].z = 0;
    points[2].x = baseLength;
    points[2].y = headWidth / 2;
    points[2].z = 0;
    points[3].x = baseLength + headLength;
    points[3].y = 0;
    points[3].z = 0;
    points[4].x = baseLength;
    points[4].y = -headWidth / 2;
    points[4].z = 0;
    points[5].x = baseLength;
    points[5].y = -baseWidth / 2;
    points[5].z = 0;
}
