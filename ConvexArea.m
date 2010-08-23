//
//  ConvexArea.m
//  TrenchBroom
//
//  Created by Kristian Duske on 23.04.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "ConvexArea.h"


@implementation ConvexArea

- (id)initWithNormal:(Vector3f *)n {
    if (!n)
        [NSException raise:NSInvalidArgumentException format:@"normal vector must not be nil"];
    
    if (self = [super init]) {
        startEdge = nil;
        normal = [[Vector3f alloc] initWithFloatVector:n];
    }
    
    return self;
}

- (BOOL)addLine:(Line *)line {
    if (!line)
        [NSException raise:NSInvalidArgumentException format:@"line must not be nil"];
    
    if (!startEdge) {
        startEdge = [[Edge alloc] initWithLine:line];
        return TRUE;
    }

    Edge *previous = nil;
    Edge *next = nil;
    Edge *current = startEdge;
    do {
        // find previous and next
        Vector3f* is = [current intersectionWithLine:line];
        if (is) {
            [is add:[line d]];
            Side s = [[current line] sideOfPoint:is up:normal];
            if (s == SLeft) {
                next = current;
            } else if (s == SRight) {
                previous = current;
            }
        }
        
        current = [current next];
    } while (!next && !previous && current && current != startEdge);
    
    if (previous) {
        Edge* newEdge = [previous insertAfter:line until:next];
        if ([startEdge previous]) { // area is bounded
            [startEdge release];
            startEdge = [newEdge retain];
        }
        
        return TRUE;
    }
    
    if (next) {
        Edge* newEdge = [next insertBefore:line];
        if (next == startEdge) {
            [startEdge release];
            startEdge = [newEdge retain];
        }
        
        return TRUE;
    }
    
    // line did not intersect with any segment
    if (![startEdge next] && ![startEdge previous]) { // 1 parallel edge
        Side side = [[startEdge line] sideOfPoint:[line p] up:normal];
        if ([[startEdge line] sameDirectionAs:line up:normal]) {
            if (side == SRight) {
                [startEdge release];
                startEdge = [[Edge alloc] initWithLine:line];
            }
            
            return TRUE;
        }
        
        if (side == SRight) {
            [startEdge insertAfter:line until:nil];
            return TRUE;
        }
        
        return FALSE;
    } else if (![startEdge previous] ^ ![startEdge previous]) { // 2 parallel edges
        Edge* first = startEdge;
        Edge* second = [first next];
        Side firstSide = [[first line] sideOfPoint:[line p] up:normal];
        Side secondSide = [[second line] sideOfPoint:[line p] up:normal];
        
        if (firstSide == SLeft && [[first line] sameDirectionAs:line up:normal])
            return TRUE;
        
        if (secondSide == SRight && [[second line] sameDirectionAs:line up:normal])
            return TRUE;
        
        if (firstSide == SRight && secondSide == SLeft) {
            if ([[first line] sameDirectionAs:line up:normal]) {
                Edge* newEdge = [first replaceWith:line];
                [startEdge release];
                startEdge = [newEdge retain];
            } else {
                [second replaceWith:line];
            }
            
            return TRUE;
        }
        
        return FALSE;
    } else {
        // line is outside of the convex area - does it nullify it?
        Vector3f* vertex = nil;
        Edge* current = startEdge;
        do {
            vertex = [current startVertex];
            current = [current next];
        } while (!vertex && current && current != startEdge);
        
        if (!vertex)
            [NSException raise:NSInternalInconsistencyException format:@"convex area has more than two edges, but no vertices"];
        
        return [line sideOfPoint:vertex up:normal] == SRight;
    }
}

- (NSArray *)vertices {
    NSMutableArray* vertices = [[NSMutableArray alloc] initWithCapacity:10];
    Edge* edge = startEdge;
    
    while (edge) {
        Vector3f* vertex = [edge startVertex];
        if (vertex)
            [vertices addObject:vertex];
        edge = [edge next];
    }
    
    return [vertices autorelease];
}

- (NSArray *)edges {
    NSMutableArray* edges = [[NSMutableArray alloc] initWithCapacity:10];
    Edge* edge = startEdge;

    while (edge) {
        [edges addObject:[edge line]];
        edge = [edge next];
    }
    
    return [edges autorelease];
}

- (void)dealloc {
    [normal release];
    [startEdge release];
    [super dealloc];
}

@end
