//
//  Brush.m
//  TrenchBroom
//
//  Created by Kristian Duske on 30.01.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "Brush.h"

NSString* const BrushFaceAdded = @"FaceAdded";
NSString* const BrushFaceRemoved = @"FaceRemoved";

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
        
        Face* bottom = [[Face alloc] initOnPlane:XZ at:position texture:nil];
        [faces addObject:bottom];
        [bottom release];

        Face* left = [[Face alloc] initOnPlane:YZ at:position texture:nil];
        [faces addObject:left];
        [left release];
        
        Face* back = [[Face alloc] initOnPlane:XY at:position texture:nil];
        [faces addObject:back];
        [back release];
        
        Face* top = [[Face alloc] initOnPlane:XZ at:pos2 texture:nil];
        [faces addObject:top];
        [top release];
        
        Face* right = [[Face alloc] initOnPlane:YZ at:pos2 texture:nil];
        [faces addObject:right];
        [right release];
        
        Face* front = [[Face alloc] initOnPlane:XY at:pos2 texture:nil];
        [faces addObject:front];
        [front release];
        
        [pos2 release];
    }
    
    return self;
}

- (void)dealloc {
    
    [faces release];
    [super dealloc];
}

@end
