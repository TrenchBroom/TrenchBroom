//
//  Plane3DTestCase.m
//  TrenchBroom
//
//  Created by Kristian Duske on 09.10.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "Plane3DTestCase.h"
#import "Line3D.h"
#import "Plane3D.h"

@implementation Plane3DTestCase
- (void)testIntersectWithLine {
    Line3D* line = [Line3D lineWithPoint1:[Vector3f vectorWithX:1 y:1 z:0] 
                                   point2:[Vector3f vectorWithX:1 y:1 z:1]];
    Plane3D* plane = [Plane3D planeWithPoint:[Vector3f vectorWithX:0 y:0 z:2] 
                                        norm:[Vector3f vectorWithX:0 y:0 z:1]];
    
    Vector3f* is = [plane intersectWith:line];
    STAssertNotNil(is, @"intersection must not be nil");
    STAssertEqualObjects([Vector3f vectorWithX:1 y:1 z:2], is, @"intersection must be at (1;1;2)");
}
@end
