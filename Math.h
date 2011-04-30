//
//  Math.h
//  TrenchBroom
//
//  Created by Kristian Duske on 24.08.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

typedef enum {
    A_X,
    A_Y,
    A_Z
} EAxis;

typedef enum {
    P_XY,
    P_XZ,
    P_YZ
} EPlane;

typedef enum {
    PS_ABOVE, // point is above the plane
    PS_BELOW, // point is below the plane
    PS_INSIDE // point is contained inside the plane
} EPointStatus;

typedef struct {
    float x,y,z,w;
} TVector4f;

typedef struct {
    float x,y,z;
} TVector3f;

typedef struct {
    TVector3f* items;
    int count;
    int capacity;
} TVector3fList;

typedef struct {
    int x,y,z;
} TVector3i;

typedef struct {
    float x,y;
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
    TVector3f min;
    TVector3f max;
} TBoundingBox;

typedef struct {
    float scalar;
    TVector3f vector;
} TQuaternion;

extern float const AlmostZero;
extern TVector3f const XAxisPos;
extern TVector3f const XAxisNeg;
extern TVector3f const YAxisPos;
extern TVector3f const YAxisNeg;
extern TVector3f const ZAxisPos;
extern TVector3f const ZAxisNeg;
extern TVector3f const NullVector;

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

float dot3f(float lx, float ly, float lz, float rx, float ry, float rz);

void addV3f(const TVector3f* l, const TVector3f* r, TVector3f* o);
void subV3f(const TVector3f* l, const TVector3f* r, TVector3f* o);
float dotV3f(const TVector3f* l, const TVector3f* r);
void crossV3f(const TVector3f* l, const TVector3f* r, TVector3f* o);
void scaleV3f(const TVector3f* v, float f, TVector3f* o);
float lengthV3f(const TVector3f* v);
float lengthSquaredV3f(const TVector3f* v);
void normalizeV3f(const TVector3f* v, TVector3f* o);
BOOL equalV3f(const TVector3f* l, const TVector3f* r);
EAxis largestComponentV3f(const TVector3f* v);
float componentV3f(const TVector3f* v, EAxis a);
void setComponentV3f(TVector3f* v, EAxis a, float f);
void roundV3f(const TVector3f* v, TVector3i* o);
void setV3f(TVector3f* l, const TVector3i* r);
BOOL parseV3f(NSString* s, NSRange r, TVector3f* o);

void addV3i(const TVector3i* l, const TVector3i* r, TVector3i* o);
void subV3i(const TVector3i* l, const TVector3i* r, TVector3i* o);
BOOL equalV3i(const TVector3i* l, const TVector3i* r);
BOOL parseV3i(NSString* s, NSRange r, TVector3i* o);

TVector3fList* newVector3fList(int c);
void freeVector3fList(TVector3fList* l);
void addVector3fToList(TVector3f v, TVector3fList* l);
void removeVector3fFromList(int idx, TVector3fList* l);

void setLinePoints(TLine* l, TVector3f* p1, TVector3f* p2);
void linePointAtDistance(TLine* l, float d, TVector3f* p);

void setPlanePoints(TPlane* p, const TVector3i* p1, const TVector3i* p2, const TVector3i* p3);
EPointStatus pointStatus(const TPlane* p, const TVector3f* v);
float intersectPlaneWithRay(const TPlane* p, const TRay* r);
float intersectPlaneWithLine(const TPlane* p, const TLine* l);
float planeX(TPlane* p, float y, float z);
float planeY(TPlane* p, float x, float z);
float planeZ(TPlane* p, float x, float y);

void centerOfBounds(const TBoundingBox* b, TVector3f* o);
void mergeBoundsWithPoint(const TBoundingBox* b, const TVector3f* p, TBoundingBox* o);
void mergeBoundsWithBounds(const TBoundingBox* b, const TBoundingBox* c, TBoundingBox* o);
void expandBounds(const TBoundingBox* b, float f, TBoundingBox* o);
void sizeOfBounds(const TBoundingBox* b, TVector3f* o);

void setQ(TQuaternion* l, const TQuaternion* r);
void setAngleAndAxisQ(TQuaternion* q, float a, const TVector3f* x);
void mulQ(const TQuaternion* l, const TQuaternion* r, TQuaternion* o);
void conjugateQ(const TQuaternion* q, TQuaternion* o);
void rotateQ(const TQuaternion* q, const TVector3f* v, TVector3f* o);

float intersectSphereWithRay(const TVector3f* c, float ra, const TRay* r);
void rayPointAtDistance(const TRay* r, float d, TVector3f* p);

void projectOntoPlane(EPlane plane, const TVector3f* v, TVector3f* o);

void makeCircle(float radius, int segments, TVector3f* points);
void makeRing(float innerRadius, float outerRadius, int segments, TVector3f* points);
