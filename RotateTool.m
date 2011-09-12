//
//  RotateTool.m
//  TrenchBroom
//
//  Created by Kristian Duske on 25.06.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "RotateTool.h"
#import "RotateCursor.h"
#import "RotateFeedbackFigure.h"
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
#import "Options.h"
#import "Grid.h"

static float M_PI_12 = M_PI / 12;

@implementation RotateTool

- (id)initWithWindowController:(MapWindowController *)theWindowController {
    NSAssert(theWindowController != nil, @"window controller must not be nil");
    
    if ((self = [self init])) {
        windowController = theWindowController;
        rotateCursor = [[RotateCursor alloc] init];
        feedbackFigure = [[RotateFeedbackFigure alloc] init];
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

        Camera* camera = [windowController camera];
        TVector3f diff;
        subV3f([camera position], &center, &diff);
        vAxis = fabs(diff.x) < fabs(diff.y) ? A_X : A_Y;

        EAxis comp = strongestComponentV3f(&diff);
        if (comp == A_X || (comp == A_Z && vAxis == A_Y)) {
            if (diff.x > 0) {
                initialHAngle = 0;
                initialVAngle = 0;
            } else {
                initialHAngle = M_PI;
                initialVAngle = 0;
            }
        } else {
            if (diff.y > 0) {
                initialHAngle = M_PI / 2;
                initialVAngle = 0;
            } else {
                initialHAngle = 3 * M_PI / 2;
                initialVAngle = 0;
            }
        }

        [rotateCursor updateCenter:&center radius:radius verticalAxis:vAxis initialHAngle:initialHAngle initialVAngle:initialVAngle];
        [rotateCursor updateHorizontalAngle:0 verticalAngle:0];

        [feedbackFigure updateCenter:&center radius:radius verticalAxis:vAxis initialHAngle:initialHAngle initialVAngle:initialVAngle];
        [feedbackFigure updateHorizontalAngle:0 verticalAngle:0];
        
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
        
        [rotateCursor updateCenter:&center radius:radius verticalAxis:vAxis initialHAngle:initialHAngle initialVAngle:initialVAngle];
        [rotateCursor updateHorizontalAngle:0 verticalAngle:0];

        [feedbackFigure updateCenter:&center radius:radius verticalAxis:vAxis initialHAngle:initialHAngle initialVAngle:initialVAngle];
        [feedbackFigure updateHorizontalAngle:0 verticalAngle:0];
    }
}

- (void)beginLeftDrag:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {
    float d = closestPointOnRay(&center, ray);
    if (isnan(d))
        return;
    
    drag = YES;
    [rotateCursor setDragging:YES];
    [feedbackFigure setDragging:YES];
    delta = NSMakePoint(0, 0);
    lastHAngle = 0;
    lastVAngle = 0;
    
    NSUndoManager* undoManager = [[windowController document] undoManager];
    [undoManager setGroupsByEvent:NO];
    [undoManager beginUndoGrouping];
}

- (void)leftDrag:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {
    if (!drag)
        return;

    if (fabsf(event.deltaX) > fabsf(event.deltaY))
        delta.x += event.deltaX;
    else
        delta.y += event.deltaY;
    
    float hAngle = (delta.x / 6) / (M_PI * 2);
    float vAngle = (-delta.y / 6) / (M_PI * 2);
    
    Options* options = [windowController options];
    if ([[options grid] snap]) {
        int hSteps = hAngle / M_PI_12;
        int vSteps = vAngle / M_PI_12;
        hAngle = hSteps * M_PI_12;
        vAngle = vSteps * M_PI_12;
    }
    
    [rotateCursor updateHorizontalAngle:hAngle verticalAngle:vAngle];
    [feedbackFigure updateHorizontalAngle:hAngle verticalAngle:vAngle];
    
    if (hAngle != lastHAngle || vAngle != lastVAngle) {
        TQuaternion rotation;
        if (hAngle != 0 && vAngle != 0) {
            TQuaternion hRotation, vRotation;
            setAngleAndAxisQ(&hRotation, hAngle, &ZAxisPos);
            setAngleAndAxisQ(&vRotation, vAngle, vAxis == A_X ? &XAxisPos : &YAxisNeg);
            mulQ(&hRotation, &vRotation, &rotation);
        } else if (hAngle != 0) {
            setAngleAndAxisQ(&rotation, hAngle, &ZAxisPos);
        } else {
            setAngleAndAxisQ(&rotation, vAngle, vAxis == A_X ? &XAxisPos : &YAxisNeg);
        }
        
        MapDocument* map = [windowController document];
        SelectionManager* selectionManager = [windowController selectionManager];
        
        TBoundingBox bounds;
        [selectionManager selectionBounds:&bounds];
        rotateBounds(&bounds, &rotation, &center, &bounds);
        
        if (boundsContainBounds([map worldBounds], &bounds)) {
            NSUndoManager* undoManager = [map undoManager];
            [undoManager endUndoGrouping];
            [undoManager undo];
            [undoManager beginUndoGrouping];
            
            [map rotateEntities:[selectionManager selectedEntities] rotation:rotation center:center];
            [map rotateBrushes:[selectionManager selectedBrushes] rotation:rotation center:center lockTextures:[options lockTextures]];

            lastHAngle = hAngle;
            lastVAngle = vAngle;
        }
    }
}

- (void)endLeftDrag:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {
    if (!drag)
        return;

    drag = NO;
    [rotateCursor setDragging:NO];
    [feedbackFigure setDragging:NO];

    NSUndoManager* undoManager = [[windowController document] undoManager];
    [undoManager setActionName:[self actionName]];
    [undoManager endUndoGrouping];
    [undoManager setGroupsByEvent:YES];
}

- (NSString *)actionName {
    return @"Rotate Objects";
}

- (void)setCursor:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {
    CursorManager* cursorManager = [windowController cursorManager];
    [cursorManager pushCursor:rotateCursor];
    
    [self updateCursor:event ray:ray hits:hits];
}

- (void)unsetCursor:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {
    CursorManager* cursorManager = [windowController cursorManager];
    [cursorManager popCursor];
}

- (void)updateCursor:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {
}

@end
