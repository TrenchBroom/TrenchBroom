//
//  CameraOrbitAnimation.m
//  TrenchBroom
//
//  Created by Kristian Duske on 05.11.11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#import "CameraOrbitAnimation.h"
#import "Camera.h"

@implementation CameraOrbitAnimation

- (id)initWithCamera:(Camera *)theCamera orbitCenter:(const TVector3f *)theOrbitCenter hDelta:(float)theHDelta vDelta:(float)theVDelta duration:(NSTimeInterval)duration {
    NSAssert(theCamera != nil, @"camera must not be nil");
    NSAssert(theOrbitCenter != NULL, @"orbit center must not be NULL");
    
    if ((self = [super initWithCamera:theCamera duration:duration])) {
        orbitCenter = *theOrbitCenter;
        hDelta = theHDelta;
        vDelta = theVDelta;
    }
    
    return self;
}

- (void)setCurrentProgress:(NSAnimationProgress)progress {
    [super setCurrentProgress:progress];

    float hAngle = hDelta * progress;
    float vAngle = vDelta * progress;
    
    [camera orbitCenter:&orbitCenter hAngle:hAngle vAngle:vAngle];
}

@end
