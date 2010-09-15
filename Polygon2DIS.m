//
//  Polygon2DIntersection.m
//  TrenchBroom
//
//  Created by Kristian Duske on 28.08.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "Polygon2DIS.h"

@interface Polygon2DIS (private)

- (Edge2D *)forward:(Edge2D *)edge to:(float)x;
- (int)nextEvent;
- (void)addUpper:(Edge2D *)edge;
- (void)addLower:(Edge2D *)edge;
- (BOOL)intersectEdge:(Edge2D *)edge1 with:(Edge2D *)edge2;
- (void)handleUpperEdge:(Edge2D *)upperEdge otherUpperEdge:(Edge2D *)otherUpperEdge otherLowerEdge:(Edge2D *)otherLowerEdge;
- (void)handleLowerEdge:(Edge2D *)lowerEdge otherUpperEdge:(Edge2D *)otherUpperEdge otherLowerEdge:(Edge2D *)otherLowerEdge;

@end

@implementation Polygon2DIS (private)

- (BOOL)intersectEdge:(Edge2D *)edge1 with:(Edge2D *)edge2 {
    Vector2f* is = [edge1 intersectWith:edge2];
    if (is == nil)
        return NO;
    if ([is isEqual:[edge1 largeVertex]])
        return NO;
    if ([is isEqual:[edge2 largeVertex]])
        return NO;
    return YES;
}

- (void)handleUpperEdge:(Edge2D *)upperEdge 
         otherUpperEdge:(Edge2D *)otherUpperEdge 
         otherLowerEdge:(Edge2D *)otherLowerEdge {
    float x = [[upperEdge endVertex] x];
    float y = [[upperEdge boundary] yAt:x];
    float otherUpperEdgeY = [[otherUpperEdge boundary] yAt:x];
    float otherLowerEdgeY = [[otherLowerEdge boundary] yAt:x];
    
    if (finxx(y, otherLowerEdgeY, otherUpperEdgeY)) {
        [self addUpper:upperEdge];
    }
    
    if ([self intersectEdge:upperEdge with:otherLowerEdge]) {
        if (y < otherLowerEdgeY) {
            [self addUpper:upperEdge];
            [self addLower:otherLowerEdge];
        }
    }
    
    if ([self intersectEdge:upperEdge with:otherUpperEdge]) {
        if (y < otherUpperEdgeY) {
            [self addUpper:otherUpperEdge];
        } else {
            [self addUpper:upperEdge];
        }
    }
}

- (void)handleLowerEdge:(Edge2D *)lowerEdge 
         otherUpperEdge:(Edge2D *)otherUpperEdge 
         otherLowerEdge:(Edge2D *)otherLowerEdge {
    float x = [[lowerEdge startVertex] x];
    float y = [[lowerEdge boundary] yAt:x];
    float otherUpperEdgeY = [[otherUpperEdge boundary] yAt:x];
    float otherLowerEdgeY = [[otherLowerEdge boundary] yAt:x];
    
    if (finxx(y, otherLowerEdgeY, otherUpperEdgeY)) {
        [self addLower:lowerEdge];
    }
    
    if ([self intersectEdge:lowerEdge with:otherUpperEdge]) {
        if (y > otherUpperEdgeY) {
            [self addUpper:otherUpperEdge];
            [self addLower:lowerEdge];
        }
    }
    
    if ([self intersectEdge:lowerEdge with:otherLowerEdge]) {
        if (y > otherLowerEdgeY) {
            [self addLower:otherLowerEdge];
        } else {
            [self addLower:lowerEdge];
        }
    }
}

- (void)addUpper:(Edge2D *)edge {
    if (lastUpperEdge == nil) {
        lastUpperEdge = [[Edge2D alloc] initWithBoundary:[edge boundary] outside:[edge outside]];
        firstUpperEdge = lastUpperEdge;
    } else {
        firstUpperEdge = [firstUpperEdge prependEdgeWithBoundary:[edge boundary] outside:[edge outside]];
    }
}

- (void)addLower:(Edge2D *)edge {
    if (firstLowerEdge == nil) {
        firstLowerEdge = [[Edge2D alloc] initWithBoundary:[edge boundary] outside:[edge outside]];
        lastLowerEdge = firstLowerEdge;
    } else {
        lastLowerEdge = [lastLowerEdge appendEdgeWithBoundary:[edge boundary] outside:[edge outside]];
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
    if (finix(x, [[edge startVertex] x], [[edge endVertex] x]))
        return edge;
    if ([edge isLower]) {
        Edge2D* next = [edge next];
        if (next != nil && [next isLower])
            return [self forward:next to:x];
    } else {
        Edge2D* previous = [edge previous];
        if (previous != nil && [previous isUpper])
            return [self forward:previous to:x];
    }
    
    return nil;
}

@end

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
            [self handleUpperEdge:polygon2UpperEdge 
                   otherUpperEdge:polygon1UpperEdge 
                   otherLowerEdge:polygon1LowerEdge];
            [self handleLowerEdge:polygon2LowerEdge 
                   otherUpperEdge:polygon1UpperEdge
                   otherLowerEdge:polygon1LowerEdge];
        } else {
            float x = [[polygon1LowerEdge startVertex] x];
            polygon2UpperEdge = [self forward:polygon2UpperEdge to:x];
            polygon2LowerEdge = [self forward:polygon2LowerEdge to:x];
            [self handleUpperEdge:polygon1UpperEdge 
                   otherUpperEdge:polygon2UpperEdge 
                   otherLowerEdge:polygon2LowerEdge];
            [self handleLowerEdge:polygon1LowerEdge 
                   otherUpperEdge:polygon2UpperEdge 
                   otherLowerEdge:polygon2LowerEdge];
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
                    [self handleUpperEdge:polygon1UpperEdge 
                           otherUpperEdge:polygon2UpperEdge 
                           otherLowerEdge:polygon2LowerEdge];
                } else {
                    polygon1UpperEdge = nil;
                }
                break;
            }
            case P1L: {
                if ([[polygon1LowerEdge next] isLower]) {
                    polygon1LowerEdge = [polygon1LowerEdge next];
                    [self handleLowerEdge:polygon1LowerEdge 
                           otherUpperEdge:polygon2UpperEdge
                           otherLowerEdge:polygon2LowerEdge];
                } else {
                    polygon1LowerEdge = nil;
                }
                break;
            }
            case P2U: {
                if ([[polygon2UpperEdge previous] isUpper]) {
                    polygon2UpperEdge = [polygon2UpperEdge previous];
                    [self handleUpperEdge:polygon2UpperEdge 
                           otherUpperEdge:polygon1UpperEdge 
                           otherLowerEdge:polygon1LowerEdge];
                } else {
                    polygon2UpperEdge = nil;
                }
                break;
            }
            case P2L: {
                if ([[polygon2LowerEdge next] isLower]) {
                    polygon2LowerEdge = [polygon2LowerEdge next];
                    [self handleLowerEdge:polygon2LowerEdge 
                           otherUpperEdge:polygon1UpperEdge 
                           otherLowerEdge:polygon1LowerEdge];
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

@end
