//
//  Polygon2DIntersection.m
//  TrenchBroom
//
//  Created by Kristian Duske on 28.08.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "Polygon2DIS.h"

@implementation Polygon2DIS

- (id)initWithPolygon1:(Polygon2D *)polygon1 polygon2:(Polygon2D *)polygon2 {
    if (polygon1 == nil)
        [NSException raise:NSInvalidArgumentException format:@"polygon1 must not be nil"];
    if (polygon2 == nil)
        [NSException raise:NSInvalidArgumentException format:@"polygon2 must not be nil"];

    if ((self = [super init]) != nil) {
        polygon1LowerEdge = [polygon1 edges];
        polygon1UpperEdge = [polygon1LowerEdge previous];
        
        polygon2LowerEdge = [polygon2 edges];
        polygon2UpperEdge = [polygon2LowerEdge previous];
        
        if ([[polygon1LowerEdge startVertex] isSmallerThan:[polygon2LowerEdge startVertex]]) {
            float x = [[polygon2LowerEdge startVertex] x];
            polygon1UpperEdge = [self forward:polygon1UpperEdge to:x];
            polygon1LowerEdge = [self forward:polygon1LowerEdge to:x];
            [self handlePolygon2UpperEdge];
            [self handlePolygon2LowerEdge];
        } else {
            float x = [[polygon1LowerEdge startVertex] x];
            polygon2UpperEdge = [self forward:polygon2UpperEdge to:x];
            polygon2LowerEdge = [self forward:polygon2LowerEdge to:x];
            [self handlePolygon1UpperEdge];
            [self handlePolygon1LowerEdge];
        }
    }

    return self;
}

- (Polygon2D *)intersection {
    int nextEvent;
    while ((nextEvent = [self nextEvent]) != -1) {
        switch (nextEvent) {
            case P1U: {
                if ([[polygon1UpperEdge previous] isUpper]) {
                    polygon1UpperEdge = [polygon1UpperEdge previous];
                    [self handlePolygon1UpperEdge];
                } else {
                    polygon1UpperEdge = nil;
                }
                break;
            }
            case P1L: {
                if ([[polygon1LowerEdge next] isLower]) {
                    polygon1LowerEdge = [polygon1LowerEdge next];
                    [self handlePolygon1LowerEdge];
                } else {
                    polygon1LowerEdge = nil;
                }
                break;
            }
            case P2U: {
                if ([[polygon2UpperEdge previous] isUpper]) {
                    polygon2UpperEdge = [polygon2UpperEdge previous];
                    [self handlePolygon2UpperEdge];
                } else {
                    polygon2UpperEdge = nil;
                }
                break;
            }
            case P2L: {
                if ([[polygon2LowerEdge next] isLower]) {
                    polygon2LowerEdge = [polygon2LowerEdge next];
                    [self handlePolygon2LowerEdge];
                } else {
                    polygon2LowerEdge = nil;
                }
                break;
            }
        }
    }
    
    // merge upper and lower
    [lastLowerEdge close:firstUpperEdge];
    [lastUpperEdge close:firstLowerEdge];
    
    // all edges now have retain count 1
    Polygon2D* polygon = [[Polygon2D alloc] initWithEdges:firstLowerEdge];
    [firstLowerEdge release]; // firstLowerEdge was retained by the polygon
    
    return [polygon autorelease];
}

- (void)handlePolygon1UpperEdge {
    float x = [[polygon1UpperEdge endVertex] x];
    float p1u = [[polygon1UpperEdge line] yAt:x];
    float p2u = [[polygon2UpperEdge line] yAt:x];
    float p2l = [[polygon2LowerEdge line] yAt:x];
    
    if ([Math is:p1u between:p2u and:p2l]) {
        [self addUpper:polygon1UpperEdge];
    }
    
    if ([polygon1UpperEdge intersectWith:polygon2LowerEdge] != nil) {
        if (p1u < p2l) {
            [self addUpper:polygon1UpperEdge];
            [self addLower:polygon2LowerEdge];
        }
    }
    
    if ([polygon1UpperEdge intersectWith:polygon2UpperEdge] != nil) {
        if (p1u < p2u) {
            [self addUpper:polygon2UpperEdge];
        } else {
            [self addUpper:polygon1UpperEdge];
        }
    }
}

- (void)handlePolygon1LowerEdge {
    float x = [[polygon1LowerEdge startVertex] x];
    float p1l = [[polygon1LowerEdge line] yAt:x];
    float p2u = [[polygon2UpperEdge line] yAt:x];
    float p2l = [[polygon2LowerEdge line] yAt:x];
    
    if ([Math is:p1l between:p2u and:p2l]) {
        [self addLower:polygon1LowerEdge];
    }
    
    if ([polygon1LowerEdge intersectWith:polygon2UpperEdge] != nil) {
        if (p1l > p2u) {
            [self addUpper:polygon2UpperEdge];
            [self addLower:polygon1LowerEdge];
        }
    }
    
    if ([polygon1LowerEdge intersectWith:polygon2LowerEdge] != nil) {
        if (p1l > p2l) {
            [self addLower:polygon2LowerEdge];
        } else {
            [self addLower:polygon1LowerEdge];
        }
    }
}

- (void)handlePolygon2UpperEdge {
    float x = [[polygon2UpperEdge endVertex] x];
    float p1u = [[polygon1UpperEdge line] yAt:x];
    float p1l = [[polygon1LowerEdge line] yAt:x];
    float p2u = [[polygon2UpperEdge line] yAt:x];
    
    if ([Math is:p2u between:p1u and:p1l]) {
        [self addUpper:polygon2UpperEdge];
    }
    
    if ([polygon2UpperEdge intersectWith:polygon1LowerEdge] != nil) {
        if (p2u < p1l) {
            [self addUpper:polygon2UpperEdge];
            [self addLower:polygon1LowerEdge];
        }
    }
    
    if ([polygon2UpperEdge intersectWith:polygon1UpperEdge] != nil) {
        if (p2u < p1u) {
            [self addUpper:polygon1UpperEdge];
        } else {
            [self addUpper:polygon2UpperEdge];
        }
    }
}

- (void)handlePolygon2LowerEdge {
    float x = [[polygon2LowerEdge startVertex] x];
    float p2l = [[polygon2LowerEdge line] yAt:x];
    float p1u = [[polygon1UpperEdge line] yAt:x];
    float p1l = [[polygon1LowerEdge line] yAt:x];
    
    if ([Math is:p2l between:p1u and:p1l]) {
        [self addLower:polygon2LowerEdge];
    }
    
    if ([polygon2LowerEdge intersectWith:polygon1UpperEdge] != nil) {
        if (p2l > p1u) {
            [self addUpper:polygon1UpperEdge];
            [self addLower:polygon2LowerEdge];
        }
    }
    
    if ([polygon2LowerEdge intersectWith:polygon1LowerEdge] != nil) {
        if (p2l > p1l) {
            [self addLower:polygon1LowerEdge];
        } else {
            [self addLower:polygon2LowerEdge];
        }
    }
    
}

- (void)addUpper:(Edge2D *)edge {
    if (lastUpperEdge == nil) {
        lastUpperEdge = [[Edge2D alloc] initWithLine:[edge line] norm:[edge norm]];
        firstUpperEdge = lastUpperEdge;
    } else {
        firstUpperEdge = [firstUpperEdge insertBeforeLine:[edge line] norm:[edge norm]];
    }
}
            
- (void)addLower:(Edge2D *)edge {
    if (firstLowerEdge == nil) {
        firstLowerEdge = [[Edge2D alloc] initWithLine:[edge line] norm:[edge norm]];
        lastLowerEdge = firstLowerEdge;
    } else {
        lastLowerEdge = [lastLowerEdge insertAfterLine:[edge line] norm:[edge norm]];
    }
}

- (int)nextEvent {
    if (polygon1UpperEdge == nil || polygon1LowerEdge == nil ||
        polygon2UpperEdge == nil || polygon2LowerEdge == nil)
        return -1;
    
    int nextIndex = P1U;
    Edge2D* nextEdge = polygon1UpperEdge;
    
    if ([[polygon1LowerEdge largeVertex] isSmallerThan:[nextEdge largeVertex]]) {
        nextIndex = P1L;
        nextEdge = polygon1LowerEdge;
    }
    
    if ([[polygon2UpperEdge largeVertex] isSmallerThan:[nextEdge largeVertex]]) {
        nextIndex = P2U;
        nextEdge = polygon2UpperEdge;
    }
    
    if ([[polygon2LowerEdge largeVertex] isSmallerThan:[nextEdge largeVertex]]) {
        nextIndex = P2L;
        nextEdge = polygon2LowerEdge;
    }
    
    return nextIndex;
}

- (Edge2D *)forward:(Edge2D *)edge to:(float)x {
    if ([edge containsX:x])
        return edge;
    if ([edge isLower]) {
        Edge2D* next = [edge next];
        if (next != nil && [next isUpper])
            return [self forward:next to:x];
    } else {
        Edge2D* previous = [edge previous];
        if (previous != nil && [previous isLower])
            return [self forward:previous to:x];
    }
    
    return nil;
}

@end
