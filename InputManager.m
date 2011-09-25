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
#import "RotateTool.h"
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
#import "DndTool.h"
#import "EntityDefinitionDndTool.h"
#import "EntityDefinitionManager.h"
#import "EntityDefinition.h"
#import "Entity.h"
#import "CreateBrushTool.h"

@interface InputManager (private)

- (BOOL)isCameraModifierPressed;
- (BOOL)isCameraOrbitModifierPressed;
- (BOOL)isApplyTextureModifierPressed;
- (BOOL)isApplyTextureAndFlagsModifierPressed;
- (BOOL)isRotateModifierPressed;
- (BOOL)isFaceDragModifierPressed;

- (void)updateEvent:(NSEvent *)event;
- (void)updateRay;
- (void)updateActiveTool;
- (void)updateCursor;
- (void)updateCursorOwner;
- (void)cameraViewChanged:(NSNotification *)notification;
- (void)mapChanged:(NSNotification *)notification;

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

- (BOOL)isRotateModifierPressed {
    return [NSEvent modifierFlags] == NSAlternateKeyMask;
}

- (BOOL)isFaceDragModifierPressed {
    return [NSEvent modifierFlags] == NSCommandKeyMask;
}

- (void)updateEvent:(NSEvent *)event {
    [lastEvent release];
    lastEvent = [event retain];
}

- (void)updateRay {
    MapView3D* mapView3D = [windowController view3D];
    Camera* camera = [windowController camera];
    [[mapView3D openGLContext] makeCurrentContext];
    
    NSPoint m = [mapView3D convertPointFromBase:[lastEvent locationInWindow]];
    lastRay = [camera pickRayX:m.x y:m.y];
    
    [currentHits release];
    currentHits = nil;
}

- (void)updateActiveTool {
    SelectionManager* selectionManager = [windowController selectionManager];

    id <Tool> newActiveTool = nil;
    if ([self isCameraModifierPressed] || [self isCameraOrbitModifierPressed]) {
        newActiveTool = cameraTool;
    }
    
    if (newActiveTool == nil && [clipTool active]) {
        newActiveTool = clipTool;
    } 
    
    if (newActiveTool == nil && ([selectionManager mode] == SM_BRUSHES || [selectionManager mode] == SM_ENTITIES || [selectionManager mode] == SM_BRUSHES_ENTITIES)) {
        if ([self isRotateModifierPressed]) {
            newActiveTool = rotateTool;
        } else if (drag || scroll) {
            if ([self isFaceDragModifierPressed]) {
                PickingHit* hit = [[self currentHits] edgeDragHit];
                if (hit != nil) {
                    newActiveTool = faceTool;
                } else {
                    hit = [[self currentHits] firstHitOfType:HT_FACE ignoreOccluders:YES];
                    if (hit != nil) {
                        id <Face> face = [hit object];
                        if ([selectionManager isFaceSelected:face])
                            newActiveTool = faceTool;
                    }
                }
            } else {
                newActiveTool = moveTool;
            }
        }
    } 
    
    if ((newActiveTool == nil && (drag && [selectionManager mode] == SM_FACES)) || 
               ([selectionManager mode] == SM_FACES && [[selectionManager selectedFaces] count] == 1 && ([self isApplyTextureModifierPressed] || [self isApplyTextureAndFlagsModifierPressed]))) {
        newActiveTool = faceTool;
    }
    
    if (newActiveTool == nil && drag) {
        newActiveTool = createBrushTool;
    }
    
    if (newActiveTool == nil) {
        newActiveTool = selectionTool;
    }
    
    if (newActiveTool != activeTool) {
        if (activeTool != nil)
            [activeTool deactivated:lastEvent ray:&lastRay hits:[self currentHits]];
        activeTool = newActiveTool;
        if (activeTool != nil) {
            [activeTool activated:lastEvent ray:&lastRay hits:[self currentHits]];
        }
    }
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
                if ([self isRotateModifierPressed]) {
                    newOwner = rotateTool;
                } else if ([self isFaceDragModifierPressed]) {
                    PickingHit* hit = [[self currentHits] edgeDragHit];
                    if (hit != nil) {
                        newOwner = faceTool;
                    } else {
                        hit = [[self currentHits] firstHitOfType:HT_FACE ignoreOccluders:YES];
                        if (hit != nil && [selectionManager isFaceSelected:[hit object]]) {
                            newOwner = faceTool;
                        }
                    }
                } else {
                    PickingHit* hit = [[self currentHits] firstHitOfType:HT_ENTITY | HT_BRUSH ignoreOccluders:YES];
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
                            default: {
                                NSLog(@"unknown hit type: %i", [hit type]);
                                break;
                            }
                        }
                    }
                }
            } else if ([selectionManager mode] == SM_FACES) {
                PickingHit* hit = [[self currentHits] firstHitOfType:HT_FACE ignoreOccluders:YES];
                if (hit != nil) {
                    id <Face> face = [hit object];
                    if ([selectionManager isFaceSelected:face] || ([[selectionManager selectedFaces] count] == 1 && ([self isApplyTextureModifierPressed] || [self isApplyTextureAndFlagsModifierPressed])))
                        newOwner = faceTool;
                }
            }
        }
        
        if (newOwner != cursorOwner) {
            if (cursorOwner != nil)
                [cursorOwner unsetCursor:lastEvent ray:&lastRay hits:[self currentHits]];
            cursorOwner = newOwner;
            if (cursorOwner != nil)
                [cursorOwner setCursor:lastEvent ray:&lastRay hits:[self currentHits]];
        }
    }
}

- (void)updateCursor {
    if (cursorOwner != nil) {
        [cursorOwner updateCursor:lastEvent ray:&lastRay hits:[self currentHits]];

        MapView3D* view3D = [windowController view3D];
        [view3D setNeedsDisplay:YES];
    }
}

- (void)cameraViewChanged:(NSNotification *)notification {
    [self updateRay];
    [self updateCursorOwner];
    [self updateCursor];
}

- (void)mapChanged:(NSNotification *)notification {
    [currentHits release];
    currentHits = nil;
}


- (void)showContextMenu {
    if (popupMenu == nil) {
        popupMenu = [[windowController view3D] menu];
        
        NSMenu* pointEntityMenu = [[windowController view3D] pointEntityMenu];
        NSMenu* brushEntityMenu = [[windowController view3D] brushEntityMenu];
        
        EntityDefinitionManager* entityDefinitionManager = [[windowController document] entityDefinitionManager];
        
        NSArray* pointDefinitions = [entityDefinitionManager definitionsOfType:EDT_POINT];
        for (int i = 0; i < [pointDefinitions count]; i++) {
            EntityDefinition* definition = [pointDefinitions objectAtIndex:i];
            NSMenuItem* definitionItem = [pointEntityMenu insertItemWithTitle:[definition name] action:@selector(createPointEntity:) keyEquivalent:@"" atIndex:i];
            [definitionItem setTag:i];
        }
        
        NSArray* brushDefinitions = [entityDefinitionManager definitionsOfType:EDT_BRUSH];
        for (int i = 0; i < [brushDefinitions count]; i++) {
            EntityDefinition* definition = [brushDefinitions objectAtIndex:i];
            NSMenuItem* definitionItem = [brushEntityMenu insertItemWithTitle:[definition name] action:@selector(createBrushEntity:) keyEquivalent:@"" atIndex:i];
            [definitionItem setTag:i];
        }
    }
    
    menuPosition = [lastEvent locationInWindow];
    menuPosition = [[windowController view3D] convertPointFromBase:menuPosition];
    [NSMenu popUpContextMenu:popupMenu withEvent:lastEvent forView:[windowController view3D]];
}

@end

@implementation InputManager
- (id)initWithWindowController:(MapWindowController *)theWindowController {
    if ((self = [self init])) {
        windowController = theWindowController; // do not retain
        MapDocument* map = [windowController document];
        
        SelectionManager* selectionManager = [map selectionManager];
        GroupManager* groupManager = [map groupManager];
        Options* options = [windowController options];
        filter = [[DefaultFilter alloc] initWithSelectionManager:selectionManager groupManager:groupManager options:options];
        
        cameraTool = [[CameraTool alloc] initWithWindowController:windowController];
        selectionTool = [[SelectionTool alloc] initWithWindowController:windowController];
        moveTool = [[MoveTool alloc] initWithWindowController:windowController];
        createBrushTool = [[CreateBrushTool alloc] initWithWindowController:windowController];
        rotateTool = [[RotateTool alloc] initWithWindowController:windowController];
        faceTool = [[FaceTool alloc] initWithWindowController:windowController];
        clipTool = [[ClipTool alloc] initWithWindowController:windowController];
        entityDefinitionDndTool = [[EntityDefinitionDndTool alloc] initWithWindowController:windowController];
        
        Camera* camera = [windowController camera];
        
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center addObserver:self selector:@selector(cameraViewChanged:) name:CameraViewChanged object:camera];
        [center addObserver:self selector:@selector(mapChanged:) name:EntitiesAdded object:map];
        [center addObserver:self selector:@selector(mapChanged:) name:EntitiesWereRemoved object:map];
        [center addObserver:self selector:@selector(mapChanged:) name:PropertiesDidChange object:map];
        [center addObserver:self selector:@selector(mapChanged:) name:BrushesAdded object:map];
        [center addObserver:self selector:@selector(mapChanged:) name:BrushesWereRemoved object:map];
        [center addObserver:self selector:@selector(mapChanged:) name:BrushesDidChange object:map];
        [center addObserver:self selector:@selector(mapChanged:) name:FacesDidChange object:map];
    }
    
    return self;
}


- (void)dealloc {
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    [entityDefinitionDndTool release];
    [cameraTool release];
    [selectionTool release];
    [moveTool release];
    [createBrushTool release];
    [rotateTool release];
    [faceTool release];
    [clipTool release];
    [lastEvent release];
    [currentHits release];
    [filter release];
    [super dealloc];
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
    [activeTool handleFlagsChanged:event ray:&lastRay hits:[self currentHits]];
    
    [self updateCursorOwner];
    [self updateCursor];
}

- (void)handleLeftMouseDragged:(NSEvent *)event sender:(id)sender {
    if (!drag) {
        drag = YES;
        [self updateActiveTool];
        [activeTool beginLeftDrag:lastEvent ray:&lastRay hits:[self currentHits]];

        [self updateCursorOwner];
    }

    [self updateEvent:event];
    [self updateRay];
    [activeTool leftDrag:lastEvent ray:&lastRay hits:[self currentHits]];
    [self updateCursor];
}

- (void)handleMouseMoved:(NSEvent *)event sender:(id)sender {
    [self updateEvent:event];
    [self updateRay];

    [self updateActiveTool];
    [activeTool handleMouseMoved:lastEvent ray:&lastRay hits:[self currentHits]];

    [self updateCursorOwner];
    [self updateCursor];
}

- (void)handleMouseEntered:(NSEvent *)event sender:(id)sender {
    hasMouse = YES;
    [self updateEvent:event];
    [self updateRay];
    [self updateCursorOwner];
    [self updateCursor];
}

- (void)handleMouseExited:(NSEvent *)event sender:(id)sender {
    hasMouse = NO;
    [self updateCursorOwner];
}

- (void)handleLeftMouseDown:(NSEvent *)event sender:(id)sender {
    [self updateEvent:event];
    [self updateRay];
    [activeTool handleLeftMouseDown:lastEvent ray:&lastRay hits:[self currentHits]];
}

- (void)handleLeftMouseUp:(NSEvent *)event sender:(id)sender {
    [self updateEvent:event];
    
    if (drag || scroll) {
        if (drag) {
            [activeTool endLeftDrag:lastEvent ray:&lastRay hits:[self currentHits]];
            drag = NO;
            [self updateActiveTool];
        }
        
        if (scroll) {
            [activeTool endLeftScroll:lastEvent ray:&lastRay hits:[self currentHits]];
            scroll = NO;
            [self updateActiveTool];
            
            [self updateCursorOwner];
            [self updateCursor];
        }
    } else {
        [activeTool handleLeftMouseUp:lastEvent ray:&lastRay hits:[self currentHits]];
    }

    [self updateCursorOwner];
    [self updateCursor];
}

- (void)handleRightMouseDragged:(NSEvent *)event sender:(id)sender {
    if (!drag) {
        drag = YES;
        [self updateActiveTool];
        [activeTool beginRightDrag:lastEvent ray:&lastRay hits:[self currentHits]];

        [self updateCursorOwner];
    }
    
    [self updateEvent:event];
    [self updateRay];
    [activeTool rightDrag:lastEvent ray:&lastRay hits:[self currentHits]];
    [self updateCursor];
}

- (void)handleRightMouseDown:(NSEvent *)event sender:(id)sender {
    [self updateEvent:event];
    [activeTool handleRightMouseDown:lastEvent ray:&lastRay hits:[self currentHits]];
}

- (void)handleRightMouseUp:(NSEvent *)event sender:(id)sender {
    [self updateEvent:event];
    if (drag) {
        [activeTool endRightDrag:lastEvent ray:&lastRay hits:[self currentHits]];
        drag = NO;
        [self updateActiveTool];

        [self updateCursorOwner];
        [self updateCursor];
    } else if (scroll) {
        [activeTool endRightScroll:lastEvent ray:&lastRay hits:[self currentHits]];
        scroll = NO;
        [self updateActiveTool];
        
        [self updateCursorOwner];
        [self updateCursor];
    } else {
        [activeTool handleRightMouseUp:lastEvent ray:&lastRay hits:[self currentHits]];
        [self showContextMenu];
    }
}

- (void)handleScrollWheel:(NSEvent *)event sender:(id)sender {
    int buttons = [NSEvent pressedMouseButtons];
    if (!scroll && (buttons == 1 || buttons == 2)) {
        scroll = YES;
        if (!drag)
            [self updateActiveTool];
        if (buttons == 1)
            [activeTool beginLeftScroll:lastEvent ray:&lastRay hits:[self currentHits]];
        else
            [activeTool beginRightScroll:lastEvent ray:&lastRay hits:[self currentHits]];
        
        [self updateCursorOwner];    
    }

    [self updateEvent:event];
    if (scroll) {
        if (buttons == 1)
            [activeTool leftScroll:lastEvent ray:&lastRay hits:[self currentHits]];
        else
            [activeTool rightScroll:lastEvent ray:&lastRay hits:[self currentHits]];
    } else {
        [activeTool handleScrollWheel:lastEvent ray:&lastRay hits:[self currentHits]];
    }
    [self updateCursor];
    
}

- (void)handleBeginGesture:(NSEvent *)event sender:(id)sender {
    [self updateEvent:event];
    [activeTool handleBeginGesture:lastEvent ray:&lastRay hits:[self currentHits]];
}

- (void)handleEndGesture:(NSEvent *)event sender:(id)sender {
    [self updateEvent:event];
    [activeTool handleEndGesture:lastEvent ray:&lastRay hits:[self currentHits]];
}

- (void)handleMagnify:(NSEvent *)event sender:(id)sender {
    [self updateEvent:event];
    [activeTool handleMagnify:lastEvent ray:&lastRay hits:[self currentHits]];
}

- (NSDragOperation)handleDraggingEntered:(id <NSDraggingInfo>)sender {
    NSPasteboard* pasteboard = [sender draggingPasteboard];
    NSString* type = [pasteboard availableTypeFromArray:[NSArray arrayWithObject:EntityDefinitionType]];
    if (type == nil)
        return NSDragOperationNone;

    if ([EntityDefinitionType isEqualToString:type])
        activeDndTool = entityDefinitionDndTool;
    
    if (activeDndTool != nil) {
        Camera* camera = [windowController camera];
        NSPoint location = [sender draggingLocation];
        TRay ray = [camera pickRayX:location.x y:location.y];
        
        Picker* picker = [[windowController document] picker];
        PickingHitList* hitList = [picker pickObjects:&ray filter:filter];
        
        return [activeDndTool handleDraggingEntered:sender ray:&ray hits:hitList];
    }
    
    return NSDragOperationNone;
}

- (NSDragOperation)handleDraggingUpdated:(id <NSDraggingInfo>)sender {
    if (activeDndTool == nil)
        return NSDragOperationNone;
    
    Camera* camera = [windowController camera];
    NSPoint location = [sender draggingLocation];
    TRay ray = [camera pickRayX:location.x y:location.y];
    
    Picker* picker = [[windowController document] picker];
    PickingHitList* hitList = [picker pickObjects:&ray filter:filter];
    
    return [activeDndTool handleDraggingUpdated:sender ray:&ray hits:hitList];
}

- (void)handleDraggingEnded:(id <NSDraggingInfo>)sender {
    if (activeDndTool == nil)
        return;
    
    Camera* camera = [windowController camera];
    NSPoint location = [sender draggingLocation];
    TRay ray = [camera pickRayX:location.x y:location.y];
    
    Picker* picker = [[windowController document] picker];
    PickingHitList* hitList = [picker pickObjects:&ray filter:filter];
    
    [activeDndTool handleDraggingEnded:sender ray:&ray hits:hitList];
    activeDndTool = nil;
}

- (void)handleDraggingExited:(id <NSDraggingInfo>)sender {
    if (activeDndTool == nil)
        return;
    
    Camera* camera = [windowController camera];
    NSPoint location = [sender draggingLocation];
    TRay ray = [camera pickRayX:location.x y:location.y];
    
    Picker* picker = [[windowController document] picker];
    PickingHitList* hitList = [picker pickObjects:&ray filter:filter];
    
    [activeDndTool handleDraggingExited:sender ray:&ray hits:hitList];
    activeDndTool = nil;
}

- (BOOL)prepareForDragOperation:(id <NSDraggingInfo>)sender {
    if (activeDndTool == nil)
        return NO;
    
    Camera* camera = [windowController camera];
    NSPoint location = [sender draggingLocation];
    TRay ray = [camera pickRayX:location.x y:location.y];
    
    Picker* picker = [[windowController document] picker];
    PickingHitList* hitList = [picker pickObjects:&ray filter:filter];
    
    return [activeDndTool prepareForDragOperation:sender ray:&ray hits:hitList];
}

- (BOOL)performDragOperation:(id <NSDraggingInfo>)sender {
    if (activeDndTool == nil)
        return NO;
    
    Camera* camera = [windowController camera];
    NSPoint location = [sender draggingLocation];
    TRay ray = [camera pickRayX:location.x y:location.y];
    
    Picker* picker = [[windowController document] picker];
    PickingHitList* hitList = [picker pickObjects:&ray filter:filter];
    
    return [activeDndTool performDragOperation:sender ray:&ray hits:hitList];
}

- (void)concludeDragOperation:(id <NSDraggingInfo>)sender {
    if (activeDndTool == nil)
        return;
    
    Camera* camera = [windowController camera];
    NSPoint location = [sender draggingLocation];
    TRay ray = [camera pickRayX:location.x y:location.y];
    
    Picker* picker = [[windowController document] picker];
    PickingHitList* hitList = [picker pickObjects:&ray filter:filter];
    
    [activeDndTool concludeDragOperation:sender ray:&ray hits:hitList];
    activeDndTool = nil;
}


- (ClipTool *)clipTool {
    return clipTool;
}

- (NSPoint)menuPosition {
    return menuPosition;
}

- (PickingHitList *)currentHits {
    if (currentHits == nil) {
        Picker* picker = [[windowController document] picker];
        currentHits = [[picker pickObjects:&lastRay filter:filter] retain];
        
        SelectionManager* selectionManager = [windowController selectionManager];
        if ([selectionManager mode] == SM_BRUSHES) {
            PickingHit* brushHit = [currentHits firstHitOfType:HT_BRUSH ignoreOccluders:NO];
            if (brushHit == nil) {
                NSEnumerator* brushEn = [[selectionManager selectedBrushes] objectEnumerator];
                id <Brush> brush;
                while ((brush = [brushEn nextObject]))
                    [brush pickFace:&lastRay maxDistance:10 hitList:currentHits];
            }
        }
    } 
    
    return currentHits;
}

@end
