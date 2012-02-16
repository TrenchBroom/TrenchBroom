/*
Copyright (C) 2010-2012 Kristian Duske

This file is part of TrenchBroom.

TrenchBroom is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

TrenchBroom is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
*/

#import "RotateTool.h"
#import "RotateFeedbackFigure.h"
#import "MapWindowController.h"
#import "MapDocument.h"
#import "SelectionManager.h"
#import "PickingHit.h"
#import "PickingHitList.h"
#import "Entity.h"
#import "Face.h"
#import "Math.h"
#import "Camera.h"
#import "Renderer.h"
#import "Options.h"
#import "Grid.h"
#import "CompassFigure.h"

@implementation RotateTool

- (id)initWithWindowController:(MapWindowController *)theWindowController {
    NSAssert(theWindowController != nil, @"window controller must not be nil");
    
    if ((self = [self init])) {
        windowController = theWindowController;
        feedbackFigure = [[RotateFeedbackFigure alloc] init];
    }
    
    return self;
}

- (void)dealloc {
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
        radius = 100;

        Camera* camera = [windowController camera];
        vAxis = *[camera right];

        float cos = dotV3f(&XAxisPos, &vAxis);
        float angle = acosf(cos);
        if (cos > -1 || cos < 1) {
            TVector3f c;
            crossV3f(&XAxisPos, &vAxis, &c);
            if (c.z < 0)
                angle *= -1;
        }
        
        Grid* grid = [[windowController options] grid];
        if ([grid snap]) {
            angle = [grid snapAngle:angle];
            
            TQuaternion rot;
            setAngleAndAxisQ(&rot, angle, &ZAxisPos);
            rotateQ(&rot, &XAxisPos, &vAxis);
        }
        
        [feedbackFigure setDragging:NO];
        [feedbackFigure setCenter:&center radius:radius];
        [feedbackFigure setVerticalAxis:&vAxis];
        [feedbackFigure setHorizontalAngle:0 verticalAngle:0];
        
        Renderer* renderer = [windowController renderer];
        [renderer addFeedbackFigure:feedbackFigure];
        
        delta = NSMakePoint(0, 0);
    }
}

- (void)deactivated:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {
    Renderer* renderer = [windowController renderer];
    [renderer removeFeedbackFigure:feedbackFigure];
}

- (void)beginLeftDrag:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {
    float d = closestPointOnRay(&center, ray);
    if (isnan(d))
        return;
    
    drag = YES;
    [feedbackFigure setDragging:YES];
    delta = NSMakePoint(0, 0);
    lastHAngle = 0;
    lastVAngle = 0;
    
    NSUndoManager* undoManager = [[windowController document] undoManager];
    [undoManager setGroupsByEvent:NO];
    [undoManager beginUndoGrouping];
    NSLog(@"begin rotate");
}

- (void)leftDrag:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {
    if (!drag)
        return;

    if (fabsf(event.deltaX) > fabsf(event.deltaY))
        delta.x += event.deltaX;
    else
        delta.y += event.deltaY;
    
    float hAngle = (delta.x / 6) / (M_PI * 2);
    float vAngle = (delta.y / 6) / (M_PI * 2);
    
    Options* options = [windowController options];
    Grid* grid = [options grid];
    if ([grid snap]) {
        hAngle = [grid snapAngle:hAngle];
        vAngle = [grid snapAngle:vAngle];
    }

    
    if (hAngle != lastHAngle || vAngle != lastVAngle) {
        TQuaternion rotation;
        TVector3f rotatedVAxis;
        if (hAngle != 0 && vAngle != 0) {
            TQuaternion hRotation, vRotation;
            setAngleAndAxisQ(&hRotation, hAngle, &ZAxisPos);
            rotateQ(&hRotation, &vAxis, &rotatedVAxis);
            
            setAngleAndAxisQ(&vRotation, vAngle, &vAxis);
            mulQ(&hRotation, &vRotation, &rotation);
        } else if (hAngle != 0) {
            setAngleAndAxisQ(&rotation, hAngle, &ZAxisPos);
            rotateQ(&rotation, &vAxis, &rotatedVAxis);
        } else {
            setAngleAndAxisQ(&rotation, vAngle, &vAxis);
            rotatedVAxis = vAxis;
        }
        
        [feedbackFigure setVerticalAxis:&rotatedVAxis];
        
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
    [feedbackFigure setDragging:NO];

    NSUndoManager* undoManager = [[windowController document] undoManager];
    [undoManager setActionName:[self actionName]];
    [undoManager endUndoGrouping];
    [undoManager setGroupsByEvent:YES];
    NSLog(@"end rotate");
}

- (NSString *)actionName {
    return @"Rotate Objects";
}

@end
