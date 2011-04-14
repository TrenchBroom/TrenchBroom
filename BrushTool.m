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

- (id)init {
    if (self = [super init]) {
        brushes = [[NSMutableSet alloc] init];
    }
    
    return self;
}

- (void)dealloc {
    CursorManager* cursorManager = [windowController cursorManager];
    [cursorManager popCursor];
    [cursor release];

    [lastPoint release];
    [plane release];
    [brushes release];
    [windowController release];
    [super dealloc];
}

# pragma mark -
# pragma mark @implementation Tool

- (id)initWithController:(MapWindowController *)theWindowController pickHits:(PickingHitList *)theHits pickRay:(Ray3D *)theRay {
    if (self = [self init]) {
        [brushes unionSet:[[theWindowController selectionManager] selectedBrushes]];
        windowController = [theWindowController retain];
        
        PickingHit* faceHit = [theHits firstHitOfType:HT_FACE ignoreOccluders:NO];
        
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

        CursorManager* cursorManager = [windowController cursorManager];
        cursor = [[BrushToolCursor alloc] init];
        [cursor setPlaneNormal:[[face norm] largestComponent]];
        [cursorManager pushCursor:cursor];
        [cursorManager updateCursor:lastPoint];
        
        Grid* grid = [[windowController options] grid];
        [grid snapToGrid:lastPoint];
    }
    return self;
}

- (void)translateTo:(Ray3D *)theRay toggleSnap:(BOOL)toggleSnap altPlane:(BOOL)altPlane {
    Vector3f* point = [theRay pointAtDistance:[plane intersectWithRay:theRay]];
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
    
    NSEnumerator* brushEn = [brushes objectEnumerator];
    id <Brush> brush;
    while ((brush = [brushEn nextObject]))
        [map translateBrush:brush 
                     xDelta:x
                     yDelta:y
                     zDelta:z];

    [lastPoint release];
    lastPoint = [point retain];

    CursorManager* cursorManager = [windowController cursorManager];
    [cursorManager updateCursor:lastPoint];
}

- (NSString *)actionName {
    return @"Move Brushes";
}

@end
