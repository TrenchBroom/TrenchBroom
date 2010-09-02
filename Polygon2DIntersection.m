//
//  Polygon2DIntersection.m
//  TrenchBroom
//
//  Created by Kristian Duske on 28.08.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "Polygon2DIntersection.h"

@implementation Polygon2DIntersection

- (id)initWithPolygon1:(Polygon2D *)p1 polygon2:(Polygon2D *)p2 {
    if (p1 == nil)
        [NSException raise:NSInvalidArgumentException format:@"polygon1 must not be nil"];
    if (p2 == nil)
        [NSException raise:NSInvalidArgumentException format:@"polygon2 must not be nil"];

    if (self = [super init]) {
        p1l = [p1 edges];
        p1u = [p1l previous];
        
        p2l = [p2 edges];
        p2u = [p2l previous];
        
        if ([[p1u startVertex] isSmallerThan:[p2u startVertex]]) {
            float x = [[p2u startVertex] x];
            p1u = [self forward:p1u to:x];
            p1l = [self forward:p1l to:x];
        } else {
            float x = [[p1u startVertex] x];
            p2u = [self forward:p2u to:x];
            p2l = [self forward:p2l to:x];
        }
    }

    return self;
}

- (Polygon2D *)intersection {
    int ni = 0;
    while (ni = [self nextEvent] != -1) {
        switch (ni) {
            case P1U: {
                Vector2f* is = [p1u intersectWith:p2u];
                if (is != nil) {
                    if ([p2u contains:[p1u smallVertex]]) {
                        [self addUpper:p1u];
                        [self addUpper:p2u];
                    } else {
                        [self addUpper:p2u];
                        [self addUpper:p1u];
                    }
                } else if ([p2u contains:[p1u smallVertex]] && [p2l contains:[p1u smallVertex]]) {
					[self addUpper:p1u];
				}

                if ([[p1u previous] isUpper])
                    p1u = [p1u previous];
                else
                    p1u = nil;

                break;
            }
            case P1L: {
                Vector2f* is = [p1l intersectWith:p2l];
                if (is != nil) {
                    if ([p2l contains:[p1l smallVertex]]) {
                        [self addLower:p1l];
                        [self addLower:p2l];
                    } else {
                        [self addLower:p2l];
                        [self addLower:p1l];
                    }
                } else if ([p2u contains:[p1l smallVertex]] && [p2l contains:[p1l smallVertex]]) {
					[self addLower:p1l];
				}
                
                if ([[p1l next] isLower])
                    p1l = [p1l next];
                else
                    p1l = nil;

                break;
            }
            case P2U: {
                Vector2f* is = [p2u intersectWith:p1u];
                if (is != nil) {
                    if ([p1u contains:[p2u smallVertex]]) {
                        [self addUpper:p2u];
                        [self addUpper:p1u];
                    } else {
                        [self addUpper:p1u];
                        [self addUpper:p2u];
                    }
                } else if ([p1u contains:[p2u smallVertex]] && [p1l contains:[p2u smallVertex]]) {
					[self addUpper:p2u];
				}
                
                if ([[p2u previous] isUpper])
                    p2u = [p2u previous];
                else
                    p2u = nil;

                break;
            }
            case P2L: {
                Vector2f* is = [p2l intersectWith:p1l];
                if (is != nil) {
                    if ([p1l contains:[p2l smallVertex]]) {
                        [self addLower:p2l];
                        [self addLower:p1l];
                    } else {
                        [self addLower:p1l];
                        [self addLower:p2l];
                    }
                } else if ([p1u contains:[p2l smallVertex]] && [p1l contains:[p2l smallVertex]]) {
					[self addLower:p2l];
				}

                if ([[p2l next] isLower])
                    p2l = [p2l next];
                else
                    p2l = nil;
                
                break;
            }
        }
    }
    
    // merge upper and lower
    [ll close:fu];
    [lu close:fl];
    
    // all edges now have retain count 1
    Polygon2D* polygon = [[Polygon2D alloc] initWithEdges:fl];
    [fl release]; // fl was retained by the polygon
    
    return [polygon autorelease];
}

- (void)addUpper:(Edge2D *)e {
    if (lu == nil) {
        lu = [[Edge2D alloc] initWithLine:[e line] normal:[e normal]];
        fu = lu;
    } else {
        fu = [fu insertAfterLine:[e line] normal:[e normal]];
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
    
    if ([[p1l endVertex] isSmallerThan:[ne endVertex]]) {
        ni = P1L;
        ne = p1l;
    }
    
    if ([[p2u endVertex] isSmallerThan:[ne endVertex]]) {
        ni = P2U;
        ne = p2u;
    }
    
    if ([[p2l endVertex] isSmallerThan:[ne endVertex]]) {
        ni = P2L;
        ne = p2l;
    }
    
    return ni;
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
