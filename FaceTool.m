//
//  FaceTool.m
//  TrenchBroom
//
//  Created by Kristian Duske on 27.03.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "FaceTool.h"
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
#import "Face.h"
#import "Brush.h"
#import "math.h"
#import "Math.h"


@implementation FaceTool
- (id)init {
    if (self = [super init]) {
        faces = [[NSMutableSet alloc] init];
        delta = [[Vector3f alloc] init];
    }
    
    return self;
}

- (void)dealloc {
    [lastRay release];
    [delta release];
    [plane release];
    [faces release];
    [windowController release];
    [super dealloc];
}

# pragma mark -
# pragma mark @implementation Tool

- (id)initWithController:(MapWindowController *)theWindowController pickHit:(PickingHit *)theHit pickRay:(Ray3D *)theRay {
    if (self = [self init]) {
        [faces unionSet:[[theWindowController selectionManager] selectedFaces]];
        windowController = [theWindowController retain];
        lastRay = [theRay retain];
        
        id <Face> face = [theHit object];
        id <Brush> brush = [face brush];
        Vector3f* dragVector = [[Vector3f alloc] initWithFloatVector:[face center]];
        [dragVector sub:[brush center]];

        dragDir = [dragVector largestComponent];
        [dragVector release];
        
        Vector3f* hitPoint = [theHit hitPoint];
        switch (dragDir) {
            case VC_X:
                if ([[theRay direction] largestComponent] == VC_Y)
                    plane = [[Plane3D alloc] initWithPoint:hitPoint norm:[Vector3f yAxisPos]];
                else
                    plane = [[Plane3D alloc] initWithPoint:hitPoint norm:[Vector3f zAxisPos]];
                break;
            case VC_Y:
                if ([[theRay direction] largestComponent] == VC_X)
                    plane = [[Plane3D alloc] initWithPoint:hitPoint norm:[Vector3f xAxisPos]];
                else
                    plane = [[Plane3D alloc] initWithPoint:hitPoint norm:[Vector3f zAxisPos]];
                break;
            default:
                if ([[theRay direction] largestComponent] == VC_X)
                    plane = [[Plane3D alloc] initWithPoint:hitPoint norm:[Vector3f xAxisPos]];
                else
                    plane = [[Plane3D alloc] initWithPoint:hitPoint norm:[Vector3f yAxisPos]];
                break;
        }
    }
    
    return self;
}

- (void)translateTo:(Ray3D *)theRay toggleSnap:(BOOL)toggleSnap altPlane:(BOOL)altPlane {
    Vector3f* diff = [theRay pointAtDistance:[plane intersectWithRay:theRay]];
    if (diff == nil)
        return;
    
    [diff sub:[lastRay pointAtDistance:[plane intersectWithRay:lastRay]]];
    [delta add:diff];
    
    Grid* grid = [[windowController options] grid];
    int gs = [grid size];
    
    int x = roundf(floorf([delta x] / gs) * gs);
    int y = roundf(floorf([delta y] / gs) * gs);
    int z = roundf(floorf([delta z] / gs) * gs);
    
    switch (dragDir) {
        case VC_X:
            y = 0;
            z = 0;
            break;
        case VC_Y:
            x = 0;
            z = 0;
            break;
        case VC_Z:
            x = 0;
            y = 0;
            break;
        default:
            break;
    }

    if (x != 0 || y != 0 || z != 0) {
        [delta setX:[delta x] - x];
        [delta setY:[delta y] - y];
        [delta setZ:[delta z] - z];
        
        MapDocument* map = [windowController document];
        
        NSEnumerator* faceEn = [faces objectEnumerator];
        id <Face> face;
        while ((face = [faceEn nextObject]))
            [map translateFace:face
                         xDelta:x
                         yDelta:y
                         zDelta:z];
    }
    [lastRay release];
    lastRay = [theRay retain];
}

- (NSString *)actionName {
    return @"Move Faces";
}

@end
