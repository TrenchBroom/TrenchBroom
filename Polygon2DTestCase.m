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
    STAssertEquals(startVertex, [edge startVertex], @"start vertex of edge %@ must be %@", [edge description], [startVertex description]);
    STAssertEquals(endVertex, [edge endVertex], @"end vertex of edge %@ must be %@", [edge description], [endVertex description]);
}

- (void)testInit {
    Vector2f* v0 = [[Vector2f alloc] initWithX:7 y:4];
    Vector2f* v1 = [[Vector2f alloc] initWithX:4 y:5];
    Vector2f* v2 = [[Vector2f alloc] initWithX:2 y:4];
    Vector2f* v3 = [[Vector2f alloc] initWithX:3 y:7];
    Vector2f* v4 = [[Vector2f alloc] initWithX:5 y:1];
    Vector2f* v5 = [[Vector2f alloc] initWithX:7 y:2];
    
    NSMutableArray* vertices = [[NSMutableArray alloc] initWithObjects:v0, v1, v2, v3, v4, v5, nil];
    Polygon2D* polygon = [[Polygon2D alloc] initWithVertices:vertices];
    
    NSArray* polyVerts = [polygon vertices];
    STAssertNotNil(polyVerts, @"polygon vertices must not be nil");
    STAssertEquals([vertices count], [polyVerts count], @"polygon must contain %i vertices", [vertices count]);
    STAssertEquals(v2, [polyVerts objectAtIndex:0], @"polygon vertex %i must be %@", 0, v2);
    STAssertEquals(v3, [polyVerts objectAtIndex:1], @"polygon vertex %i must be %@", 1, v3);
    STAssertEquals(v4, [polyVerts objectAtIndex:2], @"polygon vertex %i must be %@", 2, v4);
    STAssertEquals(v5, [polyVerts objectAtIndex:3], @"polygon vertex %i must be %@", 3, v5);
    STAssertEquals(v0, [polyVerts objectAtIndex:4], @"polygon vertex %i must be %@", 4, v0);
    STAssertEquals(v1, [polyVerts objectAtIndex:5], @"polygon vertex %i must be %@", 5, v1);
    
    NSArray* upperEdges = [polygon upperEdges];
    STAssertNotNil(upperEdges, @"upper edge array must not be nil");
    STAssertEquals((NSUInteger)2, [upperEdges count], @"upper array must contain 2 edges");
    
    [self assertEdge:[upperEdges objectAtIndex:0] startVertex:v2 endVertex:v1];
    [self assertEdge:[upperEdges objectAtIndex:1] startVertex:v1 endVertex:v0];
    
    NSArray* lowerEdges = [polygon lowerEdges];
    STAssertNotNil(lowerEdges, @"lower edge array must not be nil");
    STAssertEquals((NSUInteger)4, [lowerEdges count], @"lower array must contain 4 edges");
    
    [self assertEdge:[lowerEdges objectAtIndex:0] startVertex:v2 endVertex:v3];
    [self assertEdge:[lowerEdges objectAtIndex:1] startVertex:v3 endVertex:v4];
    [self assertEdge:[lowerEdges objectAtIndex:2] startVertex:v4 endVertex:v5];
    [self assertEdge:[lowerEdges objectAtIndex:3] startVertex:v5 endVertex:v0];
    
    [vertices release];
    [v0 release];
    [v1 release];
    [v2 release];
    [v3 release];
    [v4 release];
    [v5 release];
}

@end
