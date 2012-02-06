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

#import <Cocoa/Cocoa.h>
#import "Math.h"
#import "Tool.h"

@class PickingHitList;
@class MapWindowController;
@class CameraTool;
@class SelectionTool;
@class MoveTool;
@class CreateBrushTool;
@class RotateTool;
@class FaceTool;
@class VertexTool;
@class EdgeTool;
@class ClipTool;
@class EntityDefinitionDndTool;
@protocol Tool;
@protocol DndTool;
@protocol Filter;

typedef enum {
    MS_NONE,
    MS_LEFT,
    MS_RIGHT
} EMouseStatus;

@interface InputManager : NSObject {
    @private 
    MapWindowController* windowController;
    id <Filter> filter;

    NSEvent* lastEvent;
    TRay lastRay;
    PickingHitList* currentHits;
    NSPoint menuPosition;
    
    CameraTool* cameraTool;
    SelectionTool* selectionTool;
    CreateBrushTool* createBrushTool;
    MoveTool* moveTool;
    RotateTool* rotateTool;
    FaceTool* faceTool;
    VertexTool* vertexTool;
    EdgeTool* edgeTool;
    ClipTool* clipTool;
    id <Tool> activeTool;
    id <Tool> cursorOwner;

    EKeyStatus keyStatus;
    EMouseStatus dragStatus;
    EMouseStatus scrollStatus;
    BOOL hasMouse;
    
    id <DndTool> activeDndTool;
    EntityDefinitionDndTool* entityDefinitionDndTool;
    
    NSMenu* popupMenu;
}

- (id)initWithWindowController:(MapWindowController *)theWindowController;

- (BOOL)handleKeyDown:(NSEvent *)event sender:(id)sender;
- (BOOL)handleKeyUp:(NSEvent *)event sender:(id)sender;
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
- (PickingHitList *)currentHits;

@end
