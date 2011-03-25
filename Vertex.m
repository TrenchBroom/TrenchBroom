//
//  Vertex.m
//  TrenchBroom
//
//  Created by Kristian Duske on 25.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "Vertex.h"
#import "Vector3f.h"
#import "BoundingBox.h"
#import "PickingHit.h"
#import "Ray3D.h"
#import "Plane3D.h"
#import "Math.h"

@implementation Vertex
- (id)initWithVector:(Vector3f *)theVector {
    if (theVector == nil)
        [NSException raise:NSInvalidArgumentException format:@"vector must not be nil"];
    
    if (self = [self init]) {
        vector = [theVector retain];
        mark = VM_NEW;
        edges = [[NSMutableSet alloc] init];
    }
    
    return self;
}

- (Vector3f *)vector {
    return vector;
}

- (void)addEdge:(Edge *)theEdge {
    NSAssert(theEdge != nil, @"edge must not be nil");
    [edges addObject:theEdge];
}

- (NSSet *)edges {
    return edges;
}

- (EVertexMark)mark {
    return mark;
}

- (void)setMark:(EVertexMark)theMark {
    mark = theMark;
}

- (PickingHit *)pickWithRay:(Ray3D *)theRay {
    Plane3D* plane = [[Plane3D alloc] init];
    Vector3f* min = [[Vector3f alloc] initWithFloatVector:vector];
    Vector3f* max = [[Vector3f alloc] initWithFloatVector:vector];
    [min subX:2 y:2 z:2];
    [max addX:2 y:2 z:2];
    
    Vector3f* direction = [theRay direction];
    
    BOOL hit = NO;
    float distance;
    Vector3f* is;
    
    if (!hit) {
        if ([direction x] > 0) {
            [plane setPoint:min norm:[Vector3f xAxisNeg]];
            distance = [plane intersectWithRay:theRay];
            is = [theRay pointAtDistance:distance];
            hit = is != nil && fgte([is y], [min y]) && flte([is y], [max y]) && fgte([is z], [min z]) && flte([is z], [max z]);
        } else if ([direction x] < 0) {
            [plane setPoint:max norm:[Vector3f xAxisPos]];
            distance = [plane intersectWithRay:theRay];
            is = [theRay pointAtDistance:distance];
            hit = is != nil && fgte([is y], [min y]) && flte([is y], [max y]) && fgte([is z], [min z]) && flte([is z], [max z]);
        }
    }
    
    if (!hit) {
        if ([direction y] > 0) {
            [plane setPoint:min norm:[Vector3f yAxisNeg]];
            distance = [plane intersectWithRay:theRay];
            is = [theRay pointAtDistance:distance];
            hit = is != nil && fgte([is x], [min x]) && flte([is x], [max x]) && fgte([is z], [min z]) && flte([is z], [max z]);
        } else if ([direction y] < 0) {
            [plane setPoint:max norm:[Vector3f yAxisPos]];
            distance = [plane intersectWithRay:theRay];
            is = [theRay pointAtDistance:distance];
            hit = is != nil && fgte([is x], [min x]) && flte([is x], [max x]) && fgte([is z], [min z]) && flte([is z], [max z]);
        }
    }
    
    if (!hit) {
        if ([direction z] > 0) {
            [plane setPoint:min norm:[Vector3f zAxisNeg]];
            distance = [plane intersectWithRay:theRay];
            is = [theRay pointAtDistance:distance];
            hit = is != nil && [is x] >= [min x] && [is x] <= [max x] && [is y] >= [min y] && [is y] <= [max y];
        } else if ([direction z] < 0) {
            [plane setPoint:max norm:[Vector3f zAxisPos]];
            distance = [plane intersectWithRay:theRay];
            is = [theRay pointAtDistance:distance];
            hit = is != nil && [is x] >= [min x] && [is x] <= [max x] && [is y] >= [min y] && [is y] <= [max y];
        }
    }
    
    [plane release];
    [min release];
    [max release];
    
    if (!hit)
        return nil;
    
    return [[[PickingHit alloc] initWithObject:self type:HT_VERTEX hitPoint:is distance:distance] autorelease];
}

- (NSString *)description {
    NSMutableString* desc = [NSMutableString stringWithFormat:@"[vector: %@]", vector];
    switch (mark) {
        case VM_KEEP:
            [desc appendFormat:@", mark: VM_KEEP"];
            break;
        case VM_DROP:
            [desc appendFormat:@", mark: VM_DROP"];
            break;
        case VM_NEW:
            [desc appendFormat:@", mark: VM_NEW"];
            break;
        case VM_UNKNOWN:
            [desc appendFormat:@", mark: VM_UNKNOWN"];
            break;
        default:
            [desc appendFormat:@", mark: invalid"];
            break;
    }
    
    return desc;
}

- (void)dealloc {
    [vector release];
    [edges release];
    [super dealloc];
}

@end
