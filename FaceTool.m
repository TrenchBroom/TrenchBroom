/*
Copyright (C) 2010-2011 Kristian Duske

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

#import "FaceTool.h"
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
#import "math.h"
#import "ControllerUtils.h"
#import "MutableFace.h"
#import "DragFaceCursor.h"
#import "CursorManager.h"

@interface FaceTool (private)

- (BOOL)isApplyTextureAndFlagsModifierPressed;
- (BOOL)isApplyTextureModifierPressed;
- (BOOL)isFrontFaceModifierPressed;

- (void)applyTextureFrom:(id <Face>)source toFace:(id <Face>)destination;
- (void)applyFlagsFrom:(id <Face>)source toFace:(id <Face>)destination;

@end

@implementation FaceTool (private)

- (BOOL)isApplyTextureAndFlagsModifierPressed {
    return [NSEvent modifierFlags] == (NSAlternateKeyMask | NSCommandKeyMask);
}

- (BOOL)isApplyTextureModifierPressed {
    return [NSEvent modifierFlags] == NSAlternateKeyMask;
}

- (BOOL)isFrontFaceModifierPressed {
    return [NSEvent modifierFlags] == NSCommandKeyMask;
}

- (void)applyTextureFrom:(id <Face>)source toFace:(id <Face>)destination {
    MapDocument* map = [windowController document];

    NSArray* faceArray = [[NSArray alloc] initWithObjects:destination, nil];
    [map setFaces:faceArray texture:[source texture]];
    [faceArray release];
}

- (void)applyFlagsFrom:(id <Face>)source toFace:(id <Face>)destination {
    MapDocument* map = [windowController document];

    NSArray* faceArray = [[NSArray alloc] initWithObjects:destination, nil];
    [map setFaces:faceArray xOffset:[source xOffset]];
    [map setFaces:faceArray yOffset:[source yOffset]];
    [map setFaces:faceArray xScale:[source xScale]];
    [map setFaces:faceArray yScale:[source yScale]];
    [map setFaces:faceArray rotation:[source rotation]];
    [faceArray release];
}

@end

@implementation FaceTool

- (id)initWithWindowController:(MapWindowController *)theWindowController {
    if ((self = [self init])) {
        windowController = theWindowController;
        dragFaces = [[NSMutableArray alloc] init];
        dragFaceCursor = [[DragFaceCursor alloc] init];
    }
    
    return self;
}

- (void)dealloc {
    [dragFaces release];
    [dragFaceCursor release];
    [super dealloc];
}

# pragma mark -
# pragma mark @implementation Tool

- (void)handleLeftMouseDown:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {
    if (![self isApplyTextureModifierPressed] && ![self isApplyTextureAndFlagsModifierPressed])
        return;
    
    SelectionManager* selectionManager = [windowController selectionManager];
    NSArray* selectedFaces = [selectionManager selectedFaces];
    if ([selectedFaces count] != 1)
        return;
    
    PickingHit* hit = [hits firstHitOfType:HT_FACE ignoreOccluders:YES];
    id <Face> target = [hit object];
    NSMutableArray* targetArray = [[NSMutableArray alloc] init];
    
    if ([event clickCount] == 1) {
        [targetArray addObject:target];
    } else if ([event clickCount] == 2) {
        id <Brush> brush = [target brush];
        [targetArray addObjectsFromArray:[brush faces]];
    }

    if ([targetArray count] > 0) {
        MapDocument* map = [windowController document];
        NSUndoManager* undoManager = [map undoManager];
        [undoManager beginUndoGrouping];
        
        id <Face> source = [[selectedFaces objectEnumerator] nextObject];
        [map setFaces:targetArray texture:[source texture]];
        if ([self isApplyTextureAndFlagsModifierPressed]) {
            [map setFaces:targetArray xOffset:[source xOffset]];
            [map setFaces:targetArray yOffset:[source yOffset]];
            [map setFaces:targetArray xScale:[source xScale]];
            [map setFaces:targetArray yScale:[source yScale]];
            [map setFaces:targetArray rotation:[source rotation]];
        }
        
        [undoManager setActionName:@"Copy Face"];
        [undoManager endUndoGrouping];
    }
            
    [targetArray release];
}

- (void)beginLeftDrag:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {
    SelectionManager* selectionManager = [windowController selectionManager];

    PickingHit* hit = [hits edgeDragHit];
    if (hit != nil) {
        referenceFace = [hit object];
        [dragFaces addObject:referenceFace];
    } else {
        hit = [hits firstHitOfType:HT_FACE ignoreOccluders:YES];
        if (hit == nil)
            return;
        
        referenceFace = [hit object];
        if (![selectionManager isFaceSelected:referenceFace])
            return;

        if ([selectionManager mode] == SM_FACES) {
            [dragFaces addObjectsFromArray:[selectionManager selectedFaces]];
        } else {
            [dragFaces addObject:referenceFace];
        }
    }
    
    if ([selectionManager mode] == SM_BRUSHES || [selectionManager mode] == SM_BRUSHES_ENTITIES) {
        const TPlane* boundary = [referenceFace boundary];
        
        NSEnumerator* brushEn = [[selectionManager selectedBrushes] objectEnumerator];
        id <Brush> brush;
        while ((brush = [brushEn nextObject])) {
            NSEnumerator* faceEn = [[brush faces] objectEnumerator];
            id <Face> face;
            while ((face = [faceEn nextObject])) {
                if (face != referenceFace && equalV3f([referenceFace norm], [face norm]) && pointStatusFromPlane(boundary, &[face boundary]->point) == PS_INSIDE)
                    [dragFaces addObject:face];
            }
        }
    }
    
    MapDocument* map = [windowController document];
    NSUndoManager* undoManager = [map undoManager];
    [undoManager setGroupsByEvent:NO];
    [undoManager beginUndoGrouping];
    
    dragDir = *[referenceFace norm];
    lastPoint = *[hit hitPoint];
    plane.point = lastPoint;

    crossV3f(&dragDir, &ray->direction, &plane.norm);
    crossV3f(&plane.norm, &dragDir, &plane.norm);
    normalizeV3f(&plane.norm, &plane.norm);
    
    drag = YES;
}

- (void)leftDrag:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {
    if (!drag)
        return;
    
    float dist = intersectPlaneWithRay(&plane, ray);
    if (isnan(dist))
        return;
    
    TVector3f point, delta;
    rayPointAtDistance(ray, dist, &point);
    subV3f(&point, &lastPoint, &delta);
    
    Grid* grid = [[windowController options] grid];
    float dragDist = calculateDragDelta(grid, referenceFace, NULL, &delta);

    if (isnan(dragDist) || dragDist == 0)
        return;

    Options* options = [windowController options];
    MapDocument* map = [windowController document];

    if ([map dragFaces:dragFaces distance:dragDist lockTextures:[options lockTextures]])
        lastPoint = point;
}

- (void)endLeftDrag:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {
    if (!drag)
        return;
    
    MapDocument* map = [windowController document];
    NSUndoManager* undoManager = [map undoManager];
    [undoManager setActionName:[self actionName]];
    [undoManager endUndoGrouping];
    [undoManager setGroupsByEvent:YES];

    drag = NO;
    referenceFace = nil;
    [dragFaces removeAllObjects];
}

- (void)setCursor:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {
    CursorManager* cursorManager = [windowController cursorManager];
    [cursorManager pushCursor:dragFaceCursor];
    [self updateCursor:event ray:ray hits:hits];
}

- (void)unsetCursor:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {
    CursorManager* cursorManager = [windowController cursorManager];
    [cursorManager popCursor];
}

- (void)updateCursor:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {
    TVector3f position;
    const TVector3f* dragDirection = NULL;
    
    if (!drag) {
        PickingHit* hit = [hits edgeDragHit];
        if (hit == nil)
            hit = [hits firstHitOfType:HT_FACE ignoreOccluders:YES];
        if (hit == nil)
            return;

        position = *[hit hitPoint];
        dragDirection = [[hit object] norm];
    } else {
        float dist = intersectPlaneWithRay(&plane, ray);
        if (isnan(dist))
            return;
        
        rayPointAtDistance(ray, dist, &position);
        dragDirection = [referenceFace norm];
    }
    
    Grid* grid = [[windowController options] grid];
    [dragFaceCursor setArrowLength:[grid actualSize]];
    [dragFaceCursor setPosition:&position];
    [dragFaceCursor setRayDirection:&ray->direction];
    [dragFaceCursor setDragDirection:dragDirection];
}

- (NSString *)actionName {
    return @"Drag Faces";
}

@end
