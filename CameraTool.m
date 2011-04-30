//
//  CameraTool.m
//  TrenchBroom
//
//  Created by Kristian Duske on 14.04.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "CameraTool.h"
#import "MapWindowController.h"
#import "Camera.h"
#import "PickingHit.h"
#import "PickingHitList.h"

@interface CameraTool (private)

- (BOOL)isCameraModifierPressed;
- (BOOL)isCameraOrbitModifierPressed;

@end

@implementation CameraTool (private)

- (BOOL)isCameraModifierPressed {
    return ([NSEvent modifierFlags] & NSShiftKeyMask) == NSShiftKeyMask;
}

- (BOOL)isCameraOrbitModifierPressed {
    return ([NSEvent modifierFlags] & (NSShiftKeyMask | NSCommandKeyMask)) == (NSShiftKeyMask | NSCommandKeyMask);
}

@end

@implementation CameraTool

- (id)initWithWindowController:(MapWindowController *)theWindowController {
    if (self = [self init]) {
        windowController = [theWindowController retain];
    }
    
    return self;
}

- (void)beginLeftDrag:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {
    if (![self isCameraModifierPressed] && ![self isCameraOrbitModifierPressed])
        return;
    
    if ([self isCameraOrbitModifierPressed]) {
        PickingHit* hit = [hits firstHitOfType:HT_ANY ignoreOccluders:YES];
        if (hit != nil) {
            orbitCenter = *[hit hitPoint];
        } else {
            Camera* camera = [windowController camera];
            orbitCenter = [camera defaultPoint];
        }
        orbit = YES;
    }
}

- (void)endLeftDrag:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {
    orbit = NO;
}

- (void)leftDrag:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {
    Camera* camera = [windowController camera];
    if (orbit) {
        float h = -[event deltaX] / 70;
        float v = [event deltaY] / 70;
        [camera orbitCenter:&orbitCenter hAngle:h vAngle:v];
    } else {
        float yaw = -[event deltaX] / 70;
        float pitch = [event deltaY] / 70;
        [camera rotateYaw:yaw pitch:pitch];
    }
}

- (void)rightDrag:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {
    if (![self isCameraModifierPressed])
        return;
    
    Camera* camera = [windowController camera];
    [camera moveForward:0 right:6 * [event deltaX] up:-6 * [event deltaY]];
}

- (void)handleBeginGesture:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {
    gesture = YES;
}

- (void)handleEndGesture:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {
    gesture = NO;
}

- (void)handleScrollWheel:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {
    if (![self isCameraModifierPressed] && ![self isCameraOrbitModifierPressed])
        return;
    
    Camera* camera = [windowController camera];
    if ([self isCameraOrbitModifierPressed]) {
        if (gesture)
            [camera setZoom:[camera zoom] + [event deltaZ] / 10];
        else
            [camera setZoom:[camera zoom] + [event deltaX] / 10];
    } else {
        if (gesture)
            [camera moveForward:6 * [event deltaZ] right:-6 * [event deltaX] up:6 * [event deltaY]];
        else
            [camera moveForward:6 * [event deltaX] right:-6 * [event deltaY] up:6 * [event deltaZ]];
    }
}

- (void)handleMagnify:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {
    if (![self isCameraModifierPressed])
        return;
    
    Camera* camera = [windowController camera];
    if ([self isCameraOrbitModifierPressed])
        [camera setZoom:[camera zoom] - [event magnification] / 2];
    else
        [camera moveForward:160 * [event magnification] right:0 up:0];
}

- (void)dealloc {
    [windowController release];
    [super dealloc];
}

@end
