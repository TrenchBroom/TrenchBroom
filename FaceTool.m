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

@interface FaceTool (private)

- (BOOL)isFrontFaceModifierPressed;
- (BOOL)isFaceDragModifierPressed;
- (BOOL)isApplyTextureModifierPressed;
- (BOOL)isApplyFlagsModifierPressed;

@end

@implementation FaceTool (private)

- (BOOL)isFrontFaceModifierPressed {
    return [NSEvent modifierFlags] == NSCommandKeyMask;
}

- (BOOL)isFaceDragModifierPressed {
    return ([NSEvent modifierFlags] & NSCommandKeyMask) == NSCommandKeyMask;
}

- (BOOL)isApplyTextureModifierPressed {
    return ([NSEvent modifierFlags] & NSAlternateKeyMask) == NSAlternateKeyMask;
}

- (BOOL)isApplyFlagsModifierPressed {
    return ([NSEvent modifierFlags] & NSCommandKeyMask) == NSCommandKeyMask;
}

@end

@implementation FaceTool

- (id)init {
    if ((self = [super init])) {
        dragFaces = [[NSMutableArray alloc] init];
    }
    
    return self;
}

- (void)dealloc {
    [dragFaces release];
    [super dealloc];
}

# pragma mark -
# pragma mark @implementation Tool

- (BOOL)leftMouseUp:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {
    if (![self isApplyTextureModifierPressed])
        return NO;

    SelectionManager* selectionManager = [windowController selectionManager];
    if ([selectionManager mode] != SM_FACES)
        return NO;

    PickingHit* hit = [hits firstHitOfType:HT_FACE ignoreOccluders:NO];
    if (hit == nil)
        return NO;
    
    id <Face> source = [[selectionManager selectedFaces] lastObject];
    id <Face> target = [hit object];

    NSMutableArray* targetArray = [[NSMutableArray alloc] init];
    if ([event clickCount] == 2)
        [targetArray addObjectsFromArray:[[target brush] faces]];
    else
        [targetArray addObject:target];
    
    MapDocument* map = [windowController document];
    NSUndoManager* undoManager = [map undoManager];
    [undoManager beginUndoGrouping];

    [selectionManager removeAll:YES];
    [selectionManager addFaces:targetArray record:YES];
    
    [map setFaces:targetArray texture:[source texture]];
    if ([self isApplyFlagsModifierPressed]) {
        [map setFaces:targetArray xOffset:[source xOffset]];
        [map setFaces:targetArray yOffset:[source yOffset]];
        [map setFaces:targetArray xScale:[source xScale]];
        [map setFaces:targetArray yScale:[source yScale]];
        [map setFaces:targetArray rotation:[source rotation]];
    }

    [undoManager setActionName:@"Apply Texture"];
    [undoManager endUndoGrouping];
    
    return YES;
}

- (BOOL)beginLeftDrag:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {
    if (![self isFaceDragModifierPressed])
        return NO;
    
    SelectionManager* selectionManager = [windowController selectionManager];

    PickingHit* hit = [hits firstHitOfType:HT_CLOSE_FACE ignoreOccluders:NO];
    if (hit != nil) {
        id <Face> face = [hit object];
        id <Brush> brush = [face brush];
        if (![face selected] && ![brush selected])
            return NO;

        referenceFace = [hit object];
        [dragFaces addObject:referenceFace];
    } else {
        hit = [hits firstHitOfType:HT_FACE ignoreOccluders:NO];
        if (hit == nil)
            return NO;

        id <Face> face = [hit object];
        id <Brush> brush = [face brush];
        if (![face selected] && ![brush selected])
            return NO;

        referenceFace = [hit object];
        if (![referenceFace selected])
            return NO;

        if ([selectionManager mode] == SM_FACES) {
            [dragFaces addObjectsFromArray:[selectionManager selectedFaces]];
        } else {
            [dragFaces addObject:referenceFace];
        }
    }
    
    if ([selectionManager mode] == SM_BRUSHES || [selectionManager mode] == SM_BRUSHES_ENTITIES) {
        const TPlane* boundary = [referenceFace boundary];
        for (id <Brush> brush in [selectionManager selectedBrushes])
            for (id <Face> face in [brush faces])
                if (face != referenceFace && equalV3f([referenceFace norm], [face norm]) && pointStatusFromPlane(boundary, &[face boundary]->point) == PS_INSIDE)
                    [dragFaces addObject:face];
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
    return YES;
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
    float dragDist = [grid dragDeltaForFace:referenceFace delta:&delta];

    if (isnan(dragDist) || dragDist == 0)
        return;
    
    Options* options = [windowController options];
    MapDocument* map = [windowController document];

    if ([map dragFaces:dragFaces distance:dragDist lockTextures:[options lockTextures]])
        addV3f(&lastPoint, &delta, &lastPoint);
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

- (NSString *)actionName {
    return @"Drag Faces";
}

@end
