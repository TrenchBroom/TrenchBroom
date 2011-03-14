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
    Vector3f* targetPosition;
    Vector3f* targetDirection;
    
}

- (id)initWithCamera:(Camera *)theCamera targetPosition:(Vector3f *)thePosition direction:(Vector3f *)theDirection duration:(NSTimeInterval)duration;

@end
