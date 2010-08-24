//
//  ConvexAreaTestCase.m
//  TrenchBroom
//
//  Created by Kristian Duske on 03.05.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "ConvexAreaTestCase.h"
#import "ConvexArea.h"
#import "Line.h"
#import "Vector3f.h"

@implementation ConvexAreaTestCase

- (Line *)createLineWithPointX:(float)px pointY:(float)py dirX:(float)dx dirY:(float)dy {
    Vector3f* p = [[Vector3f alloc] initWithX:px y:py z:0];
    Vector3f* d = [[Vector3f alloc] initWithX:dx y:dy z:0];
    Line* l = [[Line alloc] initWithPoint:p direction:d];
    
    [p release];
    [d release];
    
    return [l autorelease];
}

- (void)testAddTwoIntersectingLines {
    Vector3f* normal = [[Vector3f alloc] initWithX:0 y:0 z:1];
    
    ConvexArea* ca = [[ConvexArea alloc] initWithNormal:normal];
    Line* l1 = [self createLineWithPointX:0 pointY:1 dirX:1 dirY:0];
    Line* l2 = [self createLineWithPointX:0 pointY:2 dirX:1 dirY:-1];
    
    STAssertTrue([ca addLine:l1], @"adding a boundary line must not degenerate the area");
    STAssertTrue([ca addLine:l2], @"adding a boundary line must not degenerate the area");
    
    NSArray* vertices = [ca vertices];
    STAssertNotNil(vertices, @"vertex array must not be nil");
    STAssertEquals((NSUInteger)1, [vertices count], @"vertex array must contain one vertex");
    
    Vector3f* v = [vertices objectAtIndex:0];
    STAssertEquals(1.0f, [v x], @"vertex X coordinate must be 1");
    STAssertEquals(1.0f, [v y], @"vertex Y coordinate must be 1");
    
    NSArray* edges = [ca edges];
    STAssertNotNil(edges, @"edge array must not be nil");
    STAssertEquals((NSUInteger)2, [edges count], @"edge array must contain two elements");
    
    STAssertEquals(l1, [edges objectAtIndex:0], @"first edge in array must be l1");
    STAssertEquals(l2, [edges objectAtIndex:1], @"second edge in array must be l2");
    
    [ca release];
    
    ca = [[ConvexArea alloc] initWithNormal:normal];
    l1 = [self createLineWithPointX:0 pointY:1 dirX:-1 dirY:0];
    l2 = [self createLineWithPointX:0 pointY:1 dirX:-1 dirY:1];
    
    STAssertTrue([ca addLine:l1], @"adding a boundary line must not degenerate the area");
    STAssertTrue([ca addLine:l2], @"adding a boundary line must not degenerate the area");
    
    vertices = [ca vertices];
    STAssertNotNil(vertices, @"vertex array must not be nil");
    STAssertEquals((NSUInteger)1, [vertices count], @"vertex array must contain one vertex");
    
    v = [vertices objectAtIndex:0];
    STAssertEquals(0.0f, [v x], @"vertex X coordinate must be 0");
    STAssertEquals(1.0f, [v y], @"vertex Y coordinate must be 1");
    
    edges = [ca edges];
    STAssertNotNil(edges, @"edge array must not be nil");
    STAssertEquals((NSUInteger)2, [edges count], @"edge array must contain two elements");
    
    STAssertEquals(l1, [edges objectAtIndex:0], @"first edge in array must be l1");
    STAssertEquals(l2, [edges objectAtIndex:1], @"second edge in array must be l2");
    
    [ca release];

    ca = [[ConvexArea alloc] initWithNormal:normal];
    l1 = [self createLineWithPointX:0 pointY:0 dirX:-1 dirY:1];
    l2 = [self createLineWithPointX:0 pointY:0 dirX:-1 dirY:0];
    
    STAssertTrue([ca addLine:l1], @"adding a boundary line must not degenerate the area");
    STAssertTrue([ca addLine:l2], @"adding a boundary line must not degenerate the area");
    
    vertices = [ca vertices];
    STAssertNotNil(vertices, @"vertex array must not be nil");
    STAssertEquals((NSUInteger)1, [vertices count], @"vertex array must contain one vertex");
    
    v = [vertices objectAtIndex:0];
    STAssertEquals(0.0f, [v x], @"vertex X coordinate must be 0");
    STAssertEquals(0.0f, [v y], @"vertex Y coordinate must be 0");
    
    edges = [ca edges];
    STAssertNotNil(edges, @"edge array must not be nil");
    STAssertEquals((NSUInteger)2, [edges count], @"edge array must contain two elements");
    
    STAssertEquals(l2, [edges objectAtIndex:0], @"first edge in array must be l2");
    STAssertEquals(l1, [edges objectAtIndex:1], @"second edge in array must be l1");
    
    [ca release];
    [normal release];
}

- (void)testAddTwoParallelLines {
    Vector3f* normal = [[Vector3f alloc] initWithX:0 y:0 z:1];

    // vertical line with opposite directions
    ConvexArea* ca = [[ConvexArea alloc] initWithNormal:normal];
    Line* l1 = [self createLineWithPointX:0 pointY:0 dirX:0 dirY:1];
    Line* l2 = [self createLineWithPointX:1 pointY:0 dirX:0 dirY:-1];
    
    STAssertTrue([ca addLine:l1], @"adding a boundary line must not degenerate the area");
    STAssertTrue([ca addLine:l2], @"adding a boundary line must not degenerate the area");
    
    NSArray* vertices = [ca vertices];
    STAssertNotNil(vertices, @"vertex array must not be nil");
    STAssertEquals((NSUInteger)0, [vertices count], @"vertex array must be empty");
    
    NSArray* edges = [ca edges];
    STAssertNotNil(edges, @"edge array must not be nil");
    STAssertEquals((NSUInteger)2, [edges count], @"edge array must contain two elements");
    
    STAssertEquals(l1, [edges objectAtIndex:0], @"first edge in array must be l1");
    STAssertEquals(l2, [edges objectAtIndex:1], @"second edge in array must be l2");
    
    [ca release];
    
    // two vertical lines, first is left to second
    ca = [[ConvexArea alloc] initWithNormal:normal];
    l1 = [self createLineWithPointX:0 pointY:0 dirX:0 dirY:1];
    l2 = [self createLineWithPointX:1 pointY:0 dirX:0 dirY:1];
    
    STAssertTrue([ca addLine:l1], @"adding a boundary line must not degenerate the area");
    STAssertTrue([ca addLine:l2], @"adding a parallel line must not degenerate the area");
    
    vertices = [ca vertices];
    STAssertNotNil(vertices, @"vertex array must not be nil");
    STAssertEquals((NSUInteger)0, [vertices count], @"vertex array must be empty");
    
    edges = [ca edges];
    STAssertNotNil(edges, @"edge array must not be nil");
    STAssertEquals((NSUInteger)1, [edges count], @"edge array must contain one element");
    
    STAssertEquals(l2, [edges objectAtIndex:0], @"only edge in array must be l2");

    [ca release];
    
    // two vertical lines, first is right to second
    ca = [[ConvexArea alloc] initWithNormal:normal];
    l1 = [self createLineWithPointX:1 pointY:0 dirX:0 dirY:1];
    l2 = [self createLineWithPointX:0 pointY:0 dirX:0 dirY:1];
    
    STAssertTrue([ca addLine:l1], @"adding a boundary line must not degenerate the area");
    STAssertTrue([ca addLine:l2], @"adding a parallel line must not degenerate the area");
    
    vertices = [ca vertices];
    STAssertNotNil(vertices, @"vertex array must not be nil");
    STAssertEquals((NSUInteger)0, [vertices count], @"vertex array must be empty");
    
    edges = [ca edges];
    STAssertNotNil(edges, @"edge array must not be nil");
    STAssertEquals((NSUInteger)1, [edges count], @"edge array must contain one element");
    
    STAssertEquals(l1, [edges objectAtIndex:0], @"only edge in array must be l1");
    
    [ca release];
    [normal release];
}

@end
