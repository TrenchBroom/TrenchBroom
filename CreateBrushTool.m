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
    }
    
    return self;
}

- (void)beginLeftDrag:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {
    PickingHit* hit = [hits firstHitOfType:HT_FACE ignoreOccluders:NO];
    if (hit != nil) {
        lastPoint = *[hit hitPoint];
    } else {
        Camera* camera = [windowController camera];
        lastPoint = [camera defaultPointOnRay:ray];
    }

    Grid* grid = [[windowController options] grid];
    [grid snapDownToGridV3f:&lastPoint result:&initialBounds.min];
    [grid snapUpToGridV3f:&lastPoint result:&initialBounds.max];
    [grid snapToGridV3f:&lastPoint result:&lastPoint];
    
    if (initialBounds.min.x == initialBounds.max.x)
        initialBounds.max.x += [grid actualSize];
    if (initialBounds.min.y == initialBounds.max.y)
        initialBounds.max.y += [grid actualSize];
    if (initialBounds.min.z == initialBounds.max.z)
        initialBounds.max.z += [grid actualSize];

    switch (strongestComponentV3f(&ray->direction)) {
        case A_X:
            if (ray->direction.x > 0) {
                plane.point = initialBounds.min;
                plane.norm = XAxisPos;
            } else {
                plane.point = initialBounds.max;
                plane.norm = XAxisNeg;
            }
            break;
        case A_Y:
            if (ray->direction.y > 0) {
                plane.point = initialBounds.min;
                plane.norm = YAxisPos;
            } else {
                plane.point = initialBounds.max;
                plane.norm = YAxisNeg;
            }
            break;
        case A_Z:
            if (ray->direction.z > 0) {
                plane.point = initialBounds.min;
                plane.norm = ZAxisPos;
            } else {
                plane.point = initialBounds.max;
                plane.norm = ZAxisNeg;
            }
            break;
    }

    MapDocument* map = [windowController document];
    
    NSUndoManager* undoManager = [map undoManager];
    [undoManager setGroupsByEvent:NO];
    [undoManager beginUndoGrouping];
    
    brush = [map createBrushInEntity:[map worldspawn:YES] withBounds:&initialBounds texture:@"none"];
    
    SelectionManager* selectionManager = [windowController selectionManager];
    [selectionManager addBrush:brush record:YES];
}

- (void)leftDrag:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {
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
    lastPoint = point;
    
    MapDocument* map = [windowController document];

    NSUndoManager* undoManager = [map undoManager];
    [undoManager endUndoGrouping];
    [undoManager undo];
    [undoManager beginUndoGrouping];
    
    brush = [map createBrushInEntity:[map worldspawn:YES] withBounds:&bounds texture:@"none"];
    
    SelectionManager* selectionManager = [windowController selectionManager];
    [selectionManager addBrush:brush record:YES];
}

- (void)endLeftDrag:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {
    MapDocument* map = [windowController document];
    NSUndoManager* undoManager = [map undoManager];
    [undoManager setActionName:[self actionName]];
    [undoManager endUndoGrouping];
    [undoManager setGroupsByEvent:YES];
    
    brush = nil;
}

- (void)handleScrollWheel:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {
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
    
    NSUndoManager* undoManager = [map undoManager];
    [undoManager endUndoGrouping];
    [undoManager undo];
    [undoManager beginUndoGrouping];
    
    brush = [map createBrushInEntity:[map worldspawn:YES] withBounds:&bounds texture:@"none"];
    
    SelectionManager* selectionManager = [windowController selectionManager];
    [selectionManager addBrush:brush record:YES];
}

- (NSString *)actionName {
    return @"Create Brush";
}

@end
