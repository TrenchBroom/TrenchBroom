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

- (void)updateRay:(NSEvent *)event sender:(id)sender;
- (void)updateHits;
- (void)updateCursor;

@end

@implementation InputManager (private)

- (void)updateRay:(NSEvent *)event sender:(id)sender {
    [lastEvent release];
    [lastRay release];
    [lastHits release];
    lastHits = nil;
    
    lastEvent = [event retain];
    
    MapView3D* mapView3D = (MapView3D *)sender;
    Camera* camera = [windowController camera];
    
    NSPoint m = [mapView3D convertPointFromBase:[event locationInWindow]];
    lastRay = [[camera pickRayX:m.x y:m.y] retain];
}

- (void)updateHits {
    Picker* picker = [[windowController document] picker];
    lastHits = [[picker pickObjects:lastRay include:nil exclude:nil] retain];
}

- (void)updateCursor {
    [cameraTool updateCursor:lastEvent ray:lastRay hits:lastHits];
    [selectionTool updateCursor:lastEvent ray:lastRay hits:lastHits];
    [brushTool updateCursor:lastEvent ray:lastRay hits:lastHits];
    [faceTool updateCursor:lastEvent ray:lastRay hits:lastHits];
    [clipTool updateCursor:lastEvent ray:lastRay hits:lastHits];
    
    Camera* camera = [windowController camera];
    CursorManager* cursorManager = [windowController cursorManager];
    PickingHit* hit = [lastHits firstHitOfType:HT_FACE ignoreOccluders:YES];

    if (hit != nil)
        [cursorManager updateCursor:[hit hitPoint]];
    else
        [cursorManager updateCursor:[camera defaultPoint]];
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
    }
    
    return self;
}

- (BOOL)isSelectionModifierPressed:(NSEvent *)event {
    return [event modifierFlags] == 256 || ([event modifierFlags] & NSCommandKeyMask) == NSCommandKeyMask; // this might break
}

- (BOOL)isCameraModifierPressed:(NSEvent *)event {
    return ([event modifierFlags] & NSShiftKeyMask) == NSShiftKeyMask;
}

- (BOOL)isCameraOrbitModifierPressed:(NSEvent *)event {
    return ([event modifierFlags] & (NSShiftKeyMask | NSCommandKeyMask)) == (NSShiftKeyMask | NSCommandKeyMask);
}

- (BOOL)isApplyTextureModifierPressed:(NSEvent *)event {
    return ([event modifierFlags] & NSAlternateKeyMask) == NSAlternateKeyMask;
}

- (BOOL)isApplyTextureAndAttributesModifierPressed:(NSEvent *)event {
    return ([event modifierFlags] & (NSAlternateKeyMask | NSCommandKeyMask)) == (NSAlternateKeyMask | NSCommandKeyMask);
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
    [self updateCursor];
}

- (void)handleLeftMouseDragged:(NSEvent *)event sender:(id)sender {
    if (dragTool == nil) {
        SelectionManager* selectionManager = [windowController selectionManager];
        PickingHit* hit = [lastHits firstHitOfType:HT_ANY ignoreOccluders:NO];
        if ([self isCameraModifierPressed:event])
            dragTool = cameraTool;
        else if ([selectionManager mode] == SM_GEOMETRY && hit != nil)
            dragTool = brushTool;
        else if ([selectionManager mode] == SM_FACES && hit != nil)
            dragTool = faceTool;
        else
            dragTool = selectionTool;
            
        [dragTool beginLeftDrag:lastEvent ray:lastRay hits:lastHits];
    }
    
    [self updateRay:event sender:sender];
    [self updateHits];
    [dragTool leftDrag:lastEvent ray:lastRay hits:lastHits];
    [self updateRay:event sender:sender];
    [self updateHits];
    [self updateCursor];
}

- (void)handleMouseMoved:(NSEvent *)event sender:(id)sender {
    [self updateRay:event sender:sender];
    [self updateHits];
    [self updateCursor];
}

- (void)handleLeftMouseDown:(NSEvent *)event sender:(id)sender {
    SelectionManager* selectionManager = [windowController selectionManager];
    if ([selectionManager mode] == HT_FACE && [self isApplyTextureModifierPressed:event])
        [faceTool handleLeftMouseDown:event ray:lastRay hits:lastHits];
    [self updateCursor];
}

- (void)handleLeftMouseUp:(NSEvent *)event sender:(id)sender {
    if (dragTool == nil) {
        if ([self isSelectionModifierPressed:event])
            [selectionTool handleLeftMouseUp:event ray:lastRay hits:lastHits];
    } else {
        [dragTool endLeftDrag:event ray:lastRay hits:lastHits];
        dragTool = nil;
    }
    [self updateCursor];
}

- (void)handleRightMouseDragged:(NSEvent *)event sender:(id)sender {
    if (dragTool == nil) {
        if ([self isCameraModifierPressed:event])
            dragTool = cameraTool;
        
        [dragTool beginRightDrag:lastEvent ray:lastRay hits:lastHits];
    }
    
    [self updateRay:event sender:sender];
    [self updateHits];
    [dragTool rightDrag:lastEvent ray:lastRay hits:lastHits];
    [self updateRay:event sender:sender];
    [self updateHits];
    [self updateCursor];
}

- (void)handleRightMouseDown:(NSEvent *)event sender:(id)sender {
    [self updateCursor];
}

- (void)handleRightMouseUp:(NSEvent *)event sender:(id)sender {
    if (dragTool != nil) {
        [dragTool endRightDrag:event ray:lastRay hits:lastHits];
        dragTool = nil;
    }
    [self updateCursor];
}

- (void)handleScrollWheel:(NSEvent *)event sender:(id)sender {
    if ([self isCameraModifierPressed:event]) {
        [cameraTool handleScrollWheel:event ray:lastRay hits:lastHits];
        [self updateHits];
        [self updateCursor];
    } else {
        [self updateCursor];
    }
}

- (void)handleBeginGesture:(NSEvent *)event sender:(id)sender {
    if ([self isCameraModifierPressed:event]) {
        gestureTool = cameraTool;
        [gestureTool handleBeginGesture:event ray:lastRay hits:lastHits];
    }
}

- (void)handleEndGesture:(NSEvent *)event sender:(id)sender {
    if (gestureTool != nil) {
        [gestureTool handleEndGesture:event ray:lastRay hits:lastHits];
        gestureTool = nil;
    }
}

- (void)handleMagnify:(NSEvent *)event sender:(id)sender {
    if ([self isCameraModifierPressed:event]) {
        [cameraTool handleMagnify:event ray:lastRay hits:lastHits];
        [self updateHits];
        [self updateCursor];
    } else {
        [self updateCursor];
    }
}

- (void)dealloc {
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
