//
//  Line2DTestCase.m
//  TrenchBroom
//
//  Created by Kristian Duske on 07.09.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "Line2DTestCase.h"
#import "Line2D.h"

@implementation Line2DTestCase
- (void)testIntersection {
    Line2D* l1 = [Line2D lineWithPoint:[Vector2f vectorWithX:1 y:4] direction:[Vector2f vectorWithX:1 y:0]];
    Line2D* l2 = [Line2D lineWithPoint:[Vector2f vectorWithX:5 y:4] direction:[Vector2f vectorWithX:-1.5f y:4.5f]];
    
    Vector2f* is = [l1 intersectWith:l2];
    STAssertNotNil(is, @"intersection must not be nil");
    STAssertEqualObjects([Vector2f vectorWithX:5 y:4], is, @"intersection must be at 5/4");
}

- (void)testYAt {
    Line2D* l = [Line2D lineWithPoint1:[Vector2f vectorWithX:9 y:4] point2:[Vector2f vectorWithX:1 y:2]];
    
    float y = [l yAt:1];
    STAssertEquals(2.0f, y, @"y should be 2");
}
@end
