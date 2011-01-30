//
//  InputManager.m
//  TrenchBroom
//
//  Created by Kristian Duske on 26.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "InputManager.h"
#import <OpenGL/glu.h>
#import "Camera.h"
#import "Picker.h"
#import "PickingHit.h"
#import "MapView3D.h"
#import "Ray3D.h"
#import "Vector3f.h"

@implementation InputManager
- (id)initWithPicker:(Picker *)thePicker {
    if (thePicker == nil)
        [NSException raise:NSInvalidArgumentException format:@"picker must not be nil"];
    
    if (self = [self init]) {
        picker = [thePicker retain];
    }
    
    return self;
}

- (NSUInteger)modifierFlagsForCameraLook {
    return NSAlternateKeyMask;
}

- (void)handleKeyDown:(NSEvent *)event sender:(id)sender {
    MapView3D* mapView3D = (MapView3D *)sender;
    Camera* camera = [mapView3D camera];

    switch ([event keyCode]) {
        case 0: // a
            [camera moveForward:0 right:-20 up:0];
            break;
        case 1: // s
            [camera moveForward:-20 right:0 up:0];
            break;
        case 2: // d
            [camera moveForward:0 right:20 up:0];
            break;
        case 13: // w
            [camera moveForward:20 right:0 up:0];
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

- (void)handleMouseMoved:(NSEvent *)event sender:(id)sender {
}

- (void)handleMouseDown:(NSEvent *)event sender:(id)sender {
    MapView3D* mapView3D = (MapView3D *)sender;
    [[mapView3D openGLContext] makeCurrentContext];
    Camera* camera = [mapView3D camera];

    NSPoint m = [mapView3D convertPointFromBase:[event locationInWindow]];
    
    Vector3f* direction = [camera unprojectX:m.x y:m.y];
    Vector3f* origin = [camera position];
    
    [direction sub:origin];
    [direction normalize];
    
    NSLog(@"coords:%@", direction);
    
    Ray3D* ray = [[Ray3D alloc] initWithOrigin:origin direction:direction];
    NSArray* hits = [picker objectsHitByRay:ray];
    
    NSEnumerator* hitEn = [hits objectEnumerator];
    PickingHit* hit;
    while ((hit = [hitEn nextObject])) {
        NSLog(@"hit object is %@", [hit object]);
    }
}

- (void)handleMouseUp:(NSEvent *)event sender:(id)sender {
}

- (void)dealloc {
    [picker release];
    [super dealloc];
}

@end
