//
//  Face.m
//  TrenchBroom
//
//  Created by Kristian Duske on 30.01.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "MutableFace.h"
#import "MutableBrush.h"
#import "IdGenerator.h"
#import "math.h"
#import "Math.h"
#import "Brush.h"
#import "Entity.h"
#import "Map.h"
#import "PickingHit.h"
#import "TextureManager.h"
#import "Texture.h"
#import "VBOMemBlock.h"


// ZProj projects a vector onto the XY plane (along the Z axis)
static const TMatrix4f ZProj = { 1,  0,  0,  0,
                                 0,  1,  0,  0,
                                 0,  0,  0,  0,
                                 0,  0,  0,  1 };
static const TMatrix4f ZProjInv = { 1,  0,  0,  0,
                                    0, -1,  0,  0,
                                    0,  0,  1,  0,
                                    0,  0,  0,  1 };

// YProj projects a vector onto the XZ plane (along the Y axis)
static const TMatrix4f YProj = { 1,  0,  0,  0,
                                 0,  0,  0,  0,
                                 0,  1,  0,  0,
                                 0,  0,  0,  1 };
static const TMatrix4f YProjInv = { 1,  0,  0,  0,
                                    0,  0, -1,  0,
                                    0,  1,  0,  0,
                                    0,  0,  0,  1 };

// XProj projects a vector onto the YZ plane (along the X axis)
static const TMatrix4f XProj = { 0,  0,  0,  0,
                                 1,  0,  0,  0,
                                 0,  1,  0,  0,
                                 0,  0,  0,  1 };
static const TMatrix4f XProjInv = { 0,  1,  0,  0,
                                    0,  0, -1,  0,
                                    1,  0,  0,  0,
                                    0,  0,  0,  1 };
static const TVector3f* BaseAxes[18] = { &ZAxisPos, &XAxisPos, &YAxisNeg,
                                         &ZAxisNeg, &XAxisPos, &YAxisNeg,
                                         &XAxisPos, &YAxisPos, &ZAxisNeg,
                                         &XAxisNeg, &YAxisPos, &ZAxisNeg,
                                         &YAxisPos, &XAxisPos, &ZAxisNeg,
                                         &YAxisNeg, &XAxisPos, &ZAxisNeg};

@interface MutableFace (private)

- (void)validateTexAxes;
- (const TVector3f *)texPlaneNormAndXAxis:(TVector3f *)theXAxis yAxis:(TVector3f *)theYAxis forFaceNorm:(const TVector3f *)theNorm;
- (void)updateMatrices;
- (void)geometryChanged;
- (void)validate;

- (void)updateTextureParametersForTransformation:(const TMatrix4f *)transformation;

@end

@implementation MutableFace (private)

- (const TVector3f *)texPlaneNormAndXAxis:(TVector3f *)theXAxis yAxis:(TVector3f *)theYAxis forFaceNorm:(const TVector3f *)theNorm {

    int bestIndex = 0;
    float bestDot = -1;
    for (int i = 0; i < 6; i++) {
        float dot = dotV3f(theNorm, BaseAxes[i * 3]);
        if (dot >= bestDot) {
            bestDot = dot;
            bestIndex = i;
        }
    }
    
    *theXAxis = *BaseAxes[bestIndex * 3 + 1];
    *theYAxis = *BaseAxes[bestIndex * 3 + 2];
    return BaseAxes[bestIndex * 3];
}

- (void)validateTexAxes {
    texPlaneNorm = [self texPlaneNormAndXAxis:&texAxisX yAxis:&texAxisY forFaceNorm:[self norm]];
    
    TQuaternion rot;
    float ang = rotation / 180 * M_PI;
    setAngleAndAxisQ(&rot, ang, texPlaneNorm);
    rotateQ(&rot, &texAxisX, &texAxisX);
    rotateQ(&rot, &texAxisY, &texAxisY);
    
    scaleV3f(&texAxisX, 1 / xScale, &scaledTexAxisX);
    scaleV3f(&texAxisY, 1 / yScale, &scaledTexAxisY);
    
    texAxesValid = YES;
}

- (void)updateMatrices {
    TVector3f xAxis, yAxis, zAxis;
    TLine line;
    
    zAxis = *[self norm];
    float ny = zAxis.y;
    
    if (ny < 0) {
        // surface X axis is normalized projection of positive world X axis along Y axis onto plane
        line.point = *[self center];
        line.point.x += 1;
        line.direction = YAxisPos;
        
        float dist = intersectPlaneWithLine([self boundary], &line);
        linePointAtDistance(&line, dist, &xAxis);
        subV3f(&xAxis, [self center], &xAxis);
        normalizeV3f(&xAxis, &xAxis);
    } else if (ny > 0) {
        // surface X axis is normalized projection of negative world X axis along Y axis onto plane
        line.point = *[self center];
        line.point.x -= 1;
        line.direction = YAxisPos;
        
        float dist = intersectPlaneWithLine([self boundary], &line);
        linePointAtDistance(&line, dist, &xAxis);
        subV3f(&xAxis, [self center], &xAxis);
        normalizeV3f(&xAxis, &xAxis);
    } else {
        // plane normal is on XZ plane, try X, then Z
        float nx = [self norm]->x;
        if (nx > 0) {
            // positive world Y axis is surface X axis
            xAxis = YAxisPos;
        } else if (nx < 0) {
            // negative world Y axis is surface X axis
            xAxis = YAxisNeg;
        } else {
            // surface normal is Z = 1 or Z = -1
            float nz = [self norm]->z;
            if (nz > 0) {
                // positive world X axis is surface X axis
                xAxis = XAxisPos;
            } else {
                // negative world X axis is surface X axis
                xAxis = XAxisNeg;
            }
        }
    }
    
    crossV3f(&zAxis, &xAxis, &yAxis);
    normalizeV3f(&yAxis, &yAxis);
    
    // build transformation matrix
    setColumnM4fV3f(&surfaceToWorldMatrix, &xAxis, 0, &surfaceToWorldMatrix);
    setColumnM4fV3f(&surfaceToWorldMatrix, &yAxis, 1, &surfaceToWorldMatrix);
    setColumnM4fV3f(&surfaceToWorldMatrix, &zAxis, 2, &surfaceToWorldMatrix);
    setColumnM4fV3f(&surfaceToWorldMatrix, [self center], 3, &surfaceToWorldMatrix);
    setValueM4f(&surfaceToWorldMatrix, 1, 3, 3, &surfaceToWorldMatrix);
    
    if (!invertM4f(&surfaceToWorldMatrix, &worldToSurfaceMatrix))
        [NSException raise:@"NonInvertibleMatrixException" format:@"surface transformation matrix is not invertible"];
    
    matricesValid = YES;
}

- (void)geometryChanged {
    matricesValid = NO;
    boundaryValid = NO;
    centerValid = NO;
    texAxesValid = NO;
}

- (void)validate {
    if (!centerValid && side != NULL) {
        center = side->vertices[0]->vector;
        for (int i = 1; i < side->edgeCount; i++) {
            addV3f(&center, &side->vertices[i]->vector, &center);
        }
        
        scaleV3f(&center, 1.0f / side->edgeCount, &center);
        centerValid = YES;
    }
    
    if (!boundaryValid) {
        setPlanePointsV3i(&boundary, &point1, &point2, &point3);
        boundaryValid = YES;
    }
}

- (void)updateTextureParametersForTransformation:(const TMatrix4f *)transformation {
    // Any transformation of this face moves the points on its surface around. The point of 
    // texture lock is that every point on the surface of this face is gets the same texture pixel
    // projected onto it after the transformation. The texture scaling, rotation and offset can be
    // represented with a 3x3 transformation matrix. We can calculate the new transformation matrix
    // that keeps all points invariant from the 2D coordinates of three points before and after
    // the transformation has taken place. From the matrices components, we can then calculate
    // the new scaling, rotation and offset parameters.
    

    if (!texAxesValid)
        [self validateTexAxes];
    
    TVector3f curTexAxisX, curTexAxisY, newTexAxisX, newTexAxisY, newFaceNorm, newCenter, newBaseAxisX, newBaseAxisY, offset, temp;
    TVector2f curCenterTexCoords, newCenterTexCoords;
    const TVector3f* curCenter;
    const TVector3f* newTexPlaneNorm;
    
    if (equalV3f(texPlaneNorm, &ZAxisPos))
        NSLog(@"asdf");
    
    curCenter = [self center];
    
    curCenterTexCoords.x = dotV3f(curCenter, &scaledTexAxisX) + xOffset;
    curCenterTexCoords.y = dotV3f(curCenter, &scaledTexAxisY) + yOffset;
    
    curTexAxisX = texAxisX;
    curTexAxisY = texAxisY;
    
    scaleV3f(&curTexAxisX, xScale, &curTexAxisX);
    scaleV3f(&curTexAxisY, yScale, &curTexAxisY);
    
    transformM4fV3f(transformation, &curTexAxisX, &newTexAxisX);
    transformM4fV3f(transformation, &curTexAxisY, &newTexAxisY);
    transformM4fV3f(transformation, [self norm], &newFaceNorm);
    transformM4fV3f(transformation, &NullVector, &offset);
    transformM4fV3f(transformation, curCenter, &newCenter);
    
    subV3f(&newTexAxisX, &offset, &newTexAxisX);
    subV3f(&newTexAxisY, &offset, &newTexAxisY);
    subV3f(&newFaceNorm, &offset, &newFaceNorm);
    
    newTexPlaneNorm = [self texPlaneNormAndXAxis:&newBaseAxisX yAxis:&newBaseAxisY forFaceNorm:&newFaceNorm];

    float tpnDot = dotV3f(texPlaneNorm, newTexPlaneNorm);
    if (tpnDot == 1 || tpnDot == -1) {
        TVector3f transformedTexPlaneNorm;
        transformM4fV3f(transformation, texPlaneNorm, &transformedTexPlaneNorm);
        subV3f(&transformedTexPlaneNorm, &offset, &transformedTexPlaneNorm);
        
        if (dotV3f(texPlaneNorm, &transformedTexPlaneNorm) == 0) {
            crossV3f(texPlaneNorm, &transformedTexPlaneNorm, &temp);
            const TVector3f* rotAxis = closestAxisV3f(&temp);
            
            float angle = M_PI_2;
            if (tpnDot == 1)
                angle *= -1;
            
            TQuaternion rot;
            setAngleAndAxisQ(&rot, angle, rotAxis);
            
            rotateQ(&rot, &newTexAxisX, &newTexAxisX);
            rotateQ(&rot, &newTexAxisY, &newTexAxisY);
        }
    }
    
    if (newTexPlaneNorm == &XAxisPos || newTexPlaneNorm == &XAxisNeg) {
        newTexAxisX.x = 0;
        newTexAxisY.x = 0;
    } else if (newTexPlaneNorm == &YAxisPos || newTexPlaneNorm == &YAxisNeg) {
        newTexAxisX.y = 0;
        newTexAxisY.y = 0;
    } else {
        newTexAxisX.z = 0;
        newTexAxisY.z = 0;
    }
    
    xScale = lengthV3f(&newTexAxisX);
    yScale = lengthV3f(&newTexAxisY);
    
    if (dotV3f(&newTexAxisX, &newBaseAxisX) < 0)
        xScale *= -1;
    if (dotV3f(&newTexAxisY, &newBaseAxisY) < 0)
        yScale *= -1;

    scaleV3f(&newTexAxisX, 1 / xScale, &newTexAxisX); // normalized
    scaleV3f(&newTexAxisY, 1 / yScale, &newTexAxisY);
    
    float cos = dotV3f(&newBaseAxisX, &newTexAxisX);
    float rad = acosf(cos);
    
    crossV3f(&newBaseAxisX, &newTexAxisX, &temp);
    if (dotV3f(&temp, &newFaceNorm) < 0)
        rad *= -1;
    
    rotation = rad * 180 / M_PI;

    [self validateTexAxes];
    
    scaleV3f(&newTexAxisX, 1 / xScale, &newTexAxisX);
    scaleV3f(&newTexAxisY, 1 / yScale, &newTexAxisY);

    newCenterTexCoords.x = dotV3f(&newCenter, &newTexAxisX);
    newCenterTexCoords.y = dotV3f(&newCenter, &newTexAxisY);
    
    xOffset = curCenterTexCoords.x - newCenterTexCoords.x;
    yOffset = curCenterTexCoords.y - newCenterTexCoords.y;}

@end

#pragma mark -
@implementation MutableFace

- (id)init {
    if ((self = [super init])) {
        faceId = [[[IdGenerator sharedGenerator] getId] retain];
        texture = [[NSMutableString alloc] init];
        filePosition = -1;
    }
    
    return self;
}

- (id)initWithPoint1:(const TVector3i *)aPoint1 point2:(const TVector3i *)aPoint2 point3:(const TVector3i *)aPoint3 texture:(NSString *)aTexture {
    if ((self = [self init])) {
        [self setPoint1:aPoint1 point2:aPoint2 point3:aPoint3];
        [self setTexture:aTexture];
        [self setXScale:1];
        [self setYScale:1];
    }
    
    return self;
}

- (id)initWithFaceTemplate:(id <Face>)theTemplate {
    if ((self = [self init])) {
        [self setPoint1:[theTemplate point1] point2:[theTemplate point2] point3:[theTemplate point3]];
        [self setTexture:[theTemplate texture]];
        [self setXOffset:[theTemplate xOffset]];
        [self setYOffset:[theTemplate yOffset]];
        [self setRotation:[theTemplate rotation]];
        [self setXScale:[theTemplate xScale]];
        [self setYScale:[theTemplate yScale]];
    }
    
    return self;
}

- (id)copyWithZone:(NSZone *)zone {
    MutableFace* result = [[MutableFace allocWithZone:zone] init];
    
    [result->faceId release];
    result->faceId = [faceId retain];
    [result setPoint1:&point1 point2:&point2 point3:&point3];
    [result setTexture:texture];
    [result setXOffset:xOffset];
    [result setYOffset:yOffset];
    [result setXScale:xScale];
    [result setYScale:yScale];
    [result setRotation:rotation];
    [result setFilePosition:filePosition];
    [result setBrush:brush];
    
    return result;
}

- (void) dealloc {
    [faceId release];
	[texture release];
    [memBlock free];
    if (side != NULL)
        side->face = nil;
	[super dealloc];
}

- (void)setBrush:(MutableBrush *)theBrush {
    brush = theBrush;
}

- (void)setPoint1:(const TVector3i *)thePoint1 point2:(const TVector3i *)thePoint2 point3:(const TVector3i *)thePoint3{
    if (equalV3i(&point1, thePoint1) &&
        equalV3i(&point2, thePoint2) &&
        equalV3i(&point3, thePoint3))
        return;
    
    point1 = *thePoint1;
    point2 = *thePoint2;
    point3 = *thePoint3;
    
    [self geometryChanged];
}


- (void)setTexture:(NSString *)name {
    NSAssert(name != nil, @"texture name must not be nil");
    NSString* trimmed = [name stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceAndNewlineCharacterSet]];
    NSAssert([trimmed length] > 0, @"texture name must not be empty");
    
    [texture setString:name];
}

- (void)setXOffset:(int)offset {
    if (xOffset == offset)
        return;
    
	xOffset = offset;
}

- (void)setYOffset:(int)offset {
    if (yOffset == offset)
        return;
    
	yOffset = offset;
}

- (void)setRotation:(float)angle {
    if (rotation == angle)
        return;
    
	rotation = angle;
    texAxesValid = NO;
}

- (void)setXScale:(float)factor {
    if (xScale == factor)
        return;
    
	xScale = factor;
    texAxesValid = NO;
}

- (void)setYScale:(float)factor {
    if (yScale == factor)
        return;
    
	yScale = factor;
    texAxesValid = NO;
}

- (void)translateOffsetsX:(int)x y:(int)y {
    if (x == 0 && y == 0)
        return;
    
    if (!texAxesValid)
        [self validateTexAxes];
    
    if (texPlaneNorm == &XAxisPos) {
        xOffset += x;
        yOffset += y;
    } else if (texPlaneNorm == &XAxisNeg) {
        xOffset -= x;
        yOffset += y;
    } else if (texPlaneNorm == &YAxisPos) {
        xOffset -= x;
        yOffset += y;
    } else if (texPlaneNorm == &YAxisNeg) {
        xOffset += x;
        yOffset += y;
    } else if (texPlaneNorm == &ZAxisPos) {
        xOffset -= x;
        yOffset += y;
    } else if (texPlaneNorm == &ZAxisNeg) {
        xOffset += x;
        yOffset += y;
    }
}

- (void)translateBy:(const TVector3i *)theDelta lockTexture:(BOOL)lockTexture {
    if (lockTexture) {
        TMatrix4f t;
        TVector3f d;

        setV3f(&d, theDelta);
        translateM4f(&IdentityM4f, &d, &t);
        
        [self updateTextureParametersForTransformation:&t];
    }

    addV3i(&point1, theDelta, &point1);
    addV3i(&point2, theDelta, &point2);
    addV3i(&point3, theDelta, &point3);

    [self geometryChanged];
}

- (void)rotateZ90CW:(const TVector3i *)theCenter lockTexture:(BOOL)lockTexture {
    if (lockTexture) {
        TMatrix4f t;
        TVector3f d;

        setV3f(&d, theCenter);
        translateM4f(&IdentityM4f, &d, &t);
        mulM4f(&t, &RotZ90CWM4f, &t);
        scaleV3f(&d, -1, &d);
        translateM4f(&t, &d, &t);
        
        [self updateTextureParametersForTransformation:&t];
    }

    subV3i(&point1, theCenter, &point1);
    rotateZ90CWV3i(&point1, &point1);
    addV3i(&point1, theCenter, &point1);
    
    subV3i(&point2, theCenter, &point2);
    rotateZ90CWV3i(&point2, &point2);
    addV3i(&point2, theCenter, &point2);
    
    subV3i(&point3, theCenter, &point3);
    rotateZ90CWV3i(&point3, &point3);
    addV3i(&point3, theCenter, &point3);
    
    [self geometryChanged];
}

- (void)rotateZ90CCW:(const TVector3i *)theCenter lockTexture:(BOOL)lockTexture {
    if (lockTexture) {
        TMatrix4f t;
        TVector3f d;
        
        setV3f(&d, theCenter);
        translateM4f(&IdentityM4f, &d, &t);
        mulM4f(&t, &RotZ90CCWM4f, &t);
        scaleV3f(&d, -1, &d);
        translateM4f(&t, &d, &t);
        
        [self updateTextureParametersForTransformation:&t];
    }
    
    subV3i(&point1, theCenter, &point1);
    rotateZ90CCWV3i(&point1, &point1);
    addV3i(&point1, theCenter, &point1);
    
    subV3i(&point2, theCenter, &point2);
    rotateZ90CCWV3i(&point2, &point2);
    addV3i(&point2, theCenter, &point2);
    
    subV3i(&point3, theCenter, &point3);
    rotateZ90CCWV3i(&point3, &point3);
    addV3i(&point3, theCenter, &point3);
    
    [self geometryChanged];
}

- (void)rotate:(const TQuaternion *)theRotation center:(const TVector3f *)theCenter lockTexture:(BOOL)lockTexture {
    if (lockTexture) {
        TVector3f d;
        TMatrix4f t;

        d = *theCenter;
        translateM4f(&IdentityM4f, &d, &t);
        rotateM4fQ(&t, theRotation, &t);
        scaleV3f(&d, -1, &d);
        translateM4f(&t, &d, &t);
        
        [self updateTextureParametersForTransformation:&t];
    }

    TPlane plane = *[self boundary];
    subV3f(&plane.point, theCenter, &plane.point);
    rotateQ(theRotation, &plane.point, &plane.point);
    rotateQ(theRotation, &plane.norm, &plane.norm);
    addV3f(&plane.point, theCenter, &plane.point);
    
    TBoundingBox* worldBounds = [[[[self brush] entity] map] worldBounds];
    makePointsForPlane(&plane, worldBounds, &point1, &point2, &point3);
    
    [self geometryChanged];
}

- (void)mirrorAxis:(EAxis)theAxis center:(const TVector3i *)theCenter lockTexture:(BOOL)lockTexture {
    switch (theAxis) {
        case A_X: {
            if (lockTexture) {
                TMatrix4f t1, t2;
                TVector3f d;
                
                d.x = theCenter->x;
                d.y = 0;
                d.z = 0;
                translateM4f(&IdentityM4f, &d, &t1);
                mulM4f(&t1, &MirXM4f, &t1);
                scaleV3f(&d, -1, &d);
                translateM4f(&IdentityM4f, &d, &t2);
                mulM4f(&t1, &t2, &t1);
                
                [self updateTextureParametersForTransformation:&t1];
            }
            
            point1.x -= theCenter->x;
            point1.x *= -1;
            point1.x += theCenter->x;
            
            point2.x -= theCenter->x;
            point2.x *= -1;
            point2.x += theCenter->x;
            
            point3.x -= theCenter->x;
            point3.x *= -1;
            point3.x += theCenter->x;
            break;
        }
        case A_Y: {
            if (lockTexture) {
                TMatrix4f t1, t2;
                TVector3f d;
                
                d.x = 0;
                d.y = theCenter->y;
                d.z = 0;
                translateM4f(&IdentityM4f, &d, &t1);
                mulM4f(&t1, &MirYM4f, &t1);
                scaleV3f(&d, -1, &d);
                translateM4f(&IdentityM4f, &d, &t2);
                mulM4f(&t1, &t2, &t1);
                
                [self updateTextureParametersForTransformation:&t1];
            }

            point1.y -= theCenter->y;
            point1.y *= -1;
            point1.y += theCenter->y;
            
            point2.y -= theCenter->y;
            point2.y *= -1;
            point2.y += theCenter->y;
            
            point3.y -= theCenter->y;
            point3.y *= -1;
            point3.y += theCenter->y;
            break;
        }
        default: {
            if (lockTexture) {
                TMatrix4f t1, t2;
                TVector3f d;
                
                d.x = 0;
                d.y = 0;
                d.z = theCenter->z;
                translateM4f(&IdentityM4f, &d, &t1);
                mulM4f(&t1, &MirZM4f, &t1);
                scaleV3f(&d, -1, &d);
                translateM4f(&IdentityM4f, &d, &t2);
                mulM4f(&t1, &t2, &t1);
                
                [self updateTextureParametersForTransformation:&t1];
            }

            point1.z -= theCenter->z;
            point1.z *= -1;
            point1.z += theCenter->z;
            
            point2.z -= theCenter->z;
            point2.z *= -1;
            point2.z += theCenter->z;
            
            point3.z -= theCenter->z;
            point3.z *= -1;
            point3.z += theCenter->z;
            break;
        }
    }
    
    TVector3i t = point1;
    point1 = point3;
    point3 = t;
    
    [self geometryChanged];
}

- (void)dragBy:(float)dist lockTexture:(BOOL)lockTexture {
    TVector3f f;
    scaleV3f([self norm], dist, &f);
    
    TVector3i delta = {roundf(f.x), roundf(f.y), roundf(f.z)};
    [self translateBy:&delta lockTexture:lockTexture];
}

- (void)setSide:(TSide *)theSide {
    side = theSide;
}

- (int)filePosition {
    return filePosition;
}

- (void)setFilePosition:(int)theFilePosition {
    filePosition = theFilePosition;
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

# pragma mark -
# pragma mark @implementation Face

- (NSNumber *)faceId {
    return faceId;
}

- (id <Brush>)brush {
    return brush;
}

- (TVector3i *)point1 {
	return &point1;
}

- (TVector3i *)point2 {
	return &point2;
}

- (TVector3i *)point3 {
	return &point3;
}

- (NSString *)texture {
	return texture;
}

- (int)xOffset {
	return roundf(xOffset);
}

- (int)yOffset {
	return roundf(yOffset);
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

- (const TVector3f *)norm {
    return &[self boundary]->norm;
}

- (const TVector3f *)center {
    [self validate];
    return &center;
}

- (const TPlane *)boundary {
    [self validate];
    return &boundary;
}

- (TVertex **)vertices {
    return side->vertices;
}

- (int)vertexCount {
    return side->edgeCount;
}

- (TEdge **)edges {
    return side->edges;
}

- (int)edgeCount {
    return side->edgeCount;
}

- (void)texCoords:(TVector2f *)texCoords forVertex:(TVector3f *)vertex {
    if (!texAxesValid)
        [self validateTexAxes];
    
    texCoords->x = dotV3f(vertex, &scaledTexAxisX) + xOffset;
    texCoords->y = dotV3f(vertex, &scaledTexAxisY) + yOffset;
}

- (void)gridCoords:(TVector2f *)gridCoords forVertex:(TVector3f *)vertex {
    if (!texAxesValid)
        [self validateTexAxes];
    
    switch (strongestComponentV3f([self norm])) {
        case A_X:
            gridCoords->x = (vertex->y + 0.5f) / 256;
            gridCoords->y = (vertex->z + 0.5f) / 256;
            break;
        case A_Y:
            gridCoords->x = (vertex->x + 0.5f) / 256;
            gridCoords->y = (vertex->z + 0.5f) / 256;
            break;
        default:
            gridCoords->x = (vertex->x + 0.5f) / 256;
            gridCoords->y = (vertex->y + 0.5f) / 256;
            break;
    }
}

- (void)transformSurface:(const TVector3f *)surfacePoint toWorld:(TVector3f *)worldPoint {
    if (!matricesValid)
        [self updateMatrices];
    
    transformM4fV3f(&surfaceToWorldMatrix, surfacePoint, worldPoint);
}

- (void)transformWorld:(const TVector3f *)worldPoint toSurface:(TVector3f *)surfacePoint {
    if (!matricesValid)
        [self updateMatrices];
    
    transformM4fV3f(&worldToSurfaceMatrix, worldPoint, surfacePoint);
}

- (const TMatrix4f *)surfaceToWorldMatrix {
    if (!matricesValid)
        [self updateMatrices];
    
    return &surfaceToWorldMatrix;
}

- (const TMatrix4f *)worldToSurfaceMatrix {
    if (!matricesValid)
        [self updateMatrices];
    
    return &worldToSurfaceMatrix;
}

- (VBOMemBlock *)memBlock {
    return memBlock;
}

- (void)setMemBlock:(VBOMemBlock *)theMemBlock {
    [memBlock free];
    memBlock = theMemBlock;
}

@end
