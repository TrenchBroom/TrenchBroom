//
//  HalfSpace3DTestCase.m
//  TrenchBroom
//
//  Created by Kristian Duske on 10.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "HalfSpace3DTestCase.h"
#import "Polyhedron.h"
#import "Plane3D.h"
#import "Vector3f.h"
#import "HalfSpace3D.h"


@implementation HalfSpace3DTestCase
- (void)testIntersectWithPolyhedron {
    Polyhedron* cuboid = [Polyhedron cuboidAt:[Vector3f vectorWithX:0 y:0 z:0] 
                                   dimensions:[Vector3f vectorWithX:10 y:20 z:30]];
    Plane3D* boundary = [Plane3D planeWithPoint:[Vector3f vectorWithX:0 y:0 z:0] norm:[Vector3f vectorWithX:0 y:1 z:0]];
    HalfSpace3D* halfSpace = [HalfSpace3D halfSpaceWithBoundary:boundary outside:[Vector3f vectorWithX:0 y:1 z:0]];
    cuboid = [halfSpace intersectWithPolyhedron:cuboid];

    /*
    Polyhedron* reference = [Polyhedron cuboidAt:[Vector3f vectorWithX:0 y:-5 z:0] 
                                      dimensions:[Vector3f vectorWithX:10 y:10 z:30]];
    STAssertTrue([reference isEqualToPolyhedron:cuboid], @"cuboid must be equal to reference");
     */
}

@end
