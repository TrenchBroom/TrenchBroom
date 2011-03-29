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
#import "PickingHitList.h"
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
#import "FaceTool.h"
#import "Options.h"
#import "TrackingManager.h"
#import "ClipTool.h"

@implementation InputManager
- (id)initWithWindowController:(MapWindowController *)theWindowController {
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

- (BOOL)handleKeyDown:(NSEvent *)event sender:(id)sender {
    switch ([event keyCode]) {
        case 48:
            if (clipTool != nil) {
                [clipTool toggleClipMode];
                return YES;
            }
            break;
        default:
//            NSLog(@"unknown key code: %i", [event keyCode]);
            break;
    }
    
    return NO;
}


- (void)handleLeftMouseDragged:(NSEvent *)event sender:(id)sender {
    if ([self isCameraModifierPressed:event]) {
        Camera* camera = [windowController camera];
        if ([self isCameraOrbitModifierPressed:event]) {
            if (lastHits != nil) {
                float h = -[event deltaX] / 70;
                float v = [event deltaY] / 70;
                PickingHit* lastHit = [lastHits firstHitOfType:HT_ANY ignoreOccluders:NO];
                [camera orbitCenter:[lastHit hitPoint] hAngle:h vAngle:v];
            }
        } else {
            float yaw = -[event deltaX] / 70;
            float pitch = [event deltaY] / 70;
            [camera rotateYaw:yaw pitch:pitch];
        }
    } else {
        MapView3D* mapView3D = (MapView3D *)sender;
        [[mapView3D openGLContext] makeCurrentContext];
        Camera* camera = [windowController camera];
        
        NSPoint m = [mapView3D convertPointFromBase:[event locationInWindow]];
        Ray3D* ray = [camera pickRayX:m.x y:m.y];

        if (tool != nil)
            [tool translateTo:ray toggleSnap:NO altPlane:NO];
        else if (clipTool != nil)
            [clipTool handleLeftMouseDragged:ray];
    }
    
    drag = YES;
}

- (void)handleMouseMoved:(NSEvent *)event sender:(id)sender {
    MapView3D* mapView3D = (MapView3D *)sender;
    [[mapView3D openGLContext] makeCurrentContext];
    Camera* camera = [windowController camera];
    
    NSPoint m = [mapView3D convertPointFromBase:[event locationInWindow]];
    Ray3D* ray = [camera pickRayX:m.x y:m.y];

    if (clipTool != nil)
        [clipTool handleMouseMoved:ray];
    
    TrackingManager* trackingManager = [windowController trackingManager];
    [trackingManager updateWithRay:ray];
}

- (void)handleLeftMouseDown:(NSEvent *)event sender:(id)sender {
    MapView3D* mapView3D = (MapView3D *)sender;
    [[mapView3D openGLContext] makeCurrentContext];
    Camera* camera = [windowController camera];
    
    NSPoint m = [mapView3D convertPointFromBase:[event locationInWindow]];
    Ray3D* ray = [camera pickRayX:m.x y:m.y];
    
    if (clipTool != nil && ![self isCameraModifierPressed:event]) {
        [clipTool handleLeftMouseDown:ray];
    } else {
        [lastHits release];
        Picker* picker = [[windowController document] picker];
        lastHits = [[picker pickObjects:ray include:nil exclude:nil] retain];
        
        if (lastHits != nil && ![self isCameraModifierPressed:event]) {
            PickingHit* lastHit = [lastHits firstHitOfType:HT_ANY ignoreOccluders:NO];
            SelectionManager* selectionManager = [windowController selectionManager];
            switch ([lastHit type]) {
                case HT_BRUSH:
                case HT_FACE: {
                    if ([selectionManager mode] == SM_GEOMETRY) {
                        id <Brush> brush = [lastHit type] == HT_BRUSH ? [lastHit object] : [[lastHit object] brush];
                        if ([selectionManager isBrushSelected:brush]) {
                            MapDocument* map = [windowController document];
                            NSUndoManager* undoManager = [map undoManager];
                            [undoManager beginUndoGrouping];
                            tool = [[BrushTool alloc] initWithController:windowController pickHit:lastHit pickRay:ray];
                        }
                    } else if ([selectionManager mode] == SM_FACES && [lastHit type] == HT_FACE) {
                        id <Face> face = [lastHit object];
                        if ([selectionManager isFaceSelected:face]) {
                            MapDocument* map = [windowController document];
                            NSUndoManager* undoManager = [map undoManager];
                            [undoManager beginUndoGrouping];
                            tool = [[FaceTool alloc] initWithController:windowController pickHit:lastHit pickRay:ray];
                        }
                    }
                    break;
                }
                case HT_EDGE:
                    break;
                case HT_VERTEX:
                    break;
                default:
                    break;
            }
        }
    }

    TrackingManager* trackingManager = [windowController trackingManager];
    [trackingManager updateWithRay:ray];
}

- (void)handleLeftMouseUp:(NSEvent *)event sender:(id)sender {
    MapView3D* mapView3D = (MapView3D *)sender;
    [[mapView3D openGLContext] makeCurrentContext];
    Camera* camera = [windowController camera];
    
    NSPoint m = [mapView3D convertPointFromBase:[event locationInWindow]];
    Ray3D* ray = [camera pickRayX:m.x y:m.y];
    
    if (clipTool != nil) {
        [clipTool handleLeftMouseUp:ray];
    } else {
        Options* options = [windowController options];
        if (![self isCameraModifierPressed:event] && !drag && [options isolationMode] == IM_NONE && lastHits != nil) {
            SelectionManager* selectionManager = [windowController selectionManager];
            PickingHit* lastHit = [lastHits firstHitOfType:HT_FACE ignoreOccluders:YES];
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
        if (tool != nil) {
            MapDocument* map = [windowController document];
            NSUndoManager* undoManager = [map undoManager];
            [undoManager endUndoGrouping];
            [undoManager setActionName:[tool actionName]];
            [tool release];
            tool = nil;
        }
    }
    TrackingManager* trackingManager = [windowController trackingManager];
    [trackingManager updateWithRay:ray];
    
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
        if ([self isCameraOrbitModifierPressed:event]) {
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
        if ([self isCameraOrbitModifierPressed:event])
            [camera setZoom:[camera zoom] - [event magnification] / 2];
        else
            [camera moveForward:160 * [event magnification] right:0 up:0];
    }
}

- (void)setClipTool:(ClipTool *)theClipTool {
    [clipTool release];
    clipTool = [theClipTool retain];
}

- (void)dealloc {
    [clipTool release];
    [tool release];
    [lastHits release];
    [windowController release];
    [super dealloc];
}

@end
