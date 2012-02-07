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
#import "EditingSystem.h"
#import "MoveCursor.h"
#import "CursorManager.h"
#import "CameraOrbitAnimation.h"

@interface MoveTool (private)

- (BOOL)isAlternatePlaneModifierPressed;
- (void)updateMoveDirectionWithRay:(const TRay *)theRay hits:(PickingHitList *)theHits;

@end

@implementation MoveTool (private)

- (BOOL)isAlternatePlaneModifierPressed {
    return keyStatus == KS_OPTION;
}

- (void)updateMoveDirectionWithRay:(const TRay *)theRay hits:(PickingHitList *)theHits {
    if (editingSystem != nil)
        [editingSystem release];
    
    Camera* camera = [windowController camera];
    editingSystem = [[EditingSystem alloc] initWithCamera:camera vertical:[self isAlternatePlaneModifierPressed]];
    
    /*
    EditingSystem* newEditingSystem;
    TVector3f norm;
    
    Camera* camera = [windowController camera];

    PickingHit* hit = [theHits firstHitOfType:HT_FACE | HT_ENTITY ignoreOccluders:NO];
    if (hit == nil)
        return;
    
    if ([hit type] == HT_FACE) {
        id <Face> face = [hit object];
        newEditingSystem = [[EditingSystem alloc] initWithCamera:camera yAxis:[face norm] invert:[self isAlternatePlaneModifierPressed]];
    } else {
        id <Entity> entity = [hit object];
        intersectBoundsWithRay([entity bounds], theRay, &norm);
        newEditingSystem = [[EditingSystem alloc] initWithCamera:camera yAxis:&norm invert:[self isAlternatePlaneModifierPressed]];
    }
    
    if (newEditingSystem != nil) {
        if (editingSystem != nil)
            [editingSystem release];
        editingSystem = newEditingSystem;
    }
     */
}

@end

@implementation MoveTool

- (id)initWithWindowController:(MapWindowController *)theWindowController {
    if ((self = [self init])) {
        windowController = theWindowController;
        moveCursor = [[MoveCursor alloc] init];
    }
    return self;
}

- (void)dealloc {
    if (editingSystem != nil)
        [editingSystem release];
    [moveCursor release];
    [super dealloc];
}

# pragma mark -
# pragma mark @implementation Tool

- (void)handleKeyStatusChanged:(NSEvent *)event status:(EKeyStatus)theKeyStatus ray:(TRay *)ray hits:(PickingHitList *)hits {
    keyStatus = theKeyStatus;
    
    [self updateMoveDirectionWithRay:ray hits:hits];
    
    float dist = [editingSystem intersectWithRay:ray planePosition:&editingPoint];
    rayPointAtDistance(ray, dist, &lastPoint);

    [[windowController view3D] setNeedsDisplay:YES];
}

- (void)beginLeftDrag:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {
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

    [self updateMoveDirectionWithRay:ray hits:hits];
    
    lastPoint = *[hit hitPoint];
    editingPoint = lastPoint;
    
    MapDocument* map = [windowController document];
    NSUndoManager* undoManager = [map undoManager];
    [undoManager setGroupsByEvent:NO];
    [undoManager beginUndoGrouping];

    drag = YES;
}

- (void)leftDrag:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {
    if (!drag)
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
    SelectionManager* selectionManager = [map selectionManager];
    [selectionManager selectionBounds:&bounds];

    [grid moveDeltaForBounds:&bounds worldBounds:worldBounds delta:&deltaf lastPoint:&lastPoint];
    
    if (nullV3f(&deltaf))
        return;
    
    TVector3i deltai;
    roundV3f(&deltaf, &deltai);
    
    [map translateBrushes:[selectionManager selectedBrushes] delta:deltai lockTextures:[options lockTextures]];
    [map translateEntities:[selectionManager selectedEntities] delta:deltai];
}

- (void)endLeftDrag:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {
    if (!drag)
        return;
    
    MapDocument* map = [windowController document];
    NSUndoManager* undoManager = [map undoManager];
    [undoManager setActionName:[self actionName]];
    [undoManager endUndoGrouping];
    [undoManager setGroupsByEvent:YES];
    
    if (editingSystem == nil) {
        [editingSystem release];
        editingSystem = nil;
    }

    drag = NO;
}

- (void)setCursor:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {
    CursorManager* cursorManager = [windowController cursorManager];
    [cursorManager pushCursor:moveCursor];
    [self updateCursor:event ray:ray hits:hits];
}

- (void)unsetCursor:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {
    CursorManager* cursorManager = [windowController cursorManager];
    [cursorManager popCursor];
}

- (void)updateCursor:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {
    TVector3f position;
    
    if (!drag) {
        [self updateMoveDirectionWithRay:ray hits:hits];

        PickingHit* hit = [hits firstHitOfType:HT_ENTITY | HT_FACE ignoreOccluders:YES];
        if (hit != nil) {
            float dist = [editingSystem intersectWithRay:ray planePosition:[hit hitPoint]];
            rayPointAtDistance(ray, dist, &position);
        }
    } else {
        float dist = [editingSystem intersectWithRay:ray planePosition:&editingPoint];
        rayPointAtDistance(ray, dist, &position);
    }

    [moveCursor setEditingSystem:editingSystem];
    [moveCursor setPosition:&position];
    [moveCursor setCameraPosition:[[windowController camera] position]];
}

- (NSString *)actionName {
    return @"Move Objects";
}

@end
