//
//  PolyhedronTestCase.m
//  TrenchBroom
//
//  Created by Kristian Duske on 10.10.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "PolyhedronTestCase.h"
#import "Polyhedron.h"
#import "Polygon3D.h"
#import "Plane3D.h"
#import "Vector3f.h"
#import "HalfSpace3D.h"

@implementation PolyhedronTestCase
- (void)testInitCuboid {
    Polyhedron* cuboid = [Polyhedron cuboidAt:[Vector3f vectorWithX:0 y:0 z:0] 
                                   dimensions:[Vector3f vectorWithX:10 y:20 z:30]];
    NSSet* sides = [cuboid sides];
    STAssertNotNil(sides, @"sides must not be nil");
    STAssertEquals((NSUInteger)6, [sides count], @"cuboid must have six sides");
}
@end
