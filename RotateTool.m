//
//  RotateTool.m
//  TrenchBroom
//
//  Created by Kristian Duske on 25.06.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "RotateTool.h"
#import "RotateCursor.h"
#import "RotationFeedbackFigure.h"
#import "MapWindowController.h"
#import "MapDocument.h"
#import "SelectionManager.h"
#import "CursorManager.h"
#import "PickingHit.h"
#import "PickingHitList.h"
#import "Entity.h"
#import "Face.h"
#import "Math.h"
#import "Camera.h"
#import "Renderer.h"

@implementation RotateTool

- (id)initWithWindowController:(MapWindowController *)theWindowController {
    NSAssert(theWindowController != nil, @"window controller must not be nil");
    
    if ((self = [self init])) {
        windowController = theWindowController;
        rotateCursor = [[RotateCursor alloc] init];
        feedbackFigure = [[RotationFeedbackFigure alloc] init];
    }
    
    return self;
}

- (void)dealloc {
    [rotateCursor release];
    [feedbackFigure release];
    [super dealloc];
}

# pragma mark -
# pragma mark @implementation Tool

- (void)activated:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {
    TBoundingBox bounds;
    SelectionManager* selectionManager = [windowController selectionManager];
    if ([selectionManager selectionBounds:&bounds]) {
        centerOfBounds(&bounds, &center);
        radius = distanceOfPointAndRay(&center, ray);

        [feedbackFigure updateCenter:&center radius:radius];
        
        Renderer* renderer = [windowController renderer];
        [renderer addFeedbackFigure:feedbackFigure];
    }
}

- (void)deactivated:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {
    Renderer* renderer = [windowController renderer];
    [renderer removeFeedbackFigure:feedbackFigure];
}

- (void)handleMouseMoved:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {
    TBoundingBox bounds;
    SelectionManager* selectionManager = [windowController selectionManager];
    if ([selectionManager selectionBounds:&bounds]) {
        centerOfBounds(&bounds, &center);
        radius = distanceOfPointAndRay(&center, ray);
        
        [feedbackFigure updateCenter:&center radius:radius];
    }
}

- (void)beginLeftDrag:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {
    float d = closestPointOnRay(&center, ray);
    if (isnan(d))
        return;
    
    rayPointAtDistance(ray, d, &initialVector);
    subV3f(&initialVector, &center, &initialVector);
    initialVector.z = 0;
    normalizeV3f(&initialVector, &initialVector);
    
    drag = YES;
    [feedbackFigure setDragging:YES];
    [feedbackFigure updateInitialDragVector:&initialVector];
    
    NSUndoManager* undoManager = [[windowController document] undoManager];
    [undoManager setGroupsByEvent:NO];
    [undoManager beginUndoGrouping];
}

- (void)leftDrag:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {
    if (!drag)
        return;
    
    float d = intersectSphereWithRay(&center, radius, ray);
    if (isnan(d))
        return;
    
    TVector3f vector;
    rayPointAtDistance(ray, d, &vector);
    subV3f(&vector, &center, &vector);
    vector.z = 0;
    normalizeV3f(&vector, &vector);
    
    NSLog(@"vector: %f %f %f", vector.x, vector.y, vector.z);

    float angle = acos(dotV3f(&initialVector, &vector));
    if (isnan(angle))
        return;
    
    int steps = angle / (M_PI / 12);
    if (steps == 0)
        return;

    NSUndoManager* undoManager = [[windowController document] undoManager];
    [undoManager endUndoGrouping];
    [undoManager undo];
    [undoManager beginUndoGrouping];
    
    TVector3f cross;
    crossV3f(&initialVector, &vector, &cross);

    NSLog(@"angle: %f", angle);
    
    TQuaternion rotation;
    if (cross.z < 0) {
        setAngleAndAxisQ(&rotation, steps * M_PI / 12, &ZAxisNeg);
    } else {
        setAngleAndAxisQ(&rotation, steps * M_PI / 12, &ZAxisPos);
    }
    
    SelectionManager* selectionManager = [windowController selectionManager];
    NSSet* brushes = [selectionManager selectedBrushes];
    
    MapDocument* map = [windowController document];
    [map rotate:&rotation center:&center brushes:brushes];
    
    [feedbackFigure updateCurrentDragVector:&vector];
}

- (void)endLeftDrag:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {
    if (!drag)
        return;

    drag = NO;
    [feedbackFigure setDragging:NO];

    NSUndoManager* undoManager = [[windowController document] undoManager];
    [undoManager setActionName:[self actionName]];
    [undoManager endUndoGrouping];
    [undoManager setGroupsByEvent:YES];
}

- (NSString *)actionName {
    return @"Rotate Objects";
}
@end
