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

@interface RotateTool (private)

- (BOOL)isRotateModifierPressed;
- (BOOL)isApplicable;

@end

@implementation RotateTool (private)

- (BOOL)isRotateModifierPressed {
    return [NSEvent modifierFlags] == (NSAlternateKeyMask | NSCommandKeyMask);
}

- (BOOL)isApplicable {
    SelectionManager* selectionManager = [windowController selectionManager];
    return [self isRotateModifierPressed] && ([selectionManager hasSelectedBrushes] || [selectionManager hasSelectedEntities]);
}

@end

@implementation RotateTool

- (id)initWithWindowController:(MapWindowController *)theWindowController {
    if ((self = [super initWithWindowController:theWindowController])) {
        Grid* grid = [[windowController options] grid];
        feedbackFigure = [[RotateFeedbackFigure alloc] initWithGrid:grid];
    }
    
    return self;
}

- (void)dealloc {
    [feedbackFigure release];
    [super dealloc];
}

# pragma mark -
# pragma mark @implementation Tool

- (void)flagsChanged:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {
    BOOL applicable = [self isApplicable];
    if (!active && applicable) {
        TBoundingBox bounds;
        SelectionManager* selectionManager = [windowController selectionManager];
        if ([selectionManager selectionBounds:&bounds]) {
            centerOfBounds(&bounds, &center);
            radius = 40;
            
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
            
            [self addFeedbackFigure:feedbackFigure];
            
            delta = NSMakePoint(0, 0);
        }
        
        active = YES;
    } else if (active && !applicable) {
        Renderer* renderer = [windowController renderer];
        [renderer removeFeedbackFigure:feedbackFigure];
        active = NO;
    }
}

- (void)mouseMoved:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {
    if (!active)
        return;
    
    if (fabsf(event.deltaX) > fabsf(event.deltaY))
        delta.x += event.deltaX;
    else
        delta.y += event.deltaY;
    
    float angle = (delta.x / 16) / (M_PI * 2);
    Grid* grid = [[windowController options] grid];
    if ([grid snap])
        angle = [grid snapAngle:angle];
    
    if (angle == 0)
        return;
    
    TQuaternion rot;
    setAngleAndAxisQ(&rot, angle, &ZAxisPos);
    rotateQ(&rot, &vAxis, &vAxis);
    
    [feedbackFigure setVerticalAxis:&vAxis];
    [self updateFeedbackFigure:feedbackFigure];

    delta = NSMakePoint(0, 0);
}

- (BOOL)beginLeftDrag:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {
    if (![self isApplicable])
        return NO;
    
    float d = closestPointOnRay(&center, ray);
    if (isnan(d))
        return NO;
    
    drag = YES;
    [feedbackFigure setDragging:YES];
    delta = NSMakePoint(0, 0);
    lastHAngle = 0;
    lastVAngle = 0;
    
    // to avoid confusion for the user:
    Camera* camera = [windowController camera];
    if (dotV3f(&vAxis, [camera right]) < 0)
        scaleV3f(&vAxis, -1, &vAxis);
    
    NSUndoManager* undoManager = [[windowController document] undoManager];
    [undoManager setGroupsByEvent:NO];
    [undoManager beginUndoGrouping];
    
    return YES;
}

- (void)leftDrag:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {
    if (!drag)
        return;

    if (fabsf(event.deltaX) > fabsf(event.deltaY))
        delta.x += event.deltaX;
    else
        delta.y += event.deltaY;
    
    float hAngle = (delta.x / 16) / (M_PI * 2);
    float vAngle = (delta.y / 16) / (M_PI * 2);
    
    Options* options = [windowController options];
    Grid* grid = [options grid];
    if ([grid snap]) {
        hAngle = [grid snapAngle:hAngle];
        vAngle = [grid snapAngle:vAngle];
    }

    if (hAngle != lastHAngle || vAngle != lastVAngle) {
        TQuaternion hRotation, vRotation;
        TVector3f rotatedVAxis = vAxis;
        
        float totalHAngle = hAngle;
        float totalVAngle = vAngle;
        
        int hSteps = hAngle / M_PI_2;
        hAngle -= hSteps * M_PI_2;
        
        if (hSteps > 0) {
            for (int i = 0; i < hSteps; i++)
                rotate90CCWV3f(&rotatedVAxis, A_Z, &rotatedVAxis);
        } else if (hSteps < 0) {
            for (int i = 0; i > hSteps; i--)
                rotate90CWV3f(&rotatedVAxis, A_Z, &rotatedVAxis);
        }
        
        setAngleAndAxisQ(&hRotation, hAngle, &ZAxisPos);
        rotateQ(&hRotation, &rotatedVAxis, &rotatedVAxis);

        int vSteps = 0;
        if (hAngle == 0 && equalV3f(&rotatedVAxis, firstAxisV3f(&rotatedVAxis))) {
            vSteps = vAngle / M_PI_2;
            vAngle -= vSteps * M_PI_2;
        }
        
        setAngleAndAxisQ(&vRotation, vAngle, &rotatedVAxis);
        
        [feedbackFigure setVerticalAxis:&rotatedVAxis];
        
        MapDocument* map = [windowController document];
        SelectionManager* selectionManager = [windowController selectionManager];
        
        TBoundingBox bounds;
        [selectionManager selectionBounds:&bounds];

        if (hSteps > 0) {
            for (int i = 0; i < hSteps; i++)
                rotateBounds90CCW(&bounds, A_Z, &center, &bounds);
        } else if (hSteps < 0) {
            for (int i = 0; i > hSteps; i--)
                rotateBounds90CW(&bounds, A_Z, &center, &bounds);
        }

        rotateBounds(&bounds, &hRotation, &center, &bounds);
        
        if (vSteps > 0) {
            EAxis axis = strongestComponentV3f(&rotatedVAxis);
            for (int i = 0; i < vSteps; i++)
                rotateBounds90CCW(&bounds, axis, &center, &bounds);
        } else if (vSteps < 0) {
            EAxis axis = strongestComponentV3f(&rotatedVAxis);
            for (int i = 0; i > hSteps; i--)
                rotateBounds90CW(&bounds, axis, &center, &bounds);
        }

        rotateBounds(&bounds, &vRotation, &center, &bounds);

        if (boundsContainBounds([map worldBounds], &bounds)) {
            NSUndoManager* undoManager = [map undoManager];
            [undoManager endUndoGrouping];
            [undoManager undo];
            [undoManager beginUndoGrouping];
            
            if (hSteps > 0) {
                for (int i = 0; i < hSteps; i++) {
                    [map rotateEntities90CCW:[selectionManager selectedEntities] axis:A_Z center:center];
                    [map rotateBrushes90CCW:[selectionManager selectedBrushes] axis:A_Z center:center lockTextures:[options lockTextures]];
                }
            } else if (hSteps < 0) {
                for (int i = 0; i > hSteps; i--) {
                    [map rotateEntities90CW:[selectionManager selectedEntities] axis:A_Z center:center];
                    [map rotateBrushes90CW:[selectionManager selectedBrushes] axis:A_Z center:center lockTextures:[options lockTextures]];
                }
            }

            [map rotateEntities:[selectionManager selectedEntities] rotation:hRotation center:center];
            [map rotateBrushes:[selectionManager selectedBrushes] rotation:hRotation center:center lockTextures:[options lockTextures]];

            if (vSteps > 0) {
                EAxis axis = strongestComponentV3f(&rotatedVAxis);
                for (int i = 0; i < vSteps; i++) {
                    [map rotateEntities90CCW:[selectionManager selectedEntities] axis:axis center:center];
                    [map rotateBrushes90CCW:[selectionManager selectedBrushes] axis:axis center:center lockTextures:[options lockTextures]];
                }
            } else if (vSteps < 0) {
                EAxis axis = strongestComponentV3f(&rotatedVAxis);
                for (int i = 0; i > hSteps; i--) {
                    [map rotateEntities90CW:[selectionManager selectedEntities] axis:axis center:center];
                    [map rotateBrushes90CW:[selectionManager selectedBrushes] axis:axis center:center lockTextures:[options lockTextures]];
                }
            }
            
            [map rotateEntities:[selectionManager selectedEntities] rotation:vRotation center:center];
            [map rotateBrushes:[selectionManager selectedBrushes] rotation:vRotation center:center lockTextures:[options lockTextures]];

            lastHAngle = totalHAngle;
            lastVAngle = totalVAngle;
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
}

- (NSString *)actionName {
    return @"Rotate Objects";
}

@end
