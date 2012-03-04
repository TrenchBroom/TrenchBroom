/*
Copyright (C) 2010-2012 Kristian Duske

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
#import "DragVertexTool.h"
#import "DragEdgeTool.h"
#import "DragFaceTool.h"
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
- (BOOL)isRotateModifierPressed;
- (BOOL)isFaceDragModifierPressed;
- (BOOL)isSplitModifierPressed;

- (void)updateEvent:(NSEvent *)event;
- (void)updateRay;

/*
- (void)updateActiveTool;
- (void)updateCursor;
- (void)updateCursorOwner;
*/
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

- (BOOL)isRotateModifierPressed {
    return [NSEvent modifierFlags] == (NSAlternateKeyMask | NSCommandKeyMask);
}

- (BOOL)isFaceDragModifierPressed {
    return [NSEvent modifierFlags] == NSCommandKeyMask;
}

- (BOOL)isSplitModifierPressed {
    return [NSEvent modifierFlags] == NSCommandKeyMask || [NSEvent modifierFlags] == (NSCommandKeyMask | NSAlternateKeyMask);
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

/*
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
        PickingHit* hit = [[self currentHits] firstHitOfType:HT_VERTEX ignoreOccluders:NO];
        if (hit != nil) {
            id <Brush> brush = [hit object];
            const TVertexList* vertices = [brush vertices];
            const TEdgeList* edges = [brush edges];
            int index = [hit vertexIndex];
            if (index < vertices->count || [self isSplitModifierPressed])
                newActiveTool = dragVertexTool;
            else if (index < vertices->count + edges->count)
                newActiveTool = dragEdgeTool;
            else
                newActiveTool = dragFaceTool;
        }
        
        if (newActiveTool == nil && [self isRotateModifierPressed]) {
                newActiveTool = rotateTool;
        }
        
        if (newActiveTool == nil && (dragStatus == MS_LEFT || scrollStatus == MS_LEFT)) {
            if ([self isFaceDragModifierPressed]) {
                hit = [[self currentHits] firstHitOfType:HT_CLOSE_FACE ignoreOccluders:NO];
                if (hit != nil) {
                    newActiveTool = faceTool;
                } else {
                    hit = [[self currentHits] firstHitOfType:HT_FACE ignoreOccluders:NO];
                    if (hit != nil) {
                        id <Face> face = [hit object];
                        if ([face selected])
                            newActiveTool = faceTool;
                    }
                }
            } 
            
            if (newActiveTool == nil) {
                hit = [[self currentHits] firstHitOfType:HT_ENTITY | HT_FACE ignoreOccluders:NO];
                if (hit != nil) {
                    newActiveTool = moveTool;
                }
            }
        }
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
        if (activeTool != nil)
            [activeTool activated:lastEvent ray:&lastRay hits:[self currentHits]];
    }
}

- (void)updateCursorOwner {
    if (dragStatus == MS_NONE && scrollStatus == MS_NONE) {
        SelectionManager* selectionManager = [windowController selectionManager];
        
        id <Tool> newOwner = nil;
        if (hasMouse) {
            if (newOwner == nil && ([self isCameraModifierPressed] || [self isCameraOrbitModifierPressed])) {
                newOwner = cameraTool;
            }
                
            if (newOwner == nil && [clipTool active]) {
                newOwner = clipTool;
            }
            
            if (([selectionManager mode] == SM_BRUSHES || [selectionManager mode] == SM_ENTITIES || [selectionManager mode] == SM_BRUSHES_ENTITIES)) {
                if (newOwner == nil && [self isRotateModifierPressed]) {
                    newOwner = rotateTool;
                }
                
                if (newOwner == nil && [self isFaceDragModifierPressed]) {
                    PickingHit* hit = [[self currentHits] firstHitOfType:HT_CLOSE_FACE ignoreOccluders:NO];
                    if (hit != nil) {
                        newOwner = faceTool;
                    } else {
                        hit = [[self currentHits] firstHitOfType:HT_FACE ignoreOccluders:NO];
                        if (hit != nil && [selectionManager isFaceSelected:[hit object]]) {
                            newOwner = faceTool;
                        }
                    }
                }
                
                if (newOwner == nil) {
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
                                if ([selectionManager isBrushSelected:brush]) {
                                    const TVertexList* vertices = [brush vertices];
                                    const TEdgeList* edges = [brush edges];
                                    int index = [hit vertexIndex];
                                    if (index < vertices->count || [self isSplitModifierPressed])
                                        newOwner = dragVertexTool;
                                    else if (index < vertices->count + edges->count)
                                        newOwner = dragEdgeTool;
                                    else
                                        newOwner = dragFaceTool;
                                }
                                break;
                            }
                            default: {
                                NSLog(@"unknown hit type: %i", [hit type]);
                                break;
                            }
                        }
                    }
                }
            } 
            
            if (newOwner == nil && [selectionManager mode] == SM_FACES) {
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
            if (cursorOwner != nil) {
                [cursorOwner handleKeyStatusChanged:lastEvent status:keyStatus ray:&lastRay hits:[self currentHits]];
                [cursorOwner setCursor:lastEvent ray:&lastRay hits:[self currentHits]];
            }
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
*/

- (void)cameraViewChanged:(NSNotification *)notification {
    [self updateRay];
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
        Camera* camera = [windowController camera];
        
        filter = [[DefaultFilter alloc] initWithSelectionManager:selectionManager groupManager:groupManager camera:camera options:options];
        
        cameraTool = [[CameraTool alloc] initWithWindowController:windowController];
        selectionTool = [[SelectionTool alloc] initWithWindowController:windowController];
        moveTool = [[MoveTool alloc] initWithWindowController:windowController];
        createBrushTool = [[CreateBrushTool alloc] initWithWindowController:windowController];
        rotateTool = [[RotateTool alloc] initWithWindowController:windowController];
        faceTool = [[FaceTool alloc] initWithWindowController:windowController];
        dragVertexTool = [[DragVertexTool alloc] initWithWindowController:windowController];
        dragEdgeTool = [[DragEdgeTool alloc] initWithWindowController:windowController];
        dragFaceTool = [[DragFaceTool alloc] initWithWindowController:windowController];
        clipTool = [[ClipTool alloc] initWithWindowController:windowController];
        entityDefinitionDndTool = [[EntityDefinitionDndTool alloc] initWithWindowController:windowController];
        
        mouseReceiverChain = [[NSMutableArray alloc] init];
        [mouseReceiverChain addObject:cameraTool];
        [mouseReceiverChain addObject:faceTool];
        [mouseReceiverChain addObject:moveTool];
        [mouseReceiverChain addObject:rotateTool];
        [mouseReceiverChain addObject:selectionTool];
        dragScrollReceiver = nil;
        modalReceiverIndex = -1;
        
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
    [dragVertexTool release];
    [dragEdgeTool release];
    [dragFaceTool release];
    [clipTool release];
    [lastEvent release];
    [currentHits release];
    [mouseReceiverChain release];
    [filter release];
    [super dealloc];
}

- (BOOL)keyDown:(NSEvent *)event sender:(id)sender {
    switch ([event keyCode]) {
        case 48:
            if (clipTool != nil) {
                [clipTool toggleClipMode];
                return YES;
            }
            break;
        default:
            break;
    }
    
    return NO;
}

- (BOOL)keyUp:(NSEvent *)event sender:(id)sender {
    return NO;
}

- (void)flagsChanged:(NSEvent *)event sender:(id)sender {
    for (Tool* mouseReceiver in mouseReceiverChain)
        [mouseReceiver flagsChanged:event ray:&lastRay hits:[self currentHits]];
}

- (void)leftMouseDragged:(NSEvent *)event sender:(id)sender {
    if (dragStatus == MS_NONE) {
        dragStatus = MS_LEFT;
        
        for (Tool* mouseReceiver in mouseReceiverChain) {
            if ([mouseReceiver beginLeftDrag:lastEvent ray:&lastRay hits:[self currentHits]]) {
                dragScrollReceiver = mouseReceiver;
                break;
            }
        }
    }

    [self updateEvent:event];
    [self updateRay];
    [dragScrollReceiver leftDrag:lastEvent ray:&lastRay hits:[self currentHits]];
}

- (void)mouseMoved:(NSEvent *)event sender:(id)sender {
    [self updateEvent:event];
    [self updateRay];

    for (Tool* mouseReceiver in mouseReceiverChain)
        [mouseReceiver mouseMoved:lastEvent ray:&lastRay hits:[self currentHits]];
}

- (void)mouseEntered:(NSEvent *)event sender:(id)sender {
    hasMouse = YES;
    [self updateEvent:event];
    [self updateRay];
}

- (void)mouseExited:(NSEvent *)event sender:(id)sender {
    hasMouse = NO;
}

- (void)leftMouseDown:(NSEvent *)event sender:(id)sender {
    [self updateEvent:event];
    [self updateRay];
    
    for (Tool* mouseReceiver in mouseReceiverChain) {
        if ([mouseReceiver leftMouseDown:lastEvent ray:&lastRay hits:[self currentHits]])
            break;
    }
}

- (void)leftMouseUp:(NSEvent *)event sender:(id)sender {
    [self updateEvent:event];
    
    if (dragStatus == MS_LEFT || scrollStatus == MS_LEFT) {
        if (dragStatus == MS_LEFT) {
            [dragScrollReceiver endLeftDrag:lastEvent ray:&lastRay hits:[self currentHits]];
            dragStatus = MS_NONE;
            dragScrollReceiver = nil;
        }
        
        if (scrollStatus == MS_LEFT) {
            [dragScrollReceiver endLeftScroll:lastEvent ray:&lastRay hits:[self currentHits]];
            scrollStatus = MS_NONE;
            dragScrollReceiver = nil;
        }
    } else {
        for (Tool* mouseReceiver in mouseReceiverChain) {
            if ([mouseReceiver leftMouseUp:lastEvent ray:&lastRay hits:[self currentHits]])
                break;
        }
        NSAssert([[[windowController document] undoManager] groupingLevel] == 0, @"undo grouping level must be 0 after drag ended");
    }
}

- (void)rightMouseDragged:(NSEvent *)event sender:(id)sender {
    if (dragStatus == MS_NONE) {
        dragStatus = MS_RIGHT;
        for (Tool* mouseReceiver in mouseReceiverChain) {
            if ([mouseReceiver beginRightDrag:lastEvent ray:&lastRay hits:[self currentHits]]) {
                dragScrollReceiver = mouseReceiver;
                break;
            }
        }
    }
    
    [self updateEvent:event];
    [self updateRay];
    [dragScrollReceiver rightDrag:lastEvent ray:&lastRay hits:[self currentHits]];
}

- (void)rightMouseDown:(NSEvent *)event sender:(id)sender {
    [self updateEvent:event];
    
    for (Tool* mouseReceiver in mouseReceiverChain) {
        if ([mouseReceiver rightMouseDown:lastEvent ray:&lastRay hits:[self currentHits]])
            break;
    }
}

- (void)rightMouseUp:(NSEvent *)event sender:(id)sender {
    [self updateEvent:event];
    
    if (dragStatus == MS_RIGHT || scrollStatus == MS_RIGHT) {
        if (dragStatus == MS_RIGHT) {
            [dragScrollReceiver endRightDrag:lastEvent ray:&lastRay hits:[self currentHits]];
            dragStatus = MS_NONE;
            dragScrollReceiver = nil;
        }
        
        if (scrollStatus == MS_RIGHT) {
            [dragScrollReceiver endRightScroll:lastEvent ray:&lastRay hits:[self currentHits]];
            scrollStatus = MS_NONE;
            dragScrollReceiver = nil;
        }
    } else {
        for (Tool* mouseReceiver in mouseReceiverChain) {
            if ([mouseReceiver rightMouseUp:lastEvent ray:&lastRay hits:[self currentHits]])
                break;
        }
        if (dragStatus == MS_NONE)
            [self showContextMenu];
    }
}

- (void)scrollWheel:(NSEvent *)event sender:(id)sender {
    int buttons = [NSEvent pressedMouseButtons];
    if (scrollStatus == MS_NONE && (buttons == 1 || buttons == 2)) {
        scrollStatus = buttons == 1 ? MS_LEFT : MS_RIGHT;
        if (scrollStatus == MS_LEFT) {
            if (dragScrollReceiver == nil) {
                for (Tool* mouseReceiver in mouseReceiverChain) {
                    if ([mouseReceiver beginLeftScroll:lastEvent ray:&lastRay hits:[self currentHits]]) {
                        dragScrollReceiver = mouseReceiver;
                        break;
                    }
                }
            } else {
                [dragScrollReceiver beginLeftScroll:lastEvent ray:&lastRay hits:[self currentHits]];
            }
        } else {
            if (dragScrollReceiver == nil) {
                for (Tool* mouseReceiver in mouseReceiverChain) {
                    if ([mouseReceiver beginRightScroll:lastEvent ray:&lastRay hits:[self currentHits]]) {
                        dragScrollReceiver = mouseReceiver;
                        break;
                    }
                }
            } else {
                [dragScrollReceiver beginRightScroll:lastEvent ray:&lastRay hits:[self currentHits]];
            }
        }
    }

    [self updateEvent:event];
    if (dragScrollReceiver != nil) {
        if (scrollStatus == MS_LEFT)
            [dragScrollReceiver leftScroll:lastEvent ray:&lastRay hits:[self currentHits]];
        else if (scrollStatus == MS_RIGHT)
            [dragScrollReceiver rightScroll:lastEvent ray:&lastRay hits:[self currentHits]];
        else
            [dragScrollReceiver scrollWheel:lastEvent ray:&lastRay hits:[self currentHits]];
    } else {
        for (Tool* mouseReceiver in mouseReceiverChain) {
            if ([mouseReceiver scrollWheel:lastEvent ray:&lastRay hits:[self currentHits]])
                break;
        }
    }
}

- (void)beginGesture:(NSEvent *)event sender:(id)sender {
    [self updateEvent:event];

    for (Tool* mouseReceiver in mouseReceiverChain) {
        if ([mouseReceiver beginGesture:lastEvent ray:&lastRay hits:[self currentHits]]) {
            dragScrollReceiver = mouseReceiver;
            break;
        }
    }
}

- (void)endGesture:(NSEvent *)event sender:(id)sender {
    NSAssert(dragScrollReceiver != nil, @"modal receiver must not be nil");
    [self updateEvent:event];
    [dragScrollReceiver endGesture:lastEvent ray:&lastRay hits:[self currentHits]];
}

- (void)magnify:(NSEvent *)event sender:(id)sender {
    NSAssert(dragScrollReceiver != nil, @"modal receiver must not be nil");
    [self updateEvent:event];
    [dragScrollReceiver magnify:lastEvent ray:&lastRay hits:[self currentHits]];
}

- (NSDragOperation)draggingEntered:(id <NSDraggingInfo>)sender {
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

- (NSDragOperation)draggingUpdated:(id <NSDraggingInfo>)sender {
    if (activeDndTool == nil)
        return NSDragOperationNone;
    
    Camera* camera = [windowController camera];
    NSPoint location = [sender draggingLocation];
    TRay ray = [camera pickRayX:location.x y:location.y];
    
    Picker* picker = [[windowController document] picker];
    PickingHitList* hitList = [picker pickObjects:&ray filter:filter];
    
    return [activeDndTool handleDraggingUpdated:sender ray:&ray hits:hitList];
}

- (void)draggingEnded:(id <NSDraggingInfo>)sender {
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

- (void)draggingExited:(id <NSDraggingInfo>)sender {
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

- (void)toggleTool:(Tool *)theTool index:(int)theIndex {
    Tool* modalReceiver = nil;
    if (modalReceiverIndex != -1) {
        modalReceiver = [mouseReceiverChain objectAtIndex:modalReceiverIndex];
        [modalReceiver deactivated:lastEvent ray:&lastRay hits:[self currentHits]];
        [mouseReceiverChain removeObjectAtIndex:modalReceiverIndex];
        modalReceiverIndex = -1;
    }

    if (theTool != modalReceiver) {
        modalReceiverIndex = theIndex;
        [theTool activated:lastEvent ray:&lastRay hits:[self currentHits]];
        [mouseReceiverChain insertObject:theTool atIndex:modalReceiverIndex];
    }
}

- (void)toggleDragVertexTool {
    [self toggleTool:dragVertexTool index:1];
}

- (void)toggleDragEdgeTool {
    [self toggleTool:dragEdgeTool index:1];
}

- (void)toggleDragFaceTool {
    [self toggleTool:dragFaceTool index:1];
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
            [picker pickVertices:&lastRay brushes:[selectionManager selectedBrushes] handleRadius:3 hitList:currentHits filter:filter];
        }
    } 
    
    return currentHits;
}

@end
