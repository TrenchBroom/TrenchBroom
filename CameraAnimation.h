//
//  CameraAnimator.h
//  TrenchBroom
//
//  Created by Kristian Duske on 14.03.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "Math.h"

@class Camera;

@interface CameraAnimation : NSAnimation <NSAnimationDelegate> {
    @private
    Camera* camera;
    TCubicBezierCurve positionCurve;
    TVector3f initialPosition;
    TVector3f initialDirection;
    TVector3f initialUp;
    TVector3f targetPosition;
    TVector3f targetLookAt;
    TVector3f targetUp;
    float hAngle, vAngle;
}

- (id)initWithCamera:(Camera *)theCamera targetPosition:(const TVector3f *)thePosition targetLookAt:(const TVector3f *)theLookAt duration:(NSTimeInterval)duration;

@end
