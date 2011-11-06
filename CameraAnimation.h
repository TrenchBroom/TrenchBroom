//
//  CameraAnimation.h
//  TrenchBroom
//
//  Created by Kristian Duske on 05.11.11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#import <AppKit/AppKit.h>

@class Camera;

@interface CameraAnimation : NSAnimation <NSAnimationDelegate> {
    Camera* camera;
}

- (id)initWithCamera:(Camera *)theCamera duration:(NSTimeInterval)theDuration;
 
@end
