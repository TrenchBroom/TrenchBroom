//
//  BrushTool.h
//  TrenchBroom
//
//  Created by Kristian Duske on 12.03.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class Grid;
@class Camera;
@class MapWindowController;
@class PickingHit;
@class Plane3D;
@class Ray3D;
@class Vector3f;

@interface BrushTool : NSObject {
    @private
    NSMutableSet* brushes;
    MapWindowController* windowController;
    Plane3D* plane;
    Ray3D* lastRay;
    Vector3f* delta;
}

- (id)initWithController:(MapWindowController *)theWindowController pickHit:(PickingHit *)theHit pickRay:(Ray3D *)theRay;
- (void)translateTo:(Ray3D *)theRay toggleSnap:(BOOL)toggleSnap altPlane:(BOOL)altPlane;

@end
