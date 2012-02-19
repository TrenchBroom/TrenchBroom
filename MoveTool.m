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

#import "MoveTool.h"
#import "Grid.h"
#import "Options.h"
#import "Camera.h"
#import "MapWindowController.h"
#import "MapDocument.h"
#import "SelectionManager.h"
#import "PickingHit.h"
#import "PickingHitList.h"
#import "Face.h"
#import "Brush.h"
#import "Entity.h"
#import "MutableBrush.h"
#import "math.h"
#import "Renderer.h"
#import "MapView3D.h"
#import "ControllerUtils.h"
#import "DragPlane.h"
#import "CameraOrbitAnimation.h"

@implementation MoveTool

# pragma mark -
# pragma mark @implementation Tool

- (BOOL)doBeginLeftDrag:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits lastPoint:(TVector3f *)lastPoint {
    PickingHit* hit = [hits firstHitOfType:HT_ENTITY | HT_FACE ignoreOccluders:YES];
    if (hit == nil)
        return;
    
    SelectionManager* selectionManager = [windowController selectionManager];
    if ([hit type] == HT_FACE) {
        id <Face> face = [hit object];
        id <Brush> brush = [face brush];
        
        if (![selectionManager isBrushSelected:brush])
            return;
    } else {
        id <Entity> entity = [hit object];
        
        if (![selectionManager isEntitySelected:entity])
            return;
    }
    
    *lastPoint = *[hit hitPoint];
    
    MapDocument* map = [windowController document];
    NSUndoManager* undoManager = [map undoManager];
    [undoManager setGroupsByEvent:NO];
    [undoManager beginUndoGrouping];

    return YES;
}

- (BOOL)doLeftDrag:(NSEvent *)event ray:(TRay *)ray delta:(TVector3f *)delta hits:(PickingHitList *)hits lastPoint:(TVector3f *)lastPoint {
    Options* options = [windowController options];
    Grid* grid = [options grid];
    
    MapDocument* map = [windowController document];
    const TBoundingBox* worldBounds = [map worldBounds];

    TBoundingBox bounds;
    SelectionManager* selectionManager = [map selectionManager];
    [selectionManager selectionBounds:&bounds];

    [grid moveDeltaForBounds:&bounds worldBounds:worldBounds delta:delta lastPoint:lastPoint];
    
    if (nullV3f(delta))
        return YES;
    
    [map translateBrushes:[selectionManager selectedBrushes] delta:*delta lockTextures:[options lockTextures]];
    [map translateEntities:[selectionManager selectedEntities] delta:*delta];

    return YES;
}

- (void)doEndLeftDrag:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {
    MapDocument* map = [windowController document];
    NSUndoManager* undoManager = [map undoManager];
    [undoManager setActionName:[self actionName]];
    [undoManager endUndoGrouping];
    [undoManager setGroupsByEvent:YES];
}

- (NSString *)actionName {
    return @"Move Objects";
}

@end
