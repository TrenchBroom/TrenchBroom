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

typedef struct {
    TVector3f start;
    TVector3f end;
    TVector3f control;
} TQuadraticBezierCurve;

typedef struct {
    TVector3f start;
    TVector3f end;
    TVector3f startControl;
    TVector3f endControl;
} TCubicBezierCurve;

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
int mini(int v1, int v2);
int maxi(int v1, int v2);

void addV2f(const TVector2f* l, const TVector2f* r, TVector2f* o);
void subV2f(const TVector2f* l, const TVector2f* r, TVector2f* o);
float dotV2f(const TVector2f* l, const TVector2f* r);
void scaleV2f(const TVector2f* v, float f, TVector2f* r);
float lengthSquaredV2f(const TVector2f* v);
float lengthV2f(const TVector2f* v);
void normalizeV2f(const TVector2f* v, TVector2f* r);

float dot3f(float lx, float ly, float lz, float rx, float ry, float rz);
BOOL segmentIntersectsSegment(float s11, float s12, float s21, float s22);
BOOL segmentContainsSegment(float s11, float s12, float s21, float s22);
BOOL segmentContainsPoint(float s11, float s12, float p);

void addV3f(const TVector3f* l, const TVector3f* r, TVector3f* o);
void sumV3f(const TVector3f* v, int c, TVector3f* o);
void subV3f(const TVector3f* l, const TVector3f* r, TVector3f* o);
float dotV3f(const TVector3f* l, const TVector3f* r);
void crossV3f(const TVector3f* l, const TVector3f* r, TVector3f* o);
void scaleV3f(const TVector3f* v, float f, TVector3f* o);
float lengthV3f(const TVector3f* v);
float lengthSquaredV3f(const TVector3f* v);
void normalizeV3f(const TVector3f* v, TVector3f* o);
BOOL equalV3f(const TVector3f* l, const TVector3f* r);
BOOL nullV3f(const TVector3f* v);
EAxis strongestComponentV3f(const TVector3f* v);
EAxis weakestComponentV3f(const TVector3f* v);
void closestAxisV3f(const TVector3f* v, TVector3f* o);
float componentV3f(const TVector3f* v, EAxis a);
void setComponentV3f(TVector3f* v, EAxis a, float f);
void roundV3f(const TVector3f* v, TVector3i* o);
void setV3f(TVector3f* l, const TVector3i* r);
void rotateZ90CWV3f(const TVector3f* v, TVector3f *o);
void rotateZ90CCWV3f(const TVector3f* v, TVector3f *o);
BOOL parseV3f(NSString* s, NSRange r, TVector3f* o);

void addV3i(const TVector3i* l, const TVector3i* r, TVector3i* o);
void subV3i(const TVector3i* l, const TVector3i* r, TVector3i* o);
void scaleV3i(const TVector3i* v, int i, TVector3i* o);
BOOL equalV3i(const TVector3i* l, const TVector3i* r);
BOOL nullV3i(const TVector3i* v);
void rotateZ90CWV3i(const TVector3i* v, TVector3i *o);
void rotateZ90CCWV3i(const TVector3i* v, TVector3i *o);
BOOL parseV3i(NSString* s, NSRange r, TVector3i* o);

TVector3fList* newVector3fList(int c);
void freeVector3fList(TVector3fList* l);
void addVector3fToList(TVector3f v, TVector3fList* l);
void removeVector3fFromList(int idx, TVector3fList* l);

void setLinePoints(TLine* l, TVector3f* p1, TVector3f* p2);
void linePointAtDistance(TLine* l, float d, TVector3f* p);

void setPlanePointsV3i(TPlane* p, const TVector3i* p1, const TVector3i* p2, const TVector3i* p3);
void setPlanePointsV3f(TPlane* p, const TVector3f* p1, const TVector3f* p2, const TVector3f* p3);
EPointStatus pointStatusFromPlane(const TPlane* p, const TVector3f* v);
EPointStatus pointStatusFromRay(const TVector3f* o, const TVector3f* d, const TVector3f* v);

float intersectPlaneWithRay(const TPlane* p, const TRay* r);
float intersectPlaneWithLine(const TPlane* p, const TLine* l);
float planeX(const TPlane* p, float y, float z);
float planeY(const TPlane* p, float x, float z);
float planeZ(const TPlane* p, float x, float y);

void centerOfBounds(const TBoundingBox* b, TVector3f* o);
void roundedCenterOfBounds(const TBoundingBox* b, TVector3i* o);
void translateBounds(const TBoundingBox* b, const TVector3f* d, TBoundingBox* o);
void rotateBounds(const TBoundingBox* b, const TQuaternion* q, const TVector3f* c, TBoundingBox* o);
void rotateBoundsZ90CW(const TBoundingBox* b, const TVector3f* c, TBoundingBox* o);
void rotateBoundsZ90CCW(const TBoundingBox* b, const TVector3f* c, TBoundingBox* o);
void mergeBoundsWithPoint(const TBoundingBox* b, const TVector3f* p, TBoundingBox* o);
void mergeBoundsWithBounds(const TBoundingBox* b, const TBoundingBox* c, TBoundingBox* o);
void expandBounds(const TBoundingBox* b, float f, TBoundingBox* o);
void sizeOfBounds(const TBoundingBox* b, TVector3f* o);
void roundedSizeOfBounds(const TBoundingBox* b, TVector3i* o);
float radiusOfBounds(const TBoundingBox* b);
float intersectBoundsWithRay(const TBoundingBox* b, const TRay* ray, TVector3f* n);
BOOL boundsContainPoint(const TBoundingBox* b, const TVector3f* p);
BOOL boundsIntersectWithBounds(const TBoundingBox* b1, const TBoundingBox* b2);
BOOL boundsContainBounds(const TBoundingBox* b1, const TBoundingBox *b2);

void setQ(TQuaternion* l, const TQuaternion* r);
void setAngleAndAxisQ(TQuaternion* q, float a, const TVector3f* x);
void mulQ(const TQuaternion* l, const TQuaternion* r, TQuaternion* o);
void conjugateQ(const TQuaternion* q, TQuaternion* o);
void rotateQ(const TQuaternion* q, const TVector3f* v, TVector3f* o);

void pointOnQuadraticBezierCurve(const TQuadraticBezierCurve* c, float t, TVector3f* r);
void pointOnCubicBezierCurve(const TCubicBezierCurve* c, float t, TVector3f* r);

float intersectSphereWithRay(const TVector3f* c, float ra, const TRay* r);
float distanceOfPointAndRay(const TVector3f* c, const TRay* r);
float closestPointOnRay(const TVector3f* c, const TRay* r);
float distanceOfSegmentAndRay(const TVector3f* ss, const TVector3f* se, const TRay* r, float* rd);
float distanceOfSegmentAndRaySquared(const TVector3f* ss, const TVector3f* se, const TRay* r, float* rd);
void rayPointAtDistance(const TRay* r, float d, TVector3f* p);

void projectOntoPlane(EPlane plane, const TVector3f* v, TVector3f* o);

void makeCircle(float radius, int segments, TVector3f* points);
void makeRing(float innerRadius, float outerRadius, int segments, TVector3f* points);
void makeTorus(float innerRadius, float outerRadius, int innerSegments, int outerSegments, TVector3f* points, TVector3f* normals);
void makeTorusPart(float innerRadius, float outerRadius, int innerSegments, int outerSegments, float centerAngle, float angleLength, TVector3f* points, TVector3f* normals);
void makeCone(float radius, float height, int segments, TVector3f* points, TVector3f* normals);
