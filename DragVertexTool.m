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
#import "DragPlane.h"
#import "Options.h"
#import "Grid.h"
#import "SelectionManager.h"
#import "MutableBrush.h"
#import "Face.h"

@implementation DragVertexTool

- (BOOL)doBeginLeftDrag:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits lastPoint:(TVector3f *)lastPoint {
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
            *lastPoint = vertex->position;
        } else if (index < vertices->count + edges->count) {
            TEdge* edge = edges->items[index - vertices->count];
            centerOfEdge(edge, lastPoint);
        } else {
            // the side index is not necessarily the same as the face index!!!
            id <Face> face = [[brush faces] objectAtIndex:index - edges->count - vertices->count];
            centerOfVertices([face vertices], lastPoint);
        }
    }
    
    [hits release];
    return hits != nil;
}

- (BOOL)doLeftDrag:(NSEvent *)event ray:(TRay *)ray delta:(TVector3f *)delta hits:(PickingHitList *)hits lastPoint:(TVector3f *)lastPoint {
    Options* options = [windowController options];
    Grid* grid = [options grid];
    
    MapDocument* map = [windowController document];
    const TBoundingBox* worldBounds = [map worldBounds];
    
    TVector3f nextPoint = *lastPoint;
    [grid moveDeltaForVertex:lastPoint worldBounds:worldBounds delta:delta lastPoint:&nextPoint];

    if (nullV3f(delta))
        return YES;
    
    int newIndex = [map dragVertex:index brush:brush delta:delta];
    if (newIndex == -1) {
        [self endLeftDrag:event ray:ray hits:hits];
    } else if (newIndex < [brush vertices]->count) {
        *lastPoint = nextPoint;
    }
    
    index = newIndex;
    return index != -1;
}

- (void)doEndLeftDrag:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {
    MapDocument* map = [windowController document];
    NSUndoManager* undoManager = [map undoManager];
    [undoManager setActionName:[self actionName]];
    [undoManager endUndoGrouping];
    [undoManager setGroupsByEvent:YES];

    brush = nil;
    index = -1;
}

- (NSString *)actionName {
    return @"Drag Vertex";
}
@end
