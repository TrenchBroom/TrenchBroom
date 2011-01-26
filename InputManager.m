//
//  InputManager.m
//  TrenchBroom
//
//  Created by Kristian Duske on 26.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "InputManager.h"
#import "Camera.h"
#import "MapView3D.h"

@implementation InputManager
- (NSUInteger)modifierFlagsForCameraLook {
    return NSAlternateKeyMask;
}

- (void)handleKeyDown:(NSEvent *)event sender:(id)sender {
    MapView3D* mapView3D = (MapView3D *)sender;
    Camera* camera = [mapView3D camera];

    switch ([event keyCode]) {
        case 0: // a
            [camera moveForward:0 right:-10 up:0];
            break;
        case 1: // s
            [camera moveForward:-10 right:0 up:0];
            break;
        case 2: // d
            [camera moveForward:0 right:10 up:0];
            break;
        case 13: // w
            [camera moveForward:10 right:0 up:0];
            break;
        default:
            break;
    }
}

- (void)handleMouseDragged:(NSEvent *)event sender:(id)sender {
    if ([sender isKindOfClass:[MapView3D class]]) {
        MapView3D* mapView3D = (MapView3D *)sender;
        Camera* camera = [mapView3D camera];
        
        float yaw = -[event deltaX] / 50;
        float pitch = [event deltaY] / 50;
        [camera rotateYaw:yaw pitch:pitch];
    }
}

@end
