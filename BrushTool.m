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
#import "CoordinatePlane.h"
#import "Ray3D.h"
#import "Plane3D.h"
#import "Vector3f.h"
#import "Vector3i.h"
#import "BoundingBox.h"
#import "PickingHit.h"
#import "PickingHitList.h"
#import "Face.h"
#import "Brush.h"
#import "MutableBrush.h"
#import "math.h"
#import "Math.h"
#import "Renderer.h"
#import "MapView3D.h"
#import "CursorManager.h"
#import "BrushToolCursor.h"

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
    [lastPoint release];
    [plane release];
    [windowController release];
    [super dealloc];
}

# pragma mark -
# pragma mark @implementation Tool

- (void)beginLeftDrag:(NSEvent *)event ray:(Ray3D *)ray hits:(PickingHitList *)hits {
    PickingHit* faceHit = [hits firstHitOfType:HT_FACE ignoreOccluders:NO];
    
    lastPoint = [[faceHit hitPoint] retain];
    id <Face> face = [faceHit object];
    
    switch ([[face norm] largestComponent]) {
        case VC_X:
            plane = [[Plane3D alloc] initWithPoint:lastPoint norm:[Vector3f xAxisPos]];
            break;
        case VC_Y:
            plane = [[Plane3D alloc] initWithPoint:lastPoint norm:[Vector3f yAxisPos]];
            break;
        default:
            plane = [[Plane3D alloc] initWithPoint:lastPoint norm:[Vector3f zAxisPos]];
            break;
    }
    
    Grid* grid = [[windowController options] grid];
    [grid snapToGrid:lastPoint];

    MapDocument* map = [windowController document];
    NSUndoManager* undoManager = [map undoManager];
    [undoManager setGroupsByEvent:NO];
    [undoManager beginUndoGrouping];
}

- (void)leftDrag:(NSEvent *)event ray:(Ray3D *)ray hits:(PickingHitList *)hits {
    Vector3f* point = [ray pointAtDistance:[plane intersectWithRay:ray]];
    if (point == nil)
        return;
    
    Grid* grid = [[windowController options] grid];
    [grid snapToGrid:point];
    
    if ([point isEqualToVector:lastPoint])
        return;
    
    int x = roundf([point x] - [lastPoint x]);
    int y = roundf([point y] - [lastPoint y]);
    int z = roundf([point z] - [lastPoint z]);
    
    MapDocument* map = [windowController document];
    
    SelectionManager* selectionManager = [windowController selectionManager];
    NSEnumerator* brushEn = [[selectionManager selectedBrushes] objectEnumerator];
    id <Brush> brush;
    while ((brush = [brushEn nextObject]))
        [map translateBrush:brush 
                     xDelta:x
                     yDelta:y
                     zDelta:z];
    
    [lastPoint release];
    lastPoint = [point retain];
    
    CursorManager* cursorManager = [windowController cursorManager];
    [cursorManager updateCursor:lastPoint];}

- (void)endLeftDrag:(NSEvent *)event ray:(Ray3D *)ray hits:(PickingHitList *)hits {
    MapDocument* map = [windowController document];
    NSUndoManager* undoManager = [map undoManager];
    [undoManager setActionName:[self actionName]];
    [undoManager endUndoGrouping];
    [undoManager setGroupsByEvent:YES];

    [lastPoint release];
    lastPoint = nil;
    [plane release];
    plane = nil;
}

- (void)setCursor:(NSEvent *)event ray:(Ray3D *)ray hits:(PickingHitList *)hits {
    PickingHit* hit = [hits firstHitOfType:HT_FACE ignoreOccluders:YES];
    id <Face> face = [hit object];

    CursorManager* cursorManager = [windowController cursorManager];
    [cursorManager pushCursor:cursor];
    [cursor setPlaneNormal:[[face norm] largestComponent]];
}

- (void)unsetCursor:(NSEvent *)event ray:(Ray3D *)ray hits:(PickingHitList *)hits {
    CursorManager* cursorManager = [windowController cursorManager];
    [cursorManager popCursor];
}

- (void)updateCursor:(NSEvent *)event ray:(Ray3D *)ray hits:(PickingHitList *)hits {
    PickingHit* hit = [hits firstHitOfType:HT_FACE ignoreOccluders:YES];
    id <Face> face = [hit object];
    
    [cursor setPlaneNormal:[[face norm] largestComponent]];
}

- (NSString *)actionName {
    return @"Move Brushes";
}

@end
