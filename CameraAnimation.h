//
//  CameraAnimator.h
//  TrenchBroom
//
//  Created by Kristian Duske on 14.03.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class Camera;
@class Vector3f;

@interface CameraAnimation : NSAnimation <NSAnimationDelegate> {
    @private
    Camera* camera;
    Vector3f* initialPosition;
    Vector3f* initialDirection;
    Vector3f* initialUpVector;
    Vector3f* targetPosition;
    Vector3f* targetDirection;
    Vector3f* targetUpVector;
}

- (id)initWithCamera:(Camera *)theCamera targetPosition:(Vector3f *)thePosition targetDirection:(Vector3f *)theDirection targetUp:(Vector3f *)theUpVector duration:(NSTimeInterval)duration;

@end
