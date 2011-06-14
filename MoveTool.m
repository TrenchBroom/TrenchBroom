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

@interface MoveTool (private)

- (BOOL)isAltPlaneModifierPressed;
- (void)actualPlaneNormal:(TVector3f *)norm result:(TVector3f *)result;

@end

@implementation MoveTool (private)

- (BOOL)isAltPlaneModifierPressed {
    return [NSEvent modifierFlags] == NSAlternateKeyMask;
}

- (void)actualPlaneNormal:(TVector3f *)norm result:(TVector3f *)result {
    switch (largestComponentV3f(norm)) {
        case A_X:
            if ([self isAltPlaneModifierPressed])
                *result = YAxisPos;
            else
                *result = XAxisPos;
            break;
        case A_Y:
            if ([self isAltPlaneModifierPressed])
                *result = XAxisPos;
            else
                *result = YAxisPos;
            break;
        default:
            if ([self isAltPlaneModifierPressed]) {
                Camera* camera = [windowController camera];
                const TVector3f* cameraDir = [camera direction];
                if (fabsf(cameraDir->x) > fabsf(cameraDir->y))
                    *result = XAxisPos;
                else
                    *result = YAxisPos;
            } else {
                *result = ZAxisPos;
            }
    }
}

@end

@implementation MoveTool

- (id)initWithController:(MapWindowController *)theWindowController {
    if ((self = [self init])) {
        windowController = theWindowController;
        cursor = [[MoveCursor alloc] init];
    }
    return self;
}

- (void)dealloc {
    [cursor release];
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
    [grid snapToGrid:&lastPoint result:&lastPoint];
    drag = YES;
    
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
    [grid snapToGrid:&point result:&point];
    
    if (equalV3f(&point, &lastPoint))
        return;
    
    int x = roundf(point.x - lastPoint.x);
    int y = roundf(point.y - lastPoint.y);
    int z = roundf(point.z - lastPoint.z);
    
    MapDocument* map = [windowController document];
    
    SelectionManager* selectionManager = [windowController selectionManager];
    NSEnumerator* brushEn = [[selectionManager selectedBrushes] objectEnumerator];
    id <Brush> brush;
    while ((brush = [brushEn nextObject]))
        [map translateBrush:brush 
                     xDelta:x
                     yDelta:y
                     zDelta:z];
    
    NSEnumerator* entityEn = [[selectionManager selectedEntities] objectEnumerator];
    id <Entity> entity;
    while ((entity = [entityEn nextObject]))
        [map translateEntity:entity
                     xDelta:x
                     yDelta:y
                     zDelta:z];
    
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
    [cursorManager pushCursor:cursor];

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
                    [cursor setPlaneNormal:largestComponentV3f(&norm)];
                    break;
                }
                case HT_FACE: {
                    id <Face> face = [hit object];
                    TVector3f norm;
                    [self actualPlaneNormal:[face norm] result:&norm];
                    [cursor setPlaneNormal:largestComponentV3f(&norm)];
                    break;
                }
            }
        }
        
        [cursor update:[hit hitPoint]];
    } else {
        float dist = intersectPlaneWithRay(&plane, ray);
        TVector3f position;
        rayPointAtDistance(ray, dist, &position);
        [cursor update:&position];
    }
}

- (NSString *)actionName {
    return @"Move Objects";
}

@end
