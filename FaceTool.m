//
//  FaceTool.m
//  TrenchBroom
//
//  Created by Kristian Duske on 27.03.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

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
#import "CursorManager.h"
#import "DragFaceCursor.h"
#import "ApplyFaceCursor.h"
#import "Cursor.h"

@interface FaceTool (private)

- (BOOL)isApplyTextureAndFlagsModifierPressed;
- (BOOL)isApplyTextureModifierPressed;

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

- (void)applyTextureFrom:(id <Face>)source toFace:(id <Face>)destination {
    MapDocument* map = [windowController document];

    NSSet* faceSet = [[NSSet alloc] initWithObjects:destination, nil];
    [map setFaces:faceSet texture:[source texture]];
    [faceSet release];
}

- (void)applyFlagsFrom:(id <Face>)source toFace:(id <Face>)destination {
    MapDocument* map = [windowController document];

    NSSet* faceSet = [[NSSet alloc] initWithObjects:destination, nil];
    [map setFaces:faceSet xOffset:[source xOffset]];
    [map setFaces:faceSet yOffset:[source yOffset]];
    [map setFaces:faceSet xScale:[source xScale]];
    [map setFaces:faceSet yScale:[source yScale]];
    [map setFaces:faceSet rotation:[source rotation]];
    [faceSet release];
}

@end

@implementation FaceTool

- (id)initWithWindowController:(MapWindowController *)theWindowController {
    if ((self = [self init])) {
        windowController = theWindowController;
        dragFaceCursor = [[DragFaceCursor alloc] init];
        applyFaceCursor = [[ApplyFaceCursor alloc] init];
    }
    
    return self;
}

- (void)dealloc {
    [dragFaceCursor release];
    [applyFaceCursor release];
    [super dealloc];
}

# pragma mark -
# pragma mark @implementation Tool

- (void)handleLeftMouseDown:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {
    if (![self isApplyTextureModifierPressed] && ![self isApplyTextureAndFlagsModifierPressed])
        return;
    
    SelectionManager* selectionManager = [windowController selectionManager];
    NSSet* selectedFaces = [selectionManager selectedFaces];
    if (![selectedFaces count] == 1)
        return;
    
    PickingHit* hit = [hits firstHitOfType:HT_FACE ignoreOccluders:YES];
    id <Face> target = [hit object];
    NSMutableSet* targetSet = [[NSMutableSet alloc] init];
    
    if ([event clickCount] == 1) {
        [targetSet addObject:target];
    } else if ([event clickCount] == 2) {
        id <Brush> brush = [target brush];
        [targetSet addObjectsFromArray:[brush faces]];
    }

    if ([targetSet count] > 0) {
        MapDocument* map = [windowController document];
        NSUndoManager* undoManager = [map undoManager];
        [undoManager beginUndoGrouping];
        
        id <Face> source = [[selectedFaces objectEnumerator] nextObject];
        [map setFaces:targetSet texture:[source texture]];
        if ([self isApplyTextureAndFlagsModifierPressed]) {
            [map setFaces:targetSet xOffset:[source xOffset]];
            [map setFaces:targetSet yOffset:[source yOffset]];
            [map setFaces:targetSet xScale:[source xScale]];
            [map setFaces:targetSet yScale:[source yScale]];
            [map setFaces:targetSet rotation:[source rotation]];
        }
        
        [undoManager setActionName:@"Copy Face"];
        [undoManager endUndoGrouping];
    }
            
    [targetSet release];
}

- (void)beginLeftDrag:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {
    PickingHit* hit = [hits firstHitOfType:HT_FACE ignoreOccluders:NO];
    if (hit == nil)
        return;
    
    id <Face> face = [hit object];

    SelectionManager* selectionManager = [windowController selectionManager];
    if (![selectionManager isFaceSelected:face])
        return;
    
    dragDir = *[face norm];
    lastPoint = *[hit hitPoint];
    plane.point = lastPoint;
    
    crossV3f(&dragDir, &ray->direction, &plane.norm);
    crossV3f(&plane.norm, &dragDir, &plane.norm);
    normalizeV3f(&plane.norm, &plane.norm);
    
    Grid* grid = [[windowController options] grid];
    [grid snapToGrid:&lastPoint result:&lastPoint];

    drag = YES;
    
    MapDocument* map = [windowController document];
    NSUndoManager* undoManager = [map undoManager];
    [undoManager setGroupsByEvent:NO];
    [undoManager beginUndoGrouping];
}

- (void)leftDrag:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {
    float dist = intersectPlaneWithRay(&plane, ray);
    if (isnan(dist))
        return;
    
    TVector3f point;
    rayPointAtDistance(ray, dist, &point);
    
    Grid* grid = [[windowController options] grid];
    [grid snapToGrid:&point result:&point];
    
    if (equalV3f(&point, &lastPoint))
        return;

    TVector3f diff;
    subV3f(&point, &lastPoint, &diff);
    dist = dotV3f(&diff, &dragDir);
    
    MapDocument* map = [windowController document];
    SelectionManager* selectionManager = [windowController selectionManager];

    [map dragFaces:[selectionManager selectedFaces] distance:dist];
    lastPoint = point;
}

- (void)endLeftDrag:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {
    MapDocument* map = [windowController document];
    NSUndoManager* undoManager = [map undoManager];
    [undoManager setActionName:[self actionName]];
    [undoManager endUndoGrouping];
    [undoManager setGroupsByEvent:YES];

    drag = NO;
}

- (void)setCursor:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {
    PickingHit* hit = [hits firstHitOfType:HT_FACE ignoreOccluders:YES];
    id <Face> face = [hit object];

    SelectionManager* selectionManager = [windowController selectionManager];
    if ([selectionManager isFaceSelected:face]) {
        CursorManager* cursorManager = [windowController cursorManager];
        [cursorManager pushCursor:dragFaceCursor];
        [dragFaceCursor setDragDir:[face norm]];
        currentCursor = dragFaceCursor;
    } else if ([[selectionManager selectedFaces] count] == 1) {
        CursorManager* cursorManager = [windowController cursorManager];
        [cursorManager pushCursor:applyFaceCursor];
        [applyFaceCursor setFace:face];
        [applyFaceCursor setApplyFlags:[self isApplyTextureAndFlagsModifierPressed]];
        currentCursor = applyFaceCursor;
    }
}

- (void)unsetCursor:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {
    CursorManager* cursorManager = [windowController cursorManager];
    [cursorManager popCursor];
    currentCursor = nil;
}

- (void)updateCursor:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {
    if (!drag) {
        PickingHit* hit = [hits firstHitOfType:HT_FACE ignoreOccluders:YES];
        id <Face> face = [hit object];
        
        CursorManager* cursorManager = [windowController cursorManager];
        SelectionManager* selectionManager = [windowController selectionManager];
        if ([selectionManager isFaceSelected:face]) {
            if (currentCursor != dragFaceCursor) {
                [cursorManager popCursor];
                [cursorManager pushCursor:dragFaceCursor];
                currentCursor = dragFaceCursor;
            }
            [dragFaceCursor setDragDir:[face norm]];
        } else if ([[selectionManager selectedFaces] count] == 1) {
            if (currentCursor != applyFaceCursor) {
                [cursorManager popCursor];
                [cursorManager pushCursor:applyFaceCursor];
                currentCursor = applyFaceCursor;
            }
            [applyFaceCursor setFace:face];
            [applyFaceCursor setApplyFlags:[self isApplyTextureAndFlagsModifierPressed]];
        }
        
        [currentCursor update:[hit hitPoint]];
    } else {
        TVector3f position;
        float dist = intersectPlaneWithRay(&plane, ray);
        rayPointAtDistance(ray, dist, &position);
        [currentCursor update:&position];
    }
}

- (NSString *)actionName {
    return @"Move Faces";
}

@end
