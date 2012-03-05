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
@class DragVertexTool;
@class DragEdgeTool;
@class DragFaceTool;
@class ClipTool;
@class EntityDefinitionDndTool;
@class Tool;
@protocol DndTool;
@protocol Filter;

typedef enum {
    MS_NONE,
    MS_LEFT,
    MS_RIGHT
} EMouseStatus;

@interface InputController : NSObject {
    @private 
    MapWindowController* windowController;
    id <Filter> filter;

    NSEvent* lastEvent;
    TRay lastRay;
    PickingHitList* currentHits;
    NSPoint menuPosition;
    
    NSMutableArray* receiverChain;
    Tool* dragScrollReceiver;
    int modalReceiverIndex;
    
    CameraTool* cameraTool;
    SelectionTool* selectionTool;
    CreateBrushTool* createBrushTool;
    MoveTool* moveTool;
    RotateTool* rotateTool;
    FaceTool* faceTool;
    DragVertexTool* dragVertexTool;
    DragEdgeTool* dragEdgeTool;
    DragFaceTool* dragFaceTool;
    ClipTool* clipTool;

    EMouseStatus dragStatus;
    EMouseStatus scrollStatus;
    BOOL hasMouse;
    
    id <DndTool> activeDndTool;
    EntityDefinitionDndTool* entityDefinitionDndTool;
    
    NSMenu* popupMenu;
}

- (id)initWithWindowController:(MapWindowController *)theWindowController;

- (BOOL)keyDown:(NSEvent *)event sender:(id)sender;
- (BOOL)keyUp:(NSEvent *)event sender:(id)sender;

- (void)flagsChanged:(NSEvent *)event sender:(id)sender;
- (void)leftMouseDragged:(NSEvent *)event sender:(id)sender;
- (void)mouseMoved:(NSEvent *)event sender:(id)sender;
- (void)mouseEntered:(NSEvent *)event sender:(id)sender;
- (void)mouseExited:(NSEvent *)event sender:(id)sender;
- (void)leftMouseDown:(NSEvent *)event sender:(id)sender;
- (void)leftMouseUp:(NSEvent *)event sender:(id)sender;
- (void)rightMouseDragged:(NSEvent *)event sender:(id)sender;
- (void)rightMouseDown:(NSEvent *)event sender:(id)sender;
- (void)rightMouseUp:(NSEvent *)event sender:(id)sender;
- (void)scrollWheel:(NSEvent *)event sender:(id)sender;
- (void)beginGesture:(NSEvent *)event sender:(id)sender;
- (void)endGesture:(NSEvent *)event sender:(id)sender;
- (void)magnify:(NSEvent *)event sender:(id)sender;

- (NSDragOperation)draggingEntered:(id <NSDraggingInfo>)sender;
- (NSDragOperation)draggingUpdated:(id <NSDraggingInfo>)sender;
- (void)draggingEnded:(id <NSDraggingInfo>)sender;
- (void)draggingExited:(id <NSDraggingInfo>)sender;
- (BOOL)prepareForDragOperation:(id <NSDraggingInfo>)sender;
- (BOOL)performDragOperation:(id <NSDraggingInfo>)sender;
- (void)concludeDragOperation:(id <NSDraggingInfo>)sender;

- (void)toggleDragVertexTool;
- (void)toggleDragEdgeTool;
- (void)toggleDragFaceTool;
- (void)toggleClipTool;
- (Tool *)currentModalTool;
- (ClipTool *)clipTool;

- (NSPoint)menuPosition;
- (PickingHitList *)currentHits;

@end
