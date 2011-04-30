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

- (void)applyTextureFrom:(id <Face>)source to:(id <Face>)destination;
- (void)applyFlagsFrom:(id <Face>)source to:(id <Face>)destination;

@end

@implementation FaceTool (private)

- (BOOL)isApplyTextureAndFlagsModifierPressed {
    return [NSEvent modifierFlags] == (NSAlternateKeyMask | NSCommandKeyMask);
}

- (BOOL)isApplyTextureModifierPressed {
    return [NSEvent modifierFlags] == NSAlternateKeyMask;
}

- (void)applyTextureFrom:(id <Face>)source to:(id <Face>)destination {
    MapDocument* map = [windowController document];
    [map setFace:destination texture:[source texture]];
}

- (void)applyFlagsFrom:(id <Face>)source to:(id <Face>)destination {
    MapDocument* map = [windowController document];
    [map setFace:destination xOffset:[source xOffset]];
    [map setFace:destination yOffset:[source yOffset]];
    [map setFace:destination xScale:[source xScale]];
    [map setFace:destination yScale:[source yScale]];
    [map setFace:destination rotation:[source rotation]];
}

@end

@implementation FaceTool

- (id)initWithController:(MapWindowController *)theWindowController {
    if (self = [self init]) {
        windowController = [theWindowController retain];
        dragFaceCursor = [[DragFaceCursor alloc] init];
        applyFaceCursor = [[ApplyFaceCursor alloc] init];
    }
    
    return self;
}

- (void)dealloc {
    [dragFaceCursor release];
    [applyFaceCursor release];
    [windowController release];
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
    
    id <Face> source = [[selectedFaces objectEnumerator] nextObject];
    
    PickingHit* hit = [hits firstHitOfType:HT_FACE ignoreOccluders:YES];
    id <Face> destination = [hit object];
    
    if ([event clickCount] == 1) {
        if ([self isApplyTextureAndFlagsModifierPressed]) {
            MapDocument* map = [windowController document];
            NSUndoManager* undoManager = [map undoManager];
            [undoManager beginUndoGrouping];
            [self applyTextureFrom:source to:destination];
            [self applyFlagsFrom:source to:destination];
            [undoManager endUndoGrouping];
            [undoManager setActionName:@"Copy Face Attributes"];
        } else {
            [self applyTextureFrom:source to:destination];
        }
    } else if ([event clickCount] == 2) {
        MapDocument* map = [windowController document];
        NSUndoManager* undoManager = [map undoManager];
        [undoManager beginUndoGrouping];
        if ([self isApplyTextureAndFlagsModifierPressed]) {
            id <Face> face = destination;
            id <Brush> brush = [face brush];
            NSEnumerator* faceEn = [[brush faces] objectEnumerator];
            while (destination = [faceEn nextObject]) {
                if (destination != face) {
                    [self applyTextureFrom:source to:destination];
                    [self applyFlagsFrom:source to:destination];
                }
            }
            [undoManager endUndoGrouping];
            [undoManager setActionName:@"Copy Face Attributes To Brush"];
        } else {
            id <Face> face = destination;
            id <Brush> brush = [face brush];
            NSEnumerator* faceEn = [[brush faces] objectEnumerator];
            while (destination = [faceEn nextObject]) {
                if (destination != face) {
                    [self applyTextureFrom:source to:destination];
                }
            }
            [undoManager endUndoGrouping];
            [undoManager setActionName:@"Copy Face Texture To Brush"];
        }
    }
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
    
    if (equalV3f(&point, &point))
        return;

    TVector3f diff;
    subV3f(&point, &lastPoint, &diff);
    dist = dotV3f(&diff, &dragDir);
    
    MapDocument* map = [windowController document];
    
    SelectionManager* selectionManager = [windowController selectionManager];
    NSEnumerator* faceEn = [[selectionManager selectedFaces] objectEnumerator];
    id <Face> face;
    while ((face = [faceEn nextObject]))
        [map dragFace:face dist:dist];

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
