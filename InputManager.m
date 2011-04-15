//
//  InputManager.m
//  TrenchBroom
//
//  Created by Kristian Duske on 26.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "InputManager.h"
#import <OpenGL/glu.h>
#import "CameraTool.h"
#import "SelectionTool.h"
#import "BrushTool.h"
#import "FaceTool.h"
#import "ClipTool.h"
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
#import "CursorManager.h"

@interface InputManager (private)

- (void)updateEvent:(NSEvent *)event;
- (void)updateRay;
- (void)updateHits;

- (void)updateCursor;
- (void)updateCursorOwner;
- (void)cameraViewChanged:(NSNotification *)notification;

@end

@implementation InputManager (private)

- (void)updateEvent:(NSEvent *)event {
    BOOL rayChanged = YES;
    if (lastEvent != nil)
        rayChanged = [lastEvent locationInWindow].x != [event locationInWindow].x || [lastEvent locationInWindow].y != [event locationInWindow].y;
    
    [lastEvent release];
    lastEvent = [event retain];
    
    [lastRay release];
    lastRay = nil;
    [lastHits release];
    lastHits = nil;
}

- (void)updateRay {
    [lastRay release];
    [lastHits release];
    lastHits = nil;
    
    
    MapView3D* mapView3D = [windowController view3D];
    Camera* camera = [windowController camera];
    
    NSPoint m = [mapView3D convertPointFromBase:[lastEvent locationInWindow]];
    lastRay = [[camera pickRayX:m.x y:m.y] retain];
}

- (void)updateHits {
    Picker* picker = [[windowController document] picker];
    lastHits = [[picker pickObjects:lastRay include:nil exclude:nil] retain];
}

- (void)updateCursorOwner {
    id <Tool> newOwner = nil;
    if ([cameraTool hasCursor:lastEvent ray:lastRay hits:lastHits])
        newOwner = cameraTool;
    else if ([clipTool hasCursor:lastEvent ray:lastRay hits:lastHits])
        newOwner = clipTool;
    else if ([brushTool hasCursor:lastEvent ray:lastRay hits:lastHits])
        newOwner = brushTool;
    else if ([faceTool hasCursor:lastEvent ray:lastRay hits:lastHits])
        newOwner = faceTool;
    else if ([selectionTool hasCursor:lastEvent ray:lastRay hits:lastHits])
        newOwner = selectionTool;
    
    if (newOwner != cursorOwner) {
        if (cursorOwner != nil)
            [cursorOwner unsetCursor:lastEvent ray:lastRay hits:lastHits];
        cursorOwner = newOwner;
        if (cursorOwner != nil)
            [cursorOwner setCursor:lastEvent ray:lastRay hits:lastHits];
    }
}

- (void)updateCursor {
    if (cursorOwner != nil) {
        [cursorOwner updateCursor:lastEvent ray:lastRay hits:lastHits];
        
        Camera* camera = [windowController camera];
        CursorManager* cursorManager = [windowController cursorManager];
        PickingHit* hit = [lastHits firstHitOfType:HT_FACE ignoreOccluders:YES];
        
        if (hit != nil)
            [cursorManager updateCursor:[hit hitPoint]];
        else
            [cursorManager updateCursor:[camera defaultPoint]];
        
        MapView3D* view3D = [windowController view3D];
        [view3D setNeedsDisplay:YES];
    }
}

- (void)cameraViewChanged:(NSNotification *)notification {
    [self updateRay];
    [self updateHits];
    [self updateCursorOwner];
    [self updateCursor];
}

@end

@implementation InputManager
- (id)initWithWindowController:(MapWindowController *)theWindowController {
    if (self = [self init]) {
        windowController = [theWindowController retain];
        
        cameraTool = [[CameraTool alloc] initWithWindowController:windowController];
        selectionTool = [[SelectionTool alloc] initWithWindowController:windowController];
        brushTool = [[BrushTool alloc] initWithController:windowController];
        faceTool = [[FaceTool alloc] initWithController:windowController];
        clipTool = [[ClipTool alloc] initWithWindowController:windowController];
        
        Camera* camera = [windowController camera];
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center addObserver:self selector:@selector(cameraViewChanged:) name:CameraViewChanged object:camera];
    }
    
    return self;
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

- (void)handleFlagsChanged:(NSEvent *)event sender:(id)sender {
    [cameraTool     handleFlagsChanged:event ray:lastRay hits:lastHits];
    [clipTool       handleFlagsChanged:event ray:lastRay hits:lastHits];
    [brushTool      handleFlagsChanged:event ray:lastRay hits:lastHits];
    [faceTool       handleFlagsChanged:event ray:lastRay hits:lastHits];
    [selectionTool  handleFlagsChanged:event ray:lastRay hits:lastHits];
    
    [self updateCursorOwner];
    [self updateCursor];
}

- (void)handleLeftMouseDragged:(NSEvent *)event sender:(id)sender {
    if (dragTool == nil) {
        if ([cameraTool beginLeftDrag:event ray:lastRay hits:lastHits])
            dragTool = cameraTool;
        else if ([clipTool beginLeftDrag:event ray:lastRay hits:lastHits])
            dragTool = clipTool;
        else if ([brushTool beginLeftDrag:event ray:lastRay hits:lastHits])
            dragTool = brushTool;
        else if ([faceTool beginLeftDrag:event ray:lastRay hits:lastHits])
            dragTool = faceTool;
        else if ([selectionTool beginLeftDrag:event ray:lastRay hits:lastHits])
            dragTool = selectionTool;
    }
    
    [self updateEvent:event];
    [self updateRay];
    [self updateHits];
    if (dragTool != nil)
        [dragTool leftDrag:lastEvent ray:lastRay hits:lastHits];
    
    [self updateCursorOwner];
    [self updateCursor];
}

- (void)handleMouseMoved:(NSEvent *)event sender:(id)sender {
    [self updateEvent:event];
    [self updateRay];
    [self updateHits];

    [cameraTool     handleMouseMoved:event ray:lastRay hits:lastHits];
    [clipTool       handleMouseMoved:event ray:lastRay hits:lastHits];
    [brushTool      handleMouseMoved:event ray:lastRay hits:lastHits];
    [faceTool       handleMouseMoved:event ray:lastRay hits:lastHits];
    [selectionTool  handleMouseMoved:event ray:lastRay hits:lastHits];

    [self updateCursorOwner];
    [self updateCursor];
}

- (void)handleLeftMouseDown:(NSEvent *)event sender:(id)sender {
    [cameraTool     handleLeftMouseDown:event ray:lastRay hits:lastHits];
    [clipTool       handleLeftMouseDown:event ray:lastRay hits:lastHits];
    [brushTool      handleLeftMouseDown:event ray:lastRay hits:lastHits];
    [faceTool       handleLeftMouseDown:event ray:lastRay hits:lastHits];
    [selectionTool  handleLeftMouseDown:event ray:lastRay hits:lastHits];

    [self updateCursor];
}

- (void)handleLeftMouseUp:(NSEvent *)event sender:(id)sender {
    if (dragTool != nil) {
        [dragTool endLeftDrag:event ray:lastRay hits:lastHits];
        dragTool = nil;
    } else {
        [cameraTool     handleLeftMouseUp:event ray:lastRay hits:lastHits];
        [clipTool       handleLeftMouseUp:event ray:lastRay hits:lastHits];
        [brushTool      handleLeftMouseUp:event ray:lastRay hits:lastHits];
        [faceTool       handleLeftMouseUp:event ray:lastRay hits:lastHits];
        [selectionTool  handleLeftMouseUp:event ray:lastRay hits:lastHits];
    }

    [self updateCursor];
}

- (void)handleRightMouseDragged:(NSEvent *)event sender:(id)sender {
    if (dragTool == nil) {
        if ([cameraTool beginRightDrag:event ray:lastRay hits:lastHits])
            dragTool = cameraTool;
        else if ([clipTool beginRightDrag:event ray:lastRay hits:lastHits])
            dragTool = clipTool;
        else if ([brushTool beginRightDrag:event ray:lastRay hits:lastHits])
            dragTool = brushTool;
        else if ([faceTool beginRightDrag:event ray:lastRay hits:lastHits])
            dragTool = faceTool;
        else if ([selectionTool beginRightDrag:event ray:lastRay hits:lastHits])
            dragTool = selectionTool;
    }
    
    [self updateEvent:event];
    [self updateRay];
    [self updateHits];
    if (dragTool != nil)
        [dragTool rightDrag:lastEvent ray:lastRay hits:lastHits];
    
    [self updateCursorOwner];
    [self updateCursor];
}

- (void)handleRightMouseDown:(NSEvent *)event sender:(id)sender {
    [cameraTool     handleRightMouseDown:event ray:lastRay hits:lastHits];
    [clipTool       handleRightMouseDown:event ray:lastRay hits:lastHits];
    [brushTool      handleRightMouseDown:event ray:lastRay hits:lastHits];
    [faceTool       handleRightMouseDown:event ray:lastRay hits:lastHits];
    [selectionTool  handleRightMouseDown:event ray:lastRay hits:lastHits];

    [self updateCursor];
}

- (void)handleRightMouseUp:(NSEvent *)event sender:(id)sender {
    if (dragTool != nil) {
        [dragTool endRightDrag:event ray:lastRay hits:lastHits];
        dragTool = nil;
    } else {
        [cameraTool     handleRightMouseUp:event ray:lastRay hits:lastHits];
        [clipTool       handleRightMouseUp:event ray:lastRay hits:lastHits];
        [brushTool      handleRightMouseUp:event ray:lastRay hits:lastHits];
        [faceTool       handleRightMouseUp:event ray:lastRay hits:lastHits];
        [selectionTool  handleRightMouseUp:event ray:lastRay hits:lastHits];
    }

    [self updateCursor];
}

- (void)handleScrollWheel:(NSEvent *)event sender:(id)sender {
    if (gestureTool != nil) {
        [gestureTool handleScrollWheel:event ray:lastRay hits:lastHits];
    } else {
        [cameraTool     handleScrollWheel:event ray:lastRay hits:lastHits];
        [clipTool       handleScrollWheel:event ray:lastRay hits:lastHits];
        [brushTool      handleScrollWheel:event ray:lastRay hits:lastHits];
        [faceTool       handleScrollWheel:event ray:lastRay hits:lastHits];
        [selectionTool  handleScrollWheel:event ray:lastRay hits:lastHits];
    }

    [self updateCursor];
}

- (void)handleBeginGesture:(NSEvent *)event sender:(id)sender {
    if ([cameraTool handleBeginGesture:event ray:lastRay hits:lastHits])
        gestureTool = cameraTool;
    else if ([clipTool handleBeginGesture:event ray:lastRay hits:lastHits])
        gestureTool = clipTool;
    else if ([brushTool handleBeginGesture:event ray:lastRay hits:lastHits])
        gestureTool = brushTool;
    else if ([faceTool handleBeginGesture:event ray:lastRay hits:lastHits])
        gestureTool = faceTool;
    else if ([selectionTool handleBeginGesture:event ray:lastRay hits:lastHits])
        gestureTool = selectionTool;

    [self updateCursor];
}

- (void)handleEndGesture:(NSEvent *)event sender:(id)sender {
    if (gestureTool != nil) {
        [gestureTool handleEndGesture:event ray:lastRay hits:lastHits];
        gestureTool = nil;
    }

    [self updateCursor];
}

- (void)handleMagnify:(NSEvent *)event sender:(id)sender {
    if (gestureTool != nil) {
        [gestureTool handleMagnify:event ray:lastRay hits:lastHits];
    } else {
        [cameraTool     handleMagnify:event ray:lastRay hits:lastHits];
        [clipTool       handleMagnify:event ray:lastRay hits:lastHits];
        [brushTool      handleMagnify:event ray:lastRay hits:lastHits];
        [faceTool       handleMagnify:event ray:lastRay hits:lastHits];
        [selectionTool  handleMagnify:event ray:lastRay hits:lastHits];
    }

    [self updateCursor];
}

- (ClipTool *)clipTool {
    return clipTool;
}

- (void)dealloc {
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    [cameraTool release];
    [selectionTool release];
    [brushTool release];
    [faceTool release];
    [clipTool release];
    [lastEvent release];
    [lastRay release];
    [lastHits release];
    [windowController release];
    [super dealloc];
}

@end
