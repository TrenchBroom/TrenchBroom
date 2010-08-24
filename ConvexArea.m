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
        numEdges = 0;
    }
    
    return self;
}

- (BOOL)addLine:(Line *)line {
    if (!line)
        [NSException raise:NSInvalidArgumentException format:@"line must not be nil"];
    
    if (startEdge == nil) {
        startEdge = [[Edge alloc] initWithLine:line];
        numEdges = 1;
        return TRUE;
    }
    
    if ([startEdge next] == nil) {
        Vector3f* is = [startEdge intersectionWithLine:line];
        Line* startLine = [startEdge line];
        if (is) {
            Side s = [startLine turnDirectionTo:line normal:normal];
            if (s == SRight) {
                [startEdge insertAfter:line until:nil];
            } else {
                Edge* newStart = [startEdge insertBefore:line];
                [startEdge release];
                startEdge = [newStart retain];
            }
            return TRUE;
        }
        
        // boundary lines are parallel
        if ([startLine sameDirectionAs:line normal:normal]) {
            if ([startLine sideOfPoint:[line p] normal:normal] == SRight) {
                [startEdge release];
                startEdge = [[Edge alloc] initWithLine:line];
            }
            return YES;
        }
        
        if ([startLine sideOfPoint:[line p] normal:normal] == SRight) {
            [startEdge insertAfter:line until:nil];
            return YES;
        }
        
        return NO;
    }

    
    Edge* e1 = nil;
    Edge* e2 = nil;
    Edge* e = startEdge;
    do {
        if ([e intersectionWithLine:line]) {
            if (e1 == nil)
                e1 = e;
            else
                e2 = e;
        }
        e = [e next];
    } while ((e1 == nil || e2 == nil) && e != nil && e != startEdge);
    
    if (e1 == nil && e2 == nil) { // no intersections
        Vector3f* v = [startEdge endVertex];
        return [line sideOfPoint:v normal:normal] == SRight;
    }
    
    if (e1 != nil ^ e2 != nil) { // one intersection
        e = e1 != nil ? e1 : e2;
        if ([[e line] turnDirectionTo:line normal:normal] == SRight) {
            [e insertAfter:line until:nil];
        } else {
            Edge* newStart = [e insertBefore:line];
            [startEdge release];
            startEdge = [newStart retain];
        }
        
        return TRUE;
    }
    
    // two intersections
    if ([[e1 line] turnDirectionTo:line normal:normal] == SRight) {
        [e1 insertAfter:line until:e2];
    } else {
        if (e1 != startEdge) {
            [e1 retain];
            [startEdge release];
            startEdge = e1;
        }
        
        [e2 insertAfter:line until:nil];
    }

    return TRUE;
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
