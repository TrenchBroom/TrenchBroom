//
//  CreateBrushTool.m
//  TrenchBroom
//
//  Created by Kristian Duske on 05.08.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

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

    plane.norm = *closestAxisV3f(cameraDir);
    if (plane.norm.x > 0 || plane.norm.y > 0 || plane.norm.z > 0)
        plane.point = initialBounds.min;
    else
        plane.point = initialBounds.max;


    MapDocument* map = [windowController document];
    TBoundingBox* worldBounds = [map worldBounds];
    if (!boundsContainBounds(worldBounds, &initialBounds))
        return;
    
    NSUndoManager* undoManager = [map undoManager];
    [undoManager setGroupsByEvent:NO];
    [undoManager beginUndoGrouping];
    
    SelectionManager* selectionManager = [windowController selectionManager];
    NSArray* textureMRU = [selectionManager textureMRU];
    NSString* texture = [textureMRU count] > 0 ? [textureMRU lastObject] : @"none";

    brush = [map createBrushInEntity:[map worldspawn:YES] withBounds:&initialBounds texture:texture];
    [selectionManager addBrush:brush record:YES];
    
    drag = YES;
    NSLog(@"start create brush");
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
    
    TBoundingBox bounds = initialBounds;
    mergeBoundsWithPoint(&bounds, &point, &bounds);
    
    MapDocument* map = [windowController document];
    TBoundingBox* worldBounds = [map worldBounds];
    if (!boundsContainBounds(worldBounds, &bounds))
        return;
    
    lastPoint = point;

    NSUndoManager* undoManager = [map undoManager];
    [undoManager endUndoGrouping];
    [undoManager undo];
    [undoManager beginUndoGrouping];
    
    SelectionManager* selectionManager = [windowController selectionManager];
    NSArray* textureMRU = [selectionManager textureMRU];
    NSString* texture = [textureMRU count] > 0 ? [textureMRU lastObject] : @"none";
    
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
    
    brush = nil;
    drag = NO;
    NSLog(@"end create brush");
}

- (void)leftScroll:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {
    if (!drag)
        return;
    
    Grid* grid = [[windowController options] grid];
    float delta = [grid actualSize] * ([event deltaY] / fabsf([event deltaY]));
    
    if (equalV3f(&plane.norm, &XAxisPos)) {
        initialBounds.max.x = fmaxf(initialBounds.max.x + delta, initialBounds.min.x + [grid actualSize]);
    } else if (equalV3f(&plane.norm, &XAxisNeg)) {
        initialBounds.min.x = fminf(initialBounds.min.x - delta, initialBounds.max.x - [grid actualSize]);
    } else if (equalV3f(&plane.norm, &YAxisPos)) {
        initialBounds.max.y = fmaxf(initialBounds.max.y + delta, initialBounds.min.y + [grid actualSize]);
    } else if (equalV3f(&plane.norm, &YAxisNeg)) {
        initialBounds.min.y = fminf(initialBounds.min.y - delta, initialBounds.max.y - [grid actualSize]);
    } else if (equalV3f(&plane.norm, &ZAxisPos)) {
        initialBounds.min.z = fmaxf(initialBounds.min.z + delta, initialBounds.min.z + [grid actualSize]);
    } else if (equalV3f(&plane.norm, &ZAxisNeg)) {
        initialBounds.min.z = fminf(initialBounds.min.z - delta, initialBounds.max.z - [grid actualSize]);
    }

    TBoundingBox bounds = initialBounds;
    mergeBoundsWithPoint(&bounds, &lastPoint, &bounds);

    MapDocument* map = [windowController document];
    TBoundingBox* worldBounds = [map worldBounds];
    if (!boundsContainBounds(worldBounds, &bounds))
        return;
    
    NSUndoManager* undoManager = [map undoManager];
    [undoManager endUndoGrouping];
    [undoManager undo];
    [undoManager beginUndoGrouping];
    
    SelectionManager* selectionManager = [windowController selectionManager];
    NSArray* textureMRU = [selectionManager textureMRU];
    NSString* texture = [textureMRU count] > 0 ? [textureMRU lastObject] : @"none";
    
    brush = [map createBrushInEntity:[map worldspawn:YES] withBounds:&bounds texture:texture];
    [selectionManager addBrush:brush record:YES];
}

- (NSString *)actionName {
    return @"Create Brush";
}

@end
