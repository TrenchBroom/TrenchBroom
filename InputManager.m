/*
Copyright (C) 2010-2011 Kristian Duske

This file is part of TrenchBroom.

TrenchBroom is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

TrenchBroom is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
*/

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
#import "VertexTool.h"
#import "Options.h"
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
- (void)keyStatusChanged:(NSEvent *)event;
- (void)cameraViewChanged:(NSNotification *)notification;
- (void)mapChanged:(NSNotification *)notification;

- (void)showContextMenu;
@end

@implementation InputManager (private)

- (BOOL)isCameraModifierPressed {
    return keyStatus == KS_SPACE;
}

- (BOOL)isCameraOrbitModifierPressed {
    return keyStatus == (KS_SPACE | KS_OPTION);
}

- (BOOL)isApplyTextureModifierPressed {
    return keyStatus == KS_OPTION;
}

- (BOOL)isApplyTextureAndFlagsModifierPressed {
    return keyStatus == (KS_OPTION | KS_COMMAND);
}

- (BOOL)isRotateModifierPressed {
    return keyStatus == (KS_OPTION | KS_COMMAND);
}

- (BOOL)isFaceDragModifierPressed {
    return keyStatus == KS_COMMAND;
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
        } else if (dragStatus == MS_LEFT || scrollStatus == MS_LEFT) {
            if ([self isFaceDragModifierPressed]) {
                PickingHit* hit = [[self currentHits] firstHitOfType:HT_CLOSE_FACE ignoreOccluders:NO];
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
                PickingHit* hit = [[self currentHits] firstHitOfType:HT_ENTITY | HT_FACE | HT_VERTEX ignoreOccluders:YES];
                if (hit != nil) {
                    switch ([hit type]) {
                        case HT_ENTITY:
                        case HT_FACE:
                            newActiveTool = moveTool;
                            break;
                        case HT_VERTEX:
                            newActiveTool = vertexTool;
                            break;
                        default:
                            NSLog(@"unknown hit type: %i", [hit type]);
                            break;
                    }
                }
            }
        }
    } 
    
    if ((newActiveTool == nil && (dragStatus == MS_LEFT && [selectionManager mode] == SM_FACES)) || 
               ([selectionManager mode] == SM_FACES && [[selectionManager selectedFaces] count] == 1 && ([self isApplyTextureModifierPressed] || [self isApplyTextureAndFlagsModifierPressed]))) {
        newActiveTool = faceTool;
    }
    
    if (newActiveTool == nil && dragStatus == MS_LEFT && [selectionManager mode] == SM_UNDEFINED) {
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
    if (dragStatus == MS_NONE && scrollStatus == MS_NONE) {
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
                    PickingHit* hit = [[self currentHits] firstHitOfType:HT_CLOSE_FACE ignoreOccluders:NO];
                    if (hit != nil) {
                        newOwner = faceTool;
                    } else {
                        hit = [[self currentHits] firstHitOfType:HT_FACE ignoreOccluders:YES];
                        if (hit != nil && [selectionManager isFaceSelected:[hit object]]) {
                            newOwner = faceTool;
                        }
                    }
                } else {
                    PickingHit* hit = [[self currentHits] firstHitOfType:HT_ENTITY | HT_FACE | HT_VERTEX ignoreOccluders:YES];
                    if (hit != nil) {
                        switch ([hit type]) {
                            case HT_ENTITY: {
                                id <Entity> entity = [hit object];
                                if ([selectionManager isEntitySelected:entity])
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
                            case HT_VERTEX: {
                                id <Brush> brush = [hit object];
                                if ([selectionManager isBrushSelected:brush])
                                    newOwner = vertexTool;
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

- (void)keyStatusChanged:(NSEvent *)event {
    if (dragStatus == MS_NONE && scrollStatus == MS_NONE)
        [self updateActiveTool];
    [activeTool handleKeyStatusChanged:event status:keyStatus ray:&lastRay hits:[self currentHits]];
    
    if (activeTool != cursorOwner)
        [cursorOwner handleKeyStatusChanged:event status:keyStatus ray:&lastRay hits:[self currentHits]];
    
    [self updateCursorOwner];
    [self updateCursor];
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
        vertexTool = [[VertexTool alloc] initWithWindowController:windowController];
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
    [vertexTool release];
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
        case 49:
            if (![event isARepeat]) {
                keyStatus |= KS_SPACE;
                [self keyStatusChanged:event];
            }
            return YES;
            break;
        default:
            if (![event isARepeat])
                NSLog(@"unknown key code: %i", [event keyCode]);
            break;
    }
    
    return NO;
}

- (BOOL)handleKeyUp:(NSEvent *)event sender:(id)sender {
    switch ([event keyCode]) {
        case 49:
            if (![event isARepeat]) {
                keyStatus &= ~KS_SPACE;
                [self keyStatusChanged:event];
            }
            return YES;
            break;
        default:
            if (![event isARepeat])
                NSLog(@"unknown key code: %i", [event keyCode]);
            break;
    }
    
    return NO;
}

- (void)handleFlagsChanged:(NSEvent *)event sender:(id)sender {
    keyStatus &= KS_CLEAR_MODIFIERS;
    
    NSUInteger modifierFlags = [event modifierFlags];
    if ((modifierFlags & NSShiftKeyMask) != 0)
        keyStatus |= KS_SHIFT;
    if ((modifierFlags & NSFunctionKeyMask) != 0)
        keyStatus |= KS_FUNCTION;
    if ((modifierFlags & NSControlKeyMask) != 0)
        keyStatus |= KS_CONTROL;
    if ((modifierFlags & NSAlternateKeyMask) != 0)
        keyStatus |= KS_OPTION;
    if ((modifierFlags & NSCommandKeyMask) != 0)
        keyStatus |= KS_COMMAND;
    
    [self keyStatusChanged:event];
}

- (void)handleLeftMouseDragged:(NSEvent *)event sender:(id)sender {
    if (dragStatus == MS_NONE) {
        dragStatus = MS_LEFT;
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
    
    if (dragStatus == MS_LEFT || scrollStatus == MS_LEFT) {
        if (dragStatus == MS_LEFT) {
            [activeTool endLeftDrag:lastEvent ray:&lastRay hits:[self currentHits]];
            dragStatus = MS_NONE;
            [self updateActiveTool];
        }
        
        if (scrollStatus == MS_LEFT) {
            [activeTool endLeftScroll:lastEvent ray:&lastRay hits:[self currentHits]];
            scrollStatus = MS_NONE;
            [self updateActiveTool];
            
            /*
            [self updateCursorOwner];
            [self updateCursor];
             */
        }
    } else {
        [activeTool handleLeftMouseUp:lastEvent ray:&lastRay hits:[self currentHits]];
        NSAssert([[[windowController document] undoManager] groupingLevel] == 0, @"undo grouping level must be 0 after drag ended");
    }
    
    [self updateCursorOwner];
    [self updateCursor];
}

- (void)handleRightMouseDragged:(NSEvent *)event sender:(id)sender {
    if (dragStatus == MS_NONE) {
        dragStatus = MS_RIGHT;
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
    
    if (dragStatus == MS_RIGHT || scrollStatus == MS_RIGHT) {
        if (dragStatus == MS_RIGHT) {
            [activeTool endRightDrag:lastEvent ray:&lastRay hits:[self currentHits]];
            dragStatus = MS_NONE;
            [self updateActiveTool];
        }
        
        if (scrollStatus == MS_RIGHT) {
            [activeTool endRightScroll:lastEvent ray:&lastRay hits:[self currentHits]];
            scrollStatus = MS_NONE;
            [self updateActiveTool];
        }
    } else {
        [activeTool handleRightMouseUp:lastEvent ray:&lastRay hits:[self currentHits]];
        if (dragStatus == MS_NONE)
            [self showContextMenu];
    }
    
    [self updateCursorOwner];
    [self updateCursor];
}

- (void)handleScrollWheel:(NSEvent *)event sender:(id)sender {
    int buttons = [NSEvent pressedMouseButtons];
    if (scrollStatus == MS_NONE && (buttons == 1 || buttons == 2)) {
        scrollStatus = buttons == 1 ? MS_LEFT : MS_RIGHT;
        if (dragStatus == MS_NONE)
            [self updateActiveTool];
        if (scrollStatus == MS_LEFT)
            [activeTool beginLeftScroll:lastEvent ray:&lastRay hits:[self currentHits]];
        else
            [activeTool beginRightScroll:lastEvent ray:&lastRay hits:[self currentHits]];
        
        [self updateCursorOwner];    
    }

    [self updateEvent:event];
    if (scrollStatus == MS_LEFT)
        [activeTool leftScroll:lastEvent ray:&lastRay hits:[self currentHits]];
    else if (scrollStatus == MS_RIGHT)
        [activeTool rightScroll:lastEvent ray:&lastRay hits:[self currentHits]];
    else
        [activeTool handleScrollWheel:lastEvent ray:&lastRay hits:[self currentHits]];
    
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
            [picker pickCloseFaces:&lastRay brushes:[selectionManager selectedBrushes] maxDistance:10 hitList:currentHits];
            [picker pickVertices:&lastRay brushes:[selectionManager selectedBrushes] handleRadius:2 hitList:currentHits];
        }
    } 
    
    return currentHits;
}

@end
