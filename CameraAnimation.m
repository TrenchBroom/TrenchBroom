//
//  CameraAnimator.m
//  TrenchBroom
//
//  Created by Kristian Duske on 14.03.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "CameraAnimation.h"
#import "Vector3f.h"
#import "Camera.h"
#import "MathCache.h"

@implementation CameraAnimation

- (id)initWithCamera:(Camera *)theCamera targetPosition:(Vector3f *)thePosition direction:(Vector3f *)theDirection duration:(NSTimeInterval)duration {
    if (self = [super initWithDuration:duration animationCurve:NSAnimationEaseInOut]) {
        camera = [theCamera retain];
        initialPosition = [[Vector3f alloc] initWithFloatVector:[camera position]];
        initialDirection = [[Vector3f alloc] initWithFloatVector:[camera direction]];
        targetPosition = [[Vector3f alloc] initWithFloatVector:thePosition];
        targetDirection = [[Vector3f alloc] initWithFloatVector:theDirection];
        [super setDelegate:self];
    }
    
    return self;
}

- (void)setCurrentProgress:(NSAnimationProgress)progress {
    [super setCurrentProgress:progress];
    
    MathCache* cache = [MathCache sharedCache];
    Vector3f* t = [cache vector3f];
    
    [t setFloat:targetPosition];
    [t sub:initialPosition];
    [t scale:progress];
    [t add:initialPosition];
    [camera moveTo:t];
    
    [t setFloat:targetDirection];
    [t sub:initialDirection];
    [t scale:progress];
    [t add:initialDirection];
    [camera setDirection:t];
    
    [cache returnVector3f:t];
}

- (void)animationDidEnd:(NSAnimation *)animation {
    [self release];
}

- (void)animationDidStop:(NSAnimation *)animation {
    [self release];
}

- (void)animation:(NSAnimation *)animation didReachProgressMark:(NSAnimationProgress)progress {
}

- (void)dealloc {
    [initialPosition release];
    [initialDirection release];
    [targetPosition release];
    [targetDirection release];
    [super dealloc];
}

@end
