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
    int x,y,z;
} TVector3i;

typedef struct {
    float x,y;
} TVector2f;

typedef struct {
    int x,y;
} TVector2i;

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

typedef struct {
    float values[4];
} TMatrix2f;

typedef struct {
    float values[9];
} TMatrix3f;

typedef struct {
    float values[16];
} TMatrix4f;

extern float const AlmostZero;
extern TVector3f const XAxisPos;
extern TVector3f const XAxisNeg;
extern TVector3f const YAxisPos;
extern TVector3f const YAxisNeg;
extern TVector3f const ZAxisPos;
extern TVector3f const ZAxisNeg;
extern TVector3f const NullVector;
extern TMatrix2f const IdentityM2f;
extern TMatrix3f const IdentityM3f;
extern TMatrix4f const IdentityM4f;
extern TMatrix4f const RotX90CWM4f;
extern TMatrix4f const RotY90CWM4f;
extern TMatrix4f const RotZ90CWM4f;
extern TMatrix4f const RotX90CCWM4f;
extern TMatrix4f const RotY90CCWM4f;
extern TMatrix4f const RotZ90CCWM4f;
extern TMatrix4f const MirXM4f;
extern TMatrix4f const MirYM4f;
extern TMatrix4f const MirZM4f;

extern float const PointStatusEpsilon;

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
void absV3f(const TVector3f* v, TVector3f* o);
float lengthV3f(const TVector3f* v);
float lengthSquaredV3f(const TVector3f* v);
void normalizeV3f(const TVector3f* v, TVector3f* o);
BOOL equalV3f(const TVector3f* l, const TVector3f* r);
BOOL absEqualV3f(const TVector3f* l, const TVector3f* r);
BOOL intV3f(const TVector3f* v);
BOOL nullV3f(const TVector3f* v);
EAxis strongestComponentV3f(const TVector3f* v);
EAxis weakestComponentV3f(const TVector3f* v);
const TVector3f* firstAxisV3f(const TVector3f* v);
const TVector3f* firstAxisNegV3f(const TVector3f* v);
const TVector3f* secondAxisV3f(const TVector3f* v);
const TVector3f* secondAxisNegV3f(const TVector3f* v);
const TVector3f* thirdAxisV3f(const TVector3f* v);
const TVector3f* thirdAxisNegV3f(const TVector3f* v);
float componentV3f(const TVector3f* v, EAxis a);
void setComponentV3f(TVector3f* v, EAxis a, float f);
void roundV3f(const TVector3f* v, TVector3f* o);
void roundUpV3f(const TVector3f* v, TVector3f* o);
void roundDownV3f(const TVector3f* v, TVector3f* o);
void snapV3f(const TVector3f* v, TVector3f* o);
void setV3f(TVector3f* l, const TVector3i* r);
void rotate90CWV3f(const TVector3f* v, EAxis a, TVector3f *o);
void rotate90CCWV3f(const TVector3f* v, EAxis a, TVector3f *o);
BOOL parseV3f(NSString* s, NSRange r, TVector3f* o);
BOOL opposingV3f(const TVector3f* v1, const TVector3f* v2);
BOOL normV3f(const TVector3f* v1, const TVector3f* v2, const TVector3f* v3, TVector3f* o);
void avg3V3f(const TVector3f* v1, const TVector3f* v2, const TVector3f* v3, TVector3f* o);

void addV3i(const TVector3i* l, const TVector3i* r, TVector3i* o);
void subV3i(const TVector3i* l, const TVector3i* r, TVector3i* o);
void setV3i(TVector3i* l, const TVector3f* r);
void scaleV3i(const TVector3i* v, int i, TVector3i* o);
BOOL equalV3i(const TVector3i* l, const TVector3i* r);
BOOL nullV3i(const TVector3i* v);
void rotate90CWV3i(const TVector3i* v, EAxis a, TVector3i *o);
void rotate90CCWV3i(const TVector3i* v, EAxis a, TVector3i *o);
BOOL parseV3i(NSString* s, NSRange r, TVector3i* o);

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
BOOL equalPlane(const TPlane* p1, const TPlane* p2);

NSString* axisName(EAxis a);

void centerOfBounds(const TBoundingBox* b, TVector3f* o);
void translateBounds(const TBoundingBox* b, const TVector3f* d, TBoundingBox* o);
void rotateBounds(const TBoundingBox* b, const TQuaternion* q, const TVector3f* c, TBoundingBox* o);
void rotateBounds90CW(const TBoundingBox* b, EAxis a, const TVector3f* c, TBoundingBox* o);
void rotateBounds90CCW(const TBoundingBox* b, EAxis a, const TVector3f* c, TBoundingBox* o);
void mergeBoundsWithPoint(const TBoundingBox* b, const TVector3f* p, TBoundingBox* o);
void mergeBoundsWithBounds(const TBoundingBox* b, const TBoundingBox* c, TBoundingBox* o);
void expandBounds(const TBoundingBox* b, float f, TBoundingBox* o);
void sizeOfBounds(const TBoundingBox* b, TVector3f* o);
float radiusOfBounds(const TBoundingBox* b);
float intersectBoundsWithRay(const TBoundingBox* b, const TRay* ray, TVector3f* n);
BOOL boundsContainPoint(const TBoundingBox* b, const TVector3f* p);
BOOL boundsIntersectWithBounds(const TBoundingBox* b1, const TBoundingBox* b2);
BOOL boundsContainBounds(const TBoundingBox* b1, const TBoundingBox *b2);

void setQ(TQuaternion* l, const TQuaternion* r);
void setAngleAndAxisQ(TQuaternion* q, float a, const TVector3f* x);
BOOL nullQ(const TQuaternion* q);
void mulQ(const TQuaternion* l, const TQuaternion* r, TQuaternion* o);
void conjugateQ(const TQuaternion* q, TQuaternion* o);
void rotateQ(const TQuaternion* q, const TVector3f* v, TVector3f* o);
float radiansQ(const TQuaternion* q);
float degreesQ(const TQuaternion* q);

void pointOnQuadraticBezierCurve(const TQuadraticBezierCurve* c, float t, TVector3f* r);
void pointOnCubicBezierCurve(const TCubicBezierCurve* c, float t, TVector3f* r);

float intersectSphereWithRay(const TVector3f* c, float ra, const TRay* r);
float distanceOfPointAndRay(const TVector3f* c, const TRay* r);
float closestPointOnRay(const TVector3f* c, const TRay* r);
float distanceOfSegmentAndRay(const TVector3f* ss, const TVector3f* se, const TRay* r, float* rd);
float distanceOfSegmentAndRaySquared(const TVector3f* ss, const TVector3f* se, const TRay* r, float* rd);
void rayPointAtDistance(const TRay* r, float d, TVector3f* p);

void setMatrix2fAsSubMatrix(const TMatrix4f* m4f, int i, TMatrix2f* m2f);
void setIdentityM2f(TMatrix2f* m);
void setMinorM2f(const TMatrix3f* m2f, int col, int row, TMatrix2f* o);
void setColumnM2f(const TMatrix2f* m, const TVector2f* v, int col, TMatrix2f* o);
void setValueM2f(const TMatrix2f* m, float v, int col, int row, TMatrix2f* o);
BOOL invertM2f(const TMatrix2f* m, TMatrix2f* o);
void adjugateM2f(const TMatrix2f* m, TMatrix2f* o);
float determinantM2f(const TMatrix2f* m);
void negateM2f(const TMatrix2f* m, TMatrix2f* o);
void transposeM2f(const TMatrix2f* m, TMatrix2f* o);
void addM2f(const TMatrix2f* l, const TMatrix2f* r, TMatrix2f* o);
void subM2f(const TMatrix2f* l, const TMatrix2f* r, TMatrix2f* o);
void mulM2f(const TMatrix2f* l, const TMatrix2f* r, TMatrix2f* o);
void scaleM2f(const TMatrix2f* m, float s, TMatrix2f* o);

void setIdentityM3f(TMatrix3f* m);
void setMinorM3f(const TMatrix4f* m4f, int col, int row, TMatrix3f* o);
void setColumnM3f(const TMatrix3f* m, const TVector3f* v, int col, TMatrix3f* o);
void setValueM3f(const TMatrix3f* m, float v, int col, int row, TMatrix3f* o);
BOOL invertM3f(const TMatrix3f* m, TMatrix3f* o);
void adjugateM3f(const TMatrix3f* m, TMatrix3f* o);
float determinantM3f(const TMatrix3f* m);
void negateM3f(const TMatrix3f* m, TMatrix3f* o);
void transposeM3f(const TMatrix3f* m, TMatrix3f* o);
void addM3f(const TMatrix3f* l, const TMatrix3f* r, TMatrix3f* o);
void subM3f(const TMatrix3f* l, const TMatrix3f* r, TMatrix3f* o);
void mulM3f(const TMatrix3f* l, const TMatrix3f* r, TMatrix3f* o);
void scaleM3f(const TMatrix3f* m, float s, TMatrix3f* o);

void setIdentityM4f(TMatrix4f* m);
void embedM4f(const TMatrix3f* m3f, TMatrix4f* m4f);
void setSubMatrixM4f(const TMatrix4f* m4f, const TMatrix2f* m2f, int i, TMatrix4f* o);
void setColumnM4fV4f(const TMatrix4f* m, const TVector4f* v, int col, TMatrix4f* o);
void setColumnM4fV3f(const TMatrix4f* m, const TVector3f* v, int col, TMatrix4f* o);
void setValueM4f(const TMatrix4f* m, float v, int col, int row, TMatrix4f* o);
BOOL invertM4f(const TMatrix4f* m, TMatrix4f* o);
void adjugateM4f(const TMatrix4f* m, TMatrix4f* o);
float determinantM4f(const TMatrix4f* m);
void negateM4f(const TMatrix4f* m, TMatrix4f* o);
void transposeM4f(const TMatrix4f* m, TMatrix4f* o);
void addM4f(const TMatrix4f* l, const TMatrix4f* r, TMatrix4f* o);
void subM4f(const TMatrix4f* l, const TMatrix4f* r, TMatrix4f* o);
void mulM4f(const TMatrix4f* l, const TMatrix4f* r, TMatrix4f* o);
void scaleM4f(const TMatrix4f* m, float s, TMatrix4f* o);

void rotateM4f(const TMatrix4f* m, const TVector3f* x, float a, TMatrix4f* o);
void rotateM4fQ(const TMatrix4f* m, const TQuaternion* q, TMatrix4f* o);
void translateM4f(const TMatrix4f* m, const TVector3f* d, TMatrix4f* o);
void scaleM4fV3f(const TMatrix4f* m, const TVector3f* s, TMatrix4f* o);
void transformM4fV3f(const TMatrix4f* m, const TVector3f* v, TVector3f* o);
void transformM4fV4f(const TMatrix4f* m, const TVector4f* v, TVector4f* o);

void projectOntoCoordinatePlane(EPlane plane, const TVector3f* v, TVector3f* o);
BOOL projectVectorOntoPlane(const TVector3f* planeNorm, const TVector3f* dir, const TVector3f* v, TVector3f* o);

void makeCircle(float radius, int segments, TVector3f* points);
void makeRing(float innerRadius, float outerRadius, int segments, TVector3f* points);
void makeTorus(float innerRadius, float outerRadius, int innerSegments, int outerSegments, TVector3f* points, TVector3f* normals);
void makeTorusPart(float innerRadius, float outerRadius, int innerSegments, int outerSegments, float centerAngle, float angleLength, TVector3f* points, TVector3f* normals);
void makeCone(float radius, float height, int segments, TVector3f* points, TVector3f* normals);
void makeCylinder(float radius, float height, int segments, TVector3f* points, TVector3f* normals);


