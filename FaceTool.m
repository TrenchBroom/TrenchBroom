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
    }
    
    return self;
}

- (void)dealloc {
    [lastPoint release];
    [dragDir release];
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
        
        id <Face> face = [theHit object];
        dragDir = [[face norm] retain];
        
        Vector3f* planeNorm = [[Vector3f alloc] initWithFloatVector:dragDir];
        [planeNorm cross:[theRay direction]];
        [planeNorm cross:dragDir];
        [planeNorm normalize];
        
        lastPoint = [[theHit hitPoint] retain];
        plane = [[Plane3D alloc] initWithPoint:lastPoint norm:planeNorm];
        [planeNorm release];
        
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
    
    Vector3f* diff = [[Vector3f alloc] initWithFloatVector:point];
    [diff sub:lastPoint];
    float dist = [diff dot:dragDir];
    
    MapDocument* map = [windowController document];
    
    NSEnumerator* faceEn = [faces objectEnumerator];
    id <Face> face;
    while ((face = [faceEn nextObject]))
        [map dragFace:face dist:dist];
    
    [diff release];
    [lastPoint release];
    lastPoint = [point retain];
}

- (NSString *)actionName {
    return @"Move Faces";
}

@end
