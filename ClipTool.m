//
//  ClipTool.m
//  TrenchBroom
//
//  Created by Kristian Duske on 28.03.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "ClipTool.h"
#import "Picker.h";
#import "PickingHitList.h"
#import "PickingHit.h"
#import "Renderer.h";
#import "Vector3i.h"
#import "Vector3f.h"
#import "Ray3D.h"
#import "Grid.h"
#import "Face.h"
#import "ClipPointFeedbackFigure.h"
#import "ClipLineFeedbackFigure.h"
#import "ClipPlaneFeedbackFigure.h"

@interface ClipTool (private)

- (void)updateFeedback;
- (BOOL)intersect:(Ray3D *)ray withClipPoint:(Vector3i *)point;

@end

@implementation ClipTool (private)

- (void)updateFeedback {
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
    
    Vector3i* p1 = point1;
    Vector3i* p2 = point2;
    Vector3i* p3 = point3;
    
    if (p1 == nil)
        p1 = currentPoint;
    else if (p2 == nil)
        p2 = currentPoint;
    else if (p3 == nil)
        p3 = currentPoint;
    
    if (p1 != nil) {
        point1Figure = [[ClipPointFeedbackFigure alloc] initWithPoint:p1];
        [renderer addFeedbackFigure:point1Figure];
        
        if (p2 != nil) {
            point2Figure = [[ClipPointFeedbackFigure alloc] initWithPoint:p2];
            [renderer addFeedbackFigure:point2Figure];

            line1Figure = [[ClipLineFeedbackFigure alloc] initWithStartPoint:p1 endPoint:p2];
            [renderer addFeedbackFigure:line1Figure];
            
            if (p3 != nil) {
                point3Figure = [[ClipPointFeedbackFigure alloc] initWithPoint:p3];
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
}

- (BOOL)intersect:(Ray3D *)ray withClipPoint:(Vector3i *)point {
    Vector3f* temp = [[Vector3f alloc] initWithIntVector:point];
    float dist = [ray intersectWithSphere:temp radius:3];
    [temp release];
    return !isnan(dist);
}

@end

@implementation ClipTool

- (id)initWithBrushes:(NSSet *)theBrushes picker:(Picker *)thePicker grid:(Grid *)theGrid renderer:(Renderer *)theRenderer {
    NSAssert(theBrushes != nil, @"brush set must not be nil");
    NSAssert([theBrushes count] > 0, @"brush set must not be empty");
    NSAssert(thePicker != nil, @"picker must not be nil");
    NSAssert(theGrid != nil, @"grid must not be nil");
    NSAssert(theRenderer != nil, @"renderer must not be nil");
    
    if (self = [self init]) {
        brushes = [[NSSet alloc] initWithSet:theBrushes];
        picker = [thePicker retain];
        grid = [theGrid retain];
        renderer = [theRenderer retain];
        clipMode = CM_FRONT;
    }
    
    return self;
}

- (void)handleLeftMouseDragged:(Ray3D *)ray {
    PickingHitList* hitList = [picker pickObjects:ray include:brushes exclude:nil];
    PickingHit* hit = [hitList firstHitOfType:HT_FACE ignoreOccluders:YES];

    if (hit != nil) {
        Vector3f* temp = [[Vector3f alloc] initWithFloatVector:[hit hitPoint]];
        [grid snapToGrid:temp];
        
        id <Face> face = [hit object];
        [face transformToSurface:temp];
        [temp setZ:0];
        [face transformToWorld:temp];
        
        int x = roundf([temp x]);
        int y = roundf([temp y]);
        int z = roundf([temp z]);
        [temp release];
        
        if (draggedPoint != nil) {
            if (draggedPoint == point1) {
                [point1 setX:x];
                [point1 setY:y];
                [point1 setZ:z];
            } else if (draggedPoint == point2) {
                [point2 setX:x];
                [point2 setY:y];
                [point2 setZ:z];
            } else {
                [point3 setX:x];
                [point3 setY:y];
                [point3 setZ:z];
            }
        }
    }
    
    [self updateFeedback];
}

- (void)handleLeftMouseDown:(Ray3D *)ray {
    if (currentPoint == nil)
        return;
    
    if (point1 == nil) {
        point1 = [[Vector3i alloc] initWithIntVector:currentPoint];
    } else {
        if ([self intersect:ray withClipPoint:point1]) {
            draggedPoint = point1;
        } else if (point2 == nil) {
            point2 = [[Vector3i alloc] initWithIntVector:currentPoint];
        } else {
            if ([self intersect:ray withClipPoint:point2]) {
                draggedPoint = point2;
            } else if (point3 == nil) {
                point3 = [[Vector3i alloc] initWithIntVector:currentPoint];
            } else {
                if ([self intersect:ray withClipPoint:point3])
                    draggedPoint = point3;
            }
        }
    }
}

- (void)handleLeftMouseUp:(Ray3D *)ray {
    draggedPoint = nil;
}

- (void)handleMouseMoved:(Ray3D *)ray {
    if (currentPoint != nil) {
        [currentPoint release];
        currentPoint = nil;
    }

    PickingHitList* hitList = [picker pickObjects:ray include:brushes exclude:nil];
    PickingHit* hit = [hitList firstHitOfType:HT_FACE ignoreOccluders:YES];
    if (hit != nil) {
        Vector3f* temp = [[Vector3f alloc] initWithFloatVector:[hit hitPoint]];
        [grid snapToGrid:temp];
        
        id <Face> face = [hit object];
        [face transformToSurface:temp];
        [temp setZ:0];
        [face transformToWorld:temp];
        
        int x = roundf([temp x]);
        int y = roundf([temp y]);
        int z = roundf([temp z]);
        [temp release];
        
        currentPoint = [[Vector3i alloc] initWithIntX:x y:y z:z];
    }

    [self updateFeedback];
}

- (void)toggleClipMode {
    switch (clipMode) {
        case CM_FRONT:
            clipMode = CM_BACK;
            break;
        case CM_BACK:
            clipMode = CM_SPLIT;
            break;
        default:
            clipMode = CM_FRONT;
            break;
    }
}

- (void)dealloc {
    [brushes release];
    [picker release];
    [grid release];
    [renderer release];
    [point1 release];
    [point2 release];
    [point3 release];
    if (point1Figure != nil) {
        [renderer removeFeedbackFigure:point1Figure];
        [point1Figure release];
    }
    if (point2Figure != nil) {
        [renderer removeFeedbackFigure:point2Figure];
        [point2Figure release];
    }
    if (point3Figure != nil) {
        [renderer removeFeedbackFigure:point3Figure];
        [point3Figure release];
    }
    if (line1Figure != nil) {
        [renderer removeFeedbackFigure:line1Figure];
        [line1Figure release];
    }
    if (line2Figure != nil) {
        [renderer removeFeedbackFigure:line2Figure];
        [line2Figure release];
    }
    if (line3Figure != nil) {
        [renderer removeFeedbackFigure:line3Figure];
        [line3Figure release];
    }
    if (planeFigure != nil) {
        [renderer removeFeedbackFigure:planeFigure];
        [planeFigure release];
    }
    [currentPoint release];
    [super dealloc];
}
@end
