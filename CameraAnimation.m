//
//  CameraAnimator.m
//  TrenchBroom
//
//  Created by Kristian Duske on 14.03.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "CameraAnimation.h"
#import "Camera.h"

@implementation CameraAnimation

- (id)initWithCamera:(Camera *)theCamera targetPosition:(const TVector3f *)thePosition targetDirection:(const TVector3f *)theDirection targetUp:(const TVector3f *)theUpVector duration:(NSTimeInterval)duration {
    if (self = [super initWithDuration:duration animationCurve:NSAnimationEaseInOut]) {
        camera = [theCamera retain];
        initialPosition = *[camera position];
        initialDirection = *[camera direction];
        initialUpVector = *[camera up];
        targetPosition = *thePosition;
        targetDirection = *theDirection;
        targetUpVector = *theUpVector;
        [super setFrameRate:60];
        [super setAnimationBlockingMode:NSAnimationNonblocking];
        [super setDelegate:self];
    }
    
    return self;
}

- (void)setCurrentProgress:(NSAnimationProgress)progress {
    [super setCurrentProgress:progress];
    
    TVector3f t, u;
    
    subV3f(&targetPosition, &initialPosition, &t);
    scaleV3f(&t, progress, &t);
    addV3f(&t, &initialPosition, &t);
    [camera moveTo:&t];
    
    subV3f(&targetDirection, &initialDirection, &t);
    scaleV3f(&t, progress, &t);
    addV3f(&t, &initialDirection, &t);
    normalizeV3f(&t, &t);

    subV3f(&targetUpVector, &initialUpVector, &u);
    scaleV3f(&u, progress, &u);
    addV3f(&u, &initialUpVector, &u);
    normalizeV3f(&u, &u);
    
    [camera setDirection:&t up:&u];
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
    [camera release];
    [super dealloc];
}

@end
