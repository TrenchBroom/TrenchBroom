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

#import "DragVertexTool.h"
#import "Math.h"
#import "MapWindowController.h"
#import "MapDocument.h"
#import "PickingHit.h"
#import "PickingHitList.h"
#import "Camera.h"
#import "EditingSystem.h"
#import "Options.h"
#import "Grid.h"
#import "SelectionManager.h"
#import "MutableBrush.h"
#import "Face.h"

@interface DragVertexTool (private)

- (BOOL)isAlternatePlaneModifierPressed;
- (void)updateMoveDirectionWithRay:(const TRay *)theRay hits:(PickingHitList *)theHits;

@end

@implementation DragVertexTool (private)

- (BOOL)isAlternatePlaneModifierPressed {
    return (keyStatus & KS_OPTION) == KS_OPTION;
}

- (void)updateMoveDirectionWithRay:(const TRay *)theRay hits:(PickingHitList *)theHits {
    if (editingSystem != nil)
        [editingSystem release];

    Camera* camera = [windowController camera];
    editingSystem = [[EditingSystem alloc] initWithCamera:camera vertical:[self isAlternatePlaneModifierPressed]];
}

@end

@implementation DragVertexTool

- (id)initWithWindowController:(MapWindowController *)theWindowController {
    NSAssert(theWindowController != nil, @"window controller must not be nil");
    
    if ((self = [self init])) {
        windowController = theWindowController;
        state = VTS_DEFAULT;
    }
    
    return self;
}

- (void)dealloc {
    [editingSystem release];
    [super dealloc];
}

- (void)handleKeyStatusChanged:(NSEvent *)event status:(EKeyStatus)theKeyStatus ray:(TRay *)ray hits:(PickingHitList *)hits {
    keyStatus = theKeyStatus;
    [self updateMoveDirectionWithRay:ray hits:hits];
}

- (void)beginLeftDrag:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {
    NSAssert(event != NULL, @"event must not be NULL");
    NSAssert(ray != NULL, @"ray must not be NULL");
    NSAssert(hits != nil, @"hit list must not be nil");
    
    [self updateMoveDirectionWithRay:ray hits:hits];

    [hits retain];
    
    PickingHit* hit = [hits firstHitOfType:HT_VERTEX ignoreOccluders:NO];
    if (hit != nil) {
        MapDocument* map = [windowController document];
        NSUndoManager* undoManager = [map undoManager];
        [undoManager setGroupsByEvent:NO];
        [undoManager beginUndoGrouping];
        
        brush = [hit object];
        [map snapBrushes:[NSArray arrayWithObject:brush]];
        
        index = [hit vertexIndex];
        const TVertexList* vertices = [brush vertices];
        const TEdgeList* edges = [brush edges];
        
        if (index < vertices->count) {
            TVertex* vertex = vertices->items[index];
            lastPoint = vertex->position;
        } else if (index < vertices->count + edges->count) {
            TEdge* edge = edges->items[index - vertices->count];
            centerOfEdge(edge, &lastPoint);
        } else {
            // the side index is not necessarily the same as the face index!!!
            id <Face> face = [[brush faces] objectAtIndex:index - edges->count - vertices->count];
            centerOfVertices([face vertices], &lastPoint);
        }
        editingPoint = lastPoint;
        
        state = VTS_DRAG;
    }
    
    [hits release];
}

- (void)leftDrag:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {
    if (state != VTS_DRAG)
        return;
    
    float dist = [editingSystem intersectWithRay:ray planePosition:&editingPoint];
    if (isnan(dist))
        return;
    
    TVector3f point;
    rayPointAtDistance(ray, dist, &point);
    
    TVector3f deltaf;
    subV3f(&point, &lastPoint, &deltaf);
    
    Options* options = [windowController options];
    Grid* grid = [options grid];
    
    MapDocument* map = [windowController document];
    const TBoundingBox* worldBounds = [map worldBounds];
    
    TBoundingBox bounds;
    bounds.min = lastPoint;
    bounds.max = lastPoint;
    
    [grid moveDeltaForBounds:&bounds worldBounds:worldBounds delta:&deltaf lastPoint:&lastPoint];

    if (nullV3f(&deltaf))
        return;
    
    index = [map dragVertex:index brush:brush delta:&deltaf];
    if (index == -1) {
        [self endLeftDrag:event ray:ray hits:hits];
        state = VTS_CANCEL;
    }
}

- (void)endLeftDrag:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {
    if (state == VTS_CANCEL) {
        state = VTS_DEFAULT;
    }
    
    if (state != VTS_DRAG)
        return;
    
    MapDocument* map = [windowController document];
    NSUndoManager* undoManager = [map undoManager];
    [undoManager setActionName:[self actionName]];
    [undoManager endUndoGrouping];
    [undoManager setGroupsByEvent:YES];

    brush = nil;
    index = -1;
    
    state = VTS_DEFAULT;
}

- (NSString *)actionName {
    return @"Drag Vertex";
}
@end
