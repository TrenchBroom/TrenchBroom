//
//  Brush.m
//  TrenchBroom
//
//  Created by Kristian Duske on 30.01.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "Brush.h"
#import "Vector3i.h"
#import "Face.h"
#import "Polyhedron.h"
#import "Polygon3D.h"
#import "HalfSpace3D.h"

NSString* const BrushFaceAdded = @"FaceAdded";
NSString* const BrushFaceRemoved = @"FaceRemoved";
NSString* const BrushFaceChanged = @"FaceChanged";

@implementation Brush

- (id)init {
    if (self = [super init]) {
        faces = [[NSMutableSet alloc] init];
    }
    
    return self;
}

- (id)initCuboidAt:(Vector3i *)position with:(Vector3i *)dimensions {
    if (self = [self init]) {
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        
        Vector3i* pos2 = [[Vector3i alloc]initWithVector:position];
        [pos2 add:dimensions];
        
        Face* bottom = [[Face alloc] initOnPlane:XZ at:position texture:nil];
        [faces addObject:bottom];
        [self registerAsObserverOf:bottom];
        [bottom release];

        Face* left = [[Face alloc] initOnPlane:YZ at:position texture:nil];
        [faces addObject:left];
        [self registerAsObserverOf:left];
        [left release];
        
        Face* back = [[Face alloc] initOnPlane:XY at:position texture:nil];
        [faces addObject:back];
        [self registerAsObserverOf:back];
        [back release];
        
        Face* top = [[Face alloc] initOnPlane:XZ at:pos2 texture:nil];
        [faces addObject:top];
        [self registerAsObserverOf:top];
        [top release];
        
        Face* right = [[Face alloc] initOnPlane:YZ at:pos2 texture:nil];
        [faces addObject:right];
        [self registerAsObserverOf:right];
        [right release];
        
        Face* front = [[Face alloc] initOnPlane:XY at:pos2 texture:nil];
        [faces addObject:front];
        [self registerAsObserverOf:front];
        [front release];
        
        [pos2 release];
    }
    
    return self;
}

- (void)faceChanged:(NSNotification *)notification {
    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    [center postNotification:[NSNotification notificationWithName:BrushFaceChanged object:[notification object]]];
}

- (void)registerAsObserverOf:(Face *)face {
    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    [center addObserver:self selector:@selector(faceChanged:) name:FacePoint1Changed object:face];
    [center addObserver:self selector:@selector(faceChanged:) name:FacePoint2Changed object:face];
    [center addObserver:self selector:@selector(faceChanged:) name:FacePoint3Changed object:face];
    [center addObserver:self selector:@selector(faceChanged:) name:FaceTextureChanged object:face];
    [center addObserver:self selector:@selector(faceChanged:) name:FaceXOffsetChanged object:face];
    [center addObserver:self selector:@selector(faceChanged:) name:FaceYOffsetChanged object:face];
    [center addObserver:self selector:@selector(faceChanged:) name:FaceRotationChanged object:face];
    [center addObserver:self selector:@selector(faceChanged:) name:FaceXScaleChanged object:face];
    [center addObserver:self selector:@selector(faceChanged:) name:FaceYScaleChanged object:face];
}

- (void)deregisterAsObserverOf:(Face *)face {
    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    [center removeObserver:self name:FacePoint1Changed object:face];
    [center removeObserver:self name:FacePoint2Changed object:face];
    [center removeObserver:self name:FacePoint3Changed object:face];
    [center removeObserver:self name:FaceTextureChanged object:face];
    [center removeObserver:self name:FaceXOffsetChanged object:face];
    [center removeObserver:self name:FaceYOffsetChanged object:face];
    [center removeObserver:self name:FaceRotationChanged object:face];
    [center removeObserver:self name:FaceXScaleChanged object:face];
    [center removeObserver:self name:FaceYScaleChanged object:face];
}
         
         
- (NSSet *)faces {
    return faces;
}

- (NSSet *)polygons {
    Polyhedron* polyhedron = [Polyhedron maximumCube];
        
    NSEnumerator* faceEnum = [faces objectEnumerator];
    Face* face;
    while ((face = [faceEnum nextObject]))
        polyhedron = [[face halfSpace] intersectWithPolyhedron:polyhedron];

    return [polyhedron sides];
}

- (void)dealloc {
    NSEnumerator* faceEn = [faces objectEnumerator];
    Face* face;
    while ((face = [faceEn nextObject]))
        [self deregisterAsObserverOf:face];
    
    [faces release];
    [super dealloc];
}

@end
