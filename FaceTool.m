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
#import "PickingHitList.h"
#import "Face.h"
#import "Brush.h"
#import "math.h"
#import "Math.h"

@interface FaceTool (private)

- (BOOL)isAttributesModifierPressed:(NSEvent *)event;

@end

@implementation FaceTool (private)

- (BOOL)isAttributesModifierPressed:(NSEvent *)event {
    return ([event modifierFlags] & NSCommandKeyMask) != 0;
}

@end

@implementation FaceTool

- (id)initWithController:(MapWindowController *)theWindowController {
    if (self = [self init]) {
        windowController = [theWindowController retain];
        
    }
    
    return self;
}

- (void)dealloc {
    [lastPoint release];
    [dragDir release];
    [plane release];
    [windowController release];
    [super dealloc];
}

# pragma mark -
# pragma mark @implementation Tool

- (void)handleLeftMouseDown:(NSEvent *)event ray:(Ray3D *)ray hits:(PickingHitList *)hits {
    SelectionManager* selectionManager = [windowController selectionManager];
    NSSet* selectedFaces = [selectionManager selectedFaces];
    if ([selectedFaces count] == 1) {
        id <Face> source = [[selectedFaces objectEnumerator] nextObject];
        
        PickingHit* hit = [hits firstHitOfType:HT_FACE ignoreOccluders:YES];
        id <Face> destination = [hit object];
        
        MapDocument* map = [windowController document];
        if ([self isAttributesModifierPressed:event]) {
            MapDocument* map = [windowController document];
            NSUndoManager* undoManager = [map undoManager];
            [undoManager beginUndoGrouping];
            [map setFace:destination texture:[source texture]];
            [map setFace:destination xOffset:[source xOffset]];
            [map setFace:destination yOffset:[source yOffset]];
            [map setFace:destination xScale:[source xScale]];
            [map setFace:destination yScale:[source yScale]];
            [map setFace:destination rotation:[source rotation]];
            [undoManager endUndoGrouping];
            [undoManager setActionName:@"Copy Face Attributes"];
        } else {
            [map setFace:destination texture:[source texture]];
        }
    }
}

- (void)beginLeftDrag:(NSEvent *)event ray:(Ray3D *)ray hits:(PickingHitList *)hits {
    PickingHit* hit = [hits firstHitOfType:HT_FACE ignoreOccluders:NO];

    id <Face> face = [hit object];
    dragDir = [[face norm] retain];
    
    Vector3f* planeNorm = [[Vector3f alloc] initWithFloatVector:dragDir];
    [planeNorm cross:[ray direction]];
    [planeNorm cross:dragDir];
    [planeNorm normalize];
    
    lastPoint = [[hit hitPoint] retain];
    plane = [[Plane3D alloc] initWithPoint:lastPoint norm:planeNorm];
    [planeNorm release];
    
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
    
    Vector3f* diff = [[Vector3f alloc] initWithFloatVector:point];
    [diff sub:lastPoint];
    float dist = [diff dot:dragDir];
    
    MapDocument* map = [windowController document];
    
    SelectionManager* selectionManager = [windowController selectionManager];
    NSEnumerator* faceEn = [[selectionManager selectedFaces] objectEnumerator];
    id <Face> face;
    while ((face = [faceEn nextObject]))
        [map dragFace:face dist:dist];
    
    [diff release];
    [lastPoint release];
    lastPoint = [point retain];
}

- (void)endLeftDrag:(NSEvent *)event ray:(Ray3D *)ray hits:(PickingHitList *)hits {
    MapDocument* map = [windowController document];
    NSUndoManager* undoManager = [map undoManager];
    [undoManager setActionName:[self actionName]];
    [undoManager endUndoGrouping];
    [undoManager setGroupsByEvent:YES];
    
    [plane release];
    plane = nil;
    [dragDir release];
    dragDir = nil;
    [lastPoint release];
    lastPoint = nil;
}

- (NSString *)actionName {
    return @"Move Faces";
}

@end
