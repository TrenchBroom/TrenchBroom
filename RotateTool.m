//
//  RotateTool.m
//  TrenchBroom
//
//  Created by Kristian Duske on 25.06.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "RotateTool.h"
#import "RotateCursor.h"
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

static float M_PI_12 = M_PI / 12;

@implementation RotateTool

- (id)initWithWindowController:(MapWindowController *)theWindowController {
    NSAssert(theWindowController != nil, @"window controller must not be nil");
    
    if ((self = [self init])) {
        windowController = theWindowController;
        rotateCursor = [[RotateCursor alloc] init];
    }
    
    return self;
}

- (void)dealloc {
    [rotateCursor release];
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
        
        [rotateCursor updateCenter:&center radius:radius verticalAxis:vAxis];
    }
}

- (void)handleMouseMoved:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {
    TBoundingBox bounds;
    SelectionManager* selectionManager = [windowController selectionManager];
    if ([selectionManager selectionBounds:&bounds]) {
        centerOfBounds(&bounds, &center);
        radius = distanceOfPointAndRay(&center, ray);
        
        [rotateCursor updateCenter:&center radius:radius verticalAxis:vAxis];
    }
}

- (void)beginLeftDrag:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {
    float d = closestPointOnRay(&center, ray);
    if (isnan(d))
        return;
    
    drag = YES;
    [rotateCursor setDragging:YES];
    initialLocation = [NSEvent mouseLocation];
    
    NSUndoManager* undoManager = [[windowController document] undoManager];
    [undoManager setGroupsByEvent:NO];
    [undoManager beginUndoGrouping];
}

- (void)leftDrag:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {
    if (!drag)
        return;

    NSUndoManager* undoManager = [[windowController document] undoManager];
    [undoManager endUndoGrouping];
    [undoManager undo];
    [undoManager beginUndoGrouping];
    
    NSPoint currentLocation = [NSEvent mouseLocation];
    float dx = currentLocation.x - initialLocation.x;
    float dy = currentLocation.y - initialLocation.y;
    
    float hAngle = (dx / 6) / (M_PI * 2);
    float vAngle = (-dy / 6) / (M_PI * 2);
    
    int hSteps = hAngle / M_PI_12;
    int vSteps = vAngle / M_PI_12;
    
    [rotateCursor updateHorizontalAngle:hSteps * M_PI_12 verticalAngle:vSteps * M_PI_12];
    
    if (hSteps != 0 || vSteps != 0) {
        TQuaternion rotation;
        if (hSteps != 0 && vSteps != 0) {
            TQuaternion hRotation, vRotation;
            setAngleAndAxisQ(&hRotation, hSteps * M_PI_12, &ZAxisPos);
            setAngleAndAxisQ(&vRotation, vSteps * M_PI_12, vAxis == A_X ? &XAxisPos : &YAxisPos);
            mulQ(&hRotation, &vRotation, &rotation);
        } else if (hSteps != 0) {
            setAngleAndAxisQ(&rotation, hSteps * M_PI_12, &ZAxisPos);
        } else {
            setAngleAndAxisQ(&rotation, vSteps * M_PI_12, vAxis == A_X ? &XAxisPos : &YAxisPos);
        }
        
        SelectionManager* selectionManager = [windowController selectionManager];
        //    NSSet* entities = [selectionManager selectedEntities];
        NSSet* brushes = [selectionManager selectedBrushes];
        
        MapDocument* map = [windowController document];
        //    [map rotateEntities:entities rotation:&rotation center:&center];
        [map rotateBrushes:brushes rotation:&rotation center:&center];
    }
}

- (void)endLeftDrag:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {
    if (!drag)
        return;

    drag = NO;
    [rotateCursor setDragging:NO];

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
