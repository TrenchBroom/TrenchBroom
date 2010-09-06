//
//  Polygon2DTestCase.m
//  TrenchBroom
//
//  Created by Kristian Duske on 28.08.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "Polygon2DTestCase.h"
#import "Vector2f.h"
#import "Polygon2D.h"
#import "Edge2D.h"

@implementation Polygon2DTestCase

- (void)assertEdge:(Edge2D *)edge startVertex:(Vector2f *)startVertex endVertex:(Vector2f *)endVertex {
    STAssertEqualObjects(startVertex, [edge startVertex], @"start vertex of edge must be %@", [startVertex description]);
    STAssertEqualObjects(endVertex, [edge endVertex], @"end vertex of edge must be %@", [endVertex description]);
}

- (void)testInit {
    Vector2f* v0 = [[Vector2f alloc] initWithX:2 y:4];
    Vector2f* v1 = [[Vector2f alloc] initWithX:3 y:2];
    Vector2f* v2 = [[Vector2f alloc] initWithX:4 y:5];
    
    Polygon2D* p = [[Polygon2D alloc] initWithVertices:[NSArray arrayWithObjects:v1, v2, v0, nil]];
    NSArray* v = [p vertices];
    Edge2D* e = [p edges];
    
    NSArray* vx = [NSArray arrayWithObjects:v0, v1, v2 , nil];
    STAssertTrue([v isEqualToArray:vx], @"vertex array must contain v0, v1 and v2 in that order");
    [self assertEdge:e startVertex:v0 endVertex:v1];
    
    e = [e next];
    [self assertEdge:e startVertex:v1 endVertex:v2];
    
    e = [e next];
    [self assertEdge:e startVertex:v2 endVertex:v0];
    
    [p release];
    [v0 release];
    [v1 release];
    [v2 release];
}

@end
