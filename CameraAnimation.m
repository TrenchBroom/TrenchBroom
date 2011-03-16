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

- (id)initWithCamera:(Camera *)theCamera targetPosition:(Vector3f *)thePosition targetDirection:(Vector3f *)theDirection targetUp:(Vector3f *)theUpVector duration:(NSTimeInterval)duration {
    if (self = [super initWithDuration:duration animationCurve:NSAnimationEaseInOut]) {
        camera = [theCamera retain];
        initialPosition = [[Vector3f alloc] initWithFloatVector:[camera position]];
        initialDirection = [[Vector3f alloc] initWithFloatVector:[camera direction]];
        initialUpVector = [[Vector3f alloc] initWithFloatVector:[camera up]];
        targetPosition = [[Vector3f alloc] initWithFloatVector:thePosition];
        targetDirection = [[Vector3f alloc] initWithFloatVector:theDirection];
        targetUpVector = [[Vector3f alloc] initWithFloatVector:theUpVector];
        [super setFrameRate:60];
        [super setAnimationBlockingMode:NSAnimationNonblocking];
        [super setDelegate:self];
    }
    
    return self;
}

- (void)setCurrentProgress:(NSAnimationProgress)progress {
    [super setCurrentProgress:progress];
    
    MathCache* cache = [MathCache sharedCache];
    Vector3f* t = [cache vector3f];
    Vector3f* u = [cache vector3f];
    
    [t setFloat:targetPosition];
    [t sub:initialPosition];
    [t scale:progress];
    [t add:initialPosition];
    [camera moveTo:t];
    
    [t setFloat:targetDirection];
    [t sub:initialDirection];
    [t scale:progress];
    [t add:initialDirection];
    [t normalize];
    
    [u setFloat:targetUpVector];
    [u sub:initialUpVector];
    [u scale:progress];
    [u add:initialUpVector];
    [u normalize];
    
    [camera setDirection:t up:u];
    
    [cache returnVector3f:t];
    [cache returnVector3f:u];
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
