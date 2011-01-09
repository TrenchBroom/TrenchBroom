//
//  Polyhedron.m
//  TrenchBroom
//
//  Created by Kristian Duske on 02.10.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "Polyhedron.h"
#import "Vector3f.h"
#import "HalfSpace3D.h"
#import "Polygon3D.h"

@implementation Polyhedron
+ (Polyhedron *)maximumCube {
    return [[[Polyhedron alloc] initMaximumCube] autorelease];
}

+ (Polyhedron *)cuboidAt:(Vector3f *)center dimensions:(Vector3f *)dimensions {
    return [[[Polyhedron alloc] initCuboidAt:center dimensions:dimensions] autorelease];
}

- (id)init {
    if (self = [super init])
        sides = [[NSMutableSet alloc] init];
    
    return self;
}

- (id)initMaximumCube {
    if (self == [super init]) {
        // create all vertices of the cube
        Vector3f* lbb = [[Vector3f alloc] initWithX:-MAXFLOAT y:-MAXFLOAT z:-MAXFLOAT];
        Vector3f* lbf = [[Vector3f alloc] initWithX:-MAXFLOAT y:-MAXFLOAT z: MAXFLOAT];
        Vector3f* ltf = [[Vector3f alloc] initWithX:-MAXFLOAT y: MAXFLOAT z: MAXFLOAT];
        Vector3f *ltb = [[Vector3f alloc] initWithX:-MAXFLOAT y: MAXFLOAT z:-MAXFLOAT];
        Vector3f *rbb = [[Vector3f alloc] initWithX: MAXFLOAT y:-MAXFLOAT z:-MAXFLOAT];
        Vector3f *rbf = [[Vector3f alloc] initWithX: MAXFLOAT y:-MAXFLOAT z: MAXFLOAT];
        Vector3f *rtf = [[Vector3f alloc] initWithX: MAXFLOAT y: MAXFLOAT z: MAXFLOAT];
        Vector3f *rtb = [[Vector3f alloc] initWithX: MAXFLOAT y: MAXFLOAT z:-MAXFLOAT];

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

- (id)initCuboidAt:(Vector3f *)center dimensions:(Vector3f *)dimensions {
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

- (NSSet *)sides {
    return sides;
}

- (BOOL)intersectWithHalfSpace:(HalfSpace3D *)halfSpace {
    if (halfSpace == nil)
        [NSException raise:NSInvalidArgumentException format:@"half space must not be nil"];
    
    // TODO: an intersection creates a new polygon if the boundary of the half space intersects with the polyhedron - this new polygon must be computed and added accordingly
    
    NSMutableSet* newSides = [[NSMutableSet alloc] init];
    NSEnumerator* sideEnum = [sides objectEnumerator];
    Polygon3D* side;
    while ((side = [sideEnum nextObject]) != nil) {
        if ([side intersectWithHalfSpace:halfSpace])
            [newSides addObject:side];
    }
    
    [sides release];
    sides = newSides;
    
    return [sides count] != 0;
}

- (BOOL)isEqualToPolyhedron:(Polyhedron *)polyhedron {
    if ([self isEqual:polyhedron])
        return YES;

    NSSet* otherSides = [polyhedron sides];
    if ([sides count] != [otherSides count])
        return NO;
    
    NSEnumerator* sidesEnum = [sides objectEnumerator];
    Polygon3D* side;

    while ((side = [sidesEnum nextObject])) {
        NSEnumerator* otherSidesEnum = [otherSides objectEnumerator];
        Polygon3D* otherSide;
        BOOL f = NO;
        while (!f && (otherSide = [otherSidesEnum nextObject])) {
            f = [side isEqualToPolygon:otherSide];
        }
        
        if (!f)
            return NO;
    }
    
    return YES;
}

- (void)dealloc {
    [sides release];
    [super dealloc];
}
@end
