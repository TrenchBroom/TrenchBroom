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

#import "Grid.h"
#import "math.h"
#import "FloatData.h"
#import "Face.h"
#import "MutableFace.h"
#import "Brush.h"

NSString* const GridChanged = @"GridChanged";

@implementation Grid

- (id)init {
    if ((self = [super init])) {
        size = 4;
        alpha = 0.1f;
        draw = YES;
        snap = YES;
        for (int i = 0; i < 9; i++)
            texIds[i] = 0;
    }
    
    return self;
}

- (int)size {
    return size;
}

- (float)alpha {
    return alpha;
}

- (int)actualSize {
    if (snap)
        return 1 << size;
    return 1;
}

- (BOOL)draw {
    return draw;
}

- (BOOL)snap {
    return snap;
}

- (void)setSize:(int)theSize {
    if (theSize < 0 || theSize > 8)
        [NSException raise:NSInvalidArgumentException format:@"invalid grid size: %i", theSize];

    if (size == theSize)
        return;
    
    [super willChangeValueForKey:@"actualSize"];
    size = theSize;
    [super didChangeValueForKey:@"actualSize"];
    
    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    [center postNotificationName:GridChanged object:self];
}

- (void)setAlpha:(float)theAlpha {
    if (alpha == theAlpha)
        return;
    
    alpha = theAlpha;
    for (int i = 0; i < 9; i++)
        valid[i] = NO;
    
    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    [center postNotificationName:GridChanged object:self];
}

- (void)setDraw:(BOOL)isDrawEnabled {
    if (draw == isDrawEnabled)
        return;
    
    draw = isDrawEnabled;
    
    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    [center postNotificationName:GridChanged object:self];
}

- (void)setSnap:(BOOL)isSnapEnabled {
    if (snap == isSnapEnabled)
        return;
    
    snap = isSnapEnabled;
    
    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    [center postNotificationName:GridChanged object:self];
}

- (void)toggleDraw {
    [self setDraw:![self draw]];
}

- (void)toggleSnap {
    [self setSnap:![self snap]];
}

- (float)snapToGridf:(float)f {
    int actualSize = [self actualSize];
    return actualSize * roundf(f / actualSize);
}

- (float)snapUpToGridf:(float)f {
    int actualSize = [self actualSize];
    return actualSize * ceilf(f / actualSize);
}

- (float)snapUpToNextf:(float)f {
    int actualSize = [self actualSize];
    float s = actualSize * ceilf(f / actualSize);
    if (s == f)
        s += actualSize;
    return s;
}

- (float)snapDownToGridf:(float)f {
    int actualSize = [self actualSize];
    return actualSize * floorf(f / actualSize);
}

- (float)snapDownToPreviousf:(float)f {
    int actualSize = [self actualSize];
    float s = actualSize * floorf(f / actualSize);
    if (s == f)
        s -= actualSize;
    return s;
}

- (void)snapToGridV3f:(const TVector3f *)vector result:(TVector3f *)result {
    int actualSize = [self actualSize];
    result->x = actualSize * roundf(vector->x / actualSize);
    result->y = actualSize * roundf(vector->y / actualSize);
    result->z = actualSize * roundf(vector->z / actualSize);
}

- (void)snapToFarthestGridV3f:(const TVector3f *)vector result:(TVector3f *)result {
    TVector3f ceil, floor;
    [self snapUpToGridV3f:vector result:&ceil];
    [self snapDownToGridV3f:vector result:&floor];

    result->x = vector->x < 0 ? floor.x : ceil.x;
    result->y = vector->y < 0 ? floor.y : ceil.y;
    result->z = vector->z < 0 ? floor.z : ceil.z;
}

- (void)snapUpToGridV3f:(const TVector3f *)vector result:(TVector3f *)result {
    int actualSize = [self actualSize];
    result->x = actualSize * ceilf(vector->x / actualSize);
    result->y = actualSize * ceilf(vector->y / actualSize);
    result->z = actualSize * ceilf(vector->z / actualSize);
}

- (void)snapDownToGridV3f:(const TVector3f *)vector result:(TVector3f *)result {
    int actualSize = [self actualSize];
    result->x = actualSize * floorf(vector->x / actualSize);
    result->y = actualSize * floorf(vector->y / actualSize);
    result->z = actualSize * floorf(vector->z / actualSize);
}

- (void)gridOffsetV3f:(const TVector3f *)vector result:(TVector3f *)result {
    TVector3f snapped;
    [self snapToGridV3f:vector result:&snapped];
    subV3f(vector, &snapped, result);
}

- (void)snapToGridV3i:(const TVector3i *)vector result:(TVector3i *)result {
    int actualSize = [self actualSize];
    result->x = actualSize * roundf(vector->x / (float)actualSize);
    result->y = actualSize * roundf(vector->y / (float)actualSize);
    result->z = actualSize * roundf(vector->z / (float)actualSize);
}

- (void)snapUpToGridV3i:(const TVector3i *)vector result:(TVector3i *)result {
    int actualSize = [self actualSize];
    result->x = actualSize * ceilf(vector->x / (float)actualSize);
    result->y = actualSize * ceilf(vector->y / (float)actualSize);
    result->z = actualSize * ceilf(vector->z / (float)actualSize);
}

- (void)snapDownToGridV3i:(const TVector3i *)vector result:(TVector3i *)result {
    int actualSize = [self actualSize];
    result->x = actualSize * floorf(vector->x / (float)actualSize);
    result->y = actualSize * floorf(vector->y / (float)actualSize);
    result->z = actualSize * floorf(vector->z / (float)actualSize);
}

- (void)snapToGridV3i:(const TVector3i *)vector direction:(TVector3f *)direction result:(TVector3i *)result {
    int actualSize = [self actualSize];
    if (direction->x >= 0)
        result->x = actualSize * ceilf(vector->x / (float)actualSize);
    else
        result->x = actualSize * floorf(vector->x / (float)actualSize);
    if (direction->y >= 0)
        result->y = actualSize * ceilf(vector->y / (float)actualSize);
    else
        result->y = actualSize * floorf(vector->y / (float)actualSize);
    if (direction->z >= 0)
        result->z = actualSize * ceilf(vector->z / (float)actualSize);
    else
        result->z = actualSize * floorf(vector->z / (float)actualSize);
}

- (void)gridOffsetV3i:(const TVector3i *)vector result:(TVector3i *)result {
    TVector3i snapped;
    [self snapToGridV3i:vector result:&snapped];
    subV3i(vector, &snapped, result);
}

- (float)intersectWithRay:(const TRay *)ray skip:(int)skip {
    TPlane plane;
    
    plane.point.x = ray->direction.x > 0 ? [self snapUpToNextf:ray->origin.x] + skip * [self actualSize] : [self snapDownToPreviousf:ray->origin.x] - skip * [self actualSize];
    plane.point.y = ray->direction.y > 0 ? [self snapUpToNextf:ray->origin.y] + skip * [self actualSize] : [self snapDownToPreviousf:ray->origin.y] - skip * [self actualSize];
    plane.point.z = ray->direction.z > 0 ? [self snapUpToNextf:ray->origin.z] + skip * [self actualSize] : [self snapDownToPreviousf:ray->origin.z] - skip * [self actualSize];
    
    plane.norm = XAxisPos;
    float distX = intersectPlaneWithRay(&plane, ray);
    
    plane.norm = YAxisPos;
    float distY = intersectPlaneWithRay(&plane, ray);
    
    plane.norm = ZAxisPos;
    float distZ = intersectPlaneWithRay(&plane, ray);
    
    float dist = distX;
    if (!isnan(distY) && (isnan(dist) || fabsf(distY) < fabsf(dist)))
        dist = distY;
    if (!isnan(distZ) && (isnan(dist) || fabsf(distZ) < fabsf(dist)))
        dist = distZ;
    
    return dist;
}

- (void)moveDeltaForBounds:(const TBoundingBox *)theBounds worldBounds:(const TBoundingBox *)theWorldBounds delta:(TVector3f *)theDelta lastPoint:(TVector3f *)theLastPoint {
    NSAssert(theBounds != NULL, @"bounds must not be nil");
    NSAssert(theWorldBounds != NULL, @"world bounds must not be NULL");
    NSAssert(theDelta != NULL, @"delta must not be NULL");

    if (theDelta->x > 0) {
        theDelta->x = [self snapToGridf:theBounds->max.x + theDelta->x] - theBounds->max.x;
        if (theDelta->x <= 0) {
            theDelta->x = 0;
        } else {
            /*
             if (theDelta->x < 1)
             theDelta->x = [grid actualSize];
             */
            if (theBounds->max.x + theDelta->x > theWorldBounds->max.x) {
                theDelta->x = theWorldBounds->max.x - theBounds->max.x;
                theDelta->y = 0;
                theDelta->z = 0;
            } else if (theLastPoint != NULL) {
                theLastPoint->x += theDelta->x;
            }
        }
    } else if (theDelta->x < 0) {
        theDelta->x = [self snapToGridf:theBounds->min.x + theDelta->x] - theBounds->min.x;
        if (theDelta->x >= 0) {
            theDelta->x = 0;
        } else {
            /*
             if (theDelta->x > -1)
             theDelta->x = -[grid actualSize];
             */
            if (theBounds->min.x + theDelta->x < theWorldBounds->min.x) {
                theDelta->x = theWorldBounds->min.x - theBounds->min.x;
                theDelta->y = 0;
                theDelta->z = 0;
            } else if (theLastPoint != NULL) {
                theLastPoint->x += theDelta->x;
            }
        }
    }
    
    if (theDelta->y > 0) {
        theDelta->y = [self snapToGridf:theBounds->max.y + theDelta->y] - theBounds->max.y;
        if (theDelta->y <= 0) {
            theDelta->y = 0;
        } else {
            /*
             if (theDelta->y < 1)
             theDelta->y = [grid actualSize];
             */
            if (theBounds->max.y + theDelta->y > theWorldBounds->max.y) {
                theDelta->x = 0;
                theDelta->y = theWorldBounds->max.y - theBounds->max.y;
                theDelta->z = 0;
            } else if (theLastPoint != NULL) {
                theLastPoint->y += theDelta->y;
            }
        }
    } else if (theDelta->y < 0) {
        theDelta->y = [self snapToGridf:theBounds->min.y + theDelta->y] - theBounds->min.y;
        if (theDelta->y >= 0) {
            theDelta->y = 0;
        } else {
            /*
             if (theDelta->y > -1)
             theDelta->y = -[grid actualSize];
             */
            if (theBounds->min.y + theDelta->y < theWorldBounds->min.y) {
                theDelta->x = 0;
                theDelta->y = theWorldBounds->min.y - theBounds->min.y;
                theDelta->z = 0;
            } else if (theLastPoint != NULL) {
                theLastPoint->y += theDelta->y;
            }
        }
    }
    
    if (theDelta->z > 0) {
        theDelta->z = [self snapToGridf:theBounds->max.z + theDelta->z] - theBounds->max.z;
        if (theDelta->z <= 0) {
            theDelta->z = 0;
        } else {
            /*
             if (theDelta->z < 1)
             theDelta->z = [grid actualSize];
             */
            if (theBounds->max.z + theDelta->z > theWorldBounds->max.z) {
                theDelta->x = 0;
                theDelta->y = 0;
                theDelta->z = theWorldBounds->max.z - theBounds->max.z;
            } else if (theLastPoint != NULL) {
                theLastPoint->z += theDelta->z;
            }
        }
    } else if (theDelta->z < 0) {
        theDelta->z = [self snapToGridf:theBounds->min.z + theDelta->z] - theBounds->min.z;
        if (theDelta->z >= 0) {
            theDelta->z = 0;
        } else {
            /*
             if (theDelta->z > -1)
             theDelta->z = -[grid actualSize];
             */
            if (theBounds->min.z + theDelta->z < theWorldBounds->min.z) {
                theDelta->x = 0;
                theDelta->y = 0;
                theDelta->z = theWorldBounds->min.z < theBounds->min.z;
            } else if (theLastPoint != NULL) {
                theLastPoint->z += theDelta->z;
            }
        }
    }
}

- (void)moveDeltaForVertex:(const TVector3f *)theVertex worldBounds:(const TBoundingBox *)theWorldBounds delta:(TVector3f *)theDelta lastPoint:(TVector3f *)theLastPoint {
    TVector3f original = *theVertex;
    float d;
    
    if (theDelta->x > 0) {
        theDelta->x = [self snapToGridf:original.x + theDelta->x] - original.x;
        if (theDelta->x <= 0) {
            theDelta->x = 0;
        } else {
            /*
             if (theDelta->x < 1)
             theDelta->x = [grid actualSize];
             */
            if (original.x + theDelta->x > theWorldBounds->max.x) {
                theDelta->x = theWorldBounds->max.x - original.x;
                theDelta->y = 0;
                theDelta->z = 0;
            } else if (theLastPoint != NULL) {
                theLastPoint->x += theDelta->x;
            }
        }
    } else if (theDelta->x < 0) {
        theDelta->x = [self snapToGridf:original.x + theDelta->x] - original.x;
        if (theDelta->x >= 0) {
            theDelta->x = 0;
        } else {
            /*
             if (theDelta->x > -1)
             theDelta->x = -[grid actualSize];
             */
            if (original.x + theDelta->x < theWorldBounds->min.x) {
                theDelta->x = theWorldBounds->min.x - original.x;
                theDelta->y = 0;
                theDelta->z = 0;
            } else if (theLastPoint != NULL) {
                theLastPoint->x += theDelta->x;
            }
        }
    }
    
    d = original.x + theDelta->x - (int)(original.x + theDelta->x);
    if (d != 0) {
        theDelta->x -= d;
        if (theLastPoint != NULL)
            theLastPoint->x -= d;
    }
    
    if (theDelta->y > 0) {
        theDelta->y = [self snapToGridf:original.y + theDelta->y] - original.y;
        if (theDelta->y <= 0) {
            theDelta->y = 0;
        } else {
            /*
             if (theDelta->y < 1)
             theDelta->y = [grid actualSize];
             */
            if (original.y + theDelta->y > theWorldBounds->max.y) {
                theDelta->y = theWorldBounds->max.y - original.y;
                theDelta->y = 0;
                theDelta->z = 0;
            } else if (theLastPoint != NULL) {
                theLastPoint->y += theDelta->y;
            }
        }
    } else if (theDelta->y < 0) {
        theDelta->y = [self snapToGridf:original.y + theDelta->y] - original.y;
        if (theDelta->y >= 0) {
            theDelta->y = 0;
        } else {
            /*
             if (theDelta->y > -1)
             theDelta->y = -[grid actualSize];
             */
            if (original.y + theDelta->y < theWorldBounds->min.y) {
                theDelta->y = theWorldBounds->min.y - original.y;
                theDelta->y = 0;
                theDelta->z = 0;
            } else if (theLastPoint != NULL) {
                theLastPoint->y += theDelta->y;
            }
        }
    }
    
    d = original.y + theDelta->y - (int)(original.y + theDelta->y);
    if (d != 0) {
        theDelta->y -= d;
        if (theLastPoint != NULL)
            theLastPoint->y -= d;
    }
    
    d = original.x + theDelta->x - (int)(original.x + theDelta->x);
    if (d != 0) {
        theDelta->x -= d;
        if (theLastPoint != NULL)
            theLastPoint->x -= d;
    }
    
    if (theDelta->z > 0) {
        theDelta->z = [self snapToGridf:original.z + theDelta->z] - original.z;
        if (theDelta->z <= 0) {
            theDelta->z = 0;
        } else {
            /*
             if (theDelta->z < 1)
             theDelta->z = [grid actualSize];
             */
            if (original.z + theDelta->z > theWorldBounds->max.z) {
                theDelta->z = theWorldBounds->max.z - original.z;
                theDelta->y = 0;
                theDelta->z = 0;
            } else if (theLastPoint != NULL) {
                theLastPoint->z += theDelta->z;
            }
        }
    } else if (theDelta->z < 0) {
        theDelta->z = [self snapToGridf:original.z + theDelta->z] - original.z;
        if (theDelta->z >= 0) {
            theDelta->z = 0;
        } else {
            /*
             if (theDelta->z > -1)
             theDelta->z = -[grid actualSize];
             */
            if (original.z + theDelta->z < theWorldBounds->min.z) {
                theDelta->z = theWorldBounds->min.z - original.z;
                theDelta->y = 0;
                theDelta->z = 0;
            } else if (theLastPoint != NULL) {
                theLastPoint->z += theDelta->z;
            }
        }
    }
    
    d = original.z + theDelta->z - (int)(original.z + theDelta->z);
    if (d != 0) {
        theDelta->z -= d;
        if (theLastPoint != NULL)
            theLastPoint->z -= d;
    }
}

- (float)dragDeltaForFace:(id <Face>)theFace delta:(TVector3f *)theDelta {
    NSAssert(theFace != nil, @"face must not be nil");
    NSAssert(theDelta != NULL, @"delta must not be NULL");
    
    float dist = dotV3f(theDelta, [theFace norm]);
    if (isnan(dist) || dist == 0)
        return NAN;
    
    // compute the rays which the vertices are moved on when the given face is dragged in the given direction
    const TVertexList* vertices = [theFace vertices];
    
    id <Brush> brush = [theFace brush];
    const TEdgeList* edges = [brush edges];
    
    TRay vertexRays[vertices->count];
    int rayIndex = 0;
    
    // look at each edge of the brush and test if exactly one of its end vertices belongs to the given face
    // if such an edge is detected, compute the ray which begins at the face vertex and points at the other
    // vertex
    for (int i = 0; i < edges->count; i++) {
        int c = 0;
        TRay ray;
        
        TEdge* e = edges->items[i];
        for (int j = 0; j < vertices->count; j++) {
            TVertex* v = vertices->items[j];
            
            if (v == e->startVertex) {
                c++;
                if (c == 1) {
                    ray.origin = e->startVertex->position;
                    ray.direction = e->endVertex->position;
                } else {
                    break;
                }
            } else if (v == e->endVertex) {
                c++;
                if (c == 1) {
                    ray.origin = e->endVertex->position;
                    ray.direction = e->startVertex->position;
                } else {
                    break;
                }
            }
        }
        
        if (c == 1) {
            subV3f(&ray.direction, &ray.origin, &ray.direction);
            normalizeV3f(&ray.direction, &ray.direction);
            
            // depending on the direction of the drag vector, the rays must be inverted to reflect the
            // actual movement of the vertices
            if (dist > 0)
                scaleV3f(&ray.direction, -1, &ray.direction);
            
            vertexRays[rayIndex++] = ray;
        }
    }
    
    TVector3f vertexDelta;
    int gridSkip = 0;
    float dragDist = FLT_MAX;
    do {
        // Find the smallest drag distance at which the face boundary is actually moved
        // by intersecting the vertex rays with the grid planes.
        // The distance of the vertex to the closest grid plane is then multiplied by the ray
        // direction to yield the vector by which the vertex would be moved if the face was dragged
        // and the drag would snap the vertex onto the previously selected grid plane.
        // This vector is then projected onto the face normal to yield the distance by which the face
        // must be dragged so that the vertex snaps to its closest grid plane.
        // Then, test if the resulting drag distance is smaller than the current candidate and if it is, see if
        // it is large enough so that the face boundary changes when the drag is applied.
        for (int i = 0; i < vertices->count; i++) {
            float vertexDist = [self intersectWithRay:&vertexRays[i] skip:gridSkip];
            scaleV3f(&vertexRays[i].direction, vertexDist, &vertexDelta);
            float vertexDragDist = dotV3f(&vertexDelta, [theFace norm]);
            
            if (fabsf(vertexDragDist) < fabsf(dragDist)) {
                MutableFace* testFace = [[MutableFace alloc] initWithWorldBounds:[theFace worldBounds] faceTemplate:theFace];
                [testFace dragBy:vertexDragDist lockTexture:NO];
                if (!equalPlane([theFace boundary], [testFace boundary]))
                    dragDist = vertexDragDist;
                [testFace release];
            }
        }
        
        // If we didn't find anything, the grid planes were too close to each vertex, so try the next grid planes.
        gridSkip++;
    } while (dragDist == FLT_MAX);
    
    if (fabsf(dragDist) > fabsf(dist))
        return NAN;
    
    return dragDist;
}

- (void)activateTexture {
    if (!valid[size]) {
        if (texIds[size] == 0)
            glGenTextures(1, &texIds[size]);
        
        int dim = [self actualSize];
        if (dim < 4)
            dim = 4;
        int texSize = 1 << 8; // 256 biggest grid size
        char pixel[texSize * texSize * 4];
            for (int y = 0; y < texSize; y++)
                for (int x = 0; x < texSize; x++) {
                    int i = (y * texSize + x) * 4;
                    if ((x % dim) == 0 || (y % dim) == 0) {
                        pixel[i + 0] = 0xFF;
                        pixel[i + 1] = 0xFF;
                        pixel[i + 2] = 0xFF;
                        pixel[i + 3] = 0xFF * alpha;
                    } else {
                        pixel[i + 0] = 0x00;
                        pixel[i + 1] = 0x00;
                        pixel[i + 2] = 0x00;
                        pixel[i + 3] = 0x00;
                    }
                }
        
        glBindTexture(GL_TEXTURE_2D, texIds[size]);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texSize, texSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixel);
        valid[size] = YES;
    } else {
        glBindTexture(GL_TEXTURE_2D, texIds[size]);
    }
}

- (void)deactivateTexture {
    glBindTexture(GL_TEXTURE_2D, 0);
}

- (void)dealloc {
    for (int i = 0; i < 9; i++)
        if (texIds[i] != 0)
            glDeleteTextures(1, &texIds[i]);
    [super dealloc];
}

@end
