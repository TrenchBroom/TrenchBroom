//
//  Vertex.m
//  TrenchBroom
//
//  Created by Kristian Duske on 25.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "Vertex.h"
#import "Vector3f.h"

@implementation Vertex
- (id)initWithVector:(Vector3f *)theVector {
    if (theVector == nil)
        [NSException raise:NSInvalidArgumentException format:@"vector must not be nil"];
    
    if (self = [self init]) {
        vector = [theVector retain];
        mark = VM_NEW;
    }
    
    return self;
}

- (Vector3f *)vector {
    return vector;
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
    [super dealloc];
}

@end
