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
#import "Vertex.h"
#import "math.h"
#import "Math.h"
#import "Brush.h"
#import "Matrix4f.h"
#import "Matrix3f.h"
#import "PickingHit.h"
#import "VBOBuffer.h"
#import "VBOMemBlock.h"
#import "TextureManager.h"
#import "Texture.h"
#import "RenderContext.h"
#import "IntData.h"

static TVector3f baseAxes[18];

@interface MutableFace (private)

- (void)updateTexAxes;
- (void)updateMatrices;
- (void)geometryChanged;
- (void)validate;

@end

@implementation MutableFace (private)

- (void)updateTexAxes {
    if (!texAxesValid) {
        // determine texture axes, this is from QBSP
        float best = 0;
        bestAxis = 0;
        for (int i = 0; i < 6; i++) {
            float dot = dotV3f([self norm], &baseAxes[i * 3]);
            if (dot > best) {
                best = dot;
                bestAxis = i;
            }
        }
        
        texAxisX = baseAxes[bestAxis * 3 + 1];
        texAxisY = baseAxes[bestAxis * 3 + 2];
        
        float ang = rotation / 180 * M_PI;
        float sinv = sin(ang);
        float cosv = cos(ang);
        
        EAxis sv, tv;
        if (texAxisX.x != 0)
            sv = A_X;
        else if (texAxisX.y != 0)
            sv = A_Y;
        else
            sv = A_Z;
        
        if (texAxisY.x != 0)
            tv = A_X;
        else if (texAxisY.y != 0)
            tv = A_Y;
        else
            tv = A_Z;
        
        TVector3f texAxes[2] = {texAxisX, texAxisY};
        for (int i = 0; i < 2; i++) {
            float ns = cosv * componentV3f(&texAxes[i], sv) - sinv * componentV3f(&texAxes[i], tv);
            float nt = sinv * componentV3f(&texAxes[i], sv) + cosv * componentV3f(&texAxes[i], tv);
            
            setComponentV3f(&texAxes[i], sv, ns);
            setComponentV3f(&texAxes[i], tv, nt);
        }
        
        scaleV3f(&texAxisX, 1 / xScale, &texAxisX);
        scaleV3f(&texAxisY, 1 / yScale, &texAxisY);
        
        texAxesValid = YES;
    }
}

- (void)updateMatrices {
    TVector3f xAxis, yAxis, zAxis;
    TLine line;
    
    zAxis = *[self norm];
    float ny = [self norm]->y;

    if (fneg(ny)) {
        // surface X axis is normalized projection of positive world X axis along Y axis onto plane
        line.point = *[self center];
        line.point.x += 1;
        line.direction = YAxisPos;
        
        float dist = intersectPlaneWithLine([self boundary], &line);
        linePointAtDistance(&line, dist, &xAxis);
        subV3f(&xAxis, [self center], &xAxis);
        normalizeV3f(&xAxis, &xAxis);
    } else if (fpos(ny)) {
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
        if (fpos(nx)) {
            // positive world Y axis is surface X axis
            xAxis = YAxisPos;
        } else if (fneg(nx)) {
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
    
    yAxis = zAxis;
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
    
    [brush faceGeometryChanged:self];
}

- (void)validate {
    if (!centerValid && vertices != nil) {
        NSEnumerator* vertexEn = [vertices objectEnumerator];
        Vertex* vertex = [vertexEn nextObject];
        center = *[vertex vector];
        while ((vertex = [vertexEn nextObject]))
            addV3f(&center, [vertex vector], &center);
        scaleV3f(&center, 1.0f / [vertices count], &center);

        centerValid = YES;
    }
    
    if (!boundaryValid) {
        setPlanePoints(&boundary, &point1, &point2, &point3);
        boundaryValid = YES;
    }
}

@end

#pragma mark -
@implementation MutableFace

+ (void)initialize {
    baseAxes[ 0] = XAxisPos; baseAxes[ 1] = YAxisPos; baseAxes[ 2] = ZAxisNeg;
    baseAxes[ 3] = XAxisNeg; baseAxes[ 4] = YAxisPos; baseAxes[ 5] = ZAxisNeg;
    baseAxes[ 6] = YAxisPos; baseAxes[ 7] = XAxisPos; baseAxes[ 8] = ZAxisNeg;
    baseAxes[ 9] = YAxisNeg; baseAxes[10] = XAxisPos; baseAxes[11] = ZAxisNeg;
    baseAxes[12] = ZAxisPos; baseAxes[13] = XAxisPos; baseAxes[14] = ZAxisNeg;
    baseAxes[15] = ZAxisNeg; baseAxes[16] = XAxisPos; baseAxes[17] = ZAxisNeg;
}

- (id)init {
    if (self = [super init]) {
        faceId = [[[IdGenerator sharedGenerator] getId] retain];
        texture = [[NSMutableString alloc] init];
    }
    
    return self;
}

- (id)initWithPoint1:(TVector3i *)aPoint1 point2:(TVector3i *)aPoint2 point3:(TVector3i *)aPoint3 texture:(NSString *)aTexture {
    if (self = [self init]) {
        [self setPoint1:aPoint1 point2:aPoint2 point3:aPoint3];
        [self setTexture:aTexture];
        [self setXScale:1];
        [self setYScale:1];
    }
    
    return self;
}

- (id)initWithFaceTemplate:(id <Face>)theTemplate {
    if (self = [self init]) {
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
        [self updateTexAxes];
    
    NSLog(@"best axis %i", bestAxis);
    switch (bestAxis) {
        case 0:
            xOffset += x;
            yOffset += y;
            break;
        case 1:
            xOffset -= x;
            yOffset += y;
            break;
        case 2:
            xOffset += x;
            yOffset += y;
            break;
        case 3:
            xOffset += x;
            yOffset += y;
            break;
        case 4:
            xOffset -= x;
            yOffset += y;
            break;
        case 5:
            xOffset += x;
            yOffset += y;
            break;
        default:
            break;
    }
}

- (void)translateBy:(TVector3i *)theDelta {
    addV3i(&point1, theDelta, &point1);
    addV3i(&point2, theDelta, &point2);
    addV3i(&point3, theDelta, &point3);
    
    if (!texAxesValid)
        [self updateTexAxes];
    
    switch (bestAxis) {
        case 0:
        case 1:
            xOffset -= theDelta->y;
            yOffset += theDelta->z;
            break;
        case 2:
        case 3:
            xOffset -= theDelta->x;
            yOffset += theDelta->z;
            break;
        case 4:
        case 5:
            xOffset -= theDelta->x;
            yOffset += theDelta->y;
            break;
        default:
            break;
    }
    
    [self geometryChanged];
}

- (void)rotateZ90CW:(TVector3i *)theCenter {
    subV3i(&point1, theCenter, &point1);
    int x = point1.x;
    point1.x = point1.y;
    point1.y = -x;
    addV3i(&point1, theCenter, &point1);
    
    subV3i(&point2, theCenter, &point2);
    x = point2.x;
    point2.x = point2.y;
    point2.y = -x;
    addV3i(&point2, theCenter, &point2);

    subV3i(&point3, theCenter, &point3);
    x = point3.x;
    point3.x = point3.y;
    point3.y = -x;
    addV3i(&point3, theCenter, &point3);
    
    [self geometryChanged];
}

- (void)rotateZ90CCW:(TVector3i *)theCenter {
    subV3i(&point1, theCenter, &point1);
    int x = point1.x;
    point1.x = -point1.y;
    point1.y = x;
    addV3i(&point1, theCenter, &point1);
    
    subV3i(&point2, theCenter, &point2);
    x = point2.x;
    point2.x = -point2.y;
    point2.y = x;
    addV3i(&point2, theCenter, &point2);
    
    subV3i(&point3, theCenter, &point3);
    x = point3.x;
    point3.x = -point3.y;
    point3.y = x;
    addV3i(&point3, theCenter, &point3);

    [self geometryChanged];
}

- (BOOL)canDragBy:(float)dist {
    return [brush canDrag:self by:dist];
}

- (void)dragBy:(float)dist {
    TVector3f f;
    scaleV3f([self norm], dist, &f);
    
    TVector3i delta = {roundf(f.x), roundf(f.y), roundf(f.z)};
    [self translateBy:&delta];
}

- (void)setVertices:(NSArray *)theVertices {
    [vertices release];
    vertices = [theVertices retain];
    
    centerValid = NO;
}

- (void)setEdges:(NSArray *)theEdges {
    [edges release];
    edges = [theEdges retain];
    
    centerValid = NO;
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

- (void) dealloc {
    [faceId release];
	[texture release];
    [surfaceToWorldMatrix release];
    [worldToSurfaceMatrix release];
    [memBlock free];
    [memBlock release];
    [vertices release];
    [edges release];
	[super dealloc];
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

- (TVector3f *)norm {
    return &[self boundary]->norm;
}

- (TVector3f *)center {
    [self validate];
    return &center;
}

- (TPlane *)boundary {
    [self validate];
    return &boundary;
}

- (NSArray *)vertices {
    return vertices;
}

- (NSArray *)edges {
    return edges;
}

- (void)texCoords:(TVector2f *)texCoords forVertex:(TVector3f *)vertex {
    [self updateTexAxes];
    texCoords->x = dotV3f(vertex, &texAxisX) + xOffset;
    texCoords->y = dotV3f(vertex, &texAxisY) + yOffset;
}

- (void)gridCoords:(TVector2f *)gridCoords forVertex:(TVector3f *)vertex {
    switch (largestComponentV3f([self norm])) {
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

- (void)transformToWorld:(TVector3f *)point {
    if (surfaceToWorldMatrix == nil)
        [self updateMatrices];
    
    [surfaceToWorldMatrix transformVector3f:point];
}

- (void)transformToSurface:(TVector3f *)point {
    if (worldToSurfaceMatrix == nil)
        [self updateMatrices];
    
    [worldToSurfaceMatrix transformVector3f:point];
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

- (void)setMemBlock:(VBOMemBlock *)theBlock {
    [memBlock free];
    [memBlock release];
    memBlock = [theBlock retain];
}

- (VBOMemBlock *)memBlock {
    return memBlock;
}

@end
