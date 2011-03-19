//
//  Edge.m
//  TrenchBroom
//
//  Created by Kristian Duske on 24.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "Edge.h"
#import "SideEdge.h"
#import "Side.h"
#import "Vertex.h"
#import "Face.h"
#import "MutableFace.h"
#import "Line3D.h"
#import "Plane3D.h"
#import "Ray3D.h"
#import "Vector3f.h"
#import "MathCache.h"
#import "Math.h"
#import "BoundingBox.h"

static float HANDLE_RADIUS = 2.0f;

@implementation Edge
- (id)init {
    if (self = [super init]) {
        mark = EM_NEW;
    }
    
    return self;
}

- (id)initWithStartVertex:(Vertex *)theStartVertex endVertex:(Vertex *)theEndVertex {
    if (theStartVertex == nil)
        [NSException raise:NSInvalidArgumentException format:@"start vertex must not be nil"];
    if (theEndVertex == nil)
        [NSException raise:NSInvalidArgumentException format:@"end vertex must not be nil"];
    
    if (self = [self init]) {
        startVertex = [theStartVertex retain];
        endVertex = [theEndVertex retain];
    }
    
    return self;
}

- (Vertex *)startVertex {
    return startVertex;
}

- (Vertex *)endVertex {
    return endVertex;
}

- (id <Face>)leftFace {
    return [[leftEdge side] face];
}

- (id <Face>)rightFace {
    return [[rightEdge side] face];
}

- (void)setLeftEdge:(SideEdge *)theLeftEdge {
    if (leftEdge != nil)
        [NSException raise:NSInvalidArgumentException format:@"left edge is already set"];
    
    leftEdge = [theLeftEdge retain];
}
- (void)setRightEdge:(SideEdge *)theRightEdge {
    if (rightEdge != nil)
        [NSException raise:NSInvalidArgumentException format:@"right edge is already set"];
    
    rightEdge = [theRightEdge retain];
}

- (Vertex *)splitAt:(Plane3D *)plane {
    if (mark != EM_SPLIT)
        [NSException raise:NSInvalidArgumentException format:@"cannot split edge that is not marked with EM_SPLIT"];

    MathCache* cache = [MathCache sharedCache];
    Line3D* line = [cache line3D];
    [line setPoint1:[startVertex vector] point2:[endVertex vector]];
    
    Vector3f* newVector = [line pointAtDistance:[plane intersectWithLine:line]];
    Vertex* newVertex = [[Vertex alloc] initWithVector:newVector];

    if ([startVertex mark] == VM_DROP) {
        [startVertex release];
        startVertex = [newVertex retain];
    } else {
        [endVertex release];
        endVertex = [newVertex retain];
    }

    [cache returnLine3D:line];
    return [newVertex autorelease];
}

- (PickingHit *)pickWithRay:(Ray3D *)theRay {
    MathCache* cache = [MathCache sharedCache];
    Vector3f* va = [cache vector3f];
    Vector3f* dp = [cache vector3f];
    Vector3f* r1 = [cache vector3f];
    Vector3f* r2 = [cache vector3f];
    Vector3f* t = [cache vector3f];
    Plane3D* pl = [cache plane3D];
    
    @try {
        
        // calculate intersection between an infinite cylinder and the ray
        Vector3f* p = [theRay origin];
        Vector3f* v = [theRay direction];
        
        Vector3f* pa = [startVertex vector];
        [va setFloat:[endVertex vector]];
        [va sub:[startVertex vector]];
        [va normalize];
        
        [dp setFloat:p];
        [dp sub:pa];
        
        [t setFloat:va];
        [t scale:[va dot:v]];
        [r1 setFloat:v];
        [r1 sub:t];
        
        [t setFloat:va];
        [t scale:[dp dot:va]];
        [r2 setFloat:dp];
        [r2 sub:t];
        
        float a = [r1 lengthSquared];
        float b = 2 * [r2 dot:r1];
        float c = [r2 lengthSquared] - HANDLE_RADIUS * HANDLE_RADIUS;
        
        float p2 = b / a / 2;
        float q = c / a;
        
        float d = p2 * p2 - q;
        
        float t1 = MAXFLOAT;
        float t2 = MAXFLOAT;
        if (d >= 0) {
            float s = sqrt(d);
            t1 = - p2 + s;
            t2 = - p2 - s;
        }

        // check if the found intersections (if any) are above the upper cap
        [pl setPoint:[endVertex vector] norm:va]; // upper cap

        Vector3f* is1 = nil;
        Vector3f* is2 = nil;
        if (t1 >= 0 && t1 != MAXFLOAT) {
            is1 = [theRay pointAtDistance:t1];
            if ([pl isPointAbove:is1])
                t1 = MAXFLOAT;
        } else {
            t1 = MAXFLOAT;
        }
        
        if (t2 >= 0 && t2 != MAXFLOAT) {
            is2 = [theRay pointAtDistance:t2];
            if ([pl isPointAbove:is2])
                t2 = MAXFLOAT;
        } else {
            t2 = MAXFLOAT;
        }
        
        // calculate intersection between the upper cap and the ray
        float t3 = [pl intersectWithRay:theRay];
        Vector3f* is3 = nil;
        if (!isnan(t3)) {
            is3 = [theRay pointAtDistance:t3];
            [t setFloat:is3];
            [t sub:[endVertex vector]];
            if ([t lengthSquared] > HANDLE_RADIUS * HANDLE_RADIUS)
                t3 = MAXFLOAT;
        } else {
            t3 = MAXFLOAT;
        }
        
        // check if the two cylinder intersections (if any) are below the lower cap
        [pl setPoint:[startVertex vector] norm:va]; // lower cap
        if (t1 != MAXFLOAT && ![pl isPointAbove:is1])
            t1 = MAXFLOAT;
        if (t2 != MAXFLOAT && ![pl isPointAbove:is2])
            t2 = MAXFLOAT;
        
        // calculate intersection between the lower cap and the ray
        float t4 = [pl intersectWithRay:theRay];
        Vector3f* is4 = nil;
        if (!isnan(t4)) {
            is4 = [theRay pointAtDistance:t4];
            [t setFloat:is4];
            [t sub:[startVertex vector]];
            if ([t lengthSquared] > HANDLE_RADIUS * HANDLE_RADIUS)
                t4 = MAXFLOAT;
        } else {
            t4 = MAXFLOAT;
        }
        
        // select the best candidate of all intersections
        float t = t1;
        Vector3f* is = is1;
        
        if (t2 < t) {
            t = t2;
            is = is2;
        }
        
        if (t3 < t) {
            t = t3;
            is = is3;
        }
        
        if (t4 < t) {
            t = t4;
            is = is4;
        }

        if (t == MAXFLOAT)
            return nil;
        
        return [[[PickingHit alloc] initWithObject:self type:HT_EDGE hitPoint:is distance:t] autorelease];
    } @finally {
        [cache returnVector3f:va];
        [cache returnVector3f:dp];
        [cache returnVector3f:r1];
        [cache returnVector3f:r2];
        [cache returnVector3f:t];
        [cache returnPlane3D:pl];
    }
}

- (void)expandBounds:(BoundingBox *)theBounds extremeVector:(Vector3f *)extreme axis:(Vector3f *)axis {
    MathCache* cache = [MathCache sharedCache];
    Vector3f* p = [cache vector3f];
    
    [p setFloat:extreme];
    [p scale:HANDLE_RADIUS];
    [p add:[startVertex vector]];
    [theBounds mergePoint:p];
    
    [p add:axis];
    [theBounds mergePoint:p];
    
    [p setFloat:extreme];
    [p scale:-HANDLE_RADIUS];
    [p add:[startVertex vector]];
    [theBounds mergePoint:p];
    
    [p add:axis];
    [theBounds mergePoint:p];
    
    [cache returnVector3f:p];
}

- (void)expandBounds:(BoundingBox *)theBounds {
    Vector3f* s = [startVertex vector];
    Vector3f* e = [endVertex vector];
    
    MathCache* cache = [MathCache sharedCache];
    if (feq([s x], [e x])) {
        Vector3f* min = [cache vector3f];
        Vector3f* max = [cache vector3f];

        [min setX:fminf([s x], [e x])];
        [min setY:[s y] - HANDLE_RADIUS];
        [min setZ:[s z] - HANDLE_RADIUS];
        [max setX:fmaxf([s x], [e x])];
        [max setY:[s y] + HANDLE_RADIUS];
        [max setZ:[s z] + HANDLE_RADIUS];

        [theBounds mergeMin:min max:max];
        [cache returnVector3f:min];
        [cache returnVector3f:max];
    } else if (feq([s y], [e y])) {
        Vector3f* min = [cache vector3f];
        Vector3f* max = [cache vector3f];

        [min setX:[s x] - HANDLE_RADIUS];
        [min setY:fminf([s y], [e y])];
        [min setZ:[s z] - HANDLE_RADIUS];
        [max setX:[s x] + HANDLE_RADIUS];
        [max setY:fmaxf([s y], [e y])];
        [max setZ:[s z] + HANDLE_RADIUS];

        [theBounds mergeMin:min max:max];
        [cache returnVector3f:min];
        [cache returnVector3f:max];
    } else if (feq([s z], [e z])) {
        Vector3f* min = [cache vector3f];
        Vector3f* max = [cache vector3f];

        [min setX:[s x] - HANDLE_RADIUS];
        [min setY:[s y] - HANDLE_RADIUS];
        [min setZ:fminf([s z], [e z])];
        [max setX:[s x] + HANDLE_RADIUS];
        [max setY:[s y] + HANDLE_RADIUS];
        [max setZ:fmaxf([s z], [e z])];

        [theBounds mergeMin:min max:max];
        [cache returnVector3f:min];
        [cache returnVector3f:max];
    } else {
        Vector3f* norm = [cache vector3f];
        Vector3f* extreme = [cache vector3f];
        Vector3f* axis = [cache vector3f];
        
        [axis setFloat:e];
        [axis sub:s];

        [norm setFloat:axis];
        [norm normalize];

        [extreme setFloat:norm];
        [extreme cross:[Vector3f xAxisPos]];
        [extreme normalize];
        [self expandBounds:theBounds extremeVector:extreme axis:axis];

        [extreme setFloat:norm];
        [extreme cross:[Vector3f yAxisPos]];
        [extreme normalize];
        [self expandBounds:theBounds extremeVector:extreme axis:axis];
        
        [extreme setFloat:norm];
        [extreme cross:[Vector3f zAxisPos]];
        [extreme normalize];
        [self expandBounds:theBounds extremeVector:extreme axis:axis];
        
        [cache returnVector3f:norm];
        [cache returnVector3f:extreme];
        [cache returnVector3f:axis];
    }
}

- (EEdgeMark)mark {
    return mark;
}

- (void)updateMark {
    EVertexMark s = [startVertex mark];
    EVertexMark e = [endVertex mark];
    
    if (s == VM_KEEP && e == VM_KEEP)
        mark = EM_KEEP;
    else if (s == VM_DROP && e == VM_DROP)
        mark = EM_DROP;
    else if ((s == VM_KEEP && e == VM_DROP) ||
             (s == VM_DROP && e == VM_KEEP))
        mark = EM_SPLIT;
    else
        mark = EM_UNKNOWN;
}


- (NSString *)description {
    NSMutableString* desc = [NSMutableString stringWithFormat:@"[start: %@, end: %@", startVertex, endVertex];
    switch (mark) {
        case EM_KEEP:
            [desc appendFormat:@", mark: EM_KEEP]"];
            break;
        case EM_DROP:
            [desc appendFormat:@", mark: EM_DROP]"];
            break;
        case EM_SPLIT:
            [desc appendFormat:@", mark: EM_SPLIT]"];
            break;
        case EM_NEW:
            [desc appendFormat:@", mark: EM_NEW]"];
            break;
        case EM_UNKNOWN:
            [desc appendFormat:@", mark: EM_UNKNOWN]"];
            break;
        default:
            [desc appendFormat:@", mark: invalid]"];
            break;
    }
    
    return desc;
}

- (void)dealloc {
    [startVertex release];
    [endVertex release];
    [leftEdge release];
    [rightEdge release];
    [super dealloc];
}

@end
