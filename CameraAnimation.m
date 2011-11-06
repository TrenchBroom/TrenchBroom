//
//  CameraAnimation.m
//  TrenchBroom
//
//  Created by Kristian Duske on 05.11.11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#import "CameraAnimation.h"
#import "Camera.h"

@implementation CameraAnimation

- (id)initWithCamera:(Camera *)theCamera duration:(NSTimeInterval)theDuration {
    NSAssert(theCamera != nil, @"camera must not be nil");
    
    if ((self = [super initWithDuration:theDuration animationCurve:NSAnimationEaseInOut])) {
        camera = theCamera;

        [super setFrameRate:60];
        [super setAnimationBlockingMode:NSAnimationNonblocking];
        [super setDelegate:self];
    }
    
    return self;
}

- (void)startAnimation {
    NSMutableSet* animations = [camera animations];
    
    NSSet* copy = [[NSSet alloc] initWithSet:animations];
    NSEnumerator* animationEn = [copy objectEnumerator];
    NSAnimation* animation;
    while ((animation = [animationEn nextObject]))
        [animation stopAnimation];
    [copy release];
    
    [animations addObject:self];
    [super startAnimation];
}

- (void)animationDidEnd:(NSAnimation *)animation {
    NSMutableSet* animations = [camera animations];
    
    [animations removeObject:self];
    [self release];
}

- (void)animationDidStop:(NSAnimation *)animation {
    NSMutableSet* animations = [camera animations];
    [animations removeObject:self];
    
    [self release];
}

@end
