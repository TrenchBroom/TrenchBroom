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

#import "CreateBrushTool.h"
#import "MapWindowController.h"
#import "Camera.h"
#import "Options.h"
#import "Grid.h"
#import "MapDocument.h"
#import "BoundsFeedbackFigure.h"
#import "Renderer.h"
#import "SelectionManager.h"
#import "PickingHit.h"
#import "PickingHitList.h"
#import "Camera.h"
#import "EditingPlaneFigure.h"
#import "GLResources.h"
#import "TextureManager.h"
#import "Texture.h"
#import "EditingSystem.h"

@interface CreateBrushTool (private)

- (void)updateMoveDirectionWithRay:(const TRay *)theRay hits:(PickingHitList *)theHits;

@end

@implementation CreateBrushTool (private)

- (void)updateMoveDirectionWithRay:(const TRay *)theRay hits:(PickingHitList *)theHits {
    if (editingSystem != nil)
        [editingSystem release];
    
    Camera* camera = [windowController camera];
    editingSystem = [[EditingSystem alloc] initWithCamera:camera vertical:strongestComponentV3f([camera direction]) != A_Z];
    
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

@implementation CreateBrushTool

- (id)initWithWindowController:(MapWindowController *)theWindowController {
    if ((self = [self init])) {
        windowController = theWindowController;
        drag = NO;
    }
    
    return self;
}

- (void)beginLeftDrag:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {
    Camera* camera = [windowController camera];

    PickingHit* hit = [hits firstHitOfType:HT_FACE ignoreOccluders:NO];
    if (hit != nil)
        lastPoint = *[hit hitPoint];
    else
        lastPoint = [camera defaultPointOnRay:ray];

    Grid* grid = [[windowController options] grid];
    [grid snapDownToGridV3f:&lastPoint result:&initialBounds.min];
    [grid snapUpToGridV3f:&lastPoint result:&initialBounds.max];
    [grid snapToGridV3f:&lastPoint result:&lastPoint];
    
    const TVector3f* cameraDir = [camera direction];
    
    if (initialBounds.min.x == initialBounds.max.x) {
        if (cameraDir->x > 0)
            initialBounds.min.x -= [grid actualSize];
        else
            initialBounds.max.x += [grid actualSize];
    }
    if (initialBounds.min.y == initialBounds.max.y) {
        if (cameraDir->y > 0)
            initialBounds.min.y -= [grid actualSize];
        else
            initialBounds.max.y += [grid actualSize];
    }
    if (initialBounds.min.z == initialBounds.max.z) {
        if (cameraDir->z > 0)
            initialBounds.min.z -= [grid actualSize];
        else
            initialBounds.max.z += [grid actualSize];
    }

    [self updateMoveDirectionWithRay:ray hits:hits];
    editingPoint = lastPoint;

    MapDocument* map = [windowController document];
    const TBoundingBox* worldBounds = [map worldBounds];
    if (!boundsContainBounds(worldBounds, &initialBounds))
        return;
    
    NSUndoManager* undoManager = [map undoManager];
    [undoManager setGroupsByEvent:NO];
    [undoManager beginUndoGrouping];
    
    SelectionManager* selectionManager = [windowController selectionManager];
    TextureManager* textureManager = [[map glResources] textureManager];
    
    NSArray* textureMRU = [selectionManager textureMRU];
    Texture* texture = [textureMRU count] > 0 ? [textureMRU lastObject] : [textureManager textureForName:@"none"];

    brush = [map createBrushInEntity:[map worldspawn:YES] withBounds:&initialBounds texture:texture];
    [selectionManager addBrush:brush record:YES];
    
    drag = YES;
    scrollFront = NO;
}

- (void)leftDrag:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {
    if (!drag)
        return;
    
    float dist = [editingSystem intersectWithRay:ray planePosition:&editingPoint];
    if (isnan(dist))
        return;
    
    TVector3f point;
    rayPointAtDistance(ray, dist, &point);
    
    Grid* grid = [[windowController options] grid];
    [grid snapToGridV3f:&point result:&point];
    if (equalV3f(&point, &lastPoint))
        return;
    
    TBoundingBox bounds = initialBounds;
    mergeBoundsWithPoint(&bounds, &point, &bounds);
    
    MapDocument* map = [windowController document];
    const TBoundingBox* worldBounds = [map worldBounds];
    if (!boundsContainBounds(worldBounds, &bounds))
        return;
    
    lastPoint = point;

    NSUndoManager* undoManager = [map undoManager];
    [undoManager endUndoGrouping];
    [undoManager undo];
    [undoManager beginUndoGrouping];
    
    SelectionManager* selectionManager = [windowController selectionManager];
    TextureManager* textureManager = [[map glResources] textureManager];
    
    NSArray* textureMRU = [selectionManager textureMRU];
    Texture* texture = [textureMRU count] > 0 ? [textureMRU lastObject] : [textureManager textureForName:@"none"];
    
    brush = [map createBrushInEntity:[map worldspawn:YES] withBounds:&bounds texture:texture];
    [selectionManager addBrush:brush record:YES];
}

- (void)endLeftDrag:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {
    if (!drag)
        return;
    
    MapDocument* map = [windowController document];
    NSUndoManager* undoManager = [map undoManager];
    [undoManager setActionName:[self actionName]];
    [undoManager endUndoGrouping];
    [undoManager setGroupsByEvent:YES];
    
    if (editingSystem != nil) {
        [editingSystem release];
        editingSystem = nil;
    }

    brush = nil;
    drag = NO;
    scrollFront = NO;
}

- (void)leftScroll:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {
    if (!drag)
        return;
    
    Grid* grid = [[windowController options] grid];
    float delta = [grid actualSize] * ([event deltaY] / fabsf([event deltaY]));
    
    if (equalV3f([editingSystem zAxisNeg], &XAxisPos)) {
        if (( scrollFront && initialBounds.min.x + delta >= initialBounds.max.x - [grid actualSize]) ||
            (!scrollFront && initialBounds.max.x + delta <= initialBounds.min.x + [grid actualSize]))
            scrollFront = !scrollFront;
        
        if (scrollFront)
            initialBounds.min.x = initialBounds.min.x + delta;
        else
            initialBounds.max.x = initialBounds.max.x + delta;
    } else if (equalV3f([editingSystem zAxisNeg], &XAxisNeg)) {
        if (( scrollFront && initialBounds.max.x - delta <= initialBounds.min.x + [grid actualSize]) ||
            (!scrollFront && initialBounds.min.x - delta >= initialBounds.max.x - [grid actualSize]))
            scrollFront = !scrollFront;
        
        if (scrollFront)
            initialBounds.max.x = initialBounds.max.x - delta;
        else
            initialBounds.min.x = initialBounds.min.x - delta;
    } else if (equalV3f([editingSystem zAxisNeg], &YAxisPos)) {
        if (( scrollFront && initialBounds.min.y + delta >= initialBounds.max.y - [grid actualSize]) ||
            (!scrollFront && initialBounds.max.y + delta <= initialBounds.min.y + [grid actualSize]))
            scrollFront = !scrollFront;
        
        if (scrollFront)
            initialBounds.min.y = initialBounds.min.y + delta;
        else
            initialBounds.max.y = initialBounds.max.y + delta;
    } else if (equalV3f([editingSystem zAxisNeg], &YAxisNeg)) {
        if (( scrollFront && initialBounds.max.y - delta <= initialBounds.min.y + [grid actualSize]) ||
            (!scrollFront && initialBounds.min.y - delta >= initialBounds.max.y - [grid actualSize]))
            scrollFront = !scrollFront;
        
        if (scrollFront)
            initialBounds.max.y = initialBounds.max.y - delta;
        else
            initialBounds.min.y = initialBounds.min.y - delta;
    } else if (equalV3f([editingSystem zAxisNeg], &ZAxisPos)) {
        if (( scrollFront && initialBounds.min.z + delta >= initialBounds.max.z - [grid actualSize]) ||
            (!scrollFront && initialBounds.max.z + delta <= initialBounds.min.z + [grid actualSize]))
            scrollFront = !scrollFront;
        
        if (scrollFront)
            initialBounds.min.z = initialBounds.min.z + delta;
        else
            initialBounds.max.z = initialBounds.max.z + delta;
    } else if (equalV3f([editingSystem zAxisNeg], &ZAxisNeg)) {
        if (( scrollFront && initialBounds.max.z - delta <= initialBounds.min.z + [grid actualSize]) ||
            (!scrollFront && initialBounds.min.z - delta >= initialBounds.max.z - [grid actualSize]))
            scrollFront = !scrollFront;
        
        if (scrollFront)
            initialBounds.max.z = initialBounds.max.z - delta;
        else
            initialBounds.min.z = initialBounds.min.z - delta;
    }

    TBoundingBox bounds = initialBounds;
    mergeBoundsWithPoint(&bounds, &lastPoint, &bounds);

    MapDocument* map = [windowController document];
    const TBoundingBox* worldBounds = [map worldBounds];
    if (!boundsContainBounds(worldBounds, &bounds))
        return;
    
    NSUndoManager* undoManager = [map undoManager];
    [undoManager endUndoGrouping];
    [undoManager undo];
    [undoManager beginUndoGrouping];
    
    SelectionManager* selectionManager = [windowController selectionManager];
    TextureManager* textureManager = [[map glResources] textureManager];
    
    NSArray* textureMRU = [selectionManager textureMRU];
    Texture* texture = [textureMRU count] > 0 ? [textureMRU lastObject] : [textureManager textureForName:@"none"];
    
    brush = [map createBrushInEntity:[map worldspawn:YES] withBounds:&bounds texture:texture];
    [selectionManager addBrush:brush record:YES];
}

- (NSString *)actionName {
    return @"Create Brush";
}

@end
