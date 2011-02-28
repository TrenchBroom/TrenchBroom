//
//  Edge.m
//  TrenchBroom
//
//  Created by Kristian Duske on 24.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "Edge.h"
#import "Vertex.h"
#import "Line3D.h"
#import "Plane3D.h"
#import "Vector3f.h"
#import "MathCache.h"

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

- (Vertex *)splitAt:(Plane3D *)plane {
    if (mark != EM_SPLIT)
        [NSException raise:NSInvalidArgumentException format:@"cannot split edge that is not marked with EM_SPLIT"];

    MathCache* cache = [MathCache sharedCache];
    Line3D* line = [cache line3D];
    [line setPoint1:[startVertex vector] point2:[endVertex vector]];
    
    Vector3f* newVector = [plane intersectWithLine:line];
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
    [super dealloc];
}

@end
