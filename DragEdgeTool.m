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

#import "DragEdgeTool.h"
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
#import "EdgeFeedbackFigure.h"
#import "DragEdgeToolFeedbackFigure.h"

TVector4f const CurrentEdgeColor = {1, 1, 1, 1};
TVector4f const EdgeColor = {1, 0, 0, 1};

@implementation DragEdgeTool

- (id)initWithWindowController:(MapWindowController *)theWindowController {
    if ((self = [super initWithWindowController:theWindowController])) {
        Camera* camera = [theWindowController camera];
        edgeFigure = [[EdgeFeedbackFigure alloc] initWithCamera:camera radius:3 color:&CurrentEdgeColor];
        feedbackFigure = [[DragEdgeToolFeedbackFigure alloc] initWithCamera:camera radius:3 color:&EdgeColor];
    }
    
    return self;
}

- (void)dealloc {
    [edgeFigure release];
    [feedbackFigure release];
    [super dealloc];
}

- (void)activated:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {
    SelectionManager* selectionManager = [windowController selectionManager];
    [feedbackFigure setBrushes:[selectionManager selectedBrushes]];
    
    [self addFeedbackFigure:feedbackFigure];
}

- (void)deactivated:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {
    [self removeFeedbackFigure:feedbackFigure];
}

- (BOOL)doBeginLeftDrag:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits lastPoint:(TVector3f *)lastPoint {
    [hits retain];
    
    PickingHit* hit = [hits firstHitOfType:HT_VERTEX ignoreOccluders:NO];
    if (hit != nil) {
        brush = [hit object];
        const TVertexList* vertices = [brush vertices];
        const TEdgeList* edges = [brush edges];
        index = [hit vertexIndex] - vertices->count;
        
        if (index >= 0 && index < edges->count) {
            MapDocument* map = [windowController document];
            NSUndoManager* undoManager = [map undoManager];
            [undoManager setGroupsByEvent:NO];
            [undoManager beginUndoGrouping];
            
            [map snapBrushes:[NSArray arrayWithObject:brush]];
            
            TEdge* edge = edges->items[index];
            centerOfEdge(edge, lastPoint);

            [edgeFigure setEdge:edge];
            [self addFeedbackFigure:edgeFigure];
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
    
    TBoundingBox bounds;
    TEdge* edge = [brush edges]->items[index];
    bounds.min = edge->startVertex->position;
    bounds.max = edge->startVertex->position;
    mergeBoundsWithPoint(&bounds, &edge->endVertex->position, &bounds);
    
    TVector3f nextPoint = *lastPoint;
    [grid moveDeltaForBounds:&bounds worldBounds:worldBounds delta:delta lastPoint:&nextPoint];

    if (nullV3f(delta))
        return YES;
    
    TDragResult result = [map dragEdge:index brush:brush delta:delta];
    if (result.index != -1) {
        TEdge* edge = [brush edges]->items[result.index];
        [edgeFigure setEdge:edge];
        *lastPoint = nextPoint;
    }
    
    index = result.index;
    return index != -1;
}

- (void)doEndLeftDrag:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {
    MapDocument* map = [windowController document];
    NSUndoManager* undoManager = [map undoManager];
    [undoManager setActionName:[self actionName]];
    [undoManager endUndoGrouping];
    [undoManager setGroupsByEvent:YES];

    [self removeFeedbackFigure:edgeFigure];
    brush = nil;
    index = -1;
}

- (NSString *)actionName {
    return @"Drag Edge";
}
@end
