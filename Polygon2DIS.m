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
        
        if ([[polygon1UpperEdge startVertex] isSmallerThan:[polygon2UpperEdge startVertex]]) {
            float x = [[polygon2UpperEdge startVertex] x];
            polygon1UpperEdge = [self forward:polygon1UpperEdge to:x];
            polygon1LowerEdge = [self forward:polygon1LowerEdge to:x];
        } else {
            float x = [[polygon1UpperEdge startVertex] x];
            polygon2UpperEdge = [self forward:polygon2UpperEdge to:x];
            polygon2LowerEdge = [self forward:polygon2LowerEdge to:x];
        }
    }

    return self;
}

- (Polygon2D *)intersection {
    int nextIndex = 0;
    while (nextIndex = [self nextEvent] != -1) {
        switch (nextIndex) {
            case P1U: {
                Vector2f* is = [polygon1UpperEdge intersectWith:polygon2UpperEdge];
                if (is != nil) {
                    if ([polygon2UpperEdge contains:[polygon1UpperEdge smallVertex]]) {
                        [self addUpper:polygon1UpperEdge];
                        [self addUpper:polygon2UpperEdge];
                    } else {
                        [self addUpper:polygon2UpperEdge];
                        [self addUpper:polygon1UpperEdge];
                    }
                } else if ([polygon2UpperEdge contains:[polygon1UpperEdge smallVertex]] && [polygon2LowerEdge contains:[polygon1UpperEdge smallVertex]]) {
					[self addUpper:polygon1UpperEdge];
				}

                if ([[polygon1UpperEdge previous] isUpper])
                    polygon1UpperEdge = [polygon1UpperEdge previous];
                else
                    polygon1UpperEdge = nil;

                break;
            }
            case P1L: {
                Vector2f* is = [polygon1LowerEdge intersectWith:polygon2LowerEdge];
                if (is != nil) {
                    if ([polygon2LowerEdge contains:[polygon1LowerEdge smallVertex]]) {
                        [self addLower:polygon1LowerEdge];
                        [self addLower:polygon2LowerEdge];
                    } else {
                        [self addLower:polygon2LowerEdge];
                        [self addLower:polygon1LowerEdge];
                    }
                } else if ([polygon2UpperEdge contains:[polygon1LowerEdge smallVertex]] && [polygon2LowerEdge contains:[polygon1LowerEdge smallVertex]]) {
					[self addLower:polygon1LowerEdge];
				}
                
                if ([[polygon1LowerEdge next] isLower])
                    polygon1LowerEdge = [polygon1LowerEdge next];
                else
                    polygon1LowerEdge = nil;

                break;
            }
            case P2U: {
                Vector2f* is = [polygon2UpperEdge intersectWith:polygon1UpperEdge];
                if (is != nil) {
                    if ([polygon1UpperEdge contains:[polygon2UpperEdge smallVertex]]) {
                        [self addUpper:polygon2UpperEdge];
                        [self addUpper:polygon1UpperEdge];
                    } else {
                        [self addUpper:polygon1UpperEdge];
                        [self addUpper:polygon2UpperEdge];
                    }
                } else if ([polygon1UpperEdge contains:[polygon2UpperEdge smallVertex]] && [polygon1LowerEdge contains:[polygon2UpperEdge smallVertex]]) {
					[self addUpper:polygon2UpperEdge];
				}
                
                if ([[polygon2UpperEdge previous] isUpper])
                    polygon2UpperEdge = [polygon2UpperEdge previous];
                else
                    polygon2UpperEdge = nil;

                break;
            }
            case P2L: {
                Vector2f* is = [polygon2LowerEdge intersectWith:polygon1LowerEdge];
                if (is != nil) {
                    if ([polygon1LowerEdge contains:[polygon2LowerEdge smallVertex]]) {
                        [self addLower:polygon2LowerEdge];
                        [self addLower:polygon1LowerEdge];
                    } else {
                        [self addLower:polygon1LowerEdge];
                        [self addLower:polygon2LowerEdge];
                    }
                } else if ([polygon1UpperEdge contains:[polygon2LowerEdge smallVertex]] && [polygon1LowerEdge contains:[polygon2LowerEdge smallVertex]]) {
					[self addLower:polygon2LowerEdge];
				}

                if ([[polygon2LowerEdge next] isLower])
                    polygon2LowerEdge = [polygon2LowerEdge next];
                else
                    polygon2LowerEdge = nil;
                
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
    if (polygon1UpperEdge == nil && polygon1LowerEdge == nil || 
        polygon2UpperEdge == nil && polygon2LowerEdge == nil)
        return -1;
    
    int nextIndex = P1U;
    Edge2D* nextEdge = polygon1UpperEdge;
    
    if ([[polygon1LowerEdge endVertex] isSmallerThan:[nextEdge endVertex]]) {
        nextIndex = P1L;
        nextEdge = polygon1LowerEdge;
    }
    
    if ([[polygon2UpperEdge endVertex] isSmallerThan:[nextEdge endVertex]]) {
        nextIndex = P2U;
        nextEdge = polygon2UpperEdge;
    }
    
    if ([[polygon2LowerEdge endVertex] isSmallerThan:[nextEdge endVertex]]) {
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
