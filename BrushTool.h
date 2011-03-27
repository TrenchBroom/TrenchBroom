//
//  BrushTool.h
//  TrenchBroom
//
//  Created by Kristian Duske on 12.03.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "Tool.h"

@class MapWindowController;
@class PickingHit;
@class Plane3D;
@class Ray3D;
@class Vector3f;

@interface BrushTool : NSObject <Tool> {
    @private
    NSMutableSet* brushes;
    MapWindowController* windowController;
    Plane3D* plane;
    Vector3f* lastPoint;
}

- (id)initWithController:(MapWindowController *)theWindowController pickHit:(PickingHit *)theHit pickRay:(Ray3D *)theRay;

@end
