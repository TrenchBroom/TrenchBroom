//
//  CameraOrbitAnimation.h
//  TrenchBroom
//
//  Created by Kristian Duske on 05.11.11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#import <AppKit/AppKit.h>
#import "CameraAnimation.h"
#import "Math.h"

@class Camera;

@interface CameraOrbitAnimation : CameraAnimation {
    TVector3f orbitCenter;
    float hDelta, vDelta;
}

- (id)initWithCamera:(Camera *)theCamera orbitCenter:(const TVector3f *)theOrbitCenter hDelta:(float)theHDelta vDelta:(float)theVDelta duration:(NSTimeInterval)duration;

@end
