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
#import "MoveTool.h"
#import "FaceTool.h"
#import "ClipTool.h"
#import "Camera.h"
#import "Picker.h"
#import "PickingHit.h"
#import "PickingHitList.h"
#import "MapView3D.h"
#import "SelectionManager.h"
#import "Face.h"
#import "Brush.h"
#import "MapWindowController.h"
#import "MapDocument.h"
#import "MoveTool.h"
#import "FaceTool.h"
#import "Options.h"
#import "CursorManager.h"
#import "DefaultFilter.h"
#import "EntityDefinitionManager.h"
#import "EntityDefinition.h"

@interface InputManager (private)

- (BOOL)isCameraModifierPressed;
- (BOOL)isCameraOrbitModifierPressed;
- (BOOL)isApplyTextureModifierPressed;
- (BOOL)isApplyTextureAndFlagsModifierPressed;

- (void)updateEvent:(NSEvent *)event;
- (void)updateRay;
- (void)updateHits;
- (void)updateActiveTool;
- (void)updateCursor;
- (void)updateCursorOwner;
- (void)cameraViewChanged:(NSNotification *)notification;

- (void)showContextMenu;
@end

@implementation InputManager (private)

- (BOOL)isCameraModifierPressed {
    return [NSEvent modifierFlags] == NSShiftKeyMask;
}

- (BOOL)isCameraOrbitModifierPressed {
    return [NSEvent modifierFlags] == (NSShiftKeyMask | NSCommandKeyMask);
}

- (BOOL)isApplyTextureModifierPressed {
    return [NSEvent modifierFlags] == NSAlternateKeyMask;
}

- (BOOL)isApplyTextureAndFlagsModifierPressed {
    return [NSEvent modifierFlags] == (NSAlternateKeyMask | NSCommandKeyMask);
}

- (void)updateEvent:(NSEvent *)event {
    [lastEvent release];
    lastEvent = [event retain];
}

- (void)updateRay {
    MapView3D* mapView3D = [windowController view3D];
    Camera* camera = [windowController camera];
    
    NSPoint m = [mapView3D convertPointFromBase:[lastEvent locationInWindow]];
    lastRay = [camera pickRayX:m.x y:m.y];
}

- (void)updateHits {
    Picker* picker = [[windowController document] picker];
    [lastHits release];
    lastHits = [[picker pickObjects:&lastRay filter:filter] retain];
}

- (void)updateActiveTool {
    SelectionManager* selectionManager = [windowController selectionManager];

    if ([self isCameraModifierPressed] || [self isCameraOrbitModifierPressed])
        activeTool = cameraTool;
    else if ([clipTool active])
        activeTool = clipTool;
    else if (drag && ([selectionManager mode] == SM_BRUSHES || [selectionManager mode] == SM_ENTITIES || [selectionManager mode] == SM_BRUSHES_ENTITIES))
        activeTool = moveTool;
    else if ((drag && [selectionManager mode] == SM_FACES) || 
               ([selectionManager mode] == SM_FACES && [[selectionManager selectedFaces] count] == 1 && ([self isApplyTextureModifierPressed] || [self isApplyTextureAndFlagsModifierPressed])))
        activeTool = faceTool;
    else
        activeTool = selectionTool;
}

- (void)updateCursorOwner {
    if (!drag) {
        SelectionManager* selectionManager = [windowController selectionManager];
        
        id <Tool> newOwner = nil;
        if (hasMouse) {
            if ([self isCameraModifierPressed] || [self isCameraOrbitModifierPressed]) {
                newOwner = cameraTool;
            } else if ([clipTool active]) {
                newOwner = clipTool;
            } else if ([selectionManager mode] == SM_BRUSHES || [selectionManager mode] == SM_ENTITIES || [selectionManager mode] == SM_BRUSHES_ENTITIES) {
                PickingHit* hit = [lastHits firstHitOfType:HT_ANY ignoreOccluders:NO];
                if (hit != nil) {
                    switch ([hit type]) {
                        case HT_ENTITY: {
                            id <Entity> entity = [hit object];
                            if ([selectionManager isEntitySelected:entity])
                                newOwner = moveTool;
                            break;
                        }
                        case HT_BRUSH: {
                            id <Brush> brush = [hit object];
                            if ([selectionManager isBrushSelected:brush])
                                newOwner = moveTool;
                            break;
                        }
                        case HT_FACE: {
                            id <Face> face = [hit object];
                            id <Brush> brush = [face brush];
                            if ([selectionManager isBrushSelected:brush])
                                newOwner = moveTool;
                            break;
                        }
                    }
                }
            } else if ([selectionManager mode] == SM_FACES) {
                PickingHit* hit = [lastHits firstHitOfType:HT_FACE ignoreOccluders:YES];
                if (hit != nil) {
                    id <Face> face = [hit object];
                    if ([selectionManager isFaceSelected:face] || ([[selectionManager selectedFaces] count] == 1 && ([self isApplyTextureModifierPressed] || [self isApplyTextureAndFlagsModifierPressed])))
                        newOwner = faceTool;
                }
            }
        }
        
        if (newOwner != cursorOwner) {
            if (cursorOwner != nil)
                [cursorOwner unsetCursor:lastEvent ray:&lastRay hits:lastHits];
            cursorOwner = newOwner;
            if (cursorOwner != nil)
                [cursorOwner setCursor:lastEvent ray:&lastRay hits:lastHits];
        }
    }
}

- (void)updateCursor {
    if (cursorOwner != nil) {
        [cursorOwner updateCursor:lastEvent ray:&lastRay hits:lastHits];

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

- (void)showContextMenu {
    EntityDefinitionManager* entityDefinitionManager = [[windowController document] entityDefinitionManager];

    NSMenu* menu = [[NSMenu alloc] initWithTitle:@"3D View Context Menu"];

    NSMenuItem* createPointEntityItem = [menu insertItemWithTitle:@"Create Point Entity" action:nil keyEquivalent:@"" atIndex:0];
    NSMenu* pointEntityMenu = [[NSMenu alloc] initWithTitle:@"Create Point Entity"];
    [menu setSubmenu:[pointEntityMenu autorelease] forItem:createPointEntityItem];
    
    NSArray* pointDefinitions = [entityDefinitionManager definitionsOfType:EDT_POINT];
    for (int i = 0; i < [pointDefinitions count]; i++) {
        EntityDefinition* definition = [pointDefinitions objectAtIndex:i];
        NSMenuItem* definitionItem = [pointEntityMenu insertItemWithTitle:[definition name] action:@selector(createPointEntity:) keyEquivalent:@"" atIndex:i];
        [definitionItem setTag:i];
    }
    
    NSMenuItem* createBrushEntityItem = [menu insertItemWithTitle:@"Create Brush Entity" action:nil keyEquivalent:@"" atIndex:1];
    NSMenu* brushEntityMenu = [[NSMenu alloc] initWithTitle:@"Create Brush Entity"];
    [menu setSubmenu:[brushEntityMenu autorelease] forItem:createBrushEntityItem];
    
    NSArray* brushDefinitions = [entityDefinitionManager definitionsOfType:EDT_BRUSH];
    for (int i = 0; i < [brushDefinitions count]; i++) {
        EntityDefinition* definition = [brushDefinitions objectAtIndex:i];
        NSMenuItem* definitionItem = [brushEntityMenu insertItemWithTitle:[definition name] action:@selector(createBrushEntity:) keyEquivalent:@"" atIndex:i];
        [definitionItem setTag:i];
    }
    
    menuPosition = [lastEvent locationInWindow];
    [NSMenu popUpContextMenu:[menu autorelease] withEvent:lastEvent forView:[windowController view3D]];
}

@end

@implementation InputManager
- (id)initWithWindowController:(MapWindowController *)theWindowController {
    if (self = [self init]) {
        windowController = theWindowController; // do not retain

        SelectionManager* selectionManager = [windowController selectionManager];
        Options* options = [windowController options];
        filter = [[DefaultFilter alloc] initWithSelectionManager:selectionManager options:options];
        
        cameraTool = [[CameraTool alloc] initWithWindowController:windowController];
        selectionTool = [[SelectionTool alloc] initWithWindowController:windowController];
        moveTool = [[MoveTool alloc] initWithController:windowController];
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
    [self updateActiveTool];
    [activeTool handleFlagsChanged:event ray:&lastRay hits:lastHits];
    
    [self updateCursorOwner];
    [self updateCursor];
}

- (void)handleLeftMouseDragged:(NSEvent *)event sender:(id)sender {
    if (!drag) {
        drag = YES;
        [self updateActiveTool];
        [activeTool beginLeftDrag:lastEvent ray:&lastRay hits:lastHits];

        [self updateCursorOwner];
    }

    [self updateEvent:event];
    [self updateRay];
    [self updateHits];
    [activeTool leftDrag:lastEvent ray:&lastRay hits:lastHits];
    [self updateCursor];
}

- (void)handleMouseMoved:(NSEvent *)event sender:(id)sender {
    [self updateEvent:event];
    [self updateRay];
    [self updateHits];

    [self updateActiveTool];
    [activeTool handleMouseMoved:lastEvent ray:&lastRay hits:lastHits];

    [self updateCursorOwner];
    [self updateCursor];
}

- (void)handleMouseEntered:(NSEvent *)event sender:(id)sender {
    hasMouse = YES;
}

- (void)handleMouseExited:(NSEvent *)event sender:(id)sender {
    hasMouse = NO;
}

- (void)handleLeftMouseDown:(NSEvent *)event sender:(id)sender {
    [self updateEvent:event];
    [self updateHits];
    [activeTool handleLeftMouseDown:lastEvent ray:&lastRay hits:lastHits];
}

- (void)handleLeftMouseUp:(NSEvent *)event sender:(id)sender {
    [self updateEvent:event];
    if (drag) {
        [activeTool endLeftDrag:lastEvent ray:&lastRay hits:lastHits];
        drag = NO;
        [self updateActiveTool];

        [self updateCursorOwner];
        [self updateCursor];
    } else {
        [activeTool handleLeftMouseUp:lastEvent ray:&lastRay hits:lastHits];
    }
}

- (void)handleRightMouseDragged:(NSEvent *)event sender:(id)sender {
    if (!drag) {
        drag = YES;
        [self updateActiveTool];
        [activeTool beginRightDrag:lastEvent ray:&lastRay hits:lastHits];

        [self updateCursorOwner];
    }
    
    [self updateEvent:event];
    [self updateRay];
    [self updateHits];
    [activeTool rightDrag:lastEvent ray:&lastRay hits:lastHits];
    [self updateCursor];
}

- (void)handleRightMouseDown:(NSEvent *)event sender:(id)sender {
    [self updateEvent:event];
    [activeTool handleRightMouseDown:lastEvent ray:&lastRay hits:lastHits];
}

- (void)handleRightMouseUp:(NSEvent *)event sender:(id)sender {
    [self updateEvent:event];
    if (drag) {
        [activeTool endRightDrag:lastEvent ray:&lastRay hits:lastHits];
        drag = NO;
        [self updateActiveTool];

        [self updateCursorOwner];
        [self updateCursor];
    } else {
        [activeTool handleRightMouseUp:lastEvent ray:&lastRay hits:lastHits];
        [self showContextMenu];
    }
}

- (void)handleScrollWheel:(NSEvent *)event sender:(id)sender {
    [self updateEvent:event];
    [activeTool handleScrollWheel:lastEvent ray:&lastRay hits:lastHits];
}

- (void)handleBeginGesture:(NSEvent *)event sender:(id)sender {
    [self updateEvent:event];
    [activeTool handleBeginGesture:lastEvent ray:&lastRay hits:lastHits];
}

- (void)handleEndGesture:(NSEvent *)event sender:(id)sender {
    [self updateEvent:event];
    [activeTool handleEndGesture:lastEvent ray:&lastRay hits:lastHits];
}

- (void)handleMagnify:(NSEvent *)event sender:(id)sender {
    [self updateEvent:event];
    [activeTool handleMagnify:lastEvent ray:&lastRay hits:lastHits];
}

- (ClipTool *)clipTool {
    return clipTool;
}

- (NSPoint)menuPosition {
    return menuPosition;
}

- (void)dealloc {
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    [cameraTool release];
    [selectionTool release];
    [moveTool release];
    [faceTool release];
    [clipTool release];
    [lastEvent release];
    [lastHits release];
    [filter release];
    [super dealloc];
}

@end
