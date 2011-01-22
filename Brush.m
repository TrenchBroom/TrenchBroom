//
//  Brush.m
//  TrenchBroom
//
//  Created by Kristian Duske on 30.01.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "Brush.h"
#import "IdGenerator.h"
#import "Vector3i.h"
#import "Face.h"
#import "Polyhedron.h"
#import "Polygon3D.h"
#import "HalfSpace3D.h"

@implementation Brush

- (id)init {
    if (self = [super init]) {
        brushId = [[IdGenerator sharedGenerator] getId];
        faces = [[NSMutableArray alloc] init];
    }
    
    return self;
}

- (id)initCuboidAt:(Vector3i *)position dimensions:(Vector3i *)dimensions texture:(NSString *)texture {
    if (self = [self init]) {
        Vector3i* pos2 = [[Vector3i alloc]initWithVector:position];
        [pos2 add:dimensions];
        
        Face* bottom = [[Face alloc] initOnPlane:XY at:position thirdAxisPositive:NO texture:texture];
        [faces addObject:bottom];
        [bottom release];

        Face* left = [[Face alloc] initOnPlane:XZ at:position thirdAxisPositive:NO texture:texture];
        [faces addObject:left];
        [left release];
        
        Face* back = [[Face alloc] initOnPlane:YZ at:position thirdAxisPositive:NO texture:texture];
        [faces addObject:back];
        [back release];
        
        Face* top = [[Face alloc] initOnPlane:XY at:pos2 thirdAxisPositive:YES texture:texture];
        [faces addObject:top];
        [top release];
        
        Face* right = [[Face alloc] initOnPlane:XZ at:pos2 thirdAxisPositive:YES texture:texture];
        [faces addObject:right];
        [right release];
        
        Face* front = [[Face alloc] initOnPlane:YZ at:pos2 thirdAxisPositive:YES texture:texture];
        [faces addObject:front];
        [front release];
        
        [pos2 release];
    }
    
    return self;
}

- (Face *)createFaceWithPoint1:(Vector3i *)point1 point2:(Vector3i *)point2 point3:(Vector3i *)point3 texture:(NSString *)texture {
    Face* face = [[Face alloc] initWithPoint1:point1 point2:point2 point3:point3 texture:texture];
    [faces addObject:face];
    
    return [face autorelease];
}

- (NSNumber *)getId {
    return brushId;
}
         
- (NSArray *)faces {
    return faces;
}

- (NSArray *)polygons {
    if (polyhedron == nil) {
        polyhedron = [Polyhedron maximumCube];
        
        NSEnumerator* faceEnum = [faces objectEnumerator];
        Face* face;
        while ((face = [faceEnum nextObject]))
            polyhedron = [[face halfSpace] intersectWithPolyhedron:polyhedron];
        
        [polyhedron retain];
    }

    return [polyhedron sides];
}

- (void)dealloc {
    [polyhedron release];
    [faces release];
    [super dealloc];
}

@end
