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
#import "CoordinatePlane.h"
#import "Ray3D.h"
#import "Plane3D.h"
#import "Vector3f.h"
#import "Vector3i.h"
#import "PickingHit.h"
#import "PickingHitList.h"
#import "Face.h"
#import "Brush.h"
#import "math.h"
#import "Math.h"
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
    [lastPoint release];
    [dragDir release];
    [plane release];
    [windowController release];
    [super dealloc];
}

# pragma mark -
# pragma mark @implementation Tool

- (void)handleLeftMouseDown:(NSEvent *)event ray:(Ray3D *)ray hits:(PickingHitList *)hits {
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

- (void)beginLeftDrag:(NSEvent *)event ray:(Ray3D *)ray hits:(PickingHitList *)hits {
    PickingHit* hit = [hits firstHitOfType:HT_FACE ignoreOccluders:NO];
    if (hit == nil)
        return;
    
    id <Face> face = [hit object];

    SelectionManager* selectionManager = [windowController selectionManager];
    if (![selectionManager isFaceSelected:face])
        return;
    
    dragDir = [[face norm] retain];
    
    Vector3f* planeNorm = [[Vector3f alloc] initWithFloatVector:dragDir];
    [planeNorm cross:[ray direction]];
    [planeNorm cross:dragDir];
    [planeNorm normalize];
    
    lastPoint = [[hit hitPoint] retain];
    plane = [[Plane3D alloc] initWithPoint:lastPoint norm:planeNorm];
    [planeNorm release];
    
    Grid* grid = [[windowController options] grid];
    [grid snapToGrid:lastPoint];

    drag = YES;
    
    MapDocument* map = [windowController document];
    NSUndoManager* undoManager = [map undoManager];
    [undoManager setGroupsByEvent:NO];
    [undoManager beginUndoGrouping];
}

- (void)leftDrag:(NSEvent *)event ray:(Ray3D *)ray hits:(PickingHitList *)hits {
    Vector3f* point = [ray pointAtDistance:[plane intersectWithRay:ray]];
    if (point == nil)
        return;
    
    Grid* grid = [[windowController options] grid];
    [grid snapToGrid:point];
    
    if ([point isEqualToVector:lastPoint])
        return;
    
    Vector3f* diff = [[Vector3f alloc] initWithFloatVector:point];
    [diff sub:lastPoint];
    float dist = [diff dot:dragDir];
    
    MapDocument* map = [windowController document];
    
    SelectionManager* selectionManager = [windowController selectionManager];
    NSEnumerator* faceEn = [[selectionManager selectedFaces] objectEnumerator];
    id <Face> face;
    while ((face = [faceEn nextObject]))
        [map dragFace:face dist:dist];
    
    [diff release];
    [lastPoint release];
    lastPoint = [point retain];
}

- (void)endLeftDrag:(NSEvent *)event ray:(Ray3D *)ray hits:(PickingHitList *)hits {
    MapDocument* map = [windowController document];
    NSUndoManager* undoManager = [map undoManager];
    [undoManager setActionName:[self actionName]];
    [undoManager endUndoGrouping];
    [undoManager setGroupsByEvent:YES];
    
    [plane release];
    plane = nil;
    [dragDir release];
    dragDir = nil;
    [lastPoint release];
    lastPoint = nil;
    drag = NO;
}

- (BOOL)isCursorOwner:(NSEvent *)event ray:(Ray3D *)ray hits:(PickingHitList *)hits {
    SelectionManager* selectionManager = [windowController selectionManager];
    if ([selectionManager mode] != SM_FACES)
        return NO;
    
    PickingHit* hit = [hits firstHitOfType:HT_FACE ignoreOccluders:YES];
    if (hit == nil)
        return NO;
    
    id <Face> face = [hit object];
    if ([selectionManager isFaceSelected:face])
        return YES;
    
    NSSet* selectedFaces = [selectionManager selectedFaces];
    if ([selectedFaces count] == 1 && ([self isApplyTextureModifierPressed] || [self isApplyTextureAndFlagsModifierPressed]))
        return YES;
        
    return NO;
}

- (void)setCursor:(NSEvent *)event ray:(Ray3D *)ray hits:(PickingHitList *)hits {
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

- (void)unsetCursor:(NSEvent *)event ray:(Ray3D *)ray hits:(PickingHitList *)hits {
    CursorManager* cursorManager = [windowController cursorManager];
    [cursorManager popCursor];
    currentCursor = nil;
}

- (void)updateCursor:(NSEvent *)event ray:(Ray3D *)ray hits:(PickingHitList *)hits {
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
        Vector3f* position = [ray pointAtDistance:[plane intersectWithRay:ray]];
        [currentCursor update:position];
    }
}

- (NSString *)actionName {
    return @"Move Faces";
}

@end
