//
//  BrushTool.m
//  TrenchBroom
//
//  Created by Kristian Duske on 12.03.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

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
#import "CursorManager.h"
#import "MoveCursor.h"
#import "RotateCursor.h"

@interface MoveTool (private)

- (void)actualPlaneNormal:(const TVector3f *)norm result:(TVector3f *)result;
- (BOOL)isDuplicateModifierPressed;

@end

@implementation MoveTool (private)

- (void)actualPlaneNormal:(const TVector3f *)norm result:(TVector3f *)result {
    switch (strongestComponentV3f(norm)) {
        case A_X:
            *result = XAxisPos;
            break;
        case A_Y:
            *result = YAxisPos;
            break;
        default:
            *result = ZAxisPos;
    }
}

- (BOOL)isDuplicateModifierPressed {
    return [NSEvent modifierFlags] == NSCommandKeyMask;
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
    [moveCursor release];
    [super dealloc];
}

# pragma mark -
# pragma mark @implementation Tool

- (void)beginLeftDrag:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {
    SelectionManager* selectionManager = [windowController selectionManager];
    
    PickingHit* hit = [hits firstHitOfType:HT_FACE ignoreOccluders:NO];
    if (hit != nil) {
        id <Face> face = [hit object];
        id <Brush> brush = [face brush];
        
        if (![selectionManager isBrushSelected:brush])
            return;
        
        lastPoint = *[hit hitPoint];
        
        plane.point = lastPoint;
        [self actualPlaneNormal:[face norm] result:&plane.norm];
    } else {
        hit = [hits firstHitOfType:HT_ENTITY ignoreOccluders:NO];
        if (hit != nil) {
            id <Entity> entity = [hit object];
            
            if (![selectionManager isEntitySelected:entity])
                return;
            
            lastPoint = *[hit hitPoint];

            plane.point = lastPoint;
            intersectBoundsWithRay([entity bounds], ray, &plane.norm);
            [self actualPlaneNormal:&plane.norm result:&plane.norm];
        } else {
            return;
        }
    }
    
    Grid* grid = [[windowController options] grid];
    [grid snapToGridV3f:&lastPoint result:&lastPoint];
    drag = YES;
    duplicate = [self isDuplicateModifierPressed];
    
    MapDocument* map = [windowController document];
    NSUndoManager* undoManager = [map undoManager];
    [undoManager setGroupsByEvent:NO];
    [undoManager beginUndoGrouping];
}

- (void)leftDrag:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {
    if (!drag)
        return;
    
    float dist = intersectPlaneWithRay(&plane, ray);
    if (isnan(dist))
        return;
    
    TVector3f point;
    rayPointAtDistance(ray, dist, &point);
    
    Grid* grid = [[windowController options] grid];
    [grid snapToGridV3f:&point result:&point];
    
    if (equalV3f(&point, &lastPoint))
        return;

    MapDocument* map = [windowController document];
    TBoundingBox* worldBounds = [map worldBounds];
    
    TVector3f deltaf;
    subV3f(&point, &lastPoint, &deltaf);
    
    SelectionManager* selectionManager = [windowController selectionManager];
    
    TBoundingBox bounds, translatedBounds;
    [selectionManager selectionBounds:&bounds];
    
    addV3f(&bounds.min, &deltaf, &translatedBounds.min);
    addV3f(&bounds.max, &deltaf, &translatedBounds.max);
    
    if (translatedBounds.max.x > worldBounds->max.x) {
        deltaf.x = worldBounds->max.x - bounds.max.x;
        deltaf.y = 0;
        deltaf.z = 0;
    } else if (translatedBounds.min.x < worldBounds->min.x) {
        deltaf.x = worldBounds->min.x - bounds.min.x;
        deltaf.y = 0;
        deltaf.z = 0;
    } else if (translatedBounds.max.y > worldBounds->max.y) {
        deltaf.x = 0;
        deltaf.y = worldBounds->max.y - bounds.max.y;
        deltaf.z = 0;
    } else if (translatedBounds.min.y < worldBounds->min.y) {
        deltaf.x = 0;
        deltaf.y = worldBounds->min.y - bounds.min.y;
        deltaf.z = 0;
    } else if (translatedBounds.max.z > worldBounds->max.z) {
        deltaf.x = 0;
        deltaf.y = 0;
        deltaf.z = worldBounds->max.z - bounds.max.z;
    } else if (translatedBounds.min.z < worldBounds->min.z) {
        deltaf.x = 0;
        deltaf.y = 0;
        deltaf.z = worldBounds->min.z - bounds.min.z;
    }
    
    TVector3i deltai;
    roundV3f(&deltaf, &deltai);
    
    if (duplicate) {
        NSMutableArray* newEntities = [[NSMutableArray alloc] init];
        NSMutableArray* newBrushes = [[NSMutableArray alloc] init];
        [map duplicateEntities:[selectionManager selectedEntities] newEntities:newEntities newBrushes:newBrushes];
        [map duplicateBrushes:[selectionManager selectedBrushes] newBrushes:newBrushes];
        
        [selectionManager removeAll:YES];
        [selectionManager addEntities:newEntities record:YES];
        [selectionManager addBrushes:newBrushes record:YES];
        
        [newEntities release];
        [newBrushes release];
        
        duplicate = NO;
    }
    
    [map translateBrushes:[selectionManager selectedBrushes] delta:deltai];
    [map translateEntities:[selectionManager selectedEntities] delta:deltai];
    
    if (boundsContainBounds(worldBounds, &translatedBounds))
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
    if (!drag) {
        PickingHit* hit = [hits firstHitOfType:HT_FACE | HT_ENTITY ignoreOccluders:YES];
        if (hit != nil) {
            switch ([hit type]) {
                case HT_ENTITY: {
                    id <Entity> entity = [hit object];
                    TVector3f norm;
                    intersectBoundsWithRay([entity bounds], ray, &norm);
                    [self actualPlaneNormal:&norm result:&norm];
                    [moveCursor setPlaneNormal:strongestComponentV3f(&norm)];
                    break;
                }
                case HT_FACE: {
                    id <Face> face = [hit object];
                    TVector3f norm;
                    [self actualPlaneNormal:[face norm] result:&norm];
                    [moveCursor setPlaneNormal:strongestComponentV3f(&norm)];
                    break;
                }
                default:
                    break;
            }
            [moveCursor update:[hit hitPoint]];
        }
    } else {
        float dist = intersectPlaneWithRay(&plane, ray);
        TVector3f position;
        rayPointAtDistance(ray, dist, &position);
        [moveCursor update:&position];
    }
}

- (NSString *)actionName {
    return @"Move Objects";
}

@end
