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

#import "DragFaceTool.h"
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
#import "FaceFeedbackFigure.h"
#import "DragFaceToolFeedbackFigure.h"

TVector4f const CurrentFaceColor = {1, 0, 0, 1};
TVector4f const FaceColor = {230 / 255.0f, 31 / 255.0f, 1, 1};

@implementation DragFaceTool

- (id)initWithWindowController:(MapWindowController *)theWindowController {
    if ((self = [super initWithWindowController:theWindowController])) {
        Camera* camera = [theWindowController camera];
        faceFigure = [[FaceFeedbackFigure alloc] initWithCamera:camera radius:3 color:&CurrentFaceColor];
        feedbackFigure = [[DragFaceToolFeedbackFigure alloc] initWithCamera:camera radius:3 color:&FaceColor];
    }
    
    return self;
}

- (void)dealloc {
    [faceFigure release];
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
    
    PickingHit* hit = [hits firstHitOfType:HT_FACE_HANDLE ignoreOccluders:YES];
    if (hit != nil) {
        brush = [hit object];
        NSArray* faces = [brush faces];
        index = [hit index];
        
        if (index >= 0 && index < [faces count]) {
            MapDocument* map = [windowController document];
            NSUndoManager* undoManager = [map undoManager];
            [undoManager setGroupsByEvent:NO];
            [undoManager beginUndoGrouping];
            
            [map snapBrushes:[NSArray arrayWithObject:brush]];
            
            id <Face> face = [faces objectAtIndex:index];
            centerOfVertices([face vertices], lastPoint);
            
            [faceFigure setVertices:[face vertices]];
            [self addFeedbackFigure:faceFigure];
        }
    }
    
    [hits release];
    return hit != nil;
}

- (BOOL)doLeftDrag:(NSEvent *)event ray:(TRay *)ray delta:(TVector3f *)delta hits:(PickingHitList *)hits lastPoint:(TVector3f *)lastPoint {
    Options* options = [windowController options];
    Grid* grid = [options grid];

    MapDocument* map = [windowController document];
    const TBoundingBox* worldBounds = [map worldBounds];
    
    TBoundingBox bounds;
    id <Face> face = [[brush faces] objectAtIndex:index];
    boundsOfVertices([face vertices], &bounds);
    
    TVector3f nextPoint = *lastPoint;
    [grid moveDeltaForBounds:&bounds worldBounds:worldBounds delta:delta lastPoint:&nextPoint];
    
    if (nullV3f(delta))
        return YES;
    
    TDragResult result = [map dragFace:index brush:brush delta:delta];
    if (result.index != -1) {
        face = [[brush faces] objectAtIndex:result.index];
        [faceFigure setVertices:[face vertices]];
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
    
    [self removeFeedbackFigure:faceFigure];
    brush = nil;
    index = -1;
}

- (NSString *)actionName {
    return @"Drag Face";
}
@end
