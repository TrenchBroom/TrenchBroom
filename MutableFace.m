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
#import "PickingHit.h"
#import "Ray3D.h"
#import "VBOBuffer.h"
#import "VBOMemBlock.h"
#import "TextureManager.h"
#import "Texture.h"
#import "RenderContext.h"
#import "IntData.h"
#import "CoordinatePlane.h"
#import "SegmentIterator.h"

static Vector3f* baseAxes[18];

@interface MutableFace (private)
- (void)updateTexAxes;
- (void)updateMatrices;
@end

@implementation MutableFace (private)

- (void)updateTexAxes {
    if (!texAxesValid) {
        // determine texture axes, this is from QBSP
        float best = 0;
        bestAxis = 0;
        for (int i = 0; i < 6; i++) {
            float dot = [[self norm] dot:baseAxes[i * 3]];
            if (dot > best) {
                best = dot;
                bestAxis = i;
            }
        }
        
        [texAxisX setFloat:baseAxes[bestAxis * 3 + 1]];
        [texAxisY setFloat:baseAxes[bestAxis * 3 + 2]];
        
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
        
        texAxesValid = YES;
    }
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
        [xAxis setFloat:[line pointAtDistance:[plane intersectWithLine:line]]];
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
        [xAxis setFloat:[line pointAtDistance:[plane intersectWithLine:line]]];
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
    
    [xAxis release];
    [yAxis release];
    [zAxis release];
}

@end

@implementation MutableFace

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
        texAxisX = [[Vector3f alloc] init];
        texAxisY = [[Vector3f alloc] init];
    }
    
    return self;
}

- (id)initWithPoint1:(Vector3i *)aPoint1 point2:(Vector3i *)aPoint2 point3:(Vector3i *)aPoint3 texture:(NSString *)aTexture {
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

- (NSNumber *)faceId {
    return faceId;
}

- (id <Brush>)brush {
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
    
    [surfaceMatrix release];
    surfaceMatrix = nil;
    [worldMatrix release];
    worldMatrix = nil;
    
    texAxesValid = NO;
    
    [brush faceGeometryChanged:self];
}

- (void)setBrush:(MutableBrush *)theBrush {
    brush = theBrush;
}

- (void)setPoint1:(Vector3i *)thePoint1 point2:(Vector3i *)thePoint2 point3:(Vector3i *)thePoint3{
    if ([point1 isEqualToVector:thePoint1] &&
        [point2 isEqualToVector:thePoint2] &&
        [point3 isEqualToVector:thePoint3])
        return;

    [point1 setInt:thePoint1];
    [point2 setInt:thePoint2];
    [point3 setInt:thePoint3];

    [self geometryChanged];
}

- (void)translateBy:(Vector3i *)theDelta {
    [point1 add:theDelta];
    [point2 add:theDelta];
    [point3 add:theDelta];
    
    if (texAxisX == nil || texAxisY == nil)
        [self updateTexAxes];
    
    switch (bestAxis) {
        case 0: // xAxisPos
        case 1: // xAxisNeg
            xOffset += [theDelta y];
            yOffset += [theDelta z];
            break;
        case 2: // yAxisPos
        case 3: // yAxisNeg
            xOffset += [theDelta x];
            yOffset += [theDelta z];
            break;
        case 4: // zAxisPos
        case 5: // zAxisNeg
            xOffset += [theDelta x];
            yOffset += [theDelta y];
            break;
        default:
            break;
    }
    
    [self geometryChanged];
}

- (void)dragBy:(float)dist {
    Vector3f* f = [[Vector3f alloc] initWithFloatVector:[self norm]];
    [f scale:dist];
    
    Vector3i* delta = [[Vector3i alloc] initWithIntX:roundf([f x]) y:roundf([f y]) z:roundf([f z])];
    [self translateBy:delta];
    
    [f release];
    [delta release];
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
    
    if (texAxisX == nil || texAxisY == nil)
        [self updateTexAxes];
    
    switch (bestAxis) {
        case 0:
        case 3:
        case 4:
            xOffset -= x;
            yOffset += y;
            break;
        case 1:
        case 2:
        case 5:
            xOffset += x;
            yOffset += y;
            break;
        default:
            break;
    }
}

- (HalfSpace3D *)halfSpace {
    if (halfSpace == nil) {
        halfSpace =  [[HalfSpace3D alloc] initWithIntPoint1:[self point1] 
                                                     point2:[self point2] 
                                                     point3:[self point3]];
    }
    
    return halfSpace;
}

- (NSArray *)gridWithSize:(int)gridSize {
    CoordinatePlane* plane = [CoordinatePlane projectionPlaneForNormal:norm];
    
    NSMutableArray* pVertices = [[NSMutableArray alloc] initWithCapacity:[[self vertices] count]];
    
    float sx = FLT_MAX;
    float sy = FLT_MAX;
    float lx = -FLT_MAX;
    float ly = -FLT_MAX;
    
    NSEnumerator* vertexEn = [[self vertices] objectEnumerator];
    Vertex* vertex;
    while ((vertex = [vertexEn nextObject])) {
        Vector3f* pVertex = [plane project:[vertex vector]];
        [pVertices addObject:pVertex];
        
        float x = [pVertex x];
        float y = [pVertex y];
        
        if (x < sx)
            sx = x;
        if (x > lx)
            lx = x;
        if (y < sy)
            sy = y;
        if (y > ly)
            ly = y;
    }
    
    int gridNo = sx / gridSize;
    if (sx > 0)
        gridNo++;
    float gridX = gridNo * gridSize; // first grid line to intersect with polygon
    if (feq(gridX, sx))
        gridX += gridSize;
    
    BOOL clockwise = [plane clockwise:norm];
    NSMutableArray* grid = [[NSMutableArray alloc] init];
    
    SegmentIterator* si = [[SegmentIterator alloc] initWithVertices:pVertices vertical:NO clockwise:clockwise];
    while (flt(gridX, lx)) {
        
        Vector3f* ls = [si forwardLeftTo:gridX];
        Vector3f* rs = [si forwardRightTo:gridX];
        if (ls == nil || rs == nil)
            break;
        
        Vector3f* le = [si nextLeft];
        Vector3f* re = [si nextRight];
        
        float lx = gridX;
        float ly = ([le y] - [ls y]) * (gridX - [ls x]) / ([le x] - [ls x]) + [ls y];
        float lz = ([le z] - [ls z]) * (gridX - [ls x]) / ([le x] - [ls x]) + [ls z];
        
        Vector3f* lgv = [[Vector3f alloc] init];
        [plane set:lgv toX:lx y:ly z:lz];
        
        float rx = gridX;
        float ry = ([re y] - [rs y]) * (gridX - [rs x]) / ([re x] - [rs x]) + [rs y];
        float rz = ([re z] - [rs z]) * (gridX - [rs x]) / ([re x] - [rs x]) + [rs z];
        
        Vector3f* rgv = [[Vector3f alloc] init];
        [plane set:rgv toX:rx y:ry z:rz];
        
        [grid addObject:lgv];
        [grid addObject:rgv];
        
        [lgv release];
        [rgv release];
        
        gridX += gridSize;
    }
    [si release];
    
    gridNo = sy / gridSize;
    if (sy > 0)
        gridNo++;
    float gridY = gridNo * gridSize;
    if (feq(gridY, sy))
        gridY += gridSize;
    
    si = [[SegmentIterator alloc] initWithVertices:pVertices vertical:YES clockwise:clockwise];
    
    while (flt(gridY, ly)) {
        Vector3f* ls = [si forwardLeftTo:gridY];
        Vector3f* rs = [si forwardRightTo:gridY];
        if (ls == nil || rs == nil)
            break;
        
        Vector3f* le = [si nextLeft];
        Vector3f* re = [si nextRight];
        
        float lx = ([le x] - [ls x]) * (gridY - [ls y]) / ([le y] - [ls y]) + [ls x];
        float ly = gridY;
        float lz = ([le z] - [ls z]) * (gridY - [ls y]) / ([le y] - [ls y]) + [ls z];
        
        Vector3f* lgv = [[Vector3f alloc] init];
        [plane set:lgv toX:lx y:ly z:lz];
        
        float rx = ([re x] - [rs x]) * (gridY - [rs y]) / ([re y] - [rs y]) + [rs x];
        float ry = gridY;
        float rz = ([re z] - [rs z]) * (gridY - [rs y]) / ([re y] - [rs y]) + [rs z];
        
        Vector3f* rgv = [[Vector3f alloc] init];
        [plane set:rgv toX:rx y:ry z:rz];
        
        [grid addObject:lgv];
        [grid addObject:rgv];
        
        [lgv release];
        [rgv release];
        
        gridY += gridSize;
    }
    [si release];
    
    [pVertices release];
    return [grid autorelease];
}

- (void)texCoords:(Vector2f *)texCoords forVertex:(Vector3f *)vertex {
    [self updateTexAxes];
    [texCoords setX:[vertex dot:texAxisX] + xOffset];
    [texCoords setY:[vertex dot:texAxisY] + yOffset];
}

- (void)gridCoords:(Vector2f *)gridCoords forVertex:(Vector3f *)vertex {
    switch ([[self norm] largestComponent]) {
        case VC_X:
            [gridCoords setX:([vertex y] + 0.5f) / 256];
            [gridCoords setY:([vertex z] + 0.5f) / 256];
            break;
        case VC_Y:
            [gridCoords setX:([vertex x] + 0.5f) / 256];
            [gridCoords setY:([vertex z] + 0.5f) / 256];
            break;
        default:
            [gridCoords setX:([vertex x] + 0.5f) / 256];
            [gridCoords setY:([vertex y] + 0.5f) / 256];
            break;
    }
}

- (void)transformToWorld:(Vector3f *)point {
    if (surfaceMatrix == nil)
        [self updateMatrices];
    
    [surfaceMatrix transformVector3f:point];
}

- (void)transformToSurface:(Vector3f *)point {
    if (worldMatrix == nil)
        [self updateMatrices];
    
    [worldMatrix transformVector3f:point];
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
    if (center == nil) {
        NSEnumerator* vertexEn = [vertices objectEnumerator];
        Vertex* vertex = [vertexEn nextObject];
        center = [[Vector3f alloc] initWithFloatVector:[vertex vector]];
        while ((vertex = [vertexEn nextObject]))
            [center add:[vertex vector]];
        [center scale:1.0f / [vertices count]];
    }
    
    return center;
}

- (NSArray *)vertices {
    return vertices;
}

- (NSArray *)edges {
    return edges;
}

- (void)setMemBlock:(VBOMemBlock *)theBlock {
    [memBlock free];
    [memBlock release];
    memBlock = [theBlock retain];
}

- (VBOMemBlock *)memBlock {
    return memBlock;
}

- (void)setVertices:(NSArray *)theVertices {
    [center release];
    center = nil;
    
    [vertices release];
    vertices = [theVertices retain];
}

- (void)setEdges:(NSArray *)theEdges {
    [center release];
    center = nil;
    
    [edges release];
    edges = [theEdges retain];
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
    [memBlock free];
    [memBlock release];
    [vertices release];
    [edges release];
    [center release];
	[super dealloc];
}

@end
