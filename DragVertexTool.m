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
#import "VertexFeedbackFigure.h"
#import "DragVertexToolFeedbackFigure.h"

TVector4f const CurrentVertexColor = {1, 0, 0, 1};
TVector4f const VertexColor = {0, 114 / 255.0f, 229 / 255.0f, 1};

@implementation DragVertexTool

- (id)initWithWindowController:(MapWindowController *)theWindowController {
    if ((self = [super initWithWindowController:theWindowController])) {
        Camera* camera = [theWindowController camera];
        vertexFigure = [[VertexFeedbackFigure alloc] initWithCamera:camera radius:3 color:&CurrentVertexColor];
        feedbackFigure = [[DragVertexToolFeedbackFigure alloc] initWithCamera:camera radius:3 color:&VertexColor];
    }
    
    return self;
}

- (void)dealloc {
    [vertexFigure release];
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
    
    PickingHit* hit = [hits firstHitOfType:HT_VERTEX_HANDLE | HT_EDGE_HANDLE | HT_FACE_HANDLE ignoreOccluders:YES];
    if (hit != nil) {
        MapDocument* map = [windowController document];
        NSUndoManager* undoManager = [map undoManager];
        [undoManager setGroupsByEvent:NO];
        [undoManager beginUndoGrouping];
        
        brush = [hit object];
        [map snapBrushes:[NSArray arrayWithObject:brush]];
        
        index = [hit index];
        const TVertexList* vertices = [brush vertices];
        const TEdgeList* edges = [brush edges];
        
        if ([hit type] == HT_VERTEX_HANDLE) {
            TVertex* vertex = vertices->items[index];
            *lastPoint = vertex->position;
            [vertexFigure setVertex:vertex];
        } else if ([hit type] == HT_EDGE_HANDLE) {
            TEdge* edge = edges->items[index];
            centerOfEdge(edge, lastPoint);
            [vertexFigure setPosition:lastPoint];
            index += vertices->count;
        } else {
            // the side index is not necessarily the same as the face index!!!
            id <Face> face = [[brush faces] objectAtIndex:index];
            centerOfVertices([face vertices], lastPoint);
            [vertexFigure setPosition:lastPoint];
            index += vertices->count + edges->count;
        }

        [self addFeedbackFigure:vertexFigure];
    }
    
    [hits release];
    return hit != nil;
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
    
    TDragResult result = [map dragVertex:index brush:brush delta:delta];
    if (result.index == -1) {
        [self endLeftDrag:event ray:ray hits:hits];
    } else if (result.moved) {
        *lastPoint = nextPoint;
        TVertex* vertex = [brush vertices]->items[result.index];
        [vertexFigure setVertex:vertex];
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

    [self removeFeedbackFigure:vertexFigure];
    brush = nil;
    index = -1;
}

- (NSString *)actionName {
    return @"Drag Vertex";
}
@end
