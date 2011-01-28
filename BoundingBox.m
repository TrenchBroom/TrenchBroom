//
//  BoundingBox.m
//  TrenchBroom
//
//  Created by Kristian Duske on 28.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "BoundingBox.h"
#import "Vector3f.h"

@implementation BoundingBox
- (id)initAtOrigin:(Vector3f *)theOrigin dimensions:(Vector3f *)theDimensions {
    if (self = [self init]) {
        origin = [theOrigin retain];
        dimensions = [theDimensions retain];
    }
    
    return self;
}

- (void)dealloc {
    [origin release];
    [dimensions release];
    [super dealloc];
}

@end
