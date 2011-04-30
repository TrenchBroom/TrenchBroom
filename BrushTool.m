//
//  BrushTool.m
//  TrenchBroom
//
//  Created by Kristian Duske on 12.03.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "BrushTool.h"
#import "Grid.h"
#import "Options.h"
#import "Camera.h"
#import "MapWindowController.h"
#import "MapDocument.h"
#import "SelectionManager.h"
#import "PickingHit.h"
#import "PickingHitList.h"
#import "Face.h"
#import "Brush.h"
#import "MutableBrush.h"
#import "math.h"
#import "Renderer.h"
#import "MapView3D.h"
#import "CursorManager.h"
#import "BrushToolCursor.h"

@interface BrushTool (private)

- (BOOL)isAltPlaneModifierPressed;
- (EAxis)planeNormal:(id <Face>)face;

@end

@implementation BrushTool (private)

- (BOOL)isAltPlaneModifierPressed {
    return [NSEvent modifierFlags] == NSAlternateKeyMask;
}

- (EAxis)planeNormal:(id <Face>)face {
    EAxis planeNormal;
    if (drag) {
        planeNormal = largestComponentV3f(&plane.norm);
    } else {
        planeNormal = largestComponentV3f([face norm]);
        if ([self isAltPlaneModifierPressed]) {
            if (planeNormal == A_X) {
                planeNormal = A_Y;
            } else if (planeNormal == A_Y) {
                planeNormal = A_X;
            } else {
                Camera* camera = [windowController camera];
                const TVector3f* cameraDir = [camera direction];
                if (fabsf(cameraDir->x) > fabsf(cameraDir->y))
                    planeNormal = A_X;
                else
                    planeNormal = A_Y;
            }
        }
    }
    
    return planeNormal;
}

@end

@implementation BrushTool

- (id)initWithController:(MapWindowController *)theWindowController {
    if (self = [self init]) {
        windowController = [theWindowController retain];
        cursor = [[BrushToolCursor alloc] init];
    }
    return self;
}

- (void)dealloc {
    [cursor release];
    [windowController release];
    [super dealloc];
}

# pragma mark -
# pragma mark @implementation Tool

- (void)beginLeftDrag:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {
    PickingHit* faceHit = [hits firstHitOfType:HT_FACE ignoreOccluders:NO];
    if (faceHit == nil)
        return;
    
    id <Face> face = [faceHit object];
    id <Brush> brush = [face brush];

    SelectionManager* selectionManager = [windowController selectionManager];
    if (![selectionManager isBrushSelected:brush])
        return;
    
    lastPoint = *[faceHit hitPoint];
    plane.point = lastPoint;
    
    switch ([self planeNormal:face]) {
        case A_X:
            plane.norm = XAxisPos;
            break;
        case A_Y:
            plane.norm = YAxisPos;
            break;
        default:
            plane.norm = ZAxisPos;
            break;
    }
    
    Grid* grid = [[windowController options] grid];
    [grid snapToGrid:&lastPoint result:&lastPoint];
    drag = YES;
    
    MapDocument* map = [windowController document];
    NSUndoManager* undoManager = [map undoManager];
    [undoManager setGroupsByEvent:NO];
    [undoManager beginUndoGrouping];
}

- (void)leftDrag:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {
    float dist = intersectPlaneWithRay(&plane, ray);
    if (isnan(dist))
        return;
    
    TVector3f point;
    rayPointAtDistance(ray, dist, &point);
    
    Grid* grid = [[windowController options] grid];
    [grid snapToGrid:&point result:&point];
    
    if (equalV3f(&point, &lastPoint))
        return;
    
    int x = roundf(point.x - lastPoint.x);
    int y = roundf(point.y - lastPoint.y);
    int z = roundf(point.z - lastPoint.z);
    
    MapDocument* map = [windowController document];
    
    SelectionManager* selectionManager = [windowController selectionManager];
    NSEnumerator* brushEn = [[selectionManager selectedBrushes] objectEnumerator];
    id <Brush> brush;
    while ((brush = [brushEn nextObject]))
        [map translateBrush:brush 
                     xDelta:x
                     yDelta:y
                     zDelta:z];

    lastPoint = point;
}

- (void)endLeftDrag:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {
    MapDocument* map = [windowController document];
    NSUndoManager* undoManager = [map undoManager];
    [undoManager setActionName:[self actionName]];
    [undoManager endUndoGrouping];
    [undoManager setGroupsByEvent:YES];

    drag = NO;
}

- (void)setCursor:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {
    CursorManager* cursorManager = [windowController cursorManager];
    [cursorManager pushCursor:cursor];

    PickingHit* hit = [hits firstHitOfType:HT_FACE ignoreOccluders:YES];
    id <Face> face = [hit object];

    EAxis planeNormal = [self planeNormal:face];
    [cursor setPlaneNormal:planeNormal];
}

- (void)unsetCursor:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {
    CursorManager* cursorManager = [windowController cursorManager];
    [cursorManager popCursor];
}

- (void)updateCursor:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {
    if (!drag) {
        PickingHit* hit = [hits firstHitOfType:HT_FACE ignoreOccluders:YES];
        id <Face> face = [hit object];
        
        EAxis planeNormal = [self planeNormal:face];
        [cursor setPlaneNormal:planeNormal];
        [cursor update:[hit hitPoint]];
    } else {
        float dist = intersectPlaneWithRay(&plane, ray);
        TVector3f position;
        rayPointAtDistance(ray, dist, &position);
        [cursor update:&position];
    }
}

- (NSString *)actionName {
    return @"Move Brushes";
}

@end
