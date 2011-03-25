//
//  TrackingManager.m
//  TrenchBroom
//
//  Created by Kristian Duske on 18.03.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "TrackingManager.h"
#import "MapWindowController.h"
#import "MapDocument.h"
#import "Picker.h"
#import "PickingHit.h"
#import "PickingHitList.h"
#import "SelectionManager.h"
#import "Ray3D.h"
#import "Brush.h"
#import "Face.h"
#import "Edge.h"

NSString* const TrackedObjectChanged = @"TrackedObjectChanged";
NSString* const TrackedObjectKey = @"TrackedObjectKey";
NSString* const UntrackedObjectKey = @"UntrackedObjectKey";

@implementation TrackingManager

- (id)initWithWindowController:(MapWindowController *)theWindowController {
    if (self = [self init]) {
        windowController = [theWindowController retain];
    }
    
    return self;
}

- (void)updateWithRay:(Ray3D *)theRay {
    [currentRay release];
    currentRay = [theRay retain];
    
    SelectionManager* selectionManager = [windowController selectionManager];
    Picker* picker = [[windowController document] picker];
    
    PickingHitList* hits = [picker pickObjects:currentRay include:[selectionManager selectedBrushes] exclude:nil];
    PickingHit* hit = [hits firstHitOfType:HT_VERTEX ignoreOccluders:NO];
    if (hit == nil)
        hit = [hits firstHitOfType:HT_EDGE ignoreOccluders:NO];
    if (hit == nil)
        hit = [hits firstHitOfType:HT_BRUSH ignoreOccluders:NO];
    
    if (hit == nil) {
        if (currentObject != nil) {
            NSMutableDictionary* userInfo = [[NSMutableDictionary alloc] init];
            [userInfo setObject:currentObject forKey:UntrackedObjectKey];
            
            [currentObject release];
            currentObject = nil;
            
            NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
            [center postNotificationName:TrackedObjectChanged object:self userInfo:userInfo];
            [userInfo release];

            NSLog(@"tracked object changed to nil");
        }
    } else {
        if ([hit object] != currentObject) {
            NSMutableDictionary* userInfo = [[NSMutableDictionary alloc] init];
            if (currentObject != nil)
                [userInfo setObject:currentObject forKey:UntrackedObjectKey];
            
            [currentObject release];
            currentObject = [[hit object] retain];
            [userInfo setObject:currentObject forKey:TrackedObjectKey]; 

            NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
            [center postNotificationName:TrackedObjectChanged object:self userInfo:userInfo];
            [userInfo release];
            
            NSLog(@"tracked object changed to %@", currentObject);
        }
    }
}

- (id)trackedObject {
    return currentObject;
}

- (BOOL)isBrushTracked:(id <Brush>)theBrush {
    return currentObject == theBrush;
}

- (BOOL)isFaceTracked:(id <Face>)theFace {
    return currentObject == theFace || [self isBrushTracked:[theFace brush]];
}

- (BOOL)isEdgeTracked:(Edge *)theEdge {
    return currentObject == theEdge || [self isFaceTracked:[theEdge leftFace]] || [self isFaceTracked:[theEdge rightFace]];
}

- (BOOL)isVertexTracked:(Vertex *)theVertex {
    if (currentObject == theVertex)
        return YES;
    
    NSEnumerator* edgeEn = [[theVertex edges] objectEnumerator];
    Edge* edge;
    while ((edge = [edgeEn nextObject]))
        if ([self isEdgeTracked:edge])
            return YES;
    
    return NO;
}

- (void)dealloc {
    [currentObject release];
    [currentRay release];
    [windowController release];
    [super dealloc];
}

@end
