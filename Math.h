//
//  Math.h
//  TrenchBroom
//
//  Created by Kristian Duske on 24.08.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

extern float const AlmostZero;

typedef enum {
    A_X,
    A_Y,
    A_Z
} EAxis;

typedef struct {
    float x,y,z;
} TVector3f;

typedef struct {
    int x,y,z;
} TVector3i;

typedef struct {
    float x,y,z;
} TVector2f;

typedef struct {
    TVector3f point;
    TVector3f direction;
} TLine;

typedef struct {
    TVector3f origin;
    TVector3f direction;
} TRay;

typedef struct {
    TVector3f point;
    TVector3f norm;
} TPlane;

typedef struct {
    float scalar;
    TVector3f vector;
} TQuaternion;

BOOL fzero(float v);
BOOL fpos(float v);
BOOL fneg(float v);
BOOL feq(float v1, float v2);
BOOL fgt(float v1, float v2);
BOOL flt(float v1, float v2);
BOOL fgte(float v1, float v2);
BOOL flte(float v1, float v2);
BOOL finxx(float v, float b1, float b2);
BOOL finxi(float v, float b1, float b2);
BOOL finix(float v, float b1, float b2);
BOOL finii(float v, float b1, float b2);

void setV3f(TVector3f* l, TVector3f* r);
void addV3f(TVector3f* l, TVector3f* r);
void subV3f(TVector3f* l, TVector3f* r);
float dotV3f(TVector3f* l, TVector3f* r);
void crossV3f(TVector3f* l, TVector3f* r);
void scaleV3f(TVector3f* v, float f);
float lengthV3f(TVector3f* v);
float lengthSquaredV3f(TVector3f* v);
void normalizeV3f(TVector3f* v);
BOOL equalV3f(TVector3f* l, TVector3f* r);

BOOL pointAbovePlane(TPlane* p, TVector3f* v);
float intersectPlaneWithLine(TPlane* p, TLine* l);
float intersectPlaneWithRay(TPlane* p, TRay* r);

void setQ(TQuaternion* l, TQuaternion* r);
void setAngleAndAxisQ(TQuaternion* q, float a, TVector3f* x);
void mulQ(TQuaternion* l, TQuaternion* r);
void conjugateQ(TQuaternion* q);
void rotateQ(TQuaternion* q, TVector3f* v);

TVector3f* rayPointAtDistance(TRay* r, float d);
TVector3f* linePointAtDistance(TLine* l, float d);

NSArray* makeCircle(float radius, int segments);
NSArray* makeRing(float innerRadius, float outerRadius, int segments);

int smallestXVertex2D(NSArray *vertices);
int smallestYVertex2D(NSArray *vertices);
