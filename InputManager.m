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

- (BOOL)isSelectionModifierPressed;
- (BOOL)isCameraModifierPressed;
- (BOOL)isCameraOrbitModifierPressed;
- (BOOL)isApplyTextureModifierPressed;
- (BOOL)isApplyTextureAndFlagsModifierPressed;

- (void)updateEvent:(NSEvent *)event;
- (void)updateRay;
- (void)updateHits;
- (void)updateCursorOwner;
- (void)updateCursor;
- (void)cameraViewChanged:(NSNotification *)notification;

@end

@implementation InputManager (private)

- (BOOL)isSelectionModifierPressed {
    NSUInteger flags = [NSEvent modifierFlags];
    return flags == 0 || (flags & NSCommandKeyMask) == NSCommandKeyMask; // this might break
}

- (BOOL)isCameraModifierPressed {
    return ([NSEvent modifierFlags] & NSShiftKeyMask) == NSShiftKeyMask;
}

- (BOOL)isCameraOrbitModifierPressed {
    return ([NSEvent modifierFlags] & (NSShiftKeyMask | NSCommandKeyMask)) == (NSShiftKeyMask | NSCommandKeyMask);
}

- (BOOL)isApplyTextureModifierPressed {
    return ([NSEvent modifierFlags] & NSAlternateKeyMask) == NSAlternateKeyMask;
}

- (BOOL)isApplyTextureAndFlagsModifierPressed {
    return ([NSEvent modifierFlags] & (NSAlternateKeyMask | NSCommandKeyMask)) == (NSAlternateKeyMask | NSCommandKeyMask);
}

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
    id <Tool> newOwner = selectionTool;
    SelectionManager* selectionManager = [windowController selectionManager];
    if ([self isCameraModifierPressed]) {
        newOwner = cameraTool;
    } else if ([selectionManager mode] == SM_GEOMETRY) {
        PickingHit* hit = [lastHits firstHitOfType:HT_BRUSH ignoreOccluders:YES];
        if (hit != nil && [selectionManager isBrushSelected:[hit object]])
            newOwner = brushTool;
    } else if ([selectionManager mode] == SM_FACES) {
        PickingHit* hit = [lastHits firstHitOfType:HT_FACE ignoreOccluders:YES];
        if (hit != nil) {
            if ([selectionManager isFaceSelected:[hit object]] || 
                ([[selectionManager selectedFaces] count] == 1 && [self isApplyTextureModifierPressed]))
                newOwner = faceTool;
        }
    }
    
    if (newOwner != cursorOwner) {
        [cursorOwner unsetCursor:lastEvent ray:lastRay hits:lastHits];
        cursorOwner = newOwner;
        [cursorOwner setCursor:lastEvent ray:lastRay hits:lastHits];
    }
    
    [self updateCursor];
}

- (void)updateCursor {
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

- (void)cameraViewChanged:(NSNotification *)notification {
    [self updateRay];
    [self updateHits];
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
        cursorOwner = selectionTool;
        
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
    [self updateCursorOwner];
}

- (void)handleLeftMouseDragged:(NSEvent *)event sender:(id)sender {
    if (dragTool == nil) {
        SelectionManager* selectionManager = [windowController selectionManager];
        PickingHit* hit = [lastHits firstHitOfType:HT_ANY ignoreOccluders:NO];
        if ([self isCameraModifierPressed])
            dragTool = cameraTool;
        else if ([selectionManager mode] == SM_GEOMETRY && hit != nil)
            dragTool = brushTool;
        else if ([selectionManager mode] == SM_FACES && hit != nil)
            dragTool = faceTool;
        else
            dragTool = selectionTool;

        [dragTool beginLeftDrag:lastEvent ray:lastRay hits:lastHits];
        [self updateCursorOwner];
        [self updateCursor];
    }
    
    [self updateEvent:event];
    [self updateRay];
    [self updateHits];
    [dragTool leftDrag:lastEvent ray:lastRay hits:lastHits];
    [self updateCursorOwner];
}

- (void)handleMouseMoved:(NSEvent *)event sender:(id)sender {
    [self updateEvent:event];
    [self updateRay];
    [self updateHits];
    [self updateCursorOwner];
}

- (void)handleLeftMouseDown:(NSEvent *)event sender:(id)sender {
    SelectionManager* selectionManager = [windowController selectionManager];
    if ([selectionManager mode] == SM_FACES && [self isApplyTextureModifierPressed])
        [faceTool handleLeftMouseDown:event ray:lastRay hits:lastHits];
    [self updateCursorOwner];
}

- (void)handleLeftMouseUp:(NSEvent *)event sender:(id)sender {
    if (dragTool == nil) {
        if ([self isSelectionModifierPressed] && ![self isApplyTextureModifierPressed])
            [selectionTool handleLeftMouseUp:event ray:lastRay hits:lastHits];
    } else {
        [dragTool endLeftDrag:event ray:lastRay hits:lastHits];
        dragTool = nil;
    }
    [self updateCursorOwner];
}

- (void)handleRightMouseDragged:(NSEvent *)event sender:(id)sender {
    if (dragTool == nil) {
        if ([self isCameraModifierPressed])
            dragTool = cameraTool;
        
        [dragTool beginRightDrag:lastEvent ray:lastRay hits:lastHits];
        [self updateCursor];
    }
    
    [self updateEvent:event];
    [self updateRay];
    [self updateHits];
    [dragTool rightDrag:lastEvent ray:lastRay hits:lastHits];
    [self updateCursorOwner];
}

- (void)handleRightMouseDown:(NSEvent *)event sender:(id)sender {
    [self updateCursorOwner];
}

- (void)handleRightMouseUp:(NSEvent *)event sender:(id)sender {
    if (dragTool != nil) {
        [dragTool endRightDrag:event ray:lastRay hits:lastHits];
        dragTool = nil;
    }
    [self updateCursorOwner];
}

- (void)handleScrollWheel:(NSEvent *)event sender:(id)sender {
    if ([self isCameraModifierPressed]) {
        [cameraTool handleScrollWheel:event ray:lastRay hits:lastHits];
        [self updateCursorOwner];
    }
    [self updateCursorOwner];
}

- (void)handleBeginGesture:(NSEvent *)event sender:(id)sender {
    if ([self isCameraModifierPressed]) {
        gestureTool = cameraTool;
        [gestureTool handleBeginGesture:event ray:lastRay hits:lastHits];
    }
    [self updateCursorOwner];
}

- (void)handleEndGesture:(NSEvent *)event sender:(id)sender {
    if (gestureTool != nil) {
        [gestureTool handleEndGesture:event ray:lastRay hits:lastHits];
        gestureTool = nil;
    }
    [self updateCursorOwner];
}

- (void)handleMagnify:(NSEvent *)event sender:(id)sender {
    if ([self isCameraModifierPressed]) {
        [cameraTool handleMagnify:event ray:lastRay hits:lastHits];
        [self updateCursorOwner];
    }
    [self updateCursorOwner];
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
