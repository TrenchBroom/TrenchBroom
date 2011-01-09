//
//  Polyhedron.h
//  TrenchBroom
//
//  Created by Kristian Duske on 02.10.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class Vector3f;
@class HalfSpace3D;

@interface Polyhedron : NSObject {
    NSMutableSet* sides;
}
+ (Polyhedron *)maximumCube;
+ (Polyhedron *)cuboidAt:(Vector3f *)center dimensions:(Vector3f *)dimensions;

- (id)initMaximumCube;
- (id)initCuboidAt:(Vector3f *)center dimensions:(Vector3f *)dimensions;

- (NSSet *)sides;

- (BOOL)intersectWithHalfSpace:(HalfSpace3D *)halfSpace;

- (BOOL)isEqualToPolyhedron:(Polyhedron *)polyhedron;
@end
