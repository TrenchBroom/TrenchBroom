//
//  Polyhedron.m
//  TrenchBroom
//
//  Created by Kristian Duske on 02.10.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "Polyhedron.h"


@implementation Polyhedron
- (id)init {
    if (self = [super init])
        sides = [[NSMutableSet alloc] init];
    
    return self;
}
-(id)initCuboidAt:(Vector3f *)center dimensions:(Vector3f *)dimensions {
    if (center == nil)
        [NSException raise:NSInvalidArgumentException format:@"center must not be nil"];
    if (dimensions == nil)
        [NSException raise:NSInvalidArgumentException format:@"dimensions must not be nil"];
    if ([dimensions x] <= 0 || [dimensions y] <= 0 || [dimensions z] <= 0)
        [NSException raise:NSInvalidArgumentException format:@"dimensions must not be empty"];
    
    if (self == [super init]) {
        float dx = [dimensions x] / 2;
        float dy = [dimensions y] / 2;
        float dz = [dimensions z] / 2;
        
        // create all vertices of the cube
        Vector3f* lbb = [[Vector3f alloc] initWithFloatVector:center];
        [lbb addX:-dx y:-dy z:-dz];
        
        Vector3f* lbf = [[Vector3f alloc] initWithFloatVector:center];
        [lbf addX:-dx y:-dy z:dz];
        
        Vector3f* ltf = [[Vector3f alloc] initWithFloatVector:center];
        [ltf addX:-dx y:dy z:dz];
        
        Vector3f *ltb = [[Vector3f alloc] initWithFloatVector:center];
        [ltb addX:-dx y:dy z:-dz];
        
        Vector3f *rbb = [[Vector3f alloc] initWithFloatVector:center];
        [rbb addX:dx y:-dy z:-dz];
        
        Vector3f *rbf = [[Vector3f alloc] initWithFloatVector:center];
        [rbf addX:dx y:-dy z:dz];
        
        Vector3f *rtf = [[Vector3f alloc] initWithFloatVector:center];
        [rtf addX:dx y:dy z:dz];
        
        Vector3f *rtb = [[Vector3f alloc] initWithFloatVector:center];
        [rtb addX:dx y:dy z:-dz];
        
        // create all sides of the cube with vertices in CCW order when looking from outside
        Polygon3D* left = [[Polygon3D alloc] initWithVertices:[NSArray arrayWithObjects:
                                                               [Vector3f vectorWithFloatVector:lbb], 
                                                               [Vector3f vectorWithFloatVector:lbf],
                                                               [Vector3f vectorWithFloatVector:ltf],
                                                               [Vector3f vectorWithFloatVector:ltb],
                                                               nil]];
        Polygon3D* right = [[Polygon3D alloc] initWithVertices:[NSArray arrayWithObjects:
                                                                [Vector3f vectorWithFloatVector:rbf],
                                                                [Vector3f vectorWithFloatVector:rbb],
                                                                [Vector3f vectorWithFloatVector:rtb],
                                                                [Vector3f vectorWithFloatVector:rtf],
                                                                nil]];
        Polygon3D* top = [[Polygon3D alloc] initWithVertices:[NSArray arrayWithObjects:
                                                              [Vector3f vectorWithFloatVector:ltb],
                                                              [Vector3f vectorWithFloatVector:ltf],
                                                              [Vector3f vectorWithFloatVector:rtf],
                                                              [Vector3f vectorWithFloatVector:rtb],
                                                              nil]];
        Polygon3D* bottom = [[Polygon3D alloc] initWithVertices:[NSArray arrayWithObjects:
                                                                 [Vector3f vectorWithFloatVector:lbf],
                                                                 [Vector3f vectorWithFloatVector:lbb],
                                                                 [Vector3f vectorWithFloatVector:rbb],
                                                                 [Vector3f vectorWithFloatVector:rbf],
                                                                 nil]];
        Polygon3D* front = [[Polygon3D alloc] initWithVertices:[NSArray arrayWithObjects:
                                                                [Vector3f vectorWithFloatVector:ltf],
                                                                [Vector3f vectorWithFloatVector:lbf],
                                                                [Vector3f vectorWithFloatVector:rbf],
                                                                [Vector3f vectorWithFloatVector:rtf],
                                                                nil]];
        Polygon3D* back = [[Polygon3D alloc] initWithVertices:[NSArray arrayWithObjects:
                                                               [Vector3f vectorWithFloatVector:rtb],
                                                               [Vector3f vectorWithFloatVector:rbb],
                                                               [Vector3f vectorWithFloatVector:lbb],
                                                               [Vector3f vectorWithFloatVector:ltb],
                                                               nil]];
        sides = [[NSMutableSet alloc] initWithObjects:left, right, top, bottom, front, back, nil];
        
        [left release];
        [right release];
        [top release];
        [bottom release];
        [front release];
        [back release];
        
        [lbb release];
        [lbf release];
        [ltf release];
        [ltb release];
        [rbb release];
        [rbf release];
        [rtf release];
        [rtb release];
    }
    
    return self;
}

- (void)intersectWith:(HalfSpace3D *)halfSpace {
    
}
@end
