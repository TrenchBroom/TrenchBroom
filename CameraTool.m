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

- (BOOL)isCameraOrbitModifierPressed;

@end

@implementation CameraTool (private)

- (BOOL)isCameraOrbitModifierPressed {
    return ([NSEvent modifierFlags] & NSCommandKeyMask) != 0;
}

@end

@implementation CameraTool

- (id)initWithWindowController:(MapWindowController *)theWindowController {
    if (self = [self init]) {
        windowController = [theWindowController retain];
    }
    
    return self;
}

- (void)beginLeftDrag:(NSEvent *)event ray:(Ray3D *)ray hits:(PickingHitList *)hits {
    if ([self isCameraOrbitModifierPressed]) {
        PickingHit* hit = [hits firstHitOfType:HT_ANY ignoreOccluders:YES];
        if (hit != nil) {
            orbitCenter = [[hit hitPoint] retain];
        } else {
            Camera* camera = [windowController camera];
            orbitCenter = [camera defaultPoint];
        }
    }
}

- (void)endLeftDrag:(NSEvent *)event ray:(Ray3D *)ray hits:(PickingHitList *)hits {
    if (orbitCenter != nil) {
        [orbitCenter release];
        orbitCenter = nil;
    }
}

- (void)leftDrag:(NSEvent *)event ray:(Ray3D *)ray hits:(PickingHitList *)hits {
    Camera* camera = [windowController camera];
    if (orbitCenter != nil) {
        float h = -[event deltaX] / 70;
        float v = [event deltaY] / 70;
        [camera orbitCenter:orbitCenter hAngle:h vAngle:v];
    } else {
        float yaw = -[event deltaX] / 70;
        float pitch = [event deltaY] / 70;
        [camera rotateYaw:yaw pitch:pitch];
    }
}

- (void)rightDrag:(NSEvent *)event ray:(Ray3D *)ray hits:(PickingHitList *)hits {
    Camera* camera = [windowController camera];
    [camera moveForward:0 right:6 * [event deltaX] up:-6 * [event deltaY]];
}

- (void)handleBeginGesture:(NSEvent *)event ray:(Ray3D *)ray hits:(PickingHitList *)hits {
    gesture = YES;
}

- (void)handleEndGesture:(NSEvent *)event ray:(Ray3D *)ray hits:(PickingHitList *)hits {
    gesture = NO;
}

- (void)handleScrollWheel:(NSEvent *)event ray:(Ray3D *)ray hits:(PickingHitList *)hits {
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

- (void)handleMagnify:(NSEvent *)event ray:(Ray3D *)ray hits:(PickingHitList *)hits {
    Camera* camera = [windowController camera];
    if ([self isCameraOrbitModifierPressed])
        [camera setZoom:[camera zoom] - [event magnification] / 2];
    else
        [camera moveForward:160 * [event magnification] right:0 up:0];
}

- (void)dealloc {
    if (orbitCenter != nil)
        [orbitCenter release];
    [windowController release];
    [super dealloc];
}

@end
