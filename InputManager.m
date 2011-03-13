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
#import "SelectionManager.h"
#import "Face.h"
#import "Brush.h"
#import "Vector3i.h"
#import "MapWindowController.h"
#import "MapDocument.h"
#import "BrushTool.h"

@implementation InputManager
- (id)initWithWindowController:(MapWindowController *)theWindowController {
    if (theWindowController == nil)
        [NSException raise:NSInvalidArgumentException format:@"window controller must not be nil"];
    
    if (self = [self init]) {
        windowController = [theWindowController retain];
    }
    
    return self;
}

- (BOOL)isCameraModifierPressed:(NSEvent *)event {
    return ([event modifierFlags] & NSShiftKeyMask) != 0;
}

- (BOOL)isCameraOrbitModifierPressed:(NSEvent *)event {
    return [self isCameraModifierPressed:event] && ([event modifierFlags] & NSCommandKeyMask) != 0;
}

- (void)handleKeyDown:(NSEvent *)event sender:(id)sender {
    Camera* camera = [windowController camera];

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
        case 49: // space bar
            break;
        default:
            break;
    }
}

- (void)handleLeftMouseDragged:(NSEvent *)event sender:(id)sender {
    Camera* camera = [windowController camera];
    if ([self isCameraModifierPressed:event]) {
        if ([self isCameraOrbitModifierPressed:event]) {
            if (lastHit != nil) {
                float h = -[event deltaX] / 70;
                float v = [event deltaY] / 70;
                [camera orbitCenter:[lastHit hitPoint] hAngle:h vAngle:v];
            }
        } else {
            float yaw = -[event deltaX] / 70;
            float pitch = [event deltaY] / 70;
            [camera rotateYaw:yaw pitch:pitch];
        }
    } else if (brushTool != nil) {
        MapView3D* mapView3D = (MapView3D *)sender;
        [[mapView3D openGLContext] makeCurrentContext];
        Camera* camera = [windowController camera];
        
        NSPoint m = [mapView3D convertPointFromBase:[event locationInWindow]];
        Ray3D* ray = [camera pickRayX:m.x y:m.y];

        [brushTool translateTo:ray toggleSnap:NO altPlane:NO];
    }
    
    drag = YES;
}

- (void)handleMouseMoved:(NSEvent *)event sender:(id)sender {
}

- (void)handleLeftMouseDown:(NSEvent *)event sender:(id)sender {
    MapView3D* mapView3D = (MapView3D *)sender;
    [[mapView3D openGLContext] makeCurrentContext];
    Camera* camera = [windowController camera];
    
    NSPoint m = [mapView3D convertPointFromBase:[event locationInWindow]];
    Ray3D* ray = [camera pickRayX:m.x y:m.y];
    
    Picker* picker = [[windowController document] picker];
    [lastHit release];
    
    NSArray* hits = [picker objectsHitByRay:ray];
    if ([hits count] > 0)
        lastHit = [[hits objectAtIndex:0] retain];
    else
        lastHit = nil;
    
    if (lastHit != nil && ![self isCameraModifierPressed:event]) {
        SelectionManager* selectionManager = [windowController selectionManager];
        if ([selectionManager isBrushSelected:[[lastHit object] brush]]) {
            MapDocument* map = [windowController document];
            NSUndoManager* undoManager = [map undoManager];
            [undoManager beginUndoGrouping];
            brushTool = [[BrushTool alloc] initWithController:windowController pickHit:lastHit pickRay:ray];
        }
    }
}

- (void)handleLeftMouseUp:(NSEvent *)event sender:(id)sender {
    if (![self isCameraModifierPressed:event] && !drag) {
        SelectionManager* selectionManager = [windowController selectionManager];
        if (lastHit != nil) {
            id <Face> face = [lastHit object];
            id <Brush> brush = [face brush];
            
            if ([selectionManager mode] == SM_FACES) {
                if ([selectionManager isFaceSelected:face]) {
                    [selectionManager removeFace:face];
                } else {
                    if (([event modifierFlags] & NSCommandKeyMask) == 0) {
                        if ([selectionManager hasSelectedFaces:brush]) {
                            [selectionManager removeAll];
                            [selectionManager addFace:face];
                        } else {
                            [selectionManager addBrush:brush];
                        }
                    } else {
                        [selectionManager addFace:face];
                    }
                }
            } else {
                if (([event modifierFlags] & NSCommandKeyMask) == 0) {
                    if ([selectionManager isBrushSelected:brush]) {
                        [selectionManager addFace:face];
                    } else {
                        [selectionManager removeAll];
                        [selectionManager addBrush:brush];
                    }
                } else {
                    if ([selectionManager isBrushSelected:brush]) {
                        [selectionManager removeBrush:brush];
                    } else {
                        [selectionManager addBrush:brush];
                    }
                }
            }
        } else {
            [selectionManager removeAll];
        }
    }
    
    if (brushTool != nil) {
        MapDocument* map = [windowController document];
        NSUndoManager* undoManager = [map undoManager];
        [undoManager endUndoGrouping];
        [undoManager setActionName:@"Move Brushes"];
        [brushTool release];
        brushTool = nil;
    }
    
    drag = NO;
}

- (void)handleRightMouseDragged:(NSEvent *)event sender:(id)sender {
    if ([self isCameraModifierPressed:event]) {
        Camera* camera = [windowController camera];
        [camera moveForward:0 right:6 * [event deltaX] up:-6 * [event deltaY]];
    }
}

- (void)handleScrollWheel:(NSEvent *)event sender:(id)sender {
    if ([self isCameraModifierPressed:event]) {
        Camera* camera = [windowController camera];
        if (gesture)
            [camera moveForward:6 * [event deltaZ] right:-6 * [event deltaX] up:6 * [event deltaY]];
        else
            [camera moveForward:6 * [event deltaX] right:-6 * [event deltaY] up:6 * [event deltaZ]];
    }
}

- (void)handleBeginGesture:(NSEvent *)event sender:(id)sender {
    gesture = YES;
}

- (void)handleEndGesture:(NSEvent *)event sender:(id)sender {
    gesture = NO;
}

- (void)handleMagnify:(NSEvent *)event sender:(id)sender {
    if ([self isCameraModifierPressed:event]) {
        Camera* camera = [windowController camera];
        [camera moveForward:160 * [event magnification] right:0 up:0];
    }
}

- (void)dealloc {
    [lastHit release];
    [windowController release];
    [super dealloc];
}

@end
