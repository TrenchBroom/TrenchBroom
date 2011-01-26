//
//  SideEdge.m
//  TrenchBroom
//
//  Created by Kristian Duske on 25.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "SideEdge.h"
#import "Edge.h"

@implementation SideEdge
- (id)initWithEdge:(Edge *)theEdge flipped:(BOOL)isFlipped {
    if (theEdge == nil)
        [NSException raise:NSInvalidArgumentException format:@"edge must not be nil"];
    
    if (self = [self init]) {
        edge = [theEdge retain];
        flipped = isFlipped;
    }
    
    return self;
}


- (Vertex *)startVertex {
    return flipped ? [edge endVertex] : [edge startVertex];
}

- (Vertex *)endVertex {
    return flipped ? [edge startVertex] : [edge endVertex];
}

- (EEdgeMark)mark {
    return [edge mark];
}

- (Edge *)edge {
    return edge;
}

- (NSString *)description {
    NSMutableString* desc = [NSMutableString stringWithFormat:@"[start: %@, end: %@%@", [self startVertex], [self endVertex], flipped ? @" (flipped)" : @""];
    switch ([self mark]) {
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
    [edge release];
    [super dealloc];
}

@end
