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

- (void)testIntersectionOfTriangles1 {
    Vector2f* v10 = [[Vector2f alloc] initWithX:1 y:2];
    Vector2f* v11 = [[Vector2f alloc] initWithX:9 y:4];
    Vector2f* v12 = [[Vector2f alloc] initWithX:3 y:6];
    Polygon2D* p1 = [[Polygon2D alloc] initWithVertices:[NSArray arrayWithObjects:v10, v11, v12, nil]];
    
    Vector2f* v20 = [[Vector2f alloc] initWithX:3.5f y:8.5f];
    Vector2f* v21 = [[Vector2f alloc] initWithX:1 y:4];
    Vector2f* v22 = [[Vector2f alloc] initWithX:5 y:4];
    Polygon2D* p2 = [[Polygon2D alloc] initWithVertices:[NSArray arrayWithObjects:v20, v21, v22, nil]];
    
    Polygon2D* is = [p1 intersectWith:p2];
    STAssertNotNil(is, @"intersection must not be nil");

    NSArray* v = [is vertices];
    STAssertEqualObjects([Vector2f vectorWithX:2 y:4], [v objectAtIndex:0], @"first vertex must be 2/4");
    STAssertEqualObjects([Vector2f vectorWithX:5 y:4], [v objectAtIndex:1], @"second vertex must be 5/4");
    STAssertEqualObjects([Vector2f vectorWithX:4.5f y:5.5f], [v objectAtIndex:2], @"third vertex must be 4.5/5.5");
    STAssertEqualObjects([Vector2f vectorWithX:3 y:6], [v objectAtIndex:3], @"fourth vertex must be 3/6");
}

- (void)testIntersectionOfTriangles2 {
    Vector2f* v10 = [[Vector2f alloc] initWithX:1 y:2];
    Vector2f* v11 = [[Vector2f alloc] initWithX:6 y:2];
    Vector2f* v12 = [[Vector2f alloc] initWithX:3 y:4];
    Polygon2D* p1 = [[Polygon2D alloc] initWithVertices:[NSArray arrayWithObjects:v10, v11, v12, nil]];
    
    Vector2f* v20 = [[Vector2f alloc] initWithX:2 y:1];
    Vector2f* v21 = [[Vector2f alloc] initWithX:4 y:1];
    Vector2f* v22 = [[Vector2f alloc] initWithX:5 y:4];
    Polygon2D* p2 = [[Polygon2D alloc] initWithVertices:[NSArray arrayWithObjects:v20, v21, v22, nil]];
    
    Polygon2D* is = [p1 intersectWith:p2];
    STAssertNotNil(is, @"intersection must not be nil");
    
    NSArray* v = [is vertices];
    STAssertEqualObjects([Vector2f vectorWithX:3 y:2], [v objectAtIndex:0], @"first vertex must be 3/2");
    STAssertEqualObjects([Vector2f vectorWithX:(4 + 1/3.0f) y:2], [v objectAtIndex:1], @"second vertex must be 5/4");
//    STAssertEqualObjects([Vector2f vectorWithX:4.5f y:5.5f], [v objectAtIndex:2], @"third vertex must be 4.5/5.5");
//    STAssertEqualObjects([Vector2f vectorWithX:3 y:6], [v objectAtIndex:3], @"fourth vertex must be 3/6");
}

@end
