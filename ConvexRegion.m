//
//  ConvexRegion.m
//  TrenchBroom
//
//  Created by Kristian Duske on 19.04.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "ConvexRegion.h"
#import "Math3D.h"
#import <math.h>

@implementation ConvexRegion(private)

- (void)addLeftEdge:(Line2D *)edge {
    if ([leftEdges count] == 0) {
        if ([rightEdges count] > 0) {
            Line2D* rightFirst = [rightEdges objectAtIndex:0];
            topVertex = [[rightFirst intersectionWithLine:edge] retain];
        }
    } else {
        Line2D* leftLast = [leftEdges lastObject];
        Vector2f* v = [leftLast intersectionWithLine:edge];
        if (v)
            [leftVertices addObject:v];
    }
    
    [leftEdges addObject:edge];
}

- (void)addRightEdge:(Line2D *)edge {
    if ([rightEdges count] == 0) {
        if ([leftEdges count] > 0) {
            Line2D* leftFirst = [leftEdges objectAtIndex:0];
            topVertex = [[leftFirst intersectionWithLine:edge] retain];
        }
    } else {
        Line2D* rightLast = [rightEdges lastObject];
        Vector2f* v = [rightLast intersectionWithLine:edge];
        if (v)
            [rightVertices addObject:v];
    }
    
    [rightEdges addObject:edge];
}

- (int)edgeAt:(float)y edges:(NSArray *)edges vertices:(NSArray *)vertices startingWith:(int)edgeIndex {
    if (edgeIndex < 0)
        edgeIndex = 0;
    
    for (int i = edgeIndex; i < [leftEdges count]; i++) {
        Vector2f* upper = [self upperBoundary:i edges:edges vertices:vertices];
        Vector2f* lower = [self lowerBoundary:i edges:edges vertices:vertices];
        
        if (!upper && !lower)
            return i;
        
        if (!upper && y >= [lower y])
            return i;
        
        if (!lower && y < [upper y])
            return i;
        
        if (y < [upper y] && y >= [lower y])
            return i;
    }
    
    return -1;
}

@end

@implementation ConvexRegion

- (id)init {
    if (self = [super init]) {
        leftEdges = [[NSMutableArray alloc] initWithCapacity:10];
        rightEdges = [[NSMutableArray alloc] initWithCapacity:10];
	}
	
	return self;
    
}

- (id)initWithEdge:(Line2D *)edge {
    if (!edge)
        return nil;
    
    if (self = [self init]) {
        [self addEdge:edge];
    }
    
    return self;
}

- (void)addEdge:(Line2D *)edge {
    Vector2f* d = [edge d];
    float x = [d x];
    float y = [d y];
    
    if (y > AlmostZero)
        [self addLeftEdge:edge];
    else if (y < -AlmostZero)
        [self addRightEdge:edge];
    else if (x > 0)
        [self addLeftEdge:edge];
    else
        [self addRightEdge:edge];
    
    if (!bottomVertex && [leftEdges count] > 0 && [rightEdges count] > 0) {
        Line2D* lastLeft = [leftEdges lastObject];
        Line2D* lastRight = [rightEdges lastObject];
        
        bottomVertex = [[lastLeft intersectionWithLine:lastRight] retain];
    }
}

- (int)leftEdgeAt:(float)y startingWith:(int)edgeIndex {
    return [self edgeAt:y edges:leftEdges vertices:leftVertices startingWith:edgeIndex];
}

- (int)rightEdgeAt:(float)y startingWith:(int)edgeIndex {
    return [self edgeAt:y edges:rightEdges vertices:rightVertices startingWith:edgeIndex];
}

- (Vector2f *)lowerBoundary:(int)edgeIndex edges:(NSArray *)edges vertices:(NSArray *)vertices {
    if (edgeIndex < 0 || edgeIndex >= [edges count])
        return nil;
    
    if (edgeIndex == [edges count] - 1)
        return bottomVertex;
    
    return [vertices objectAtIndex:edgeIndex];
}

- (Vector2f *)upperBoundary:(int)edgeIndex edges:(NSArray *)edges vertices:(NSArray *)vertices {
    if (edgeIndex < 0 || edgeIndex >= [edges count])
        return nil;
    
    if (edgeIndex == 0)
        return topVertex;
    
    return [vertices objectAtIndex:edgeIndex - 1];
}

- (float)top {
    if (!topVertex)
        return INFINITY;
    
    return [topVertex y];
}

- (void)dealloc {
    [leftEdges release];
    [rightEdges release];
    [topVertex release];
    [bottomVertex release];
    [leftVertices release];
    [rightVertices release];
    [super dealloc];
}
@end
