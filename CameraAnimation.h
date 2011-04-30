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
    TVector3f initialPosition;
    TVector3f initialDirection;
    TVector3f initialUpVector;
    TVector3f targetPosition;
    TVector3f targetDirection;
    TVector3f targetUpVector;
}

- (id)initWithCamera:(Camera *)theCamera targetPosition:(const TVector3f *)thePosition targetDirection:(const TVector3f *)theDirection targetUp:(const TVector3f *)theUpVector duration:(NSTimeInterval)duration;

@end
