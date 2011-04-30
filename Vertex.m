//
//  Vertex.m
//  TrenchBroom
//
//  Created by Kristian Duske on 25.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "Vertex.h"
#import "PickingHit.h"
#import "Math.h"

@implementation Vertex
- (id)initWithVector:(TVector3f *)theVector {
    if (self = [self init]) {
        vector = *theVector;
        mark = VM_NEW;
        edges = [[NSMutableSet alloc] init];
    }
    
    return self;
}

- (id)initWithX:(float)x y:(float)y z:(float)z {
    if (self = [self init]) {
        vector.x = x;
        vector.y = y;
        vector.z = z;
        mark = VM_NEW;
        edges = [[NSMutableSet alloc] init];
    }
    
    return self;
}

- (TVector3f *)vector {
    return &vector;
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

- (NSString *)description {
    NSMutableString* desc = [NSMutableString stringWithFormat:@"[vector: %@]", vector];
    switch (mark) {
        case VM_KEEP:
            [desc appendFormat:@", mark: VM_KEEP"];
            break;
        case VM_DROP:
            [desc appendFormat:@", mark: VM_DROP"];
            break;
        case VM_UNDECIDED:
            [desc appendFormat:@", mark: VM_UNDECIDED"];
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
    [edges release];
    [super dealloc];
}

@end
