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
- (id)initWithMin:(Vector3f *)theMin max:(Vector3f *)theMax {
    if (self = [self init]) {
        min = [theMin retain];
        max = [theMax retain];
    }
    
    return self;
}

- (Vector3f *)min {
    return min;
}

- (Vector3f *)max {
    return max;
}

- (void)dealloc {
    [min release];
    [max release];
    [super dealloc];
}

@end
