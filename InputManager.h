//
//  InputManager.h
//  TrenchBroom
//
//  Created by Kristian Duske on 26.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "Math.h"

@class PickingHitList;
@class MapWindowController;
@class CameraTool;
@class SelectionTool;
@class MoveTool;
@class RotateTool;
@class FaceTool;
@class ClipTool;
@class EntityDefinitionDndTool;
@protocol Tool;
@protocol DndTool;
@protocol Filter;

@interface InputManager : NSObject {
    @private 
    MapWindowController* windowController;
    id <Filter> filter;

    NSEvent* lastEvent;
    TRay lastRay;
    PickingHitList* lastHits;
    NSPoint menuPosition;
    
    CameraTool* cameraTool;
    SelectionTool* selectionTool;
    MoveTool* moveTool;
    RotateTool* rotateTool;
    FaceTool* faceTool;
    ClipTool* clipTool;
    id <Tool> activeTool;
    id <Tool> cursorOwner;
    BOOL drag;
    BOOL hasMouse;
    
    id <DndTool> activeDndTool;
    EntityDefinitionDndTool* entityDefinitionDndTool;
}

- (id)initWithWindowController:(MapWindowController *)theWindowController;

- (BOOL)handleKeyDown:(NSEvent *)event sender:(id)sender;
- (void)handleFlagsChanged:(NSEvent *)event sender:(id)sender;
- (void)handleLeftMouseDragged:(NSEvent *)event sender:(id)sender;
- (void)handleMouseMoved:(NSEvent *)event sender:(id)sender;
- (void)handleMouseEntered:(NSEvent *)event sender:(id)sender;
- (void)handleMouseExited:(NSEvent *)event sender:(id)sender;
- (void)handleLeftMouseDown:(NSEvent *)event sender:(id)sender;
- (void)handleLeftMouseUp:(NSEvent *)event sender:(id)sender;
- (void)handleRightMouseDragged:(NSEvent *)event sender:(id)sender;
- (void)handleRightMouseDown:(NSEvent *)event sender:(id)sender;
- (void)handleRightMouseUp:(NSEvent *)event sender:(id)sender;
- (void)handleScrollWheel:(NSEvent *)event sender:(id)sender;
- (void)handleBeginGesture:(NSEvent *)event sender:(id)sender;
- (void)handleEndGesture:(NSEvent *)event sender:(id)sender;
- (void)handleMagnify:(NSEvent *)event sender:(id)sender;

- (NSDragOperation)handleDraggingEntered:(id <NSDraggingInfo>)sender;
- (NSDragOperation)handleDraggingUpdated:(id <NSDraggingInfo>)sender;
- (void)handleDraggingEnded:(id <NSDraggingInfo>)sender;
- (void)handleDraggingExited:(id <NSDraggingInfo>)sender;
- (BOOL)prepareForDragOperation:(id <NSDraggingInfo>)sender;
- (BOOL)performDragOperation:(id <NSDraggingInfo>)sender;
- (void)concludeDragOperation:(id <NSDraggingInfo>)sender;

- (ClipTool *)clipTool;
- (NSPoint)menuPosition;
- (PickingHitList *)currentHitList;

@end
