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
#import "ToolManager.h"
#import "FaceOffsetTool.h"

@implementation InputManager
- (id)initWithPicker:(Picker *)thePicker selectionManager:(SelectionManager *)theSelectionManager toolManager:(ToolManager *)theToolManager {
    if (thePicker == nil)
        [NSException raise:NSInvalidArgumentException format:@"picker must not be nil"];
    if (theSelectionManager == nil)
        [NSException raise:NSInvalidArgumentException format:@"selection manager must not be nil"];
    
    if (self = [self init]) {
        picker = [thePicker retain];
        selectionManager = [theSelectionManager retain];
        toolManager = [theToolManager retain];
        currentTools = [[NSMutableArray alloc] init];
    }
    
    return self;
}

- (BOOL)isCameraModifierPressed:(NSEvent *)event {
    return ([event modifierFlags] & NSAlternateKeyMask) != 0;
}

- (BOOL)isCameraOrbitModifierPressed:(NSEvent *)event {
    return [self isCameraModifierPressed:event] && ([event modifierFlags] & NSCommandKeyMask) != 0;
}

- (void)handleKeyDown:(NSEvent *)event sender:(id)sender {
    MapView3D* mapView3D = (MapView3D *)sender;
    Camera* camera = [[[mapView3D window] windowController] camera];

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
        case 124: {// right arrow
            NSSet* brushes = [selectionManager selectedBrushes];
            NSEnumerator* brushEn = [brushes objectEnumerator];
            Brush* brush;
            while ((brush = [brushEn nextObject]))
                [brush translateBy:[Vector3i vectorWithX:32 y:32 z:32]];
            break;
        }
        default:
            NSLog(@"unrecognized key code: %i", [event keyCode]);
            break;
    }
}

- (void)handleLeftMouseDragged:(NSEvent *)event sender:(id)sender {
    MapView3D* mapView3D = (MapView3D *)sender;
    if ([self isCameraModifierPressed:event]) {
        Camera* camera = [[[mapView3D window] windowController] camera];
        
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
    } else if ([currentTools count] > 0) {
        Camera* camera = [[[mapView3D window] windowController] camera];
        
        NSPoint m = [mapView3D convertPointFromBase:[event locationInWindow]];
        Ray3D* ray = [camera pickRayX:m.x y:m.y];
        
        NSEnumerator* toolEn = [currentTools objectEnumerator];
        id tool;
        while ((tool = [toolEn nextObject]))
            [tool drag:ray];
    }
}

- (void)handleMouseMoved:(NSEvent *)event sender:(id)sender {
}

- (void)handleLeftMouseDown:(NSEvent *)event sender:(id)sender {
    MapView3D* mapView3D = (MapView3D *)sender;
    [[mapView3D openGLContext] makeCurrentContext];
    Camera* camera = [[[mapView3D window] windowController] camera];
    
    NSPoint m = [mapView3D convertPointFromBase:[event locationInWindow]];
    Ray3D* ray = [camera pickRayX:m.x y:m.y];

    NSArray* tools = [toolManager toolsHitByRay:ray];
    [currentTools setArray:tools];
        
    if ([currentTools count] == 0) {
        NSArray* hits = [picker objectsHitByRay:ray];
        
        [lastHit release];
        if ([hits count] > 0)
            lastHit = [[hits objectAtIndex:0] retain];
        else
            lastHit = nil;
        
        if (![self isCameraModifierPressed:event]) {
            if (lastHit != nil) {
                Face* face = [lastHit object];
                Brush* brush = [face brush];
                
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
                    if ([selectionManager isBrushSelected:brush]) {
                        [selectionManager addFace:face];
                    } else {
                        if (([event modifierFlags] & NSCommandKeyMask) == 0)
                            [selectionManager removeAll];
                        [selectionManager addBrush:brush];
                    }
                }
            } else {
                [selectionManager removeAll];
            }
        }
    } else {
        NSEnumerator* toolEn = [currentTools objectEnumerator];
        id tool;
        while ((tool = [toolEn nextObject]))
            [tool startDrag:ray];
    }
}

- (void)handleLeftMouseUp:(NSEvent *)event sender:(id)sender {
    MapView3D* mapView3D = (MapView3D *)sender;
    Camera* camera = [[[mapView3D window] windowController] camera];
    
    NSPoint m = [mapView3D convertPointFromBase:[event locationInWindow]];
    Ray3D* ray = [camera pickRayX:m.x y:m.y];

    NSEnumerator* toolEn = [currentTools objectEnumerator];
    id tool;
    while ((tool = [toolEn nextObject]))
        [tool endDrag:ray];
}

- (void)handleRightMouseDragged:(NSEvent *)event sender:(id)sender {
    MapView3D* mapView3D = (MapView3D *)sender;
    if ([self isCameraModifierPressed:event]) {
        Camera* camera = [[[mapView3D window] windowController] camera];
        [camera moveForward:0 right:6 * [event deltaX] up:-6 * [event deltaY]];
    }
}

- (void)handleScrollWheel:(NSEvent *)event sender:(id)sender {
    MapView3D* mapView3D = (MapView3D *)sender;
    
    if ([self isCameraModifierPressed:event]) {
        Camera* camera = [[[mapView3D window] windowController] camera];
        [camera moveForward:6 * [event deltaY] right:-6 * [event deltaX] up:6 * [event deltaZ]];
    }
}

- (void)dealloc {
    [currentTools release];
    [toolManager release];
    [selectionManager release];
    [lastHit release];
    [picker release];
    [super dealloc];
}

@end
