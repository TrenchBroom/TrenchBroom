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
#import "PickingHit.h"
#import "Brush.h"
#import "MutableBrush.h"
#import "math.h"
#import "Math.h"

@implementation BrushTool

- (id)init {
    if (self = [super init]) {
        brushes = [[NSMutableSet alloc] init];
        delta = [[Vector3f alloc] init];
    }
    
    return self;
}

- (id)initWithController:(MapWindowController *)theWindowController pickHit:(PickingHit *)theHit pickRay:(Ray3D *)theRay {
    if (self = [self init]) {
        [brushes unionSet:[[theWindowController selectionManager] selectedBrushes]];
        windowController = [theWindowController retain];
        lastRay = [theRay retain];

        Vector3f* hitPoint = [theHit hitPoint];
        switch ([[theRay direction] largestComponent]) {
            case VC_X:
                plane = [[Plane3D alloc] initWithPoint:hitPoint norm:[Vector3f xAxisPos]];
                break;
            case VC_Y:
                plane = [[Plane3D alloc] initWithPoint:hitPoint norm:[Vector3f yAxisPos]];
                break;
            default:
                plane = [[Plane3D alloc] initWithPoint:hitPoint norm:[Vector3f zAxisPos]];
                break;
        }
    }
    
    return self;
}

- (void)translateTo:(Ray3D *)theRay toggleSnap:(BOOL)toggleSnap altPlane:(BOOL)altPlane {
    Vector3f* diff = [plane intersectWithRay:theRay];
    [diff sub:[plane intersectWithRay:lastRay]];
    
    [delta add:diff];
    Grid* grid = [[windowController options] grid];
    int gs = [grid size];
    
    int x = roundf(floorf([delta x] / gs) * gs);
    int y = roundf(floorf([delta y] / gs) * gs);
    int z = roundf(floorf([delta z] / gs) * gs);
    
    [delta setX:[delta x] - x];
    [delta setY:[delta y] - y];
    [delta setZ:[delta z] - z];
    
    MapDocument* map = [windowController document];
    
    NSEnumerator* brushEn = [brushes objectEnumerator];
    MutableBrush* brush;
    while ((brush = [brushEn nextObject]))
        [map translateBrush:brush 
                     xDelta:x
                     yDelta:y
                     zDelta:z];

    [lastRay release];
    lastRay = [theRay retain];
}

- (void)dealloc {
    [lastRay release];
    [delta release];
    [plane release];
    [brushes release];
    [windowController release];
    [super dealloc];
}

@end
