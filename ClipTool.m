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
#import "Brush.h"
#import "Entity.h"
#import "MapDocument.h"
#import "ClipPlane.h"
#import "ClipPointFeedbackFigure.h"
#import "ClipLineFeedbackFigure.h"
#import "ClipPlaneFeedbackFigure.h"
#import "ClipBrushFeedbackFigure.h"

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
    
    Vector3i* p1 = [clipPlane point1];
    Vector3i* p2 = [clipPlane point2];
    Vector3i* p3 = [clipPlane point3];
    
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

    NSEnumerator* figureEn = [brushFigures objectEnumerator];
    ClipBrushFeedbackFigure* figure;
    while ((figure = [figureEn nextObject]))
        [renderer removeFeedbackFigure:figure];
    [brushFigures removeAllObjects];
    
    NSEnumerator* brushEn = [brushes objectEnumerator];
    id <Brush> brush;
    while ((brush = [brushEn nextObject])) {
        figure = [[ClipBrushFeedbackFigure alloc] initWithBrush:brush clipPlane:clipPlane];
        [brushFigures addObject:figure];
        [renderer addFeedbackFigure:figure];
        [figure release];
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
        brushFigures = [[NSMutableSet alloc] init];
        clipPlane = [[ClipPlane alloc] init];
        [self updateFeedback];
    }
    
    return self;
}

- (void)handleLeftMouseDragged:(Ray3D *)ray {
    PickingHitList* hitList = [picker pickObjects:ray include:brushes exclude:nil];
    PickingHit* hit = [[hitList firstHitOfType:HT_FACE ignoreOccluders:YES] retain];

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
            if (draggedPoint == [clipPlane point1]) {
                [[clipPlane point1] setX:x];
                [[clipPlane point1] setY:y];
                [[clipPlane point1] setZ:z];
                [clipPlane setFace1:[hit object]];
            } else if (draggedPoint == [clipPlane point2]) {
                [[clipPlane point2] setX:x];
                [[clipPlane point2] setY:y];
                [[clipPlane point2] setZ:z];
                [clipPlane setFace2:[hit object]];
            } else {
                [[clipPlane point3] setX:x];
                [[clipPlane point3] setY:y];
                [[clipPlane point3] setZ:z];
                [clipPlane setFace3:[hit object]];
            }
        }
    }
    
    [self updateFeedback];
}

- (void)handleLeftMouseDown:(Ray3D *)ray {
    if (currentPoint == nil || currentHit == nil)
        return;
    
    Vector3i* p1 = [clipPlane point1];
    Vector3i* p2 = [clipPlane point2];
    Vector3i* p3 = [clipPlane point3];
    
    if (p1 == nil) {
        p1 = [[Vector3i alloc] initWithIntVector:currentPoint];
        [clipPlane setPoint1:p1];
        [clipPlane setFace1:[currentHit object]];
        [p1 release];
    } else {
        if ([self intersect:ray withClipPoint:p1]) {
            draggedPoint = p1;
        } else if (p2 == nil) {
            if (![p1 isEqualToVector:currentPoint]) {
                p2 = [[Vector3i alloc] initWithIntVector:currentPoint];
                [clipPlane setPoint2:p2];
                [clipPlane setFace2:[currentHit object]];
                [p2 release];
            }
        } else {
            if ([self intersect:ray withClipPoint:p2]) {
                draggedPoint = p2;
            } else if (p3 == nil) {
                if (![p1 isEqualToVector:currentPoint] && ![p2 isEqualToVector:currentPoint]) {
                    p3 = [[Vector3i alloc] initWithIntVector:currentPoint];
                    [clipPlane setPoint3:p3];
                    [clipPlane setFace3:[currentHit object]];
                    [p3 release];
                }
            } else {
                if ([self intersect:ray withClipPoint:p3])
                    draggedPoint = p3;
            }
        }
    }
    
    [self updateFeedback];
    
    if (draggedPoint != nil && currentFigure != nil) {
        [renderer removeFeedbackFigure:currentFigure];
        [currentFigure release];
        currentFigure = nil;
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
    
    if (currentHit != nil) {
        [currentHit release];
        currentHit = nil;
    }
    
    if (currentFigure != nil) {
        [renderer removeFeedbackFigure:currentFigure];
        [currentFigure release];
        currentFigure = nil;
    }

    PickingHitList* hitList = [picker pickObjects:ray include:brushes exclude:nil];
    currentHit = [[hitList firstHitOfType:HT_FACE ignoreOccluders:YES] retain];
    if (currentHit != nil) {
        Vector3f* temp = [[Vector3f alloc] initWithFloatVector:[currentHit hitPoint]];
        [grid snapToGrid:temp];
        
        id <Face> face = [currentHit object];
        [face transformToSurface:temp];
        [temp setZ:0];
        [face transformToWorld:temp];
        
        int x = roundf([temp x]);
        int y = roundf([temp y]);
        int z = roundf([temp z]);
        [temp release];
        
        currentPoint = [[Vector3i alloc] initWithIntX:x y:y z:z];
        
        if ([clipPlane point3] == nil) {
            currentFigure = [[ClipPointFeedbackFigure alloc] initWithPoint:currentPoint];
            [renderer addFeedbackFigure:currentFigure];
        }
    }
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
    [self updateFeedback];
}

- (void)deleteLastPoint {
    if ([clipPlane point3] != nil) {
        [clipPlane setPoint3:nil];
        [clipPlane setFace3:nil];
    } else if ([clipPlane point2] != nil) {
        [clipPlane setPoint2:nil];
        [clipPlane setFace2:nil];
    } else if ([clipPlane point1] != nil) {
        [clipPlane setPoint1:nil];
        [clipPlane setFace1:nil];
    } else {
        return;
    }
    
    [self updateFeedback];
}

- (int)numPoints {
    if ([clipPlane point1] == nil)
        return 0;
    if ([clipPlane point2] == nil)
        return 1;
    if ([clipPlane point3] == nil)
        return 2;
    return 3;
}

- (NSSet *)performClip:(MapDocument* )map {
    NSUndoManager* undoManager = [map undoManager];
    [undoManager beginUndoGrouping];

    NSMutableSet* result = [[NSMutableSet alloc] init];
    
    NSEnumerator* brushEn = [brushes objectEnumerator];
    id <Brush> brush;
    while ((brush = [brushEn nextObject])) {
        id <Brush> firstResult = nil;
        id <Brush> secondResult = nil;
        [clipPlane clipBrush:brush firstResult:&firstResult secondResult:&secondResult];
        
        id <Entity> entity = [brush entity];
        [map deleteBrush:brush];
        
        if (firstResult != nil)
            [result addObject:[map createBrushInEntity:entity fromTemplate:firstResult]];
        
        if (secondResult != nil)
            [result addObject:[map createBrushInEntity:entity fromTemplate:secondResult]];
    }
    
    [undoManager endUndoGrouping];
    [undoManager setActionName:[brushes count] == 1 ? @"Clip Brush" : @"Clip Brushes"];
    
    return [result autorelease];
}

- (void)dealloc {
    [brushes release];
    [picker release];
    [grid release];
    [renderer release];
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
    if (currentFigure != nil) {
        [renderer removeFeedbackFigure:currentFigure];
        [currentFigure release];
    }
    
    NSEnumerator* figureEn = [brushFigures objectEnumerator];
    ClipBrushFeedbackFigure* figure;
    while ((figure = [figureEn nextObject]))
        [renderer removeFeedbackFigure:figure];
    [brushFigures release];
    
    [currentPoint release];
    [currentHit release];
    [clipPlane release];
    [super dealloc];
}
@end
