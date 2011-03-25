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
#import "Math.h"
#import "BoundingBox.h"
#import "RenderContext.h"
#import "VBOBuffer.h"
#import "VBOMemBlock.h"
#import "IntData.h"
#import "PickingHit.h"

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
        [startVertex addEdge:self];
        endVertex = [theEndVertex retain];
        [endVertex addEdge:self];
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

    Line3D* line = [[Line3D alloc] initWithPoint1:[startVertex vector] point2:[endVertex vector]];
    Vector3f* newVector = [line pointAtDistance:[plane intersectWithLine:line]];
    [line release];
    
    Vertex* newVertex = [[Vertex alloc] initWithVector:newVector];
    [newVertex addEdge:self];

    if ([startVertex mark] == VM_DROP) {
        [startVertex release];
        startVertex = [newVertex retain];
    } else {
        [endVertex release];
        endVertex = [newVertex retain];
    }
    
    return [newVertex autorelease];
}

- (PickingHit *)pickWithRay:(Ray3D *)theRay {
    Vector3f* u = [[Vector3f alloc] initWithFloatVector:[endVertex vector]];
    Vector3f* v = [[Vector3f alloc] initWithFloatVector:[theRay direction]];
    Vector3f* w = [[Vector3f alloc] initWithFloatVector:[startVertex vector]];
    [u sub:[startVertex vector]];
    [w sub:[theRay origin]];
    
    float a = [u dot:u];
    float b = [u dot:v];
    float c = [v dot:v];
    float d = [u dot:w];
    float e = [v dot:w];
    float D = a * c - b * b;
    float ec, eN;
    float eD = D;
    float rc, rN;
    float rD = D;
    
    if (fzero(D)) {
        eN = 0;
        eD = 1;
        rN = e;
        rD = c;
    } else {
        eN = b * e - c * d;
        rN = a * e - b * d;
        if (fneg(eN)) { // point is beyond the start point of this edge
            eN = 0;
            rN = e;
            rD = c;
        } else if (fgt(eN, eD)) { // point is beyond the end point of this edge
            eN = eD;
            rN = e + b;
            rD = c;
        }
    }
    
    if (fneg(rN)) { // point before ray origin
        rN = 0;
        if (fneg(-d)) {
            eN = 0;
        } else if (fgt(-d, a)) {
            eN = eD;
        } else {
            eN = -d;
            eD = a;
        }
    }
    
    ec = fzero(eN) ? 0 : eN / eD;
    rc = fzero(rN) ? 0 : rN / rD;
    
    [u scale:ec];
    [v scale:rc];
    
    [w add:u];
    [w sub:v];
    
    
    if (flte([w lengthSquared], HANDLE_RADIUS * HANDLE_RADIUS)) {
        Vector3f* is = [theRay pointAtDistance:rc];
        return [[[PickingHit alloc] initWithObject:self type:HT_EDGE hitPoint:is distance:rc] autorelease];
    }
    
    [u release];
    [v release];
    [w release];
    
    return nil;
}

- (void)expandBounds:(BoundingBox *)theBounds {
    Vector3f* s = [startVertex vector];
    Vector3f* e = [endVertex vector];
    
    Vector3f* t = [[Vector3f alloc] initWithFloatVector:s];
    Vector3f* r = [[Vector3f alloc] initWithX:HANDLE_RADIUS y:HANDLE_RADIUS z:HANDLE_RADIUS];

    [t sub:r];
    [theBounds mergePoint:t];
    
    [t setFloat:s];
    [t add:r];
    [theBounds mergePoint:t];
    
    [t setFloat:e];
    [t sub:r];
    [theBounds mergePoint:t];
    
    [t setFloat:e];
    [t add:r];
    [theBounds mergePoint:t];
    
    [t release];
    [r release];
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
