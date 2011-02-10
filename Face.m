//
//  Face.m
//  TrenchBroom
//
//  Created by Kristian Duske on 30.01.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "Face.h"
#import "IdGenerator.h"
#import "Vector3i.h"
#import "HalfSpace3D.h"
#import "Vector3f.h"
#import "Vector2f.h"
#import "Quaternion.h"
#import "math.h"
#import "Math.h"
#import "Brush.h"
#import "Matrix4f.h"
#import "Matrix3f.h"
#import "Plane3D.h"
#import "Line3D.h"

NSString* const FaceGeometryChanged = @"FaceGeometryChanged";
NSString* const FaceFlagsChanged = @"FaceFlagsChanged";

NSString* const TextureOldKey = @"TextureOld";
NSString* const TextureNewKey = @"TextureNew";

static Vector3f* baseAxes[18];

@implementation Face

+ (void)initialize {
    baseAxes[ 0] = [Vector3f xAxisPos]; baseAxes[ 1] = [Vector3f yAxisPos]; baseAxes[ 2] = [Vector3f zAxisNeg];
    baseAxes[ 3] = [Vector3f xAxisNeg]; baseAxes[ 4] = [Vector3f yAxisPos]; baseAxes[ 5] = [Vector3f zAxisNeg];
    baseAxes[ 6] = [Vector3f yAxisPos]; baseAxes[ 7] = [Vector3f xAxisPos]; baseAxes[ 8] = [Vector3f zAxisNeg];
    baseAxes[ 9] = [Vector3f yAxisNeg]; baseAxes[10] = [Vector3f xAxisPos]; baseAxes[11] = [Vector3f zAxisNeg];
    baseAxes[12] = [Vector3f zAxisPos]; baseAxes[13] = [Vector3f xAxisPos]; baseAxes[14] = [Vector3f yAxisNeg];
    baseAxes[15] = [Vector3f zAxisNeg]; baseAxes[16] = [Vector3f xAxisPos]; baseAxes[17] = [Vector3f yAxisNeg];
}

- (id)init {
    if (self = [super init]) {
        faceId = [[[IdGenerator sharedGenerator] getId] retain];
        point1 = [[Vector3i alloc] init];
        point2 = [[Vector3i alloc] init];
        point3 = [[Vector3i alloc] init];
        texture = [[NSMutableString alloc] init];
    }
    
    return self;
}

- (id)initInBrush:(Brush *)theBrush point1:(Vector3i *)aPoint1 point2:(Vector3i *)aPoint2 point3:(Vector3i *)aPoint3 texture:(NSString *)aTexture {
    if (self = [self init]) {
        brush = theBrush; // do not retain
        [self setPoint1:aPoint1 point2:aPoint2 point3:aPoint3];
        [self setTexture:aTexture];
        [self setXScale:1];
        [self setYScale:1];
    }
    
    return self;
}

- (NSNumber *)faceId {
    return faceId;
}

- (Brush *)brush {
    return brush;
}

- (Vector3i *)point1 {
	return point1;
}

- (Vector3i *)point2 {
	return point2;
}

- (Vector3i *)point3 {
	return point3;
}

- (NSString *)texture {
	return texture;
}

- (int)xOffset {
	return xOffset;
}

- (int)yOffset {
	return yOffset;
}

- (float)rotation {
	return rotation;
}

- (float)xScale {
	return xScale;
}

- (float)yScale {
	return yScale;
}

- (Vector3f *)norm {
    if (norm == nil) {
        Vector3f* p1f = [[Vector3f alloc] initWithIntVector:point1];
        Vector3f* v1 = [[Vector3f alloc] initWithIntVector:point3];
        Vector3f* v2 = [[Vector3f alloc]initWithIntVector:point2];
        
        [v1 sub:p1f];
        [v2 sub:p1f];
        [v1 cross:v2];
        
        [v1 normalize];
        norm = v1;
        [v2 release];
        [p1f release];
    }
    
    return norm;
}

- (void)geometryChanged {
    [norm release];
    norm = nil;
    
    [halfSpace release];
    halfSpace = nil;
    
    [texAxisX release];
    texAxisX = nil;
    [texAxisY release];
    texAxisY = nil;
    [surfaceMatrix release];
    surfaceMatrix = nil;
    [worldMatrix release];
    worldMatrix = nil;
    
    [self notifyObservers:FaceGeometryChanged];
}

- (void)setPoint1:(Vector3i *)thePoint1 point2:(Vector3i *)thePoint2 point3:(Vector3i *)thePoint3{
    if (thePoint1 == nil)
        [NSException raise:NSInvalidArgumentException format:@"point 1 must not be nil"];
    if (thePoint2 == nil)
        [NSException raise:NSInvalidArgumentException format:@"point 2 must not be nil"];
    if (thePoint3 == nil)
        [NSException raise:NSInvalidArgumentException format:@"point 3 must not be nil"];

    if ([point1 isEqualToVector:thePoint1] &&
        [point2 isEqualToVector:thePoint2] &&
        [point3 isEqualToVector:thePoint3])
        return;

    Vector3i* oldPoint1 = [Vector3i vectorWithVector:point1];
    Vector3i* oldPoint2 = [Vector3i vectorWithVector:point2];
    Vector3i* oldPoint3 = [Vector3i vectorWithVector:point3];
    
    NSUndoManager* undoManager = [self undoManager];
    [[undoManager prepareWithInvocationTarget:self] setPoint1:oldPoint1 point2:oldPoint2 point3:oldPoint3];
    [undoManager setActionName:@"Change Face"];
    
    [point1 set:thePoint1];
    [point2 set:thePoint2];
    [point3 set:thePoint3];

    [self geometryChanged];
}

- (void)translateBy:(Vector3i *)theDelta {
    if (theDelta == nil)
        [NSException raise:NSInvalidArgumentException format:@"direction must not be nil"];

    Vector3i* inverse = [Vector3i vectorWithVector:theDelta];
    [inverse scale:-1];
    
    NSUndoManager* undoManager = [self undoManager];
    [[undoManager prepareWithInvocationTarget:self] translateBy:inverse];
    [undoManager setActionName:@"Move Face"];
    
    [point1 add:theDelta];
    [point2 add:theDelta];
    [point3 add:theDelta];
    
    [self geometryChanged];
}

- (void)setTexture:(NSString *)name {
    if (name == nil)
        [NSException raise:NSInvalidArgumentException format:@"texture name must not be nil"];

    if ([texture isEqualTo:name])
        return;
    
    NSUndoManager* undoManager = [self undoManager];
    [[undoManager prepareWithInvocationTarget:self] setTexture:[NSString stringWithString:texture]];

    NSMutableDictionary* userInfo = [NSMutableDictionary dictionary];
    [userInfo setObject:[NSString stringWithString:texture] forKey:TextureOldKey];
    [userInfo setObject:name forKey:TextureNewKey];
    
    [texture setString:name];
    [self notifyObservers:FaceFlagsChanged userInfo:userInfo];
}

- (void)setXOffset:(int)offset {
    if (xOffset == offset)
        return;
    
    NSUndoManager* undoManager = [self undoManager];
    [[undoManager prepareWithInvocationTarget:self] setXOffset:xOffset];
    
	xOffset = offset;

    [texAxisX release];
    texAxisX = nil;
    [texAxisY release];
    texAxisY = nil;

    [self notifyObservers:FaceFlagsChanged];
}

- (void)setYOffset:(int)offset {
    if (yOffset == offset)
        return;
    
    NSUndoManager* undoManager = [self undoManager];
    [[undoManager prepareWithInvocationTarget:self] setYOffset:yOffset];
    
	yOffset = offset;
    
    [texAxisX release];
    texAxisX = nil;
    [texAxisY release];
    texAxisY = nil;
    
    [self notifyObservers:FaceFlagsChanged];
}

- (void)setRotation:(float)angle {
    if (rotation == angle)
        return;
    
    NSUndoManager* undoManager = [self undoManager];
    [[undoManager prepareWithInvocationTarget:self] setRotation:rotation];
    
	rotation = angle;
    
    [texAxisX release];
    texAxisX = nil;
    [texAxisY release];
    texAxisY = nil;
    
    [self notifyObservers:FaceFlagsChanged];
}

- (void)setXScale:(float)factor {
    if (xScale == factor)
        return;
    
    NSUndoManager* undoManager = [self undoManager];
    [[undoManager prepareWithInvocationTarget:self] setXScale:xScale];
    
	xScale = factor;
    
    [texAxisX release];
    texAxisX = nil;
    [texAxisY release];
    texAxisY = nil;
    
    [self notifyObservers:FaceFlagsChanged];
}

- (void)setYScale:(float)factor {
    if (yScale == factor)
        return;
    
    NSUndoManager* undoManager = [self undoManager];
    [[undoManager prepareWithInvocationTarget:self] setYScale:yScale];
    
	yScale = factor;
    
    [texAxisX release];
    texAxisX = nil;
    [texAxisY release];
    texAxisY = nil;
    
    [self notifyObservers:FaceFlagsChanged];
}

- (HalfSpace3D *)halfSpace {
    if (halfSpace == nil) {
        halfSpace =  [[HalfSpace3D alloc] initWithIntPoint1:[self point1] 
                                                     point2:[self point2] 
                                                     point3:[self point3]];
    }
    
    return halfSpace;
}

- (void)texCoords:(Vector2f *)texCoords forVertex:(Vector3f *)vertex {
    if (texCoords == nil)
        [NSException raise:NSInvalidArgumentException format:@"texture coordinate vector must not be nil"];
    if (vertex == nil)
        [NSException raise:NSInvalidArgumentException format:@"vertex must not be nil"];
    
    if (texAxisX == nil || texAxisY == nil) {
        // determine texture axes, this is from QBSP
        float best = 0;
        int bestAxis = 0;
        for (int i = 0; i < 6; i++) {
            float dot = [[self norm] dot:baseAxes[i * 3]];
            if (dot > best) {
                best = dot;
                bestAxis = i;
            }
        }
        
        texAxisX = [[Vector3f alloc] initWithFloatVector:baseAxes[bestAxis * 3 + 1]];
        texAxisY = [[Vector3f alloc] initWithFloatVector:baseAxes[bestAxis * 3 + 2]];
        
        float ang = rotation / 180 * M_PI;
        float sinv = sin(ang);
        float cosv = cos(ang);
        
        int sv, tv;
        if ([texAxisX x] != 0)
            sv = 0;
        else if ([texAxisX y] != 0)
            sv = 1;
        else
            sv = 2;
        
        if ([texAxisY x] != 0)
            tv = 0;
        else if ([texAxisY y] != 0)
            tv = 1;
        else
            tv = 2;
        
        Vector3f* texAxes[2] = {texAxisX, texAxisY};
        for (int i = 0; i < 2; i++) {
            float ns = cosv * [texAxes[i] component:sv] - sinv * [texAxes[i] component:tv];
            float nt = sinv * [texAxes[i] component:sv] + cosv * [texAxes[i] component:tv];
            
            [texAxes[i] setComponent:sv value:ns];
            [texAxes[i] setComponent:tv value:nt];
        }
        
        [texAxisX scale:1 / xScale];
        [texAxisY scale:1 / yScale];
    }
    
    [texCoords setX:[vertex dot:texAxisX] + xOffset];
    [texCoords setY:[vertex dot:texAxisY] + yOffset];
}

- (void)updateMatrices {
    Vector3f* xAxis = [[Vector3f alloc] init];
    Vector3f* yAxis = [[Vector3f alloc] init];
    Vector3f* zAxis = [[Vector3f alloc] initWithFloatVector:[self norm]];
    
    float ny = [[self norm] y];
    if (fneg(ny)) {
        // surface X axis is normalized projection of positive world X axis along Y axis onto plane
        Vector3f* point = [[Vector3f alloc] initWithFloatVector:[self center]];
        [point setX:[point x] + 1];
        Line3D* line = [[Line3D alloc] initWithPoint:point normalizedDirection:[Vector3f yAxisPos]];
        Plane3D* plane = [[self halfSpace] boundary];
        [xAxis setFloat:[plane intersectWithLine:line]];
        [xAxis sub:[self center]];
        [xAxis normalize];
        [line release];
        [point release];
    } else if (fpos(ny)) {
        // surface X axis is normalized projection of negative world X axis along Y axis onto plane
        Vector3f* point = [[Vector3f alloc] initWithFloatVector:[self center]];
        [point setX:[point x] - 1];
        Line3D* line = [[Line3D alloc] initWithPoint:point normalizedDirection:[Vector3f yAxisPos]];
        Plane3D* plane = [[self halfSpace] boundary];
        [xAxis setFloat:[plane intersectWithLine:line]];
        [xAxis sub:[self center]];
        [xAxis normalize];
        [line release];
        [point release];
    } else {
        // plane normal is on XZ plane, try X, then Z
        float nx = [[self norm] x];
        if (fpos(nx)) {
            // positive world Y axis is surface X axis
            [xAxis setFloat:[Vector3f yAxisPos]];
        } else if (fneg(nx)) {
            // negative world Y axis is surface X axis
            [xAxis setFloat:[Vector3f yAxisNeg]];
        } else {
            // surface normal is Z = 1 or Z = -1
            float nz = [[self norm] z];
            if (nz > 0) {
                // positive world X axis is surface X axis
                [xAxis setFloat:[Vector3f xAxisPos]];
            } else {
                // negative world X axis is surface X axis
                [xAxis setFloat:[Vector3f xAxisNeg]];
            }
        }
    }
    
    [yAxis setFloat:zAxis];
    [yAxis cross:xAxis];
    [yAxis normalize];
    
    // build transformation matrix
    surfaceMatrix = [[Matrix4f alloc] init];
    [surfaceMatrix setColumn:0 values:xAxis];
    [surfaceMatrix setColumn:1 values:yAxis];
    [surfaceMatrix setColumn:2 values:zAxis];
    [surfaceMatrix setColumn:3 values:[self center]];
    [surfaceMatrix setColumn:3 row:3 value:1];

    worldMatrix = [[Matrix4f alloc] initWithMatrix4f:surfaceMatrix];
    if (![worldMatrix invert])
        [NSException raise:@"NonInvertibleMatrixException" format:@"surface transformation matrix is not invertible"];

    Matrix4f* m = [[Matrix4f alloc] initWithMatrix4f:surfaceMatrix];
    [m mul:worldMatrix];
    
    [xAxis release];
    [yAxis release];
    [zAxis release];
}

- (Vector3f *)worldCoordsOf:(Vector3f *)sCoords {
    if (sCoords == nil)
        [NSException raise:NSInvalidArgumentException format:@"surface coordinates must not be nil"];
    
    if (surfaceMatrix == nil)
        [self updateMatrices];

    Vector3f* result = [[Vector3f alloc] initWithFloatVector:sCoords];
    [surfaceMatrix transformVector3f:result];

    return [result autorelease];
}

- (Vector3f *)surfaceCoordsOf:(Vector3f *)wCoords {
    if (wCoords == nil)
        [NSException raise:NSInvalidArgumentException format:@"world coordinates must not be nil"];
    
    if (worldMatrix == nil)
        [self updateMatrices];
    
    Vector3f* result = [[Vector3f alloc] initWithFloatVector:wCoords];
    [worldMatrix transformVector3f:result];
    
    return [result autorelease];
}

- (NSString *)description {
    return [NSString stringWithFormat:@"ID: %i, point 1: %@, point 2: %@, point 3: %@, texture: %@, X offset: %i, Y offset: %i, rotation: %f, X scale: %f, Y scale: %f", 
            [faceId intValue], 
            point1, 
            point2, 
            point3, 
            texture, 
            xOffset, 
            yOffset, 
            rotation, 
            xScale, 
            yScale];
}

- (Vector3f *)center {
    return [brush centerOfFace:self];
}

- (NSArray *)vertices {
    return [brush verticesForFace:self];
}

- (NSUndoManager *)undoManager {
    return [brush undoManager];
}

- (BOOL)postNotifications {
    return [brush postNotifications];
}

- (void) dealloc {
    [faceId release];
    [halfSpace release];
	[point1 release];
	[point2 release];
	[point3 release];
	[texture release];
    [norm release];
    [texAxisX release];
    [texAxisY release];
    [surfaceMatrix release];
    [worldMatrix release];
	[super dealloc];
}

@end
