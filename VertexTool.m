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

#import "VertexTool.h"
#import "Math.h"
#import "MapWindowController.h"
#import "MapDocument.h"
#import "CursorManager.h"
#import "DragVertexCursor.h"
#import "PickingHit.h"
#import "PickingHitList.h"
#import "Camera.h"
#import "EditingSystem.h"
#import "Options.h"
#import "Grid.h"
#import "SelectionManager.h"
#import "MutableBrush.h"
#import "Face.h"

@interface VertexTool (private)

- (BOOL)isAlternatePlaneModifierPressed;
- (void)updateMoveDirectionWithRay:(const TRay *)theRay hits:(PickingHitList *)theHits;

@end

@implementation VertexTool (private)

- (BOOL)isAlternatePlaneModifierPressed {
    return keyStatus == KS_OPTION;
}

- (void)updateMoveDirectionWithRay:(const TRay *)theRay hits:(PickingHitList *)theHits {
    if (editingSystem != nil)
        [editingSystem release];

    Camera* camera = [windowController camera];
    editingSystem = [[EditingSystem alloc] initWithCamera:camera vertical:[self isAlternatePlaneModifierPressed]];
}

@end

@implementation VertexTool

- (id)initWithWindowController:(MapWindowController *)theWindowController {
    NSAssert(theWindowController != nil, @"window controller must not be nil");
    
    if ((self = [self init])) {
        windowController = theWindowController;
        cursor = [[DragVertexCursor alloc] init];
        state = VTS_DEFAULT;
    }
    
    return self;
}

- (void)dealloc {
    [cursor release];
    [editingSystem release];
    [super dealloc];
}

- (void)handleKeyStatusChanged:(NSEvent *)event status:(EKeyStatus)theKeyStatus ray:(TRay *)ray hits:(PickingHitList *)hits {
    keyStatus = theKeyStatus;
}

- (void)beginLeftDrag:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {
    NSAssert(event != NULL, @"event must not be NULL");
    NSAssert(ray != NULL, @"ray must not be NULL");
    NSAssert(hits != nil, @"hit list must not be nil");
    
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
    
    Options* options = [windowController options];
    Grid* grid = [options grid];
    [grid snapToGridV3f:&point result:&point];

    TVector3f deltaf;
    subV3f(&point, &lastPoint, &deltaf);

    if (nullV3f(&deltaf))
        return;
    
    MapDocument* map = [windowController document];
    index = [map dragVertex:index brush:brush delta:&deltaf];
    if (index == -1) {
        [self endLeftDrag:event ray:ray hits:hits];
        state = VTS_CANCEL;
        [self unsetCursor:event ray:ray hits:hits];
    } else if (index < [brush vertices]->count) {
        lastPoint = point;
    }
}

- (void)endLeftDrag:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {
    if (state == VTS_CANCEL) {
        [self setCursor:event ray:ray hits:hits];
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

- (void)setCursor:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {
    CursorManager* cursorManager = [windowController cursorManager];
    [cursorManager pushCursor:cursor];
    [self updateCursor:event ray:ray hits:hits];
}

- (void)unsetCursor:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {
    CursorManager* cursorManager = [windowController cursorManager];
    [cursorManager popCursor];
}

- (void)updateCursor:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {
    TVector3f position;

    [self updateMoveDirectionWithRay:ray hits:hits];
    
    if (state == VTS_DRAG) {
        float dist = [editingSystem intersectWithRay:ray planePosition:&editingPoint];
        if (isnan(dist))
            return;
        
        rayPointAtDistance(ray, dist, &position);

        Camera* camera = [windowController camera];
        [cursor setEditingSystem:editingSystem];
        [cursor setCameraPosition:[camera position]];
        [cursor setAttention:NO];
    } else if (state == VTS_CANCEL) {
        
    } else {
        NSArray* vertexHits = [hits hitsOfType:HT_VERTEX];
        if ([vertexHits count] == 0)
            return;
        
        position = *[[vertexHits objectAtIndex:0] hitPoint];
        
        BOOL attention = NO;
        NSEnumerator* hitEn = [vertexHits objectEnumerator];
        PickingHit* hit;
        while (!attention && (hit = [hitEn nextObject])) {
            id <Brush> hitBrush = [hit object];
            const TVertexList* vertices = [hitBrush vertices];
            for (int i = 0; i < vertices->count && !attention; i++)
                attention = !intV3f(&vertices->items[i]->position);
        }
        
        Camera* camera = [windowController camera];
        [cursor setEditingSystem:editingSystem];
        [cursor setCameraPosition:[camera position]];
        [cursor setAttention:attention];
    }
    
    [cursor setPosition:&position];
}

- (NSString *)actionName {
    return @"Drag Vertices";
}
@end
