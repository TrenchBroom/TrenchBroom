//
//  RenderContext.m
//  TrenchBroom
//
//  Created by Kristian Duske on 16.03.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "RenderContext.h"
#import "Camera.h"

@implementation RenderContext

- (Camera *)camera {
    return camera;
}

- (void)setCamera:(Camera *)aCamera {
    if (aCamera == nil)
        [NSException raise:NSInvalidArgumentException format:@"camera must not be nil"];
    
    [camera release]
    camera = aCamera;
    [camera retain];
}

- (void)dealloc {
    [camera release];
    [super dealloc];
}

@end
