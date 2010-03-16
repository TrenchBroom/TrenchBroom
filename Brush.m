//
//  Brush.m
//  TrenchBroom
//
//  Created by Kristian Duske on 30.01.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "Brush.h"

NSString* const BrushFaceAddedNotification = @"FaceAdded";
NSString* const BrushFaceRemovedNotification = @"FaceRemoved";

@implementation Brush

- (id)init {
    
    if (self = [super init]) {
        faces = [[NSMutableSet alloc] init];
    }
    
    return self;
}

- (id)initCuboidAt:(Vector3i *)position with:(Vector3i *)dimensions {
    
    if (self = [self init]) {
        Vector3i* pos2 = [[Vector3i alloc]initWithVector:position];
        [pos2 add:dimensions];
        
        [faces addObject:[[Face alloc] initOnPlane:XZ at:position texture:nil]];
        [faces addObject:[[Face alloc] initOnPlane:YZ at:position texture:nil]];
        [faces addObject:[[Face alloc] initOnPlane:XY at:position texture:nil]];
        [faces addObject:[[Face alloc] initOnPlane:XZ at:pos2 texture:nil]];
        [faces addObject:[[Face alloc] initOnPlane:YZ at:pos2 texture:nil]];
        [faces addObject:[[Face alloc] initOnPlane:XY at:pos2 texture:nil]];
        
        [pos2 release];
    }
    
    return self;
}

- (void)dealloc {
    
    [faces release];
    [super dealloc];
}

@end
