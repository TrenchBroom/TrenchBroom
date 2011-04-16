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

@interface ClipTool (private)

- (Vector3i *)setClipPointWithRay:(Ray3D *)ray hits:(PickingHitList *)hits;
- (void)updateFeedback;
- (BOOL)intersect:(Ray3D *)ray withClipPoint:(Vector3i *)point;

@end

@implementation ClipTool (private)

- (Vector3i *)setClipPointWithRay:(Ray3D *)ray hits:(PickingHitList *)hits {
    Vector3i* p1 = [clipPlane point1];
    Vector3i* p2 = [clipPlane point2];
    Vector3i* p3 = [clipPlane point3];
    
    if (p1 == nil) {
        p1 = [[Vector3i alloc] initWithIntVector:currentPoint];
        [clipPlane setPoint1:p1];
        [clipPlane setHitList1:hits];
        return [p1 autorelease];
    }
    
    if (p2 == nil && ![self intersect:ray withClipPoint:p1]) {
        p2 = [[Vector3i alloc] initWithIntVector:currentPoint];
        [clipPlane setPoint2:p2];
        [clipPlane setHitList2:hits];
        return [p2 autorelease];
    }
    
    if (p3 == nil && ![self intersect:ray withClipPoint:p1] && ![self intersect:ray withClipPoint:p2]) {
        p3 = [[Vector3i alloc] initWithIntVector:currentPoint];
        [clipPlane setPoint3:p3];
        [clipPlane setHitList3:hits];
        return [p3 autorelease];
    }
    
    return nil;
}

- (void)updateFeedback {
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
    
    if (clipPlane != nil) {
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
    }

    NSEnumerator* figureEn = [brushFigures objectEnumerator];
    ClipBrushFeedbackFigure* figure;
    while ((figure = [figureEn nextObject]))
        [renderer removeFeedbackFigure:figure];
    [brushFigures removeAllObjects];
    
    if (clipPlane != nil) {
        SelectionManager* selectionManager = [windowController selectionManager];
        NSEnumerator* brushEn = [[selectionManager selectedBrushes] objectEnumerator];
        id <Brush> brush;
        while ((brush = [brushEn nextObject])) {
            figure = [[ClipBrushFeedbackFigure alloc] initWithBrush:brush clipPlane:clipPlane];
            [brushFigures addObject:figure];
            [renderer addFeedbackFigure:figure];
            [figure release];
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

- (id)init {
    if (self = [super init]) {
        brushFigures = [[NSMutableSet alloc] init];
    }
    
    return self;
}

- (id)initWithWindowController:(MapWindowController *)theWindowController {
    if (self = [self init]) {
        windowController = [theWindowController retain];
    }
    
    return self;
}

- (void)beginLeftDrag:(NSEvent *)event ray:(Ray3D *)ray hits:(PickingHitList *)hits {
    if ([self numPoints] > 0 && [self intersect:ray withClipPoint:[clipPlane point1]]) {
        draggedPoint = [clipPlane point1];
    } else if ([self numPoints] > 1 && [self intersect:ray withClipPoint:[clipPlane point2]]) {
        draggedPoint = [clipPlane point2];
    } else if ([self numPoints] > 2 && [self intersect:ray withClipPoint:[clipPlane point3]]) {
        draggedPoint = [clipPlane point3];
    }
    
    if (draggedPoint == nil && [self numPoints] < 3)
        draggedPoint = [self setClipPointWithRay:ray hits:hits];

    if (draggedPoint != nil && currentFigure != nil) {
        Renderer* renderer = [windowController renderer];
        [renderer removeFeedbackFigure:currentFigure];
        [currentFigure release];
        currentFigure = nil;
    }
    
    [self updateFeedback];
}

- (void)endLeftDrag:(NSEvent *)event ray:(Ray3D *)ray hits:(PickingHitList *)hits {
    draggedPoint = nil;
}

- (void)leftDrag:(NSEvent *)event ray:(Ray3D *)ray hits:(PickingHitList *)hits {
    PickingHit* hit = [hits firstHitOfType:HT_FACE ignoreOccluders:YES];
    if (hit != nil) {
        Vector3f* temp = [[Vector3f alloc] initWithFloatVector:[hit hitPoint]];
        
        Grid* grid = [[windowController options] grid];
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
                [clipPlane setHitList1:hits];
            } else if (draggedPoint == [clipPlane point2]) {
                [[clipPlane point2] setX:x];
                [[clipPlane point2] setY:y];
                [[clipPlane point2] setZ:z];
                [clipPlane setHitList2:hits];
            } else {
                [[clipPlane point3] setX:x];
                [[clipPlane point3] setY:y];
                [[clipPlane point3] setZ:z];
                [clipPlane setHitList3:hits];
            }
        }
    }
    
    [self updateFeedback];
}

- (void)handleLeftMouseUp:(NSEvent *)event ray:(Ray3D *)ray hits:(PickingHitList *)hits {
    if (clipPlane == nil)
        return;
    
    if (currentPoint == nil)
        return;
    
    [self setClipPointWithRay:ray hits:hits];
    [self updateFeedback];
    
    if (currentFigure != nil) {
        Renderer* renderer = [windowController renderer];
        [renderer removeFeedbackFigure:currentFigure];
        [currentFigure release];
        currentFigure = nil;
    }
}

- (void)handleMouseMoved:(NSEvent *)event ray:(Ray3D *)ray hits:(PickingHitList *)hits {
    if (clipPlane == nil)
        return;
    
    if (currentPoint != nil) {
        [currentPoint release];
        currentPoint = nil;
    }
    
    if (currentFigure != nil) {
        Renderer* renderer = [windowController renderer];
        [renderer removeFeedbackFigure:currentFigure];
        [currentFigure release];
        currentFigure = nil;
    }
    
    PickingHit* hit = [hits firstHitOfType:HT_FACE ignoreOccluders:YES];
    if (hit != nil) {
        Vector3f* temp = [[Vector3f alloc] initWithFloatVector:[hit hitPoint]];
        
        Grid* grid = [[windowController options] grid];
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
        
        if ([clipPlane point3] == nil) {
            Renderer* renderer = [windowController renderer];
            currentFigure = [[ClipPointFeedbackFigure alloc] initWithPoint:currentPoint];
            [renderer addFeedbackFigure:currentFigure];
        }
    }
}

- (BOOL)isCursorOwner:(NSEvent *)event ray:(Ray3D *)ray hits:(PickingHitList *)hits {
    return [self active];
}

- (void)activate {
    clipPlane = [[ClipPlane alloc] init];
    [self updateFeedback];
}

- (void)deactivate {
    [clipPlane release];
    clipPlane = nil;
    
    [currentPoint release];
    currentPoint = nil;
    if (currentFigure != nil) {
        Renderer* renderer = [windowController renderer];
        [renderer removeFeedbackFigure:currentFigure];
        [currentFigure release];
        currentFigure = nil;
    }

    [self updateFeedback];
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
    [self updateFeedback];
}

- (void)deleteLastPoint {
    if ([clipPlane point3] != nil) {
        [clipPlane setPoint3:nil];
        [clipPlane setHitList3:nil];
    } else if ([clipPlane point2] != nil) {
        [clipPlane setPoint2:nil];
        [clipPlane setHitList2:nil];
    } else if ([clipPlane point1] != nil) {
        [clipPlane setPoint1:nil];
        [clipPlane setHitList1:nil];
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
    SelectionManager* selectionManager = [windowController selectionManager];
    
    NSSet* brushes = [[NSSet alloc] initWithSet:[selectionManager selectedBrushes]];
    [selectionManager removeAll:YES];
    
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
    
    [selectionManager addBrushes:result record:YES];
    
    [undoManager endUndoGrouping];
    [undoManager setActionName:[brushes count] == 1 ? @"Clip Brush" : @"Clip Brushes"];
    
    [clipPlane reset];
    [currentPoint release];
    currentPoint = nil;
    [self updateFeedback];
    
    return [result autorelease];
}

- (void)cancel {
    [clipPlane reset];
    [currentPoint release];
    currentPoint = nil;
    [self updateFeedback];
}

- (void)dealloc {
    [clipPlane reset];
    [self updateFeedback];
    [clipPlane release];
    [currentPoint release];
    [brushFigures release];
    [windowController release];
    [super dealloc];
}
@end
