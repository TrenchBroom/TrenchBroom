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

@implementation CreateBrushTool

- (id)initWithWindowController:(MapWindowController *)theWindowController {
    if ((self = [self init])) {
        windowController = theWindowController;
    }
    
    return self;
}

- (void)beginLeftDrag:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {
    Camera* camera = [windowController camera];
    lastPoint = [camera defaultPointOnRay:ray];

    Grid* grid = [[windowController options] grid];
    [grid snapDownToGridV3f:&lastPoint result:&bounds.min];
    [grid snapUpToGridV3f:&lastPoint result:&bounds.max];
    [grid snapToGridV3f:&lastPoint result:&lastPoint];

    switch (largestComponentV3f(&ray->direction)) {
        case A_X:
            if (ray->direction.x > 0) {
                plane.point = bounds.min;
                plane.norm = XAxisNeg;
            } else {
                plane.point = bounds.max;
                plane.norm = XAxisPos;
            }
            break;
        case A_Y:
            if (ray->direction.y > 0) {
                plane.point = bounds.min;
                plane.norm = YAxisNeg;
            } else {
                plane.point = bounds.max;
                plane.norm = YAxisPos;
            }
            break;
        case A_Z:
            if (ray->direction.z > 0) {
                plane.point = bounds.min;
                plane.norm = ZAxisNeg;
            } else {
                plane.point = bounds.max;
                plane.norm = ZAxisPos;
            }
            break;
    }

    MapDocument* map = [windowController document];
    
    NSUndoManager* undoManager = [map undoManager];
    [undoManager setGroupsByEvent:NO];
    [undoManager beginUndoGrouping];
    
    brush = [map createBrushInEntity:[map worldspawn:YES] withBounds:&bounds texture:@""];
    
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
    
    mergeBoundsWithPoint(&bounds, &point, &bounds);
    lastPoint = point;
    
    MapDocument* map = [windowController document];

    NSUndoManager* undoManager = [map undoManager];
    [undoManager endUndoGrouping];
    [undoManager undo];
    [undoManager beginUndoGrouping];
    
    brush = [map createBrushInEntity:[map worldspawn:YES] withBounds:&bounds texture:@""];
    
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

- (NSString *)actionName {
    return @"Create Brush";
}

@end
