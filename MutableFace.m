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
#import "Matrix4f.h"
#import "Matrix3f.h"
#import "PickingHit.h"
#import "TextureManager.h"
#import "Texture.h"
#import "VBOMemBlock.h"

@interface MutableFace (private)

- (void)validateTexAxes;
- (void)updateMatrices;
- (void)geometryChanged;
- (void)validate;
- (void)rotate:(const TQuaternion *)theRotation center:(const TVector3f *)theCenter;

@end

@implementation MutableFace (private)

- (void)validateTexAxes {
    texNorm = closestAxisV3f([self norm]);
    
    if (texNorm == &XAxisPos || texNorm == &XAxisNeg) {
        texAxisX = YAxisPos;
        texAxisY = ZAxisNeg;
    } else if (texNorm == &YAxisPos || texNorm == &YAxisNeg) {
        texAxisX = XAxisPos;
        texAxisY = ZAxisNeg;
    } else if (texNorm == &ZAxisPos || texNorm == &ZAxisNeg) {
        texAxisX = XAxisPos;
        texAxisY = YAxisNeg;
    }
    
    TQuaternion rot;
    float ang = rotation / 180 * M_PI;
    setAngleAndAxisQ(&rot, ang, texNorm);
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
    surfaceToWorldMatrix = [[Matrix4f alloc] init];
    [surfaceToWorldMatrix setColumn:0 values:&xAxis];
    [surfaceToWorldMatrix setColumn:1 values:&yAxis];
    [surfaceToWorldMatrix setColumn:2 values:&zAxis];
    [surfaceToWorldMatrix setColumn:3 values:[self center]];
    [surfaceToWorldMatrix setColumn:3 row:3 value:1];
    
    worldToSurfaceMatrix = [[Matrix4f alloc] initWithMatrix4f:surfaceToWorldMatrix];
    if (![worldToSurfaceMatrix invert])
        [NSException raise:@"NonInvertibleMatrixException" format:@"surface transformation matrix is not invertible"];
}

- (void)geometryChanged {
    [surfaceToWorldMatrix release];
    surfaceToWorldMatrix = nil;
    [worldToSurfaceMatrix release];
    worldToSurfaceMatrix = nil;
    
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

- (void)rotate:(const TQuaternion *)theRotation center:(const TVector3f *)theCenter {
    TVector3f p1, p2, p3;
    setV3f(&p1, &point1);
    setV3f(&p2, &point2);
    setV3f(&p3, &point3);
    
    subV3f(&p1, theCenter, &p1);
    subV3f(&p2, theCenter, &p2);
    subV3f(&p3, theCenter, &p3);
    
    rotateQ(theRotation, &p1, &p1);
    rotateQ(theRotation, &p2, &p2);
    rotateQ(theRotation, &p3, &p3);
    
    addV3f(&p1, theCenter, &p1);
    addV3f(&p2, theCenter, &p2);
    addV3f(&p3, theCenter, &p3);
    
    roundV3f(&p1, &point1);
    roundV3f(&p2, &point2);
    roundV3f(&p3, &point3);
}

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

- (id)initWithPoint1:(TVector3i *)aPoint1 point2:(TVector3i *)aPoint2 point3:(TVector3i *)aPoint3 texture:(NSString *)aTexture {
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
    [surfaceToWorldMatrix release];
    [worldToSurfaceMatrix release];
    [memBlock free];
    if (side != NULL)
        side->face = nil;
	[super dealloc];
}

- (void)setBrush:(MutableBrush *)theBrush {
    brush = theBrush;
}

- (void)setPoint1:(TVector3i *)thePoint1 point2:(TVector3i *)thePoint2 point3:(TVector3i *)thePoint3{
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

    if (texNorm == &XAxisPos) {
        xOffset += x;
        yOffset += y;
    } else if (texNorm == &XAxisNeg) {
        xOffset -= x;
        yOffset += y;
    } else if (texNorm == &YAxisPos) {
        xOffset -= x;
        yOffset += y;
    } else if (texNorm == &YAxisNeg) {
        xOffset += x;
        yOffset += y;
    } else if (texNorm == &ZAxisPos) {
        xOffset -= x;
        yOffset += y;
    } else if (texNorm == &ZAxisNeg) {
        xOffset += x;
        yOffset += y;
    }
}

- (void)translateBy:(TVector3i *)theDelta lockTexture:(BOOL)lockTexture {
    addV3i(&point1, theDelta, &point1);
    addV3i(&point2, theDelta, &point2);
    addV3i(&point3, theDelta, &point3);
    
    if (lockTexture) {
        if (!texAxesValid)
            [self validateTexAxes];
        
        if (texNorm == &XAxisPos || texNorm == &XAxisNeg) {
            xOffset -= theDelta->y;
            yOffset += theDelta->z;
        } else if (texNorm == &YAxisPos || texNorm == &YAxisNeg) {
            xOffset -= theDelta->x;
            yOffset += theDelta->z;
        } else if (texNorm == &ZAxisPos || texNorm == &ZAxisNeg) {
            xOffset -= theDelta->x;
            yOffset += theDelta->y;
        }
    }

    [self geometryChanged];
}

- (void)rotateZ90CW:(TVector3i *)theCenter lockTexture:(BOOL)lockTexture {
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

- (void)rotateZ90CCW:(TVector3i *)theCenter lockTexture:(BOOL)lockTexture {
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
        TVector3f p, pr;
        
        // take one point on the surface of this face and apply the rotation to it
        // this is needed later to calculate the offsets
        p = [self vertices][0]->vector;
        subV3f(&p, theCenter, &pr);
        rotateQ(theRotation, &pr, &pr);
        addV3f(&pr, theCenter, &pr);
        
        // the texture coordinates which p had before the rotation
        float ox1 = dotV3f(&p, &scaledTexAxisX) + xOffset;
        float oy1 = dotV3f(&p, &scaledTexAxisY) + yOffset;

        if (!texAxesValid)
            [self validateTexAxes];
        
        // rotate the current tex axes in order to determine their new lengths and the rotation angle
        TVector3f tx, ty;
        rotateQ(theRotation, &scaledTexAxisX, &tx);
        rotateQ(theRotation, &scaledTexAxisY, &ty);
        
        // project them back on the appropriate texture plane
        if (texNorm == &XAxisPos || texNorm == &XAxisNeg) {
            tx.x = 0;
            ty.x = 0;
        } else if (texNorm == &YAxisPos || texNorm == &YAxisNeg) {
            tx.y = 0;
            ty.y = 0;
        } else if (texNorm == &ZAxisPos || texNorm == &ZAxisNeg) {
            tx.z = 0;
            ty.z = 0;
        }

        // apply the rotation
        [self rotate:theRotation center:theCenter];
        
        // the new scale factors are determined by the lengths of the rotated texture axes
        xScale = lengthV3f(&tx);
        yScale = lengthV3f(&ty);
        
        scaleV3f(&tx, 1 / xScale, &tx);
        scaleV3f(&ty, 1 / yScale, &ty);
        
        // determine the rotation angle of the texture by measuring the X texture axis
        TVector3f r;
        float rad = acos(dotV3f(&texAxisX, &tx));
        crossV3f(&texAxisX, &tx, &r);
        rotation += dotV3f(&r, texNorm) < 0 ? -rad * 180 / M_PI : rad * 180 / M_PI;
        
        [self validateTexAxes];

        // the texture coordinates which p has after the rotation without offset
        float ox2 = dotV3f(&pr, &scaledTexAxisX);
        float oy2 = dotV3f(&pr, &scaledTexAxisY);
        
        // the current offset is the distance between the two points in texture coordinates
        // since the points' texture coordinates should be invariant
        xOffset = ox1 - ox2;
        yOffset = oy1 - oy2;
    } else {
        [self rotate:theRotation center:theCenter];
    }

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
    if (surfaceToWorldMatrix == nil)
        [self updateMatrices];
    
    [surfaceToWorldMatrix transformVector3f:surfacePoint result:worldPoint];
}

- (void)transformWorld:(const TVector3f *)worldPoint toSurface:(TVector3f *)surfacePoint {
    if (worldToSurfaceMatrix == nil)
        [self updateMatrices];
    
    [worldToSurfaceMatrix transformVector3f:worldPoint result:surfacePoint];
}

- (Matrix4f *)surfaceToWorldMatrix {
    if (surfaceToWorldMatrix == nil)
        [self updateMatrices];
    
    return surfaceToWorldMatrix;
}

- (Matrix4f *)worldToSurfaceMatrix {
    if (worldToSurfaceMatrix == nil)
        [self updateMatrices];

    return worldToSurfaceMatrix;
}

- (VBOMemBlock *)memBlock {
    return memBlock;
}

- (void)setMemBlock:(VBOMemBlock *)theMemBlock {
    [memBlock free];
    memBlock = theMemBlock;
}

@end
