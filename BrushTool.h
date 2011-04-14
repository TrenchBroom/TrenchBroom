//
//  BrushTool.h
//  TrenchBroom
//
//  Created by Kristian Duske on 12.03.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "Tool.h"
#import "GridFeedbackFigure.h"

@class MapWindowController;
@class PickingHitList;
@class Plane3D;
@class Ray3D;
@class Vector3f;
@class BrushToolCursor;

@interface BrushTool : NSObject <Tool> {
    @private
    NSMutableSet* brushes;
    MapWindowController* windowController;
    Plane3D* plane;
    Vector3f* lastPoint;
    BrushToolCursor* cursor;
}

- (id)initWithController:(MapWindowController *)theWindowController pickHits:(PickingHitList *)theHits pickRay:(Ray3D *)theRay;

@end
