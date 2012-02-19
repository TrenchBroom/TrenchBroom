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


static const TVector3f* BaseAxes[18] = { &ZAxisPos, &XAxisPos, &YAxisNeg,
                                         &ZAxisNeg, &XAxisPos, &YAxisNeg,
                                         &XAxisPos, &YAxisPos, &ZAxisNeg,
                                         &XAxisNeg, &YAxisPos, &ZAxisNeg,
                                         &YAxisPos, &XAxisPos, &ZAxisNeg,
                                         &YAxisNeg, &XAxisPos, &ZAxisNeg};

@interface MutableFace (private)

- (void)validateTexAxesForFaceNorm:(const TVector3f *)theNorm;
- (const TVector3f *)texPlaneNormAndXAxis:(TVector3f *)theXAxis yAxis:(TVector3f *)theYAxis forFaceNorm:(const TVector3f *)theNorm;
- (void)validateMatrices;
- (void)invalidate;

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

- (void)validateTexAxesForFaceNorm:(const TVector3f *)theNorm {
    texPlaneNorm = [self texPlaneNormAndXAxis:&texAxisX yAxis:&texAxisY forFaceNorm:theNorm];
    
    TQuaternion rot;
    TVector3f rotAxis;

    float ang = rotation * M_PI / 180;
    absV3f(texPlaneNorm, &rotAxis);
    
    setAngleAndAxisQ(&rot, ang, &rotAxis);
    rotateQ(&rot, &texAxisX, &texAxisX);
    rotateQ(&rot, &texAxisY, &texAxisY);
    
    scaleV3f(&texAxisX, 1 / xScale, &scaledTexAxisX);
    scaleV3f(&texAxisY, 1 / yScale, &scaledTexAxisY);
    
    texAxesValid = YES;
}

- (void)validateMatrices {
    TVector3f xAxis, yAxis, zAxis, center;

    centerOfVertices([self vertices], &center);
    zAxis = *[self norm];

    const TVector3f* closestAxis = firstAxisV3f(&zAxis);
    if (closestAxis == &XAxisPos) {
        addV3f(&YAxisPos, &center, &xAxis);
        [self projectToSurface:&xAxis axis:&XAxisPos result:&xAxis];
    } else if (closestAxis == &XAxisNeg) {
        addV3f(&YAxisNeg, &center, &xAxis);
        [self projectToSurface:&xAxis axis:&XAxisNeg result:&xAxis];
    } else if (closestAxis == &YAxisPos) {
        addV3f(&XAxisNeg, &center, &xAxis);
        [self projectToSurface:&xAxis axis:&YAxisPos result:&xAxis];
    } else if (closestAxis == &YAxisNeg) {
        addV3f(&XAxisPos, &center, &xAxis);
        [self projectToSurface:&xAxis axis:&YAxisNeg result:&xAxis];
    } else if (closestAxis == &ZAxisPos) {
        addV3f(&XAxisPos, &center, &xAxis);
        [self projectToSurface:&xAxis axis:&ZAxisPos result:&xAxis];
    } else {
        addV3f(&XAxisNeg, &center, &xAxis);
        [self projectToSurface:&xAxis axis:&ZAxisNeg result:&xAxis];
    }
    
    subV3f(&xAxis, &center, &xAxis);
    normalizeV3f(&xAxis, &xAxis);
    crossV3f(&zAxis, &xAxis, &yAxis);
    normalizeV3f(&yAxis, &yAxis);
    
    // build transformation matrix
    setColumnM4fV3f(&surfaceToWorldMatrix, &xAxis, 0, &surfaceToWorldMatrix);
    setColumnM4fV3f(&surfaceToWorldMatrix, &yAxis, 1, &surfaceToWorldMatrix);
    setColumnM4fV3f(&surfaceToWorldMatrix, &zAxis, 2, &surfaceToWorldMatrix);
    setColumnM4fV3f(&surfaceToWorldMatrix, &center, 3, &surfaceToWorldMatrix);
    setValueM4f(&surfaceToWorldMatrix, 1, 3, 3, &surfaceToWorldMatrix);
    
    if (!invertM4f(&surfaceToWorldMatrix, &worldToSurfaceMatrix))
        [NSException raise:@"NonInvertibleMatrixException" format:@"surface transformation matrix is not invertible"];
    
    matricesValid = YES;
}

- (void)invalidate {
    matricesValid = NO;
    texAxesValid = NO;
}

- (void)updateTextureParametersForTransformation:(const TMatrix4f *)transformation {
    if (!texAxesValid)
        [self validateTexAxesForFaceNorm:[self norm]];
    
    
    TVector3f newTexAxisX, newTexAxisY, newFaceNorm, newCenter, newBaseAxisX, newBaseAxisY, offset, temp;
    TVector2f curCenterTexCoords, newCenterTexCoords;
    TPlane plane;
    TVector3f curCenter;
    const TVector3f* newTexPlaneNorm;
    
    // calculate the current texture coordinates of the face's center
    centerOfVertices([self vertices], &curCenter);
    curCenterTexCoords.x = dotV3f(&curCenter, &scaledTexAxisX) + xOffset;
    curCenterTexCoords.y = dotV3f(&curCenter, &scaledTexAxisY) + yOffset;
    
    // invert the scale of the current texture axes
    scaleV3f(&texAxisX, xScale, &newTexAxisX);
    scaleV3f(&texAxisY, yScale, &newTexAxisY);
    
    // project the inversely scaled texture axes onto the boundary plane
    plane.point = NullVector;
    plane.norm = [self boundary]->norm;
    if (texPlaneNorm == &XAxisPos || texPlaneNorm == &XAxisNeg) {
        newTexAxisX.x = planeX(&plane, newTexAxisX.y, newTexAxisX.z);
        newTexAxisY.x = planeX(&plane, newTexAxisY.y, newTexAxisY.z);
    } else if (texPlaneNorm == &YAxisPos || texPlaneNorm == &YAxisNeg) {
        newTexAxisX.y = planeY(&plane, newTexAxisX.x, newTexAxisX.z);
        newTexAxisY.y = planeY(&plane, newTexAxisY.x, newTexAxisY.z);
    } else {
        newTexAxisX.z = planeZ(&plane, newTexAxisX.x, newTexAxisX.y);
        newTexAxisY.z = planeZ(&plane, newTexAxisY.x, newTexAxisY.y);
    }
    
    // apply the transformation
    transformM4fV3f(transformation, &newTexAxisX, &newTexAxisX);
    transformM4fV3f(transformation, &newTexAxisY, &newTexAxisY);
    transformM4fV3f(transformation, [self norm], &newFaceNorm);
    transformM4fV3f(transformation, &NullVector, &offset);
    transformM4fV3f(transformation, &curCenter, &newCenter);
    
    // correct the directional vectors by the translational part of the transformation
    subV3f(&newTexAxisX, &offset, &newTexAxisX);
    subV3f(&newTexAxisY, &offset, &newTexAxisY);
    subV3f(&newFaceNorm, &offset, &newFaceNorm);
    
    // obtain the new texture plane norm and the new base texture axes
    newTexPlaneNorm = [self texPlaneNormAndXAxis:&newBaseAxisX yAxis:&newBaseAxisY forFaceNorm:&newFaceNorm];

    /*
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
     */
    
    // project the transformed texture axes onto the new texture plane
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
    
    // the new scaling factors are the lengths of the transformed texture axes
    xScale = lengthV3f(&newTexAxisX);
    yScale = lengthV3f(&newTexAxisY);
    
    // normalize the transformed texture axes
    scaleV3f(&newTexAxisX, 1 / xScale, &newTexAxisX);
    scaleV3f(&newTexAxisY, 1 / yScale, &newTexAxisY);
    
    // WARNING: the texture plane norm is not the rotation axis of the texture (it's always the absolute axis)
    
    // determine the rotation angle from the dot product of the new base axes and the transformed texture axes
    float radX = acosf(dotV3f(&newBaseAxisX, &newTexAxisX));
    float radY = acosf(dotV3f(&newBaseAxisY, &newTexAxisY));
    float rad;
    if (radX < radY) {
        // the sign depends on the direction of the cross product
        crossV3f(&newBaseAxisX, &newTexAxisX, &temp);
        if (dotV3f(&temp, &newFaceNorm) < 0)
            radX *= -1;
        
        rad = radX;
    } else {
        // the sign depends on the direction of the cross product
        crossV3f(&newBaseAxisY, &newTexAxisY, &temp);
        if (dotV3f(&temp, &newFaceNorm) < 0)
            radY *= -1;
        
        rad = radY;
    }

    rotation = rad * 180 / M_PI;

    // apply the rotation to the new base axes
    TQuaternion rot;
    TVector3f rotAxis;
    absV3f(newTexPlaneNorm, &rotAxis);
    setAngleAndAxisQ(&rot, rad, &rotAxis);
    rotateQ(&rot, &newBaseAxisX, &newBaseAxisX);
    rotateQ(&rot, &newBaseAxisY, &newBaseAxisY);
    
    // the sign of the scaling factors depends on the angle between the new base axis and the new texture axis
    if (dotV3f(&newBaseAxisX, &newTexAxisX) < 0)
        xScale *= -1;
    if (dotV3f(&newBaseAxisY, &newTexAxisY) < 0)
        yScale *= -1;
    
    [self validateTexAxesForFaceNorm:&newFaceNorm];

    // determine the new texture coordinates of the transformed center of the face, sans offsets
    newCenterTexCoords.x = dotV3f(&newCenter, &scaledTexAxisX);
    newCenterTexCoords.y = dotV3f(&newCenter, &scaledTexAxisY);
    
    // since the center should be invariant, the offsets are determined by the difference of the current and
    // the original texture coordinates of the center
    xOffset = curCenterTexCoords.x - newCenterTexCoords.x;
    yOffset = curCenterTexCoords.y - newCenterTexCoords.y;
    
    xOffset -= ((int)xOffset / [texture width]) * [texture width];
    yOffset -= ((int)yOffset / [texture height]) * [texture height];
}

@end

#pragma mark -
@implementation MutableFace

- (id)init {
    if ((self = [super init])) {
        faceId = [[[IdGenerator sharedGenerator] getId] retain];
        vboBlock = NULL;
    }
    
    return self;
}

- (id)initWithWorldBounds:(const TBoundingBox *)theWorldBounds {
    NSAssert(theWorldBounds != NULL, @"world bounds must not be NULL");
    if ((self = [self init])) {
        worldBounds = theWorldBounds;
        texture = nil;
        filePosition = -1;
    }
    
    return self;
}

- (id)initWithWorldBounds:(const TBoundingBox *)theWorldBounds point1:(const TVector3f *)aPoint1 point2:(const TVector3f *)aPoint2 point3:(const TVector3f *)aPoint3 {
    if ((self = [self initWithWorldBounds:theWorldBounds])) {
        points[0] = *aPoint1;
        points[1] = *aPoint2;
        points[2] = *aPoint3;
        [self setXScale:1];
        [self setYScale:1];

        if (!setPlanePointsV3f(&boundary, &points[0], &points[1], &points[2])) {
            [self release];
            return nil;
        }
    }
    
    return self;
}

- (id)initWithWorldBounds:(const TBoundingBox *)theWorldBounds faceTemplate:(id <Face>)theTemplate {
    if ((self = [self initWithWorldBounds:theWorldBounds])) {
        [theTemplate point1:&points[0] point2:&points[1] point3:&points[2]];
        boundary = *[theTemplate boundary];
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
    result->worldBounds = worldBounds;
    [self point1:&result->points[0] point2:&result->points[1] point3:&result->points[2]];
    result->boundary = boundary;
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
    if (vboBlock != NULL)
        freeVboBlock(vboBlock);
    if (side != NULL)
        side->face = nil;
	[super dealloc];
}

- (void)setPlanePointsFromVertices {
    TVector3f v1, v2;
    
    float bestDot = 1;
    int best = -1;
    const TVertexList* vertices = [self vertices];
    for (int i = 0; i < vertices->count && bestDot > 0; i++) {
        points[2] = vertices->items[(i - 1 + vertices->count) % vertices->count]->position;
        points[0] = vertices->items[i]->position;
        points[1] = vertices->items[(i + 1) % vertices->count]->position;
        
        subV3f(&points[2], &points[0], &v1);
        normalizeV3f(&v1, &v1);
        subV3f(&points[1], &points[0], &v2);
        normalizeV3f(&v2, &v2);
        
        float dot = fabsf(dotV3f(&v1, &v2));
        if (dot < bestDot) {
            bestDot = dot;
            best = i;
        }
    }
    
    assert(best != -1);
    
    points[2] = vertices->items[(best - 1 + vertices->count) % vertices->count]->position;
    points[0] = vertices->items[best]->position;
    points[1] = vertices->items[(best + 1) % vertices->count]->position;
    
    setPlanePointsV3f(&boundary, &points[0], &points[1], &points[2]);
}

- (void)setBrush:(MutableBrush *)theBrush {
    brush = theBrush;
}

- (void)setTexture:(Texture *)theTexture {
    if (texture != nil) {
        [texture decUsageCount];
        [texture release];
    }
    
    texture = nil;
    
    if (theTexture != nil) {
        texture = [theTexture retain];
        [texture incUsageCount];
    }
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
        [self validateTexAxesForFaceNorm:[self norm]];
    
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

- (void)translateBy:(const TVector3f *)theDelta lockTexture:(BOOL)lockTexture {
    if (lockTexture) {
        TMatrix4f t;
        translateM4f(&IdentityM4f, theDelta, &t);
        [self updateTextureParametersForTransformation:&t];
    }

    addV3f(&boundary.point, theDelta, &boundary.point);
    for (int i = 0; i < 3; i++)
        addV3f(&points[i], theDelta, &points[i]);

    [self invalidate];
}

- (void)rotate90CW:(EAxis)theAxis center:(const TVector3f *)theCenter lockTexture:(BOOL)lockTexture {
    if (lockTexture) {
        TMatrix4f t;
        TVector3f invCenter;

        translateM4f(&IdentityM4f, theCenter, &t);
        if (theAxis == A_X)
            mulM4f(&t, &RotX90CWM4f, &t);
        else if (theAxis == A_Y)
            mulM4f(&t, &RotY90CWM4f, &t);
        else
            mulM4f(&t, &RotZ90CWM4f, &t);
        
        scaleV3f(theCenter, -1, &invCenter);
        translateM4f(&t, &invCenter, &t);
        
        [self updateTextureParametersForTransformation:&t];
    }

    subV3f(&boundary.point, theCenter, &boundary.point);
    rotate90CWV3f(&boundary.point, theAxis, &boundary.point);
    rotate90CWV3f(&boundary.norm, theAxis, &boundary.norm);
    addV3f(&boundary.point, theCenter, &boundary.point);
    
    for (int i = 0; i < 3; i++) {
        subV3f(&points[i], theCenter, &points[i]);
        rotate90CWV3f(&points[i], theAxis, &points[i]);
        addV3f(&points[i], theCenter, &points[i]);
    }

    [self invalidate];
}

- (void)rotate90CCW:(EAxis)theAxis center:(const TVector3f *)theCenter lockTexture:(BOOL)lockTexture {
    if (lockTexture) {
        TMatrix4f t;
        TVector3f invCenter;

        translateM4f(&IdentityM4f, theCenter, &t);
        if (theAxis == A_X)
            mulM4f(&t, &RotX90CCWM4f, &t);
        else if (theAxis == A_Y)
            mulM4f(&t, &RotY90CCWM4f, &t);
        else
            mulM4f(&t, &RotZ90CCWM4f, &t);
        
        scaleV3f(theCenter, -1, &invCenter);
        translateM4f(&t, &invCenter, &t);
        
        [self updateTextureParametersForTransformation:&t];
    }
    
    subV3f(&boundary.point, theCenter, &boundary.point);
    rotate90CCWV3f(&boundary.point, theAxis, &boundary.point);
    rotate90CCWV3f(&boundary.norm, theAxis, &boundary.norm);
    addV3f(&boundary.point, theCenter, &boundary.point);
    
    
    for (int i = 0; i < 3; i++) {
        subV3f(&points[i], theCenter, &points[i]);
        rotate90CCWV3f(&points[i], theAxis, &points[i]);
        addV3f(&points[i], theCenter, &points[i]);
    }
    [self invalidate];
}

- (void)rotate:(const TQuaternion *)theRotation center:(const TVector3f *)theCenter lockTexture:(BOOL)lockTexture {
    if (lockTexture) {
        TMatrix4f t;
        TVector3f invCenter;

        translateM4f(&IdentityM4f, theCenter, &t);
        rotateM4fQ(&t, theRotation, &t);
        scaleV3f(theCenter, -1, &invCenter);
        translateM4f(&t, &invCenter, &t);
        
        [self updateTextureParametersForTransformation:&t];
    }

    subV3f(&boundary.point, theCenter, &boundary.point);
    rotateQ(theRotation, &boundary.point, &boundary.point);
    rotateQ(theRotation, &boundary.norm, &boundary.norm);
    addV3f(&boundary.point, theCenter, &boundary.point);
    
    for (int i = 0; i < 3; i++) {
        subV3f(&points[i], theCenter, &points[i]);
        rotateQ(theRotation, &points[i], &points[i]);
        addV3f(&points[i], theCenter, &points[i]);
    }

    [self invalidate];
}

- (void)flipAxis:(EAxis)theAxis center:(const TVector3f *)theCenter lockTexture:(BOOL)lockTexture {
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
            
            boundary.point.x -= theCenter->x;
            boundary.point.x *= -1;
            boundary.point.x += theCenter->x;
            boundary.norm.x *= -1;
            
            for (int i = 0; i < 3; i++) {
                points[i].x -= theCenter->x;
                points[i].x *= -1;
                points[i].x += theCenter->x;
            }
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

            boundary.point.y -= theCenter->y;
            boundary.point.y *= -1;
            boundary.point.y += theCenter->y;
            boundary.norm.y *= -1;
            
            for (int i = 0; i < 3; i++) {
                points[i].y -= theCenter->y;
                points[i].y *= -1;
                points[i].y += theCenter->y;
            }
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

            boundary.point.z -= theCenter->z;
            boundary.point.z *= -1;
            boundary.point.z += theCenter->z;
            boundary.norm.z *= -1;
            
            for (int i = 0; i < 3; i++) {
                points[i].z -= theCenter->z;
                points[i].z *= -1;
                points[i].z += theCenter->z;
            }
            break;
        }
    }
    
    [self invalidate];
}

- (void)dragBy:(float)dist lockTexture:(BOOL)lockTexture {
    TVector3f deltaf;
    scaleV3f([self norm], dist, &deltaf);
    
    addV3f(&boundary.point, &deltaf, &boundary.point);
    for (int i = 0; i < 3; i++)
        addV3f(&points[i], &deltaf, &points[i]);
    
    [self invalidate];
}

- (void)setSide:(TSide *)theSide {
    side = theSide;
}

- (const TSide *)side {
    return side;
}

- (int)filePosition {
    return filePosition;
}

- (void)setFilePosition:(int)theFilePosition {
    filePosition = theFilePosition;
}

- (void)restore:(id <Face>)theTemplate {
    NSAssert(theTemplate != nil, @"template must not be nil");
    NSAssert([faceId isEqual:[theTemplate faceId]], @"face id must be equal");

    boundary = *[theTemplate boundary];
    xOffset = [theTemplate xOffset];
    yOffset = [theTemplate yOffset];
    xScale = [theTemplate xScale];
    yScale = [theTemplate yScale];
    rotation = [theTemplate rotation];

    [self invalidate];
}

- (NSString *)description {
    return [NSString stringWithFormat:@"ID: %i, texture: %@, X offset: %f, Y offset: %f, rotation: %f, X scale: %f, Y scale: %f", 
            [faceId intValue], 
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

- (Texture *)texture {
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

- (void)point1:(TVector3f *)thePoint1 point2:(TVector3f *)thePoint2 point3:(TVector3f *)thePoint3 {
    *thePoint1 = points[0];
    *thePoint2 = points[1];
    *thePoint3 = points[2];
}

- (const TVector3f *)norm {
    return &[self boundary]->norm;
}

- (const TPlane *)boundary {
    return &boundary;
}

- (const TVertexList *)vertices {
    NSAssert(side != NULL, @"side must not be NULL");
    return &side->vertices;
}

- (const TEdgeList *)edges {
    NSAssert(side != NULL, @"side must not be NULL");
    return &side->edges;
}

- (const TBoundingBox *)worldBounds {
    return worldBounds;
}

- (void)texCoords:(TVector2f *)texCoords forVertex:(TVector3f *)vertex {
    if (!texAxesValid)
        [self validateTexAxesForFaceNorm:[self norm]];
    
    texCoords->x = dotV3f(vertex, &scaledTexAxisX) + xOffset;
    texCoords->y = dotV3f(vertex, &scaledTexAxisY) + yOffset;
}

- (void)gridCoords:(TVector2f *)gridCoords forVertex:(TVector3f *)vertex {
    if (!texAxesValid)
        [self validateTexAxesForFaceNorm:[self norm]];
    
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
        [self validateMatrices];
    transformM4fV3f(&surfaceToWorldMatrix, surfacePoint, worldPoint);
}

- (void)transformWorld:(const TVector3f *)worldPoint toSurface:(TVector3f *)surfacePoint {
    if (!matricesValid)
        [self validateMatrices];
    transformM4fV3f(&worldToSurfaceMatrix, worldPoint, surfacePoint);
}

- (const TMatrix4f *)surfaceToWorldMatrix {
    if (!matricesValid)
        [self validateMatrices];
    return &surfaceToWorldMatrix;
}

- (const TMatrix4f *)worldToSurfaceMatrix {
    if (!matricesValid)
        [self validateMatrices];
    return &worldToSurfaceMatrix;
}

- (BOOL)projectToSurface:(const TVector3f *)worldPoint axis:(const TVector3f *)axis result:(TVector3f *)result {
    TLine line;
    line.point = *worldPoint;
    line.direction = *axis;
    
    float dist = intersectPlaneWithLine([self boundary], &line);
    if (isnan(dist))
        return NO;
    
    linePointAtDistance(&line, dist, result);
    return YES;
}

- (VboBlock *)vboBlock {
    return vboBlock;
}

- (void)setVboBlock:(VboBlock *)theVboBlock {
    if (vboBlock != NULL)
        freeVboBlock(vboBlock);
    vboBlock = theVboBlock;
}

@end
