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

#import "ClipTool.h"
#import "Picker.h"
#import "PickingHitList.h"
#import "PickingHit.h"
#import "Renderer.h"
#import "Options.h"
#import "Grid.h"
#import "Face.h"
#import "Brush.h"
#import "Entity.h"
#import "MapWindowController.h"
#import "SelectionManager.h"
#import "MapDocument.h"
#import "ClipPlane.h"
#import "ClipPointFeedbackFigure.h"
#import "ClipLineFeedbackFigure.h"
#import "ClipPlaneFeedbackFigure.h"
#import "ClipBrushFeedbackFigure.h"
#import "EditingPlaneFigure.h"

@interface ClipTool (private)

- (int)setClipPointWithRay:(TRay *)ray hits:(PickingHitList *)hits;
- (void)updateFeedback:(TRay *)ray;
- (BOOL)intersect:(TRay *)ray withClipPoint:(const TVector3f *)point;

@end

@implementation ClipTool (private)

- (int)setClipPointWithRay:(TRay *)ray hits:(PickingHitList *)hits {
    if ([clipPlane numPoints] < 3)
        [clipPlane addPoint:currentPoint hitList:hits];
    return [clipPlane numPoints] - 1;
}

- (void)updateFeedback:(TRay *)ray {
    Renderer* renderer = [windowController renderer];
    
    if (point1Figure != nil) {
        [renderer removeFeedbackFigure:point1Figure];
        [point1Figure release];
        point1Figure = nil;
    }

    if (point2Figure != nil) {
        [renderer removeFeedbackFigure:point2Figure];
        [point2Figure release];
        point2Figure = nil;
    }
    
    if (point3Figure != nil) {
        [renderer removeFeedbackFigure:point3Figure];
        [point3Figure release];
        point3Figure = nil;
    }
    
    if (line1Figure != nil) {
        [renderer removeFeedbackFigure:line1Figure];
        [line1Figure release];
        line1Figure = nil;
    }
    
    if (line2Figure != nil) {
        [renderer removeFeedbackFigure:line2Figure];
        [line2Figure release];
        line2Figure = nil;
    }
    
    if (line3Figure != nil) {
        [renderer removeFeedbackFigure:line3Figure];
        [line3Figure release];
        line3Figure = nil;
    }
    
    if (planeFigure != nil) {
        [renderer removeFeedbackFigure:planeFigure];
        [planeFigure release];
        planeFigure = nil;
    }
    
    if (editingPlaneFigure != nil) {
        [renderer removeFeedbackFigure:editingPlaneFigure];
        [editingPlaneFigure release];
        editingPlaneFigure = nil;
    }
    
    for (ClipBrushFeedbackFigure* figure in brushFigures) 
        [renderer removeFeedbackFigure:figure];
    [brushFigures removeAllObjects];
    

    if (clipPlane != nil) {
        if ([clipPlane numPoints] > 0) {
            Camera* camera = [windowController camera];
            
            TVector3f* p1 = [clipPlane point:0];
            point1Figure = [[ClipPointFeedbackFigure alloc] initWithPoint:p1 camera:camera];
            [renderer addFeedbackFigure:point1Figure];

            if ([clipPlane numPoints] > 1) {
                TVector3f* p2 = [clipPlane point:1];
                
                point2Figure = [[ClipPointFeedbackFigure alloc] initWithPoint:p2 camera:camera];
                [renderer addFeedbackFigure:point2Figure];
                
                line1Figure = [[ClipLineFeedbackFigure alloc] initWithStartPoint:p1 endPoint:p2];
                [renderer addFeedbackFigure:line1Figure];

                if ([clipPlane numPoints] > 2) {
                    TVector3f* p3 = [clipPlane point:2];

                    point3Figure = [[ClipPointFeedbackFigure alloc] initWithPoint:p3 camera:camera];
                    [renderer addFeedbackFigure:point3Figure];
                    
                    line2Figure = [[ClipLineFeedbackFigure alloc] initWithStartPoint:p2 endPoint:p3];
                    [renderer addFeedbackFigure:line2Figure];
                    
                    line3Figure = [[ClipLineFeedbackFigure alloc] initWithStartPoint:p3 endPoint:p1];
                    [renderer addFeedbackFigure:line3Figure];
                    
                    planeFigure = [[ClipPlaneFeedbackFigure alloc] initWithPoint1:p1 point2:p2 point3:p3];
                    [renderer addFeedbackFigure:planeFigure];
                }
            }
        }

        SelectionManager* selectionManager = [windowController selectionManager];
        for (id <Brush> brush in [selectionManager selectedBrushes]) {
            ClipBrushFeedbackFigure* figure = [[ClipBrushFeedbackFigure alloc] initWithBrush:brush clipPlane:clipPlane];
            if (figure != nil) {
                [brushFigures addObject:figure];
                [renderer addFeedbackFigure:figure];
                [figure release];
            }
        }
        
        if (draggedPoint > -1 && ray != nil) {
            Grid* grid = [[windowController options] grid];
            PickingHit* hit;
            if (draggedPoint == 0)
                hit = [[clipPlane hitList:0] firstHitOfType:HT_FACE ignoreOccluders:YES];
            else if (draggedPoint == 1)
                hit = [[clipPlane hitList:1] firstHitOfType:HT_FACE ignoreOccluders:YES];
            else if (draggedPoint == 2)
                hit = [[clipPlane hitList:2] firstHitOfType:HT_FACE ignoreOccluders:YES];
                
            id <Face> face = [hit object];
            editingPlaneFigure = [[EditingPlaneFigure alloc] initWithGrid:grid plane:[face boundary] bounds:[[face brush] bounds] ray:ray];
            [renderer addFeedbackFigure:editingPlaneFigure];
        }
    }
}

- (BOOL)intersect:(TRay *)ray withClipPoint:(const TVector3f *)point {
    TVector3f center = {point->x, point->y, point->z};
    return !isnan(intersectSphereWithRay(&center, 3, ray));
}

@end

@implementation ClipTool

- (id)init {
    if ((self = [super init])) {
        brushFigures = [[NSMutableArray alloc] init];
        draggedPoint = -1;
    }
    
    return self;
}

- (void)beginLeftDrag:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {
    if ([clipPlane numPoints] > 0 && [self intersect:ray withClipPoint:[clipPlane point:0]]) {
        draggedPoint = 0;
    } else if ([clipPlane numPoints] > 1 && [self intersect:ray withClipPoint:[clipPlane point:1]]) {
        draggedPoint = 1;
    } else if ([clipPlane numPoints] > 2 && [self intersect:ray withClipPoint:[clipPlane point:2]]) {
        draggedPoint = 2;
    }
    
    if (draggedPoint == -1 && [self numPoints] < 3)
        draggedPoint = [self setClipPointWithRay:ray hits:hits];

    if (draggedPoint > -1 && currentFigure != nil) {
        Renderer* renderer = [windowController renderer];
        [renderer removeFeedbackFigure:currentFigure];
        [currentFigure release];
        currentFigure = nil;
    }
    
    [self updateFeedback:ray];
}

- (void)endLeftDrag:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {
    draggedPoint = -1;
    [self updateFeedback:ray];
}

- (void)leftDrag:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {
    if (draggedPoint > -1) {
        PickingHit* hit = [[clipPlane hitList:draggedPoint] firstHitOfType:HT_FACE ignoreOccluders:YES];
        id <Face> face = [hit object];
        
        TVector3f hitPoint;
        float diff = intersectPlaneWithRay([face boundary], ray);
        rayPointAtDistance(ray, diff, &hitPoint);
        
        Grid* grid = [[windowController options] grid];
        [grid snapToGridV3f:&hitPoint result:&hitPoint];
        
        const TVector3f* axis = firstAxisV3f([face norm]);
        [face projectToSurface:&hitPoint axis:axis result:&hitPoint];
        
        [clipPlane updatePoint:draggedPoint x:roundf(hitPoint.x) y:roundf(hitPoint.y) z:roundf(hitPoint.z)];
    }
    
    [self updateFeedback:ray];
}

- (void)handleLeftMouseUp:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {
    if (clipPlane == nil)
        return;
    
    if (currentPoint == NULL)
        return;
    
    [self setClipPointWithRay:ray hits:hits];
    [self updateFeedback:ray];
    
    if (currentFigure != nil) {
        Renderer* renderer = [windowController renderer];
        [renderer removeFeedbackFigure:currentFigure];
        [currentFigure release];
        currentFigure = nil;
    }
}

- (void)handleMouseMoved:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {
    if (clipPlane == nil)
        return;
    
    if (currentPoint != NULL) {
        free(currentPoint);
        currentPoint = NULL;
    }
    
    if (currentFigure != nil) {
        Renderer* renderer = [windowController renderer];
        [renderer removeFeedbackFigure:currentFigure];
        [currentFigure release];
        currentFigure = nil;
    }
    
    PickingHit* hit = [hits firstHitOfType:HT_FACE ignoreOccluders:YES];
    if (hit != nil) {
        TVector3f t;
        
        Grid* grid = [[windowController options] grid];
        [grid snapToGridV3f:[hit hitPoint] result:&t];
        
        
        id <Face> face = [hit object];
        const TVector3f* axis = firstAxisV3f([face norm]);
        [face projectToSurface:&t axis:axis result:&t];
        
        currentPoint = malloc(sizeof(TVector3i));
        roundV3f(&t, currentPoint);
        
        if ([self numPoints] < 3) {
            Camera* camera = [windowController camera];
            Renderer* renderer = [windowController renderer];
            currentFigure = [[ClipPointFeedbackFigure alloc] initWithPoint:currentPoint camera:camera];
            [renderer addFeedbackFigure:currentFigure];
        }
    }
}

- (void)activate {
    clipPlane = [[ClipPlane alloc] init];
    [self updateFeedback:nil];
}

- (void)deactivate {
    [clipPlane release];
    clipPlane = nil;

    free(currentPoint);
    currentPoint = NULL;
    
    if (currentFigure != nil) {
        Renderer* renderer = [windowController renderer];
        [renderer removeFeedbackFigure:currentFigure];
        [currentFigure release];
        currentFigure = nil;
    }

    [self updateFeedback:nil];
}

- (BOOL)active {
    return clipPlane != nil;
}

- (void)toggleClipMode {
    switch ([clipPlane clipMode]) {
        case CM_FRONT:
            [clipPlane setClipMode:CM_BACK];
            break;
        case CM_BACK:
            [clipPlane setClipMode:CM_SPLIT];
            break;
        default:
            [clipPlane setClipMode:CM_FRONT];
            break;
    }
    [self updateFeedback:nil];
}

- (void)deleteLastPoint {
    if ([clipPlane numPoints] > 0) {
        [clipPlane removeLastPoint];
        [self updateFeedback:NULL];
    }
}

- (int)numPoints {
    return [clipPlane numPoints];
}

- (NSArray *)performClip:(MapDocument* )map {
    NSUndoManager* undoManager = [map undoManager];
    [undoManager beginUndoGrouping];

    NSMutableArray* result = [[NSMutableArray alloc] init];
    SelectionManager* selectionManager = [windowController selectionManager];
    
    NSArray* brushes = [[NSArray alloc] initWithArray:[selectionManager selectedBrushes]];
    [selectionManager removeAll:YES];
    
    for (id <Brush> brush in brushes) {
        id <Brush> firstResult = nil;
        id <Brush> secondResult = nil;
        [clipPlane clipBrush:brush firstResult:&firstResult secondResult:&secondResult];
        
        id <Entity> entity = [brush entity];
        if (firstResult != nil)
            [result addObject:[map createBrushInEntity:entity fromTemplate:firstResult]];
        
        if (secondResult != nil)
            [result addObject:[map createBrushInEntity:entity fromTemplate:secondResult]];
    }
    
    [map deleteBrushes:brushes];
    [selectionManager addBrushes:result record:YES];
    
    [undoManager setActionName:[brushes count] == 1 ? @"Clip Brush" : @"Clip Brushes"];
    [undoManager endUndoGrouping];
    
    [brushes release];
    [clipPlane reset];
    if (currentPoint != NULL) {
        free(currentPoint);
        currentPoint = NULL;
    }

    [self updateFeedback:nil];
    
    return [result autorelease];
}

- (void)cancel {
    [clipPlane reset];
    if (currentPoint != NULL) {
        free(currentPoint);
        currentPoint = NULL;
    }
    [self updateFeedback:nil];
}

- (void)dealloc {
    [clipPlane reset];
    [self updateFeedback:nil];
    [clipPlane release];
    if (currentPoint != NULL) {
        free(currentPoint);
        currentPoint = NULL;
    }
    [brushFigures release];
    [super dealloc];
}
@end
