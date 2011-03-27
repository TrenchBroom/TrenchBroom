//
//  FaceTool.h
//  TrenchBroom
//
//  Created by Kristian Duske on 27.03.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "Tool.h"
#import "Vector3f.h"

@class MapWindowController;
@class PickingHit;
@class Plane3D;
@class Ray3D;

@interface FaceTool : NSObject <Tool> {
@private
    NSMutableSet* faces;
    MapWindowController* windowController;
    Plane3D* plane;
    Ray3D* lastRay;
    Vector3f* delta;
    EVectorComponent dragDir;
}

- (id)initWithController:(MapWindowController *)theWindowController pickHit:(PickingHit *)theHit pickRay:(Ray3D *)theRay;

@end
