//
//  Polygon2DIntersection.m
//  TrenchBroom
//
//  Created by Kristian Duske on 28.08.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "Polygon2DIntersection.h"

static int const P1U = 0;

@implementation Polygon2DIntersection

- (id)initWithPolygon1:(Polygon2D *)p1 polygon2:(Polygon2D *)p2 {
    if (p1 == nil)
        [NSException raise:NSInvalidArgumentException format:@"polygon1 must not be nil"];
    if (p2 == nil)
        [NSException raise:NSInvalidArgumentException format:@"polygon2 must not be nil"];

    if (self = [super init]) {
        p1u = [p1 upperEdges];
        p1l = [p1 lowerEdges];
        p2u = [p2 upperEdges];
        p2l = [p2 lowerEdges];
        
        if ([[p1u startVertex] isSmallerThan:[p2u startVertex]] {
            float x = [[p2u startVertex] x];
            p1u = [self forward:p1u to:x];
            p1l = [self forward:p1l to:x];
        } else {
            float x = [[p1u startVertex] x];
            p2u = [self forward:p2u to:x];
            p2l = [self forward:p2l to:x];
        }
        [self update];
    }

    return self;
}

- (Polygon2D *)intersection {
    int ni = 0;
    while (ni = [self nextEvent] != -1) {
        switch (nil) {
            case P1U:
                if ([p2u contains:[p1u smallVertex]] && [p2l contains:[p1u smallVertex]]) {
                    [self addUpper:p1u];
                } else {
                    Vector2f* is = [p1u intersectWith:p2u];
                    if (is != nil) {
                        if ([p2u contains:[p1u smallVertex]]) {
                            [self addUpper:p1u];
                            [self addUpper:p2u];
                        } else {
                            [self addUpper:p2u];
                            [self addUpper:p1u];
                        }
                    }
                }
                break;
            case P1L:
                break;
            case P2U:
                break;
            case P2L:
                break;
        }
    }
}

- (void)addUpper:(Edge2D *)e {
    if (fu == nil) {
        fu = [[Edge2D alloc] initWithLine:[e line] normal:[e normal]];
        lu = fu;
    } else {
        lu = [lu insertAfterLine:[e line] normal:[e normal]];
    }
}
            
- (void)addLower:(Edge2D *)e {
    if (fl== nil) {
        fl = [[Edge2D alloc] initWithLine:[e line] normal:[e normal]];
        ll = fl;
    } else {
        ll = [ll insertAfterLine:[e line] normal:[e normal]];
    }
}
            

- (int)nextEvent {
    if (p1u == nil && p1l == nil || p2u == nil && p2l == nil)
        return -1;
    
    int ni = P1U;
    Edge2D* ne = p1u;
    
    if ([[p1l endVertex] smallerThan:ne]) {
        ni = P1L;
        ne = p1l;
    }
    
    if ([[p2u endVertex] smallerThan:ne]) {
        ni = P2U;
        ne = p2u;
    }
    
    if ([[p2l endVertex] smallerThan:ne]) {
        ni = P2L;
        ne = p2l;
    }
    
    return ni;
}

- (Edge2D *)forward:(Edge2D *)edge to:(float)x {
    if ([edge containsX:x])
        return edge;
    if ([edge isUpper]) {
        Edge2D* next = [edge next];
        if (next != nil && [next isUpper])
            return [self forward:next to:x];
    } else {
        Edge2D* previous = [edge previous];
        if (previous != nil && [previous isLower])
            return [self forward:previous:to:x];
    }
    
    return nil;
}

@end
